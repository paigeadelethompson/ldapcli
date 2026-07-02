#include "PowerDNSManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <algorithm>
#include <cctype>
#include <getopt.h>
#include <sstream>
#include <unordered_map>

PowerDNSManager::PowerDNSManager(LDAPConnection &connection)
    : m_connection(connection) {}

void PowerDNSManager::printUsage() const {
  console::e("DNS Commands:");
  console::e("  create-zone <zone-name> [type] [base-dn]");
  console::e("  delete-zone <zone-name> [base-dn]");
  console::e("  update-zone <zone-name> [base-dn]");
  console::e("  list-zones [base-dn]");
  console::e("  add-record <zone> <name> <type> <value> [ttl]");
  console::e("  update-record <zone> <name> <type> <value> [-t ttl]");
  console::e("  delete-record <zone> <name> <type>");
  console::e("  list-records <zone> [base-dn]");
}

std::string PowerDNSManager::getServiceName() const { return "dns"; }

std::string
PowerDNSManager::recordTypeToAttribute(const std::string &recordType) const {
  static const std::unordered_map<std::string, std::string> knownTypes = {
      {"A", "aRecord"},         {"AAAA", "aAAARecord"},
      {"MX", "mXRecord"},       {"NS", "nSRecord"},
      {"CNAME", "cNAMERecord"}, {"SOA", "sOARecord"},
      {"TXT", "tXTRecord"},     {"PTR", "pTRRecord"},
      {"SRV", "sRVRecord"},     {"CAA", "cAARecord"},
      {"HINFO", "hInfoRecord"}, {"MINFO", "mInfoRecord"},
      {"RP", "rPRecord"},       {"AFSDB", "aFSDBRecord"},
      {"SIG", "SigRecord"},     {"KEY", "KeyRecord"},
      {"GPOS", "gPosRecord"},   {"LOC", "LocRecord"},
      {"NXT", "nXTRecord"},     {"NAPTR", "nAPTRRecord"},
      {"KX", "kXRecord"},       {"CERT", "certRecord"},
      {"DS", "dSRecord"},       {"SSHFP", "sSHFPRecord"},
      {"TLSA", "tLSARecord"},   {"SPF", "sPFRecord"},
      {"URI", "uRIRecord"},     {"ALIAS", "ALIASRecord"},
  };

  std::string upper = recordType;
  std::transform(upper.begin(), upper.end(), upper.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  auto it = knownTypes.find(upper);
  if (it != knownTypes.end()) {
    return it->second;
  }

  if (upper.empty()) {
    return "Record";
  }

  upper[0] = static_cast<char>(std::tolower(upper[0]));
  return upper + "Record";
}

bool PowerDNSManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-zone") {
    static struct option long_options[] = {{"type", required_argument, 0, 't'},
                                           {0, 0, 0, 0}};

    std::string zoneName;
    std::optional<std::string> type = std::string("master");

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:", long_options, &option_index)) !=
           -1) {
      switch (opt) {
      case 't':
        type = optarg;
        break;
      default:
        console::e("Usage: ldapcli create-zone <zone-name> [-t type]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-zone <zone-name> [-t type]");
      return false;
    }

    zoneName = argv[optind];

    return createZone(zoneName, baseDN, type);
  } else if (command == "delete-zone") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-zone <zone-name>");
      return false;
    }

    std::string zoneName = argv[optind];

    return deleteZone(zoneName, baseDN);
  } else if (command == "update-zone") {
    static struct option long_options[] = {
        {"notified-serial", required_argument, 0, 'n'},
        {"last-check", required_argument, 0, 'l'},
        {"master", required_argument, 0, 'm'},
        {0, 0, 0, 0}};

    std::string zoneName;
    std::optional<std::string> notifiedSerial;
    std::optional<std::string> lastCheck;
    std::optional<std::string> master;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "n:l:m:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'n':
        notifiedSerial = optarg;
        break;
      case 'l':
        lastCheck = optarg;
        break;
      case 'm':
        master = optarg;
        break;
      default:
        console::e("Usage: ldapcli update-zone <zone-name> [-n serial] [-l "
                   "last-check] [-m master]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli update-zone <zone-name> [-n serial] [-l "
                 "last-check] [-m master]");
      return false;
    }

    zoneName = argv[optind];

    return updateZone(zoneName, baseDN, notifiedSerial, lastCheck, master);
  } else if (command == "list-zones") {
    return listZones(baseDN);
  } else if (command == "add-record") {
    static struct option long_options[] = {{"ttl", required_argument, 0, 't'},
                                           {0, 0, 0, 0}};

    std::string zone;
    std::string name;
    std::string type;
    std::string value;
    std::optional<int> ttl = 3600;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:", long_options, &option_index)) !=
           -1) {
      switch (opt) {
      case 't':
        ttl = std::atoi(optarg);
        break;
      default:
        console::e(
            "Usage: ldapcli add-record <zone> <name> <type> <value> [-t ttl]");
        return false;
      }
    }

    if (optind + 3 >= argc) {
      console::e(
          "Usage: ldapcli add-record <zone> <name> <type> <value> [-t ttl]");
      return false;
    }

    zone = argv[optind];
    name = argv[optind + 1];
    type = argv[optind + 2];
    value = argv[optind + 3];

    return addRecord(zone, baseDN, name, type, value, ttl);
  } else if (command == "update-record") {
    static struct option long_options[] = {{"ttl", required_argument, 0, 't'},
                                           {0, 0, 0, 0}};

    std::string zone;
    std::string name;
    std::string type;
    std::string value;
    std::optional<int> ttl;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:", long_options, &option_index)) !=
           -1) {
      switch (opt) {
      case 't':
        ttl = std::atoi(optarg);
        break;
      default:
        console::e("Usage: ldapcli update-record <zone> <name> <type> <value> "
                   "[-t ttl]");
        return false;
      }
    }

    if (optind + 3 >= argc) {
      console::e("Usage: ldapcli update-record <zone> <name> <type> <value> "
                 "[-t ttl]");
      return false;
    }

    zone = argv[optind];
    name = argv[optind + 1];
    type = argv[optind + 2];
    value = argv[optind + 3];

    return updateRecord(zone, baseDN, name, type,
                        std::optional<std::string>(value), ttl);
  } else if (command == "delete-record") {
    if (optind + 2 >= argc) {
      console::e("Usage: ldapcli delete-record <zone> <name> <type>");
      return false;
    }

    std::string zone = argv[optind];
    std::string name = argv[optind + 1];
    std::string type = argv[optind + 2];

    return deleteRecord(zone, baseDN, name, type);
  } else if (command == "list-records") {
    if (optind >= argc) {
      console::e("Usage: ldapcli list-records <zone>");
      return false;
    }

    std::string zone = argv[optind];

    return listRecords(zone, baseDN);
  } else {
    console::e("Unknown DNS command: {}", command);
    printUsage();
    return false;
  }
}

bool PowerDNSManager::listZones(const std::string &baseDN) {
  console::e("Listing PowerDNS zones:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=PdnsDomain)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No zones found.");
    return true;
  }

  console::e("Found {} zones:", results.size());

  std::vector<std::string> flatData;
  flatData.reserve(results.size() * 2);
  for (const auto &entry : results) {
    for (const auto &[attr, value] : entry) {
      flatData.push_back(attr);
      flatData.push_back(value);
    }
  }

  std::mdspan<std::string, std::dextents<size_t, 2>> tableData(
      flatData.data(), results.size() + 1, 2);

  console::printTable(tableData);
  return true;
}

bool PowerDNSManager::createZone(const std::string &zoneName,
                                 const std::string &baseDN,
                                 const std::optional<std::string> &type) {
  if (!validateZoneName(zoneName)) {
    console::e("Error: Invalid zone name format");
    return false;
  }

  std::string zoneDN = getZoneDN(zoneName, baseDN);

  console::e("Creating PowerDNS zone:");
  console::e("  Zone Name: {}", zoneName);
  console::e("  Zone DN: {}", zoneDN);
  console::e("  Type: {}", type.value_or("master"));

  std::vector<LDAPMod> mods;

  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(zoneName.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[3];
  objectClassMod.mod_vals.modv_strvals[0] =
      const_cast<char *>("organizationalRole");
  objectClassMod.mod_vals.modv_strvals[1] = const_cast<char *>("PdnsDomain");
  objectClassMod.mod_vals.modv_strvals[2] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod domainIdMod;
  domainIdMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  domainIdMod.mod_type = const_cast<char *>("PdnsDomainId");
  domainIdMod.mod_vals.modv_strvals = new char *[2];
  domainIdMod.mod_vals.modv_strvals[0] = const_cast<char *>("0");
  domainIdMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(domainIdMod);

  if (type.has_value()) {
    LDAPMod domainTypeMod;
    domainTypeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    domainTypeMod.mod_type = const_cast<char *>("PdnsDomainType");
    domainTypeMod.mod_vals.modv_strvals = new char *[2];
    domainTypeMod.mod_vals.modv_strvals[0] = const_cast<char *>(type->c_str());
    domainTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(domainTypeMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(zoneDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Zone created successfully!");
  return true;
}

bool PowerDNSManager::updateZone(
    const std::string &zoneName, const std::string &baseDN,
    const std::optional<std::string> &notifiedSerial,
    const std::optional<std::string> &lastCheck,
    const std::optional<std::string> &master) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  console::e("Updating PowerDNS zone:");
  console::e("  Zone Name: {}", zoneName);
  console::e("  Zone DN: {}", zoneDN);

  std::vector<LDAPMod> mods;

  auto addReplace = [&](const char *attr,
                        const std::optional<std::string> &val) {
    if (!val.has_value()) {
      return;
    }
    LDAPMod mod;
    mod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    mod.mod_type = const_cast<char *>(attr);
    mod.mod_vals.modv_strvals = new char *[2];
    mod.mod_vals.modv_strvals[0] = const_cast<char *>(val->c_str());
    mod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mod);
  };

  addReplace("PdnsDomainNotifiedSerial", notifiedSerial);
  addReplace("PdnsDomainLastCheck", lastCheck);
  addReplace("PdnsDomainMaster", master);

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(zoneDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Zone updated successfully!");
  return true;
}

bool PowerDNSManager::deleteZone(const std::string &zoneName,
                                 const std::string &baseDN) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  console::e("Deleting PowerDNS zone:");
  console::e("  Zone Name: {}", zoneName);
  console::e("  Zone DN: {}", zoneDN);

  if (!m_connection.deleteEntry(zoneDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Zone deleted successfully!");
  return true;
}

bool PowerDNSManager::addRecord(const std::string &zoneName,
                                const std::string &baseDN,
                                const std::string &recordName,
                                const std::string &recordType,
                                const std::string &recordValue,
                                const std::optional<int> &ttl) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);
  std::string recordAttr = recordTypeToAttribute(recordType);

  console::e("Adding PowerDNS record:");
  console::e("  Zone: {}", zoneName);
  console::e("  Record Name: {}", recordName);
  console::e("  Record Type: {}", recordType);
  console::e("  Record Attribute: {}", recordAttr);
  console::e("  Record Value: {}", recordValue);
  if (ttl.has_value()) {
    console::e("  TTL: {}", ttl.value());
  }

  std::string recordDN = "cn=" + recordName + "," + zoneDN;

  std::vector<LDAPMod> mods;

  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(recordName.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[3];
  objectClassMod.mod_vals.modv_strvals[0] = const_cast<char *>("dNSDomain2");
  objectClassMod.mod_vals.modv_strvals[1] =
      const_cast<char *>("PdnsRecordData");
  objectClassMod.mod_vals.modv_strvals[2] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod recordValueMod;
  recordValueMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  recordValueMod.mod_type = const_cast<char *>(recordAttr.c_str());
  recordValueMod.mod_vals.modv_strvals = new char *[2];
  recordValueMod.mod_vals.modv_strvals[0] =
      const_cast<char *>(recordValue.c_str());
  recordValueMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(recordValueMod);

  if (ttl.has_value() && ttl.value() > 0) {
    std::string ttlStr = std::to_string(ttl.value());
    LDAPMod ttlMod;
    ttlMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    ttlMod.mod_type = const_cast<char *>("dNSTTL");
    ttlMod.mod_vals.modv_strvals = new char *[2];
    ttlMod.mod_vals.modv_strvals[0] = const_cast<char *>(ttlStr.c_str());
    ttlMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(ttlMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(recordDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Record added successfully!");
  return true;
}

bool PowerDNSManager::updateRecord(
    const std::string &zoneName, const std::string &baseDN,
    const std::string &recordName, const std::string &recordType,
    const std::optional<std::string> &recordValue,
    const std::optional<int> &ttl) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);
  std::string recordDN = "cn=" + recordName + "," + zoneDN;
  std::string recordAttr = recordTypeToAttribute(recordType);

  console::e("Updating PowerDNS record:");
  console::e("  Zone: {}", zoneName);
  console::e("  Record Name: {}", recordName);
  console::e("  Record Type: {}", recordType);
  console::e("  Record Attribute: {}", recordAttr);

  std::vector<LDAPMod> mods;

  if (recordValue.has_value()) {
    LDAPMod recordValueMod;
    recordValueMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    recordValueMod.mod_type = const_cast<char *>(recordAttr.c_str());
    recordValueMod.mod_vals.modv_strvals = new char *[2];
    recordValueMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(recordValue->c_str());
    recordValueMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(recordValueMod);
  }

  if (ttl.has_value()) {
    std::string ttlStr = std::to_string(ttl.value());
    LDAPMod ttlMod;
    ttlMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    ttlMod.mod_type = const_cast<char *>("dNSTTL");
    ttlMod.mod_vals.modv_strvals = new char *[2];
    ttlMod.mod_vals.modv_strvals[0] = const_cast<char *>(ttlStr.c_str());
    ttlMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(ttlMod);
  }

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(recordDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Record updated successfully!");
  return true;
}

bool PowerDNSManager::deleteRecord(const std::string &zoneName,
                                   const std::string &baseDN,
                                   const std::string &recordName,
                                   const std::string &recordType) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  console::e("Deleting PowerDNS record:");
  console::e("  Zone: {}", zoneName);
  console::e("  Record Name: {}", recordName);
  console::e("  Record Type: {}", recordType);

  std::string recordDN = "cn=" + recordName + "," + zoneDN;

  if (!m_connection.deleteEntry(recordDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Record deleted successfully!");
  return true;
}

bool PowerDNSManager::listRecords(const std::string &zoneName,
                                  const std::string &baseDN) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  console::e("Listing PowerDNS records for zone: {}", zoneName);
  console::e("Zone DN: {}", zoneDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=dNSDomain2)";

  if (!m_connection.search(zoneDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No records found.");
    return true;
  }

  std::vector<std::string> flatData;
  flatData.reserve(results.size() * 2);
  for (const auto &entry : results) {
    for (const auto &[attr, value] : entry) {
      flatData.push_back(attr);
      flatData.push_back(value);
    }
  }

  std::mdspan<std::string, std::dextents<size_t, 2>> ldifData(
      flatData.data(), results.size(), 2);

  console::printLdif(ldifData);
  return true;
}

std::string PowerDNSManager::getZoneDN(const std::string &zoneName,
                                       const std::string &baseDN) const {
  return "cn=" + zoneName + "," + baseDN;
}

bool PowerDNSManager::validateZoneName(const std::string &zoneName) const {
  return !zoneName.empty() && zoneName.find('.') != std::string::npos;
}
