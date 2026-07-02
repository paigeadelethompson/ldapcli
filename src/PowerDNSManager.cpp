#include "PowerDNSManager.hpp"
#include "Config.hpp"
#include <iostream>
#include <sstream>

PowerDNSManager::PowerDNSManager(LDAPConnection &connection)
    : m_connection(connection) {}

void PowerDNSManager::printUsage() const {
  std::cout << "DNS Commands:" << std::endl;
  std::cout << "  create-zone <zone-name> [type] [base-dn]" << std::endl;
  std::cout << "  delete-zone <zone-name> [base-dn]" << std::endl;
  std::cout << "  update-zone <zone-name> [base-dn]" << std::endl;
  std::cout << "  list-zones [base-dn]" << std::endl;
  std::cout << "  add-record <zone> <name> <type> <value> [ttl]" << std::endl;
  std::cout << "  delete-record <zone> <name> <type>" << std::endl;
  std::cout << "  list-records <zone> [base-dn]" << std::endl;
}

std::string PowerDNSManager::getServiceName() const { return "dns"; }

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
    std::string type = "master";

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:", long_options, &option_index)) !=
           -1) {
      switch (opt) {
      case 't':
        type = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-zone <zone-name> [-t type]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-zone <zone-name> [-t type]"
                << std::endl;
      return false;
    }

    zoneName = argv[optind];

    return createZone(zoneName, baseDN, type);
  } else if (command == "delete-zone") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-zone <zone-name>" << std::endl;
      return false;
    }

    std::string zoneName = argv[optind];

    return deleteZone(zoneName, baseDN);
  } else if (command == "update-zone") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-zone <zone-name>" << std::endl;
      return false;
    }

    std::string zoneName = argv[optind];

    return updateZone(zoneName, baseDN);
  } else if (command == "list-zones") {
    return listZones(baseDN);
  } else if (command == "add-record") {
    static struct option long_options[] = {{"ttl", required_argument, 0, 't'},
                                           {0, 0, 0, 0}};

    std::string zone;
    std::string name;
    std::string type;
    std::string value;
    int ttl = 3600;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:", long_options, &option_index)) !=
           -1) {
      switch (opt) {
      case 't':
        ttl = std::atoi(optarg);
        break;
      default:
        std::cerr
            << "Usage: ldapcli add-record <zone> <name> <type> <value> [-t ttl]"
            << std::endl;
        return false;
      }
    }

    if (optind + 3 >= argc) {
      std::cerr
          << "Usage: ldapcli add-record <zone> <name> <type> <value> [-t ttl]"
          << std::endl;
      return false;
    }

    zone = argv[optind];
    name = argv[optind + 1];
    type = argv[optind + 2];
    value = argv[optind + 3];

    return addRecord(zone, baseDN, name, type, value, ttl);
  } else if (command == "delete-record") {
    if (optind + 2 >= argc) {
      std::cerr << "Usage: ldapcli delete-record <zone> <name> <type>"
                << std::endl;
      return false;
    }

    std::string zone = argv[optind];
    std::string name = argv[optind + 1];
    std::string type = argv[optind + 2];

    return deleteRecord(zone, baseDN, name, type);
  } else if (command == "list-records") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli list-records <zone>" << std::endl;
      return false;
    }

    std::string zone = argv[optind];

    return listRecords(zone, baseDN);
  } else {
    std::cerr << "Unknown DNS command: " << command << std::endl;
    printUsage();
    return false;
  }
}

bool PowerDNSManager::listZones(const std::string &baseDN) {
  std::cout << "Listing PowerDNS zones:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=*)";

  if (m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cout << "Found " << results.size() << " zones:" << std::endl;

    for (size_t i = 0; i < results.size(); i++) {
      std::cout << "\nZone " << (i + 1) << ":" << std::endl;
      for (const auto &[attr, value] : results[i]) {
        std::cout << "  " << attr << ": " << value << std::endl;
      }
    }

    return true;
  }

  std::cerr << "Error: " << m_connection.getError() << std::endl;
  return false;
}

bool PowerDNSManager::createZone(const std::string &zoneName,
                                 const std::string &baseDN,
                                 const std::string &type) {
  if (!validateZoneName(zoneName)) {
    std::cerr << "Error: Invalid zone name format" << std::endl;
    return false;
  }

  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Creating PowerDNS zone:" << std::endl;
  std::cout << "  Zone Name: " << zoneName << std::endl;
  std::cout << "  Zone DN: " << zoneDN << std::endl;
  std::cout << "  Type: " << type << std::endl;

  // Create LDAP mods for PdnsDomain object class
  std::vector<LDAPMod> mods;

  // PdnsDomainId is required
  LDAPMod domainIdMod;
  domainIdMod.mod_op = LDAP_MOD_ADD;
  domainIdMod.mod_type = const_cast<char *>("PdnsDomainId");
  domainIdMod.mod_vals.modv_bvals = new struct berval *[2];
  domainIdMod.mod_vals.modv_bvals[0] = new struct berval;
  domainIdMod.mod_vals.modv_bvals[0]->bv_len = 1;
  domainIdMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("0");
  domainIdMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(domainIdMod);

  // PdnsDomainType (optional)
  if (!type.empty()) {
    LDAPMod domainTypeMod;
    domainTypeMod.mod_op = LDAP_MOD_ADD;
    domainTypeMod.mod_type = const_cast<char *>("PdnsDomainType");
    domainTypeMod.mod_vals.modv_bvals = new struct berval *[2];
    domainTypeMod.mod_vals.modv_bvals[0] = new struct berval;
    domainTypeMod.mod_vals.modv_bvals[0]->bv_len = type.length();
    domainTypeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(type.c_str());
    domainTypeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(domainTypeMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(zoneDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Zone created successfully!" << std::endl;
  return true;
}

bool PowerDNSManager::updateZone(const std::string &zoneName,
                                 const std::string &baseDN) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Updating PowerDNS zone:" << std::endl;
  std::cout << "  Zone Name: " << zoneName << std::endl;
  std::cout << "  Zone DN: " << zoneDN << std::endl;

  // TODO: Implement zone update logic
  // This would require parsing command line arguments for what to update

  std::cout << "Zone update not yet implemented" << std::endl;
  return true;
}

bool PowerDNSManager::deleteZone(const std::string &zoneName,
                                 const std::string &baseDN) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Deleting PowerDNS zone:" << std::endl;
  std::cout << "  Zone Name: " << zoneName << std::endl;
  std::cout << "  Zone DN: " << zoneDN << std::endl;

  if (!m_connection.deleteEntry(zoneDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Zone deleted successfully!" << std::endl;
  return true;
}

bool PowerDNSManager::addRecord(const std::string &zoneName,
                                const std::string &baseDN,
                                const std::string &recordName,
                                const std::string &recordType,
                                const std::string &recordValue, int ttl) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Adding PowerDNS record:" << std::endl;
  std::cout << "  Zone: " << zoneName << std::endl;
  std::cout << "  Record Name: " << recordName << std::endl;
  std::cout << "  Record Type: " << recordType << std::endl;
  std::cout << "  Record Value: " << recordValue << std::endl;
  std::cout << "  TTL: " << ttl << std::endl;

  // Create record DN
  std::string recordDN = "cn=" + recordName + "," + zoneDN;

  // Create LDAP mods for DNS resource record
  std::vector<LDAPMod> mods;

  // Required attributes for DNS resource records
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = recordName.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(recordName.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod recordNameMod;
  recordNameMod.mod_op = LDAP_MOD_ADD;
  recordNameMod.mod_type = const_cast<char *>("PdnsRecordName");
  recordNameMod.mod_vals.modv_bvals = new struct berval *[2];
  recordNameMod.mod_vals.modv_bvals[0] = new struct berval;
  recordNameMod.mod_vals.modv_bvals[0]->bv_len = recordName.length();
  recordNameMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>(recordName.c_str());
  recordNameMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(recordNameMod);

  LDAPMod recordTypeMod;
  recordTypeMod.mod_op = LDAP_MOD_ADD;
  recordTypeMod.mod_type = const_cast<char *>("PdnsRecordType");
  recordTypeMod.mod_vals.modv_bvals = new struct berval *[2];
  recordTypeMod.mod_vals.modv_bvals[0] = new struct berval;
  recordTypeMod.mod_vals.modv_bvals[0]->bv_len = recordType.length();
  recordTypeMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>(recordType.c_str());
  recordTypeMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(recordTypeMod);

  LDAPMod recordValueMod;
  recordValueMod.mod_op = LDAP_MOD_ADD;
  recordValueMod.mod_type = const_cast<char *>("PdnsRecordValue");
  recordValueMod.mod_vals.modv_bvals = new struct berval *[2];
  recordValueMod.mod_vals.modv_bvals[0] = new struct berval;
  recordValueMod.mod_vals.modv_bvals[0]->bv_len = recordValue.length();
  recordValueMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>(recordValue.c_str());
  recordValueMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(recordValueMod);

  // Optional TTL
  if (ttl > 0) {
    std::string ttlStr = std::to_string(ttl);
    LDAPMod ttlMod;
    ttlMod.mod_op = LDAP_MOD_ADD;
    ttlMod.mod_type = const_cast<char *>("PdnsRecordTTL");
    ttlMod.mod_vals.modv_bvals = new struct berval *[2];
    ttlMod.mod_vals.modv_bvals[0] = new struct berval;
    ttlMod.mod_vals.modv_bvals[0]->bv_len = ttlStr.length();
    ttlMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(ttlStr.c_str());
    ttlMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(ttlMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(recordDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Record added successfully!" << std::endl;
  return true;
}

bool PowerDNSManager::deleteRecord(const std::string &zoneName,
                                   const std::string &baseDN,
                                   const std::string &recordName,
                                   const std::string &recordType) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Deleting PowerDNS record:" << std::endl;
  std::cout << "  Zone: " << zoneName << std::endl;
  std::cout << "  Record Name: " << recordName << std::endl;
  std::cout << "  Record Type: " << recordType << std::endl;

  // Create record DN
  std::string recordDN = "cn=" + recordName + "," + zoneDN;

  if (!m_connection.deleteEntry(recordDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Record deleted successfully!" << std::endl;
  return true;
}

bool PowerDNSManager::listRecords(const std::string &zoneName,
                                  const std::string &baseDN) {
  std::string zoneDN = getZoneDN(zoneName, baseDN);

  std::cout << "Listing PowerDNS records for zone: " << zoneName << std::endl;
  std::cout << "Zone DN: " << zoneDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=*)";

  if (m_connection.search(zoneDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cout << "Found " << results.size() << " records:" << std::endl;

    for (size_t i = 0; i < results.size(); i++) {
      std::cout << "\nRecord " << (i + 1) << ":" << std::endl;
      for (const auto &[attr, value] : results[i]) {
        std::cout << "  " << attr << ": " << value << std::endl;
      }
    }

    return true;
  }

  std::cerr << "Error: " << m_connection.getError() << std::endl;
  return false;
}

std::string PowerDNSManager::getZoneDN(const std::string &zoneName,
                                       const std::string &baseDN) const {
  return "cn=" + zoneName + "," + baseDN;
}

bool PowerDNSManager::validateZoneName(const std::string &zoneName) const {
  // Basic validation for domain names
  return !zoneName.empty() && zoneName.find('.') != std::string::npos;
}