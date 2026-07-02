#include "AsteriskManager.hpp"
#include "Config.hpp"
#include <iostream>
#include <sstream>

AsteriskManager::AsteriskManager(LDAPConnection &connection)
    : m_connection(connection) {}

void AsteriskManager::printUsage() const {
  std::cout << "Asterisk Commands:" << std::endl;
  std::cout << "  create-account <account> [base-dn]" << std::endl;
  std::cout << "  delete-account <account> [base-dn]" << std::endl;
  std::cout << "  update-account <account> [base-dn]" << std::endl;
  std::cout << "  list-accounts [base-dn]" << std::endl;
  std::cout << "  create-voicemail <mailbox> [base-dn]" << std::endl;
  std::cout << "  delete-voicemail <mailbox> [base-dn]" << std::endl;
  std::cout << "  list-voicemail [base-dn]" << std::endl;
}

std::string AsteriskManager::getServiceName() const { return "asterisk"; }

bool AsteriskManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-account") {
    static struct option long_options[] = {
        {"secret", required_argument, 0, 's'},
        {"caller-id", required_argument, 0, 'c'},
        {"mailbox", required_argument, 0, 'm'},
        {0, 0, 0, 0}};

    std::string accountName;
    std::string secret;
    std::string callerId;
    std::string mailbox;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "s:c:m:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 's':
        secret = optarg;
        break;
      case 'c':
        callerId = optarg;
        break;
      case 'm':
        mailbox = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-account <account-name> [-s secret] "
                     "[-c caller-id] [-m mailbox]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-account <account-name> [-s secret] "
                   "[-c caller-id] [-m mailbox]"
                << std::endl;
      return false;
    }

    accountName = argv[optind];

    return createAccount(accountName, baseDN, secret, callerId, mailbox);
  } else if (command == "delete-account") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-account <account-name>" << std::endl;
      return false;
    }

    std::string accountName = argv[optind];

    return deleteAccount(accountName, baseDN);
  } else if (command == "update-account") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-account <account-name>" << std::endl;
      return false;
    }

    std::string accountName = argv[optind];

    return updateAccount(accountName, baseDN);
  } else if (command == "list-accounts") {
    return listAccounts(baseDN);
  } else if (command == "create-voicemail") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"fullname", required_argument, 0, 'f'},
        {"email", required_argument, 0, 'e'},
        {0, 0, 0, 0}};

    std::string mailbox;
    std::string password;
    std::string fullname;
    std::string email;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "p:f:e:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'p':
        password = optarg;
        break;
      case 'f':
        fullname = optarg;
        break;
      case 'e':
        email = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-voicemail <mailbox> [-p password] "
                     "[-f fullname] [-e email]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-voicemail <mailbox> [-p password] "
                   "[-f fullname] [-e email]"
                << std::endl;
      return false;
    }

    mailbox = argv[optind];

    return createVoicemailBox(mailbox, baseDN, password, fullname, email);
  } else if (command == "delete-voicemail") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-voicemail <mailbox>" << std::endl;
      return false;
    }

    std::string mailbox = argv[optind];

    return deleteVoicemailBox(mailbox, baseDN);
  } else if (command == "list-voicemail") {
    return listVoicemailBoxes(baseDN);
  }

  printUsage();
  return false;
}

bool AsteriskManager::createAccount(const std::string &accountName,
                                    const std::string &baseDN,
                                    const std::string &secret,
                                    const std::string &callerId,
                                    const std::string &mailbox) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  std::cout << "Creating Asterisk account:" << std::endl;
  std::cout << "  Account Name: " << accountName << std::endl;
  std::cout << "  Account DN: " << accountDN << std::endl;

  // Create LDAP mods for AsteriskSIPUser object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = accountName.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>(accountName.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod extMod;
  extMod.mod_op = LDAP_MOD_ADD;
  extMod.mod_type = const_cast<char *>("objectClass");
  extMod.mod_vals.modv_bvals = new struct berval *[2];
  extMod.mod_vals.modv_bvals[0] = new struct berval;
  extMod.mod_vals.modv_bvals[0]->bv_len = 21;
  extMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("AsteriskExtension");
  extMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(extMod);

  LDAPMod sipMod;
  sipMod.mod_op = LDAP_MOD_ADD;
  sipMod.mod_type = const_cast<char *>("objectClass");
  sipMod.mod_vals.modv_bvals = new struct berval *[2];
  sipMod.mod_vals.modv_bvals[0] = new struct berval;
  sipMod.mod_vals.modv_bvals[0]->bv_len = 16;
  sipMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("AsteriskSIPUser");
  sipMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(sipMod);

  // Optional attributes
  if (!secret.empty()) {
    LDAPMod secretMod;
    secretMod.mod_op = LDAP_MOD_ADD;
    secretMod.mod_type = const_cast<char *>("AstMD5secret");
    secretMod.mod_vals.modv_bvals = new struct berval *[2];
    secretMod.mod_vals.modv_bvals[0] = new struct berval;
    secretMod.mod_vals.modv_bvals[0]->bv_len = secret.length();
    secretMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(secret.c_str());
    secretMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(secretMod);
  }
  if (!callerId.empty()) {
    LDAPMod callerIdMod;
    callerIdMod.mod_op = LDAP_MOD_ADD;
    callerIdMod.mod_type = const_cast<char *>("AstCallerID");
    callerIdMod.mod_vals.modv_bvals = new struct berval *[2];
    callerIdMod.mod_vals.modv_bvals[0] = new struct berval;
    callerIdMod.mod_vals.modv_bvals[0]->bv_len = callerId.length();
    callerIdMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(callerId.c_str());
    callerIdMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(callerIdMod);
  }
  if (!mailbox.empty()) {
    LDAPMod mailboxMod;
    mailboxMod.mod_op = LDAP_MOD_ADD;
    mailboxMod.mod_type = const_cast<char *>("AstMailbox");
    mailboxMod.mod_vals.modv_bvals = new struct berval *[2];
    mailboxMod.mod_vals.modv_bvals[0] = new struct berval;
    mailboxMod.mod_vals.modv_bvals[0]->bv_len = mailbox.length();
    mailboxMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(mailbox.c_str());
    mailboxMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(mailboxMod);
  }

  // Default attributes
  LDAPMod typeMod;
  typeMod.mod_op = LDAP_MOD_ADD;
  typeMod.mod_type = const_cast<char *>("AstAccountType");
  typeMod.mod_vals.modv_bvals = new struct berval *[2];
  typeMod.mod_vals.modv_bvals[0] = new struct berval;
  typeMod.mod_vals.modv_bvals[0]->bv_len = 5;
  typeMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("friend");
  typeMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(typeMod);

  LDAPMod hostMod;
  hostMod.mod_op = LDAP_MOD_ADD;
  hostMod.mod_type = const_cast<char *>("AstAccountHost");
  hostMod.mod_vals.modv_bvals = new struct berval *[2];
  hostMod.mod_vals.modv_bvals[0] = new struct berval;
  hostMod.mod_vals.modv_bvals[0]->bv_len = 7;
  hostMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("dynamic");
  hostMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(hostMod);

  LDAPMod contextMod;
  contextMod.mod_op = LDAP_MOD_ADD;
  contextMod.mod_type = const_cast<char *>("AstAccountContext");
  contextMod.mod_vals.modv_bvals = new struct berval *[2];
  contextMod.mod_vals.modv_bvals[0] = new struct berval;
  contextMod.mod_vals.modv_bvals[0]->bv_len = 7;
  contextMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("default");
  contextMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(contextMod);

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(accountDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Account created successfully!" << std::endl;
  return true;
}

bool AsteriskManager::updateAccount(const std::string &accountName,
                                    const std::string &baseDN) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  std::cout << "Updating Asterisk account:" << std::endl;
  std::cout << "  Account Name: " << accountName << std::endl;
  std::cout << "  Account DN: " << accountDN << std::endl;

  // TODO: Implement update logic with command line arguments
  std::cerr << "Update functionality not yet implemented" << std::endl;
  return false;
}

bool AsteriskManager::deleteAccount(const std::string &accountName,
                                    const std::string &baseDN) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  std::cout << "Deleting Asterisk account:" << std::endl;
  std::cout << "  Account Name: " << accountName << std::endl;
  std::cout << "  Account DN: " << accountDN << std::endl;

  if (!m_connection.deleteEntry(accountDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Account deleted successfully!" << std::endl;
  return true;
}

bool AsteriskManager::listAccounts(const std::string &baseDN) {
  std::cout << "Listing Asterisk accounts:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=AsteriskSIPUser)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " accounts:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\nAccount " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

bool AsteriskManager::createVoicemailBox(const std::string &mailbox,
                                         const std::string &baseDN,
                                         const std::string &password,
                                         const std::string &fullname,
                                         const std::string &email) {
  std::string mailboxDN = getMailboxDN(mailbox, baseDN);

  std::cout << "Creating Asterisk voicemail box:" << std::endl;
  std::cout << "  Mailbox: " << mailbox << std::endl;
  std::cout << "  Mailbox DN: " << mailboxDN << std::endl;

  // Create LDAP mods for AsteriskVoiceMail object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = mailbox.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(mailbox.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod extMod;
  extMod.mod_op = LDAP_MOD_ADD;
  extMod.mod_type = const_cast<char *>("objectClass");
  extMod.mod_vals.modv_bvals = new struct berval *[2];
  extMod.mod_vals.modv_bvals[0] = new struct berval;
  extMod.mod_vals.modv_bvals[0]->bv_len = 21;
  extMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("AsteriskExtension");
  extMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(extMod);

  LDAPMod vmMod;
  vmMod.mod_op = LDAP_MOD_ADD;
  vmMod.mod_type = const_cast<char *>("objectClass");
  vmMod.mod_vals.modv_bvals = new struct berval *[2];
  vmMod.mod_vals.modv_bvals[0] = new struct berval;
  vmMod.mod_vals.modv_bvals[0]->bv_len = 16;
  vmMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("AsteriskVoiceMail");
  vmMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(vmMod);

  // Optional attributes
  if (!password.empty()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD;
    passwordMod.mod_type = const_cast<char *>("AstVoicemailPassword");
    passwordMod.mod_vals.modv_bvals = new struct berval *[2];
    passwordMod.mod_vals.modv_bvals[0] = new struct berval;
    passwordMod.mod_vals.modv_bvals[0]->bv_len = password.length();
    passwordMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(password.c_str());
    passwordMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (!fullname.empty()) {
    LDAPMod fullnameMod;
    fullnameMod.mod_op = LDAP_MOD_ADD;
    fullnameMod.mod_type = const_cast<char *>("AstVoicemailUser");
    fullnameMod.mod_vals.modv_bvals = new struct berval *[2];
    fullnameMod.mod_vals.modv_bvals[0] = new struct berval;
    fullnameMod.mod_vals.modv_bvals[0]->bv_len = fullname.length();
    fullnameMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(fullname.c_str());
    fullnameMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(fullnameMod);
  }
  if (!email.empty()) {
    LDAPMod emailMod;
    emailMod.mod_op = LDAP_MOD_ADD;
    emailMod.mod_type = const_cast<char *>("AstVoicemailEmail");
    emailMod.mod_vals.modv_bvals = new struct berval *[2];
    emailMod.mod_vals.modv_bvals[0] = new struct berval;
    emailMod.mod_vals.modv_bvals[0]->bv_len = email.length();
    emailMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(email.c_str());
    emailMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(emailMod);
  }

  // Default attributes
  LDAPMod contextMod;
  contextMod.mod_op = LDAP_MOD_ADD;
  contextMod.mod_type = const_cast<char *>("AstVoicemailContext");
  contextMod.mod_vals.modv_bvals = new struct berval *[2];
  contextMod.mod_vals.modv_bvals[0] = new struct berval;
  contextMod.mod_vals.modv_bvals[0]->bv_len = 7;
  contextMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>("default");
  contextMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(contextMod);

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(mailboxDN, modPtrs.data())) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Voicemail box created successfully!" << std::endl;
  return true;
}

bool AsteriskManager::deleteVoicemailBox(const std::string &mailbox,
                                         const std::string &baseDN) {
  std::string mailboxDN = getMailboxDN(mailbox, baseDN);

  std::cout << "Deleting Asterisk voicemail box:" << std::endl;
  std::cout << "  Mailbox: " << mailbox << std::endl;
  std::cout << "  Mailbox DN: " << mailboxDN << std::endl;

  if (!m_connection.deleteEntry(mailboxDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Voicemail box deleted successfully!" << std::endl;
  return true;
}

bool AsteriskManager::listVoicemailBoxes(const std::string &baseDN) {
  std::cout << "Listing Asterisk voicemail boxes:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=AsteriskVoiceMail)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " voicemail boxes:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\nVoicemail Box " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

std::string AsteriskManager::getAccountDN(const std::string &accountName,
                                          const std::string &baseDN) const {
  return "cn=" + accountName + "," + baseDN;
}

std::string AsteriskManager::getMailboxDN(const std::string &mailbox,
                                          const std::string &baseDN) const {
  return "cn=" + mailbox + "," + baseDN;
}
