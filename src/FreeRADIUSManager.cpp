#include "FreeRADIUSManager.hpp"
#include "Config.hpp"
#include <iostream>
#include <sstream>

FreeRADIUSManager::FreeRADIUSManager(LDAPConnection &connection)
    : m_connection(connection) {}

void FreeRADIUSManager::printUsage() const {
  std::cout << "FreeRADIUS Commands:" << std::endl;
  std::cout << "  create-client <client> [base-dn]" << std::endl;
  std::cout << "  delete-client <client> [base-dn]" << std::endl;
  std::cout << "  update-client <client> [base-dn]" << std::endl;
  std::cout << "  list-clients [base-dn]" << std::endl;
  std::cout << "  create-user <username> [base-dn]" << std::endl;
  std::cout << "  delete-user <username> [base-dn]" << std::endl;
  std::cout << "  list-users [base-dn]" << std::endl;
}

std::string FreeRADIUSManager::getServiceName() const { return "freeradius"; }

bool FreeRADIUSManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-client") {
    static struct option long_options[] = {
        {"secret", required_argument, 0, 's'},
        {"shortname", required_argument, 0, 'n'},
        {"type", required_argument, 0, 't'},
        {0, 0, 0, 0}};

    std::string clientName;
    std::string secret;
    std::string shortname;
    std::string type = "auth";

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "s:n:t:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 's':
        secret = optarg;
        break;
      case 'n':
        shortname = optarg;
        break;
      case 't':
        type = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-client <client-name> [-s secret] "
                     "[-n shortname] [-t type]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-client <client-name> [-s secret] "
                   "[-n shortname] [-t type]"
                << std::endl;
      return false;
    }

    clientName = argv[optind];

    return createClient(clientName, baseDN, secret, shortname, type);
  } else if (command == "delete-client") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-client <client-name>" << std::endl;
      return false;
    }

    std::string clientName = argv[optind];

    return deleteClient(clientName, baseDN);
  } else if (command == "update-client") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-client <client-name>" << std::endl;
      return false;
    }

    std::string clientName = argv[optind];

    return updateClient(clientName, baseDN);
  } else if (command == "list-clients") {
    return listClients(baseDN);
  } else if (command == "create-user") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"service-type", required_argument, 0, 's'},
        {"framed-protocol", required_argument, 0, 'f'},
        {0, 0, 0, 0}};

    std::string username;
    std::string password;
    std::string serviceType = "Framed-User";
    std::string framedProtocol = "PPP";

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "s:f:p:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'p':
        password = optarg;
        break;
      case 's':
        serviceType = optarg;
        break;
      case 'f':
        framedProtocol = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-user <username> [-p password] "
                     "[-s service-type] [-f framed-protocol]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-user <username> [-p password] "
                   "[-s service-type] [-f framed-protocol]"
                << std::endl;
      return false;
    }

    username = argv[optind];

    return createUser(username, baseDN, password, serviceType, framedProtocol);
  } else if (command == "delete-user") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-user <username>" << std::endl;
      return false;
    }

    std::string username = argv[optind];

    return deleteUser(username, baseDN);
  } else if (command == "list-users") {
    return listUsers(baseDN);
  }

  printUsage();
  return false;
}

bool FreeRADIUSManager::createClient(const std::string &clientName,
                                     const std::string &baseDN,
                                     const std::string &secret,
                                     const std::string &shortname,
                                     const std::string &type) {
  std::string clientDN = getClientDN(clientName, baseDN);

  std::cout << "Creating FreeRADIUS client:" << std::endl;
  std::cout << "  Client Name: " << clientName << std::endl;
  std::cout << "  Client DN: " << clientDN << std::endl;

  // Create LDAP mods for FreeRADIUSClient object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = clientName.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(clientName.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[2];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 18;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("FreeRADIUSClient");
  objectClassMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(objectClassMod);

  // Optional attributes
  if (!secret.empty()) {
    LDAPMod secretMod;
    secretMod.mod_op = LDAP_MOD_ADD;
    secretMod.mod_type = const_cast<char *>("radiusClientSecret");
    secretMod.mod_vals.modv_bvals = new struct berval *[2];
    secretMod.mod_vals.modv_bvals[0] = new struct berval;
    secretMod.mod_vals.modv_bvals[0]->bv_len = secret.length();
    secretMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(secret.c_str());
    secretMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(secretMod);
  }
  if (!shortname.empty()) {
    LDAPMod shortnameMod;
    shortnameMod.mod_op = LDAP_MOD_ADD;
    shortnameMod.mod_type = const_cast<char *>("radiusClientShortname");
    shortnameMod.mod_vals.modv_bvals = new struct berval *[2];
    shortnameMod.mod_vals.modv_bvals[0] = new struct berval;
    shortnameMod.mod_vals.modv_bvals[0]->bv_len = shortname.length();
    shortnameMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(shortname.c_str());
    shortnameMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(shortnameMod);
  }
  if (!type.empty()) {
    LDAPMod typeMod;
    typeMod.mod_op = LDAP_MOD_ADD;
    typeMod.mod_type = const_cast<char *>("radiusClientType");
    typeMod.mod_vals.modv_bvals = new struct berval *[2];
    typeMod.mod_vals.modv_bvals[0] = new struct berval;
    typeMod.mod_vals.modv_bvals[0]->bv_len = type.length();
    typeMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(type.c_str());
    typeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(typeMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(clientDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Client created successfully!" << std::endl;
  return true;
}

bool FreeRADIUSManager::updateClient(const std::string &clientName,
                                     const std::string &baseDN) {
  std::string clientDN = getClientDN(clientName, baseDN);

  std::cout << "Updating FreeRADIUS client:" << std::endl;
  std::cout << "  Client Name: " << clientName << std::endl;
  std::cout << "  Client DN: " << clientDN << std::endl;

  // TODO: Implement update logic
  std::cerr << "Update functionality not yet implemented" << std::endl;
  return false;
}

bool FreeRADIUSManager::deleteClient(const std::string &clientName,
                                     const std::string &baseDN) {
  std::string clientDN = getClientDN(clientName, baseDN);

  std::cout << "Deleting FreeRADIUS client:" << std::endl;
  std::cout << "  Client Name: " << clientName << std::endl;
  std::cout << "  Client DN: " << clientDN << std::endl;

  if (!m_connection.deleteEntry(clientDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Client deleted successfully!" << std::endl;
  return true;
}

bool FreeRADIUSManager::listClients(const std::string &baseDN) {
  std::cout << "Listing FreeRADIUS clients:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=FreeRADIUSClient)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " clients:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\nClient " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

bool FreeRADIUSManager::createUser(const std::string &username,
                                   const std::string &baseDN,
                                   const std::string &password,
                                   const std::string &serviceType,
                                   const std::string &framedProtocol) {
  std::string userDN = getUserDN(username, baseDN);

  std::cout << "Creating FreeRADIUS user:" << std::endl;
  std::cout << "  Username: " << username << std::endl;
  std::cout << "  User DN: " << userDN << std::endl;

  // Create LDAP mods for FreeRADIUSUser object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = username.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(username.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[2];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 15;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("FreeRADIUSUser");
  objectClassMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(objectClassMod);

  // Optional attributes
  if (!password.empty()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD;
    passwordMod.mod_type = const_cast<char *>("radiusUserPassword");
    passwordMod.mod_vals.modv_bvals = new struct berval *[2];
    passwordMod.mod_vals.modv_bvals[0] = new struct berval;
    passwordMod.mod_vals.modv_bvals[0]->bv_len = password.length();
    passwordMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(password.c_str());
    passwordMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (!serviceType.empty()) {
    LDAPMod serviceTypeMod;
    serviceTypeMod.mod_op = LDAP_MOD_ADD;
    serviceTypeMod.mod_type = const_cast<char *>("radiusServiceType");
    serviceTypeMod.mod_vals.modv_bvals = new struct berval *[2];
    serviceTypeMod.mod_vals.modv_bvals[0] = new struct berval;
    serviceTypeMod.mod_vals.modv_bvals[0]->bv_len = serviceType.length();
    serviceTypeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(serviceType.c_str());
    serviceTypeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(serviceTypeMod);
  }
  if (!framedProtocol.empty()) {
    LDAPMod framedProtocolMod;
    framedProtocolMod.mod_op = LDAP_MOD_ADD;
    framedProtocolMod.mod_type = const_cast<char *>("radiusFramedProtocol");
    framedProtocolMod.mod_vals.modv_bvals = new struct berval *[2];
    framedProtocolMod.mod_vals.modv_bvals[0] = new struct berval;
    framedProtocolMod.mod_vals.modv_bvals[0]->bv_len = framedProtocol.length();
    framedProtocolMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(framedProtocol.c_str());
    framedProtocolMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(framedProtocolMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(userDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "User created successfully!" << std::endl;
  return true;
}

bool FreeRADIUSManager::deleteUser(const std::string &username,
                                   const std::string &baseDN) {
  std::string userDN = getUserDN(username, baseDN);

  std::cout << "Deleting FreeRADIUS user:" << std::endl;
  std::cout << "  Username: " << username << std::endl;
  std::cout << "  User DN: " << userDN << std::endl;

  if (!m_connection.deleteEntry(userDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "User deleted successfully!" << std::endl;
  return true;
}

bool FreeRADIUSManager::listUsers(const std::string &baseDN) {
  std::cout << "Listing FreeRADIUS users:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=FreeRADIUSUser)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " users:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\nUser " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

std::string FreeRADIUSManager::getClientDN(const std::string &clientName,
                                           const std::string &baseDN) const {
  return "cn=" + clientName + "," + baseDN;
}

std::string FreeRADIUSManager::getUserDN(const std::string &username,
                                         const std::string &baseDN) const {
  return "cn=" + username + "," + baseDN;
}
