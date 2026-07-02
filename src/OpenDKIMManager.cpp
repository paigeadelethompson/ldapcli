#include "OpenDKIMManager.hpp"
#include "Config.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>

OpenDKIMManager::OpenDKIMManager(LDAPConnection &connection)
    : m_connection(connection) {}

void OpenDKIMManager::printUsage() const {
  std::cout << "OpenDKIM Commands:" << std::endl;
  std::cout << "  create-identity <identity> [base-dn]" << std::endl;
  std::cout << "  delete-identity <identity> [base-dn]" << std::endl;
  std::cout << "  update-identity <identity> [base-dn]" << std::endl;
  std::cout << "  list-identities [base-dn]" << std::endl;
}

std::string OpenDKIMManager::getServiceName() const { return "opendkim"; }

bool OpenDKIMManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-identity") {
    static struct option long_options[] = {
        {"selector", required_argument, 0, 's'},
        {"key", required_argument, 0, 'k'},
        {"domain", required_argument, 0, 'd'},
        {0, 0, 0, 0}};

    std::string identity;
    std::string selector;
    std::string key;
    std::string domain;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "s:k:d:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 's':
        selector = optarg;
        break;
      case 'k':
        key = optarg;
        break;
      case 'd':
        domain = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-identity <identity> [-s selector] "
                     "[-k key] [-d domain]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-identity <identity> [-s selector] "
                   "[-k key] [-d domain]"
                << std::endl;
      return false;
    }

    identity = argv[optind];

    return createIdentity(identity, baseDN);
  } else if (command == "delete-identity") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-identity <identity>" << std::endl;
      return false;
    }

    std::string identity = argv[optind];

    return deleteIdentity(identity, baseDN);
  } else if (command == "update-identity") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-identity <identity>" << std::endl;
      return false;
    }

    std::string identity = argv[optind];

    return updateIdentity(identity, baseDN, argc, argv);
  } else if (command == "list-identities") {
    return listIdentities(baseDN);
  } else {
    std::cerr << "Unknown OpenDKIM command: " << command << std::endl;
    printUsage();
    return false;
  }
}

bool OpenDKIMManager::listIdentities(const std::string &baseDN) {
  std::cout << "Listing OpenDKIM identities:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=*)";

  if (m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cout << "Found " << results.size() << " identities:" << std::endl;

    for (size_t i = 0; i < results.size(); i++) {
      std::cout << "\nIdentity " << (i + 1) << ":" << std::endl;
      for (const auto &[attr, value] : results[i]) {
        std::cout << "  " << attr << ": " << value << std::endl;
      }
    }

    return true;
  }

  std::cerr << "Error: " << m_connection.getError() << std::endl;
  return false;
}

bool OpenDKIMManager::createIdentity(const std::string &identity,
                                     const std::string &baseDN,
                                     const std::string &selector,
                                     const std::string &key,
                                     const std::string &domain) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  std::cout << "Creating OpenDKIM identity:" << std::endl;
  std::cout << "  Identity: " << identity << std::endl;
  std::cout << "  Identity DN: " << identityDN << std::endl;

  // Create LDAP mods for DKIM object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = identity.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(identity.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[2];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 4;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("DKIM");
  objectClassMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(objectClassMod);

  // Required attributes from schema
  if (!selector.empty()) {
    LDAPMod selectorMod;
    selectorMod.mod_op = LDAP_MOD_ADD;
    selectorMod.mod_type = const_cast<char *>("DKIMSelector");
    selectorMod.mod_vals.modv_bvals = new struct berval *[2];
    selectorMod.mod_vals.modv_bvals[0] = new struct berval;
    selectorMod.mod_vals.modv_bvals[0]->bv_len = selector.length();
    selectorMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(selector.c_str());
    selectorMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(selectorMod);
  }
  if (!key.empty()) {
    LDAPMod keyMod;
    keyMod.mod_op = LDAP_MOD_ADD;
    keyMod.mod_type = const_cast<char *>("DKIMKey");
    keyMod.mod_vals.modv_bvals = new struct berval *[2];
    keyMod.mod_vals.modv_bvals[0] = new struct berval;
    keyMod.mod_vals.modv_bvals[0]->bv_len = key.length();
    keyMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(key.c_str());
    keyMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(keyMod);
  }

  // Optional attributes
  if (!domain.empty()) {
    LDAPMod domainMod;
    domainMod.mod_op = LDAP_MOD_ADD;
    domainMod.mod_type = const_cast<char *>("DKIMDomain");
    domainMod.mod_vals.modv_bvals = new struct berval *[2];
    domainMod.mod_vals.modv_bvals[0] = new struct berval;
    domainMod.mod_vals.modv_bvals[0]->bv_len = domain.length();
    domainMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(domain.c_str());
    domainMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(domainMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(identityDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Identity created successfully!" << std::endl;
  return true;
}

bool OpenDKIMManager::updateIdentity(const std::string &identity,
                                     const std::string &baseDN, int argc,
                                     char *argv[]) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  std::cout << "Updating OpenDKIM identity:" << std::endl;
  std::cout << "  Identity: " << identity << std::endl;
  std::cout << "  Identity DN: " << identityDN << std::endl;

  static struct option long_options[] = {
      {"selector", required_argument, 0, 's'},
      {"key", required_argument, 0, 'k'},
      {"domain", required_argument, 0, 'd'},
      {0, 0, 0, 0}};

  std::string selector;
  std::string key;
  std::string domain;

  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "s:k:d:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 's':
      selector = optarg;
      break;
    case 'k':
      key = optarg;
      break;
    case 'd':
      domain = optarg;
      break;
    default:
      std::cerr << "Usage: ldapcli update-identity <identity> [-s selector] "
                   "[-k key] [-d domain]"
                << std::endl;
      return false;
    }
  }

  if (optind >= argc) {
    std::cerr << "Usage: ldapcli update-identity <identity> [-s selector] [-k "
                 "key] [-d domain]"
              << std::endl;
    return false;
  }

  // Get current identity data
  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(cn=" + identity + ")";

  if (!m_connection.search(identityDN, LDAP_SCOPE_BASE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  if (results.empty()) {
    std::cerr << "Error: Identity not found" << std::endl;
    return false;
  }

  // Create LDAP mods for updates
  std::vector<LDAPMod> mods;

  if (!selector.empty()) {
    LDAPMod selectorMod;
    selectorMod.mod_op = LDAP_MOD_REPLACE;
    selectorMod.mod_type = const_cast<char *>("DKIMSelector");
    selectorMod.mod_vals.modv_bvals = new struct berval *[2];
    selectorMod.mod_vals.modv_bvals[0] = new struct berval;
    selectorMod.mod_vals.modv_bvals[0]->bv_len = selector.length();
    selectorMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(selector.c_str());
    selectorMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(selectorMod);
  }
  if (!key.empty()) {
    LDAPMod keyMod;
    keyMod.mod_op = LDAP_MOD_REPLACE;
    keyMod.mod_type = const_cast<char *>("DKIMKey");
    keyMod.mod_vals.modv_bvals = new struct berval *[2];
    keyMod.mod_vals.modv_bvals[0] = new struct berval;
    keyMod.mod_vals.modv_bvals[0]->bv_len = key.length();
    keyMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(key.c_str());
    keyMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(keyMod);
  }
  if (!domain.empty()) {
    LDAPMod domainMod;
    domainMod.mod_op = LDAP_MOD_REPLACE;
    domainMod.mod_type = const_cast<char *>("DKIMDomain");
    domainMod.mod_vals.modv_bvals = new struct berval *[2];
    domainMod.mod_vals.modv_bvals[0] = new struct berval;
    domainMod.mod_vals.modv_bvals[0]->bv_len = domain.length();
    domainMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(domain.c_str());
    domainMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(domainMod);
  }

  if (mods.empty()) {
    std::cerr << "Error: No attributes specified for update" << std::endl;
    return false;
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(identityDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Identity updated successfully!" << std::endl;
  return true;
}

bool OpenDKIMManager::deleteIdentity(const std::string &identity,
                                     const std::string &baseDN) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  std::cout << "Deleting OpenDKIM identity:" << std::endl;
  std::cout << "  Identity: " << identity << std::endl;
  std::cout << "  Identity DN: " << identityDN << std::endl;

  if (!m_connection.deleteEntry(identityDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Identity deleted successfully!" << std::endl;
  return true;
}

std::string OpenDKIMManager::getIdentityDN(const std::string &identity,
                                           const std::string &baseDN) const {
  return "cn=" + identity + "," + baseDN;
}