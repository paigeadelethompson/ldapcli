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
  console::e("  create-zone <zone> [--type|--t TYPE]");
  console::e("  delete-zone <zone> [--zone ZONE]");
  console::e(
      "  update-zone <zone> [--notified-serial|--n N] [--last-check|--l N] "
      "[--master|--m HOST]");
  console::e("  list-zones");
  console::e(
      "  add-record [--zone|--z ZONE] [--name|--n NAME] [--ttl|--t SEC] "
      "(--type|--y TYPE --value|--v VALUE | --a ADDR | --aaaa ADDR | ...)");
  console::e(
      "  update-record [--zone|--z ZONE] [--name|--n NAME] "
      "[--type|--y TYPE] [--value|--v VALUE | --a ADDR | ...] [--ttl|--t SEC]");
  console::e(
      "  delete-record [--zone|--z ZONE] [--name|--n NAME] [--type|--y TYPE]");
  console::e("  list-records [--zone|--z ZONE]");
  printRecordTypeOptions();
}

void PowerDNSManager::printRecordTypeOptions() const {
  console::e("Record type long options (each sets type and value):");
#define POWERDNS_RECORD_OPTION(name, type)                                     \
  console::e("  --{} <value>  ({} record)", name, type);
#include "PowerDNSRecordOptions.inc"
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

namespace {

const std::unordered_map<std::string, std::string> &recordTypeLongOptions() {
  static const std::unordered_map<std::string, std::string> opts = {
#define POWERDNS_RECORD_OPTION(name, type) {name, type},
#include "PowerDNSRecordOptions.inc"
#undef POWERDNS_RECORD_OPTION
  };
  return opts;
}

struct option *recordLongOptions() {
  static struct option options[] = {{"zone", required_argument, 0, 'z'},
                                    {"name", required_argument, 0, 'n'},
                                    {"ttl", required_argument, 0, 't'},
                                    {"value", required_argument, 0, 'v'},
                                    {"type", required_argument, 0, 'y'},
#define POWERDNS_RECORD_OPTION(name, type) {name, required_argument, 0, 0},
#include "PowerDNSRecordOptions.inc"
#undef POWERDNS_RECORD_OPTION
                                    {nullptr, 0, 0, 0}};
  return options;
}

struct RecordArgs {
  std::string zone;
  std::string name;
  std::string type;
  std::optional<std::string> value;
  std::optional<int> ttl;
};

bool setRecordType(RecordArgs &args, const std::string &type,
                   const std::string &val) {
  if (!args.type.empty() && args.type != type) {
    console::e("Error: multiple record types specified");
    return false;
  }
  args.type = type;
  args.value = val;
  return true;
}

bool handleRecordOpt(int opt, int option_index, struct option *long_options,
                     RecordArgs &args) {
  switch (opt) {
  case 'z':
    args.zone = optarg;
    return true;
  case 'n':
    args.name = optarg;
    return true;
  case 't':
    args.ttl = std::atoi(optarg);
    return true;
  case 'v':
    args.value = optarg;
    return true;
  case 'y':
    args.type = optarg;
    return true;
  case 0: {
    const char *optName = long_options[option_index].name;
    auto it = recordTypeLongOptions().find(optName);
    if (it == recordTypeLongOptions().end()) {
      return false;
    }
    return setRecordType(args, it->second, optarg);
  }
  default:
    return false;
  }
}

bool parseRecordArgs(int argc, char *argv[], RecordArgs &args,
                     bool requireValue) {
  optind = 3;
  int opt;
  int option_index = 0;
  struct option *long_options = recordLongOptions();

  while ((opt = getopt_long(argc, argv, "z:n:t:v:y:", long_options,
                            &option_index)) != -1) {
    if (!handleRecordOpt(opt, option_index, long_options, args)) {
      return false;
    }
  }

  if (args.zone.empty() && optind < argc) {
    args.zone = argv[optind++];
  }
  if (args.name.empty() && optind < argc) {
    args.name = argv[optind++];
  }
  if (args.type.empty() && optind < argc) {
    args.type = argv[optind++];
  }
  if (!args.value.has_value() && optind < argc) {
    args.value = argv[optind++];
  }

  if (args.zone.empty() || args.name.empty() || args.type.empty()) {
    console::e("Error: zone, name, and record type are required");
    return false;
  }
  if (requireValue && !args.value.has_value()) {
    console::e("Error: record value is required");
    return false;
  }
  return true;
}

std::string parseZoneArg(int argc, char *argv[], const char *usage) {
  optind = 3;
  static struct option long_options[] = {{"zone", required_argument, 0, 'z'},
                                         {nullptr, 0, 0, 0}};
  std::string zone;
  int opt;
  int option_index = 0;
  while ((opt = getopt_long(argc, argv, "z:", long_options, &option_index)) !=
         -1) {
    if (opt == 'z') {
      zone = optarg;
    } else {
      console::e("{}", usage);
      return "";
    }
  }
  if (zone.empty() && optind < argc) {
    zone = argv[optind];
  }
  if (zone.empty()) {
    console::e("{}", usage);
  }
  return zone;
}

} // namespace

bool PowerDNSManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-zone") {
    static struct option long_options[] = {{"zone", required_argument, 0, 'z'},
                                           {"type", required_argument, 0, 't'},
                                           {0, 0, 0, 0}};

    std::string zoneName;
    std::optional<std::string> type = std::string("master");

    optind = 3;
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "z:t:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'z':
        zoneName = optarg;
        break;
      case 't':
        type = optarg;
        break;
      default:
        console::e("Usage: ldapcli dns create-zone <zone> [--zone ZONE] "
                   "[--type|--t TYPE]");
        return false;
      }
    }

    if (zoneName.empty() && optind < argc) {
      zoneName = argv[optind];
    }
    if (zoneName.empty()) {
      console::e("Usage: ldapcli dns create-zone <zone> [--zone ZONE] "
                 "[--type|--t TYPE]");
      return false;
    }

    return createZone(zoneName, baseDN, type);
  } else if (command == "delete-zone") {
    std::string zoneName = parseZoneArg(
        argc, argv, "Usage: ldapcli dns delete-zone <zone> [--zone|--z ZONE]");
    if (zoneName.empty()) {
      return false;
    }
    return deleteZone(zoneName, baseDN);
  } else if (command == "update-zone") {
    static struct option long_options[] = {
        {"zone", required_argument, 0, 'z'},
        {"notified-serial", required_argument, 0, 'n'},
        {"last-check", required_argument, 0, 'l'},
        {"master", required_argument, 0, 'm'},
        {0, 0, 0, 0}};

    std::string zoneName;
    std::optional<std::string> notifiedSerial;
    std::optional<std::string> lastCheck;
    std::optional<std::string> master;

    optind = 3;
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "z:n:l:m:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'z':
        zoneName = optarg;
        break;
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
        console::e("Usage: ldapcli dns update-zone <zone> [--zone ZONE] "
                   "[--notified-serial|--n N] [--last-check|--l N] "
                   "[--master|--m HOST]");
        return false;
      }
    }

    if (zoneName.empty() && optind < argc) {
      zoneName = argv[optind];
    }
    if (zoneName.empty()) {
      console::e("Usage: ldapcli dns update-zone <zone> [--zone ZONE] "
                 "[--notified-serial|--n N] [--last-check|--l N] "
                 "[--master|--m HOST]");
      return false;
    }

    return updateZone(zoneName, baseDN, notifiedSerial, lastCheck, master);
  } else if (command == "list-zones") {
    return listZones(baseDN);
  } else if (command == "add-record") {
    RecordArgs args;
    args.ttl = 3600;
    if (!parseRecordArgs(argc, argv, args, true)) {
      console::e("Usage: ldapcli dns add-record [--zone|--z ZONE] "
                 "[--name|--n NAME] [--ttl|--t SEC] "
                 "(--type|--y TYPE --value|--v VALUE | --a ADDR | ...)");
      return false;
    }
    return addRecord(args.zone, baseDN, args.name, args.type, *args.value,
                     args.ttl);
  } else if (command == "update-record") {
    RecordArgs args;
    if (!parseRecordArgs(argc, argv, args, false)) {
      console::e("Usage: ldapcli dns update-record [--zone|--z ZONE] "
                 "[--name|--n NAME] [--type|--y TYPE] "
                 "[--value|--v VALUE | --a ADDR | ...] [--ttl|--t SEC]");
      return false;
    }
    if (!args.value.has_value() && !args.ttl.has_value()) {
      console::e("Error: specify --value or a record type option and/or --ttl");
      return false;
    }
    return updateRecord(args.zone, baseDN, args.name, args.type, args.value,
                        args.ttl);
  } else if (command == "delete-record") {
    optind = 3;
    static struct option long_options[] = {{"zone", required_argument, 0, 'z'},
                                           {"name", required_argument, 0, 'n'},
                                           {"type", required_argument, 0, 'y'},
                                           {0, 0, 0, 0}};
    std::string zone;
    std::string name;
    std::string type;
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "z:n:y:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'z':
        zone = optarg;
        break;
      case 'n':
        name = optarg;
        break;
      case 'y':
        type = optarg;
        break;
      default:
        console::e("Usage: ldapcli dns delete-record [--zone|--z ZONE] "
                   "[--name|--n NAME] [--type|--y TYPE]");
        return false;
      }
    }
    if (zone.empty() && optind < argc) {
      zone = argv[optind++];
    }
    if (name.empty() && optind < argc) {
      name = argv[optind++];
    }
    if (type.empty() && optind < argc) {
      type = argv[optind++];
    }
    if (zone.empty() || name.empty() || type.empty()) {
      console::e("Usage: ldapcli dns delete-record [--zone|--z ZONE] "
                 "[--name|--n NAME] [--type|--y TYPE]");
      return false;
    }
    return deleteRecord(zone, baseDN, name, type);
  } else if (command == "list-records") {
    std::string zone = parseZoneArg(
        argc, argv, "Usage: ldapcli dns list-records <zone> [--zone|--z ZONE]");
    if (zone.empty()) {
      return false;
    }
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
