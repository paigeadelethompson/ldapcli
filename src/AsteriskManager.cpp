#include "AsteriskManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <getopt.h>
#include <sstream>

AsteriskManager::AsteriskManager(LDAPConnection &connection)
    : m_connection(connection) {}

void AsteriskManager::printUsage() const {
  console::e("Asterisk Commands:");
  console::e(
      "  create-account <account> [-s|--secret SECRET] [-c|--caller-id ID] "
      "[-m|--mailbox BOX]");
  console::e(
      "  update-account <account> [-s|--secret SECRET] [-c|--caller-id ID] "
      "[-m|--mailbox BOX]");
  console::e("  delete-account <account> [--account ACCOUNT]");
  console::e("  list-accounts");
  console::e(
      "  create-voicemail <mailbox> [-p|--password PASS] [-f|--fullname NAME] "
      "[-e|--email ADDR]");
  console::e(
      "  update-voicemail <mailbox> [-p|--password PASS] [-f|--fullname NAME] "
      "[-e|--email ADDR]");
  console::e("  delete-voicemail <mailbox> [--mailbox BOX]");
  console::e("  list-voicemail");
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

    optind = 3;
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
        console::e(
            "Usage: ldapcli asterisk create-account <account> "
            "[-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli asterisk create-account <account> "
                 "[-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]");
      return false;
    }

    accountName = argv[optind];

    return createAccount(accountName, baseDN, secret, callerId, mailbox);
  } else if (command == "delete-account") {
    static struct option long_options[] = {
        {"account", required_argument, 0, 'a'}, {nullptr, 0, 0, 0}};

    std::string accountName;
    int opt;
    int option_index = 0;

    optind = 3;
    while ((opt = getopt_long(argc, argv, "a:", long_options, &option_index)) !=
           -1) {
      if (opt == 'a') {
        accountName = optarg;
      } else {
        console::e("Usage: ldapcli asterisk delete-account <account> "
                   "[--account ACCOUNT]");
        return false;
      }
    }

    if (accountName.empty() && optind < argc) {
      accountName = argv[optind];
    }
    if (accountName.empty()) {
      console::e("Usage: ldapcli asterisk delete-account <account> "
                 "[--account ACCOUNT]");
      return false;
    }

    return deleteAccount(accountName, baseDN);
  } else if (command == "update-account") {
    static struct option long_options[] = {
        {"secret", required_argument, 0, 's'},
        {"caller-id", required_argument, 0, 'c'},
        {"mailbox", required_argument, 0, 'm'},
        {0, 0, 0, 0}};

    std::string accountName;
    std::optional<std::string> secret;
    std::optional<std::string> callerId;
    std::optional<std::string> mailbox;

    int opt;
    int option_index = 0;

    optind = 3;
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
        console::e(
            "Usage: ldapcli asterisk update-account <account> "
            "[-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli asterisk update-account <account> "
                 "[-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]");
      return false;
    }

    accountName = argv[optind];

    return updateAccount(accountName, baseDN, secret, callerId, mailbox);
  } else if (command == "list-accounts") {
    return listAccounts(baseDN);
  } else if (command == "create-voicemail") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"fullname", required_argument, 0, 'f'},
        {"email", required_argument, 0, 'e'},
        {0, 0, 0, 0}};

    std::string mailbox;
    std::optional<std::string> password;
    std::optional<std::string> fullname;
    std::optional<std::string> email;

    int opt;
    int option_index = 0;

    optind = 3;
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
        console::e(
            "Usage: ldapcli asterisk create-voicemail <mailbox> "
            "[-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli asterisk create-voicemail <mailbox> "
                 "[-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]");
      return false;
    }

    mailbox = argv[optind];

    return createVoicemailBox(mailbox, baseDN, password, fullname, email);
  } else if (command == "update-voicemail") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"fullname", required_argument, 0, 'f'},
        {"email", required_argument, 0, 'e'},
        {0, 0, 0, 0}};

    std::string mailbox;
    std::optional<std::string> password;
    std::optional<std::string> fullname;
    std::optional<std::string> email;

    int opt;
    int option_index = 0;

    optind = 3;
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
        console::e(
            "Usage: ldapcli asterisk update-voicemail <mailbox> "
            "[-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli asterisk update-voicemail <mailbox> "
                 "[-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]");
      return false;
    }

    mailbox = argv[optind];

    return updateVoicemailBox(mailbox, baseDN, password, fullname, email);
  } else if (command == "delete-voicemail") {
    static struct option long_options[] = {
        {"mailbox", required_argument, 0, 'm'}, {nullptr, 0, 0, 0}};

    std::string mailbox;
    int opt;
    int option_index = 0;

    optind = 3;
    while ((opt = getopt_long(argc, argv, "m:", long_options, &option_index)) !=
           -1) {
      if (opt == 'm') {
        mailbox = optarg;
      } else {
        console::e("Usage: ldapcli asterisk delete-voicemail <mailbox> "
                   "[--mailbox BOX]");
        return false;
      }
    }

    if (mailbox.empty() && optind < argc) {
      mailbox = argv[optind];
    }
    if (mailbox.empty()) {
      console::e("Usage: ldapcli asterisk delete-voicemail <mailbox> "
                 "[--mailbox BOX]");
      return false;
    }

    return deleteVoicemailBox(mailbox, baseDN);
  } else if (command == "list-voicemail") {
    return listVoicemailBoxes(baseDN);
  }

  printUsage();
  return false;
}

bool AsteriskManager::createAccount(const std::string &accountName,
                                    const std::string &baseDN,
                                    const std::optional<std::string> &secret,
                                    const std::optional<std::string> &callerId,
                                    const std::optional<std::string> &mailbox) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  console::e("Creating Asterisk account:");
  console::e("  Account Name: {}", accountName);
  console::e("  Account DN: {}", accountDN);

  // Create LDAP mods for AsteriskSIPUser object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(accountName.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod extMod;
  extMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  extMod.mod_type = const_cast<char *>("objectClass");
  extMod.mod_vals.modv_strvals = new char *[2];
  extMod.mod_vals.modv_strvals[0] = const_cast<char *>("AsteriskExtension");
  extMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(extMod);

  LDAPMod sipMod;
  sipMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  sipMod.mod_type = const_cast<char *>("objectClass");
  sipMod.mod_vals.modv_strvals = new char *[2];
  sipMod.mod_vals.modv_strvals[0] = const_cast<char *>("AsteriskSIPUser");
  sipMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(sipMod);

  // Optional attributes
  if (secret.has_value()) {
    LDAPMod secretMod;
    secretMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    secretMod.mod_type = const_cast<char *>("AstMD5secret");
    secretMod.mod_vals.modv_strvals = new char *[2];
    secretMod.mod_vals.modv_strvals[0] = const_cast<char *>(secret->c_str());
    secretMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(secretMod);
  }
  if (callerId.has_value()) {
    LDAPMod callerIdMod;
    callerIdMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    callerIdMod.mod_type = const_cast<char *>("AstAccountCallerID");
    callerIdMod.mod_vals.modv_strvals = new char *[2];
    callerIdMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(callerId->c_str());
    callerIdMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(callerIdMod);
  }
  if (mailbox.has_value()) {
    LDAPMod mailboxMod;
    mailboxMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mailboxMod.mod_type = const_cast<char *>("AstAccountMailbox");
    mailboxMod.mod_vals.modv_strvals = new char *[2];
    mailboxMod.mod_vals.modv_strvals[0] = const_cast<char *>(mailbox->c_str());
    mailboxMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mailboxMod);
  }

  // Default attributes
  LDAPMod typeMod;
  typeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  typeMod.mod_type = const_cast<char *>("AstAccountType");
  typeMod.mod_vals.modv_strvals = new char *[2];
  typeMod.mod_vals.modv_strvals[0] = const_cast<char *>("friend");
  typeMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(typeMod);

  LDAPMod hostMod;
  hostMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  hostMod.mod_type = const_cast<char *>("AstAccountHost");
  hostMod.mod_vals.modv_strvals = new char *[2];
  hostMod.mod_vals.modv_strvals[0] = const_cast<char *>("dynamic");
  hostMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(hostMod);

  LDAPMod contextMod;
  contextMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  contextMod.mod_type = const_cast<char *>("AstAccountContext");
  contextMod.mod_vals.modv_strvals = new char *[2];
  contextMod.mod_vals.modv_strvals[0] = const_cast<char *>("default");
  contextMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(contextMod);

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(accountDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Account created successfully!");
  return true;
}

bool AsteriskManager::updateAccount(const std::string &accountName,
                                    const std::string &baseDN,
                                    const std::optional<std::string> &secret,
                                    const std::optional<std::string> &callerId,
                                    const std::optional<std::string> &mailbox) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  console::e("Updating Asterisk account:");
  console::e("  Account Name: {}", accountName);
  console::e("  Account DN: {}", accountDN);

  // Create LDAP mods for AsteriskSIPUser object class
  std::vector<LDAPMod> mods;

  // Optional attributes
  if (secret.has_value()) {
    LDAPMod secretMod;
    secretMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    secretMod.mod_type = const_cast<char *>("AstMD5secret");
    secretMod.mod_vals.modv_strvals = new char *[2];
    secretMod.mod_vals.modv_strvals[0] = const_cast<char *>(secret->c_str());
    secretMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(secretMod);
  }
  if (callerId.has_value()) {
    LDAPMod callerIdMod;
    callerIdMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    callerIdMod.mod_type = const_cast<char *>("AstAccountCallerID");
    callerIdMod.mod_vals.modv_strvals = new char *[2];
    callerIdMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(callerId->c_str());
    callerIdMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(callerIdMod);
  }
  if (mailbox.has_value()) {
    LDAPMod mailboxMod;
    mailboxMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    mailboxMod.mod_type = const_cast<char *>("AstAccountMailbox");
    mailboxMod.mod_vals.modv_strvals = new char *[2];
    mailboxMod.mod_vals.modv_strvals[0] = const_cast<char *>(mailbox->c_str());
    mailboxMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mailboxMod);
  }

  if (mods.empty()) {
    console::e("No attributes to update.");
    return false;
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(accountDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Account updated successfully!");
  return true;
}

bool AsteriskManager::deleteAccount(const std::string &accountName,
                                    const std::string &baseDN) {
  std::string accountDN = getAccountDN(accountName, baseDN);

  console::e("Deleting Asterisk account:");
  console::e("  Account Name: {}", accountName);
  console::e("  Account DN: {}", accountDN);

  if (!m_connection.deleteEntry(accountDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Account deleted successfully!");
  return true;
}

bool AsteriskManager::listAccounts(const std::string &baseDN) {
  console::e("Listing Asterisk accounts:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=AsteriskSIPUser)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No accounts found.");
    return true;
  }

  // Convert results to table format for display
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

bool AsteriskManager::createVoicemailBox(
    const std::string &mailbox, const std::string &baseDN,
    const std::optional<std::string> &password,
    const std::optional<std::string> &fullname,
    const std::optional<std::string> &email) {
  std::string mailboxDN = getMailboxDN(mailbox, baseDN);

  console::e("Creating Asterisk voicemail box:");
  console::e("  Mailbox: {}", mailbox);
  console::e("  Mailbox DN: {}", mailboxDN);

  // Create LDAP mods for AsteriskVoiceMail object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(mailbox.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod extMod;
  extMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  extMod.mod_type = const_cast<char *>("objectClass");
  extMod.mod_vals.modv_strvals = new char *[2];
  extMod.mod_vals.modv_strvals[0] = const_cast<char *>("AsteriskExtension");
  extMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(extMod);

  LDAPMod vmMod;
  vmMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  vmMod.mod_type = const_cast<char *>("objectClass");
  vmMod.mod_vals.modv_strvals = new char *[2];
  vmMod.mod_vals.modv_strvals[0] = const_cast<char *>("AsteriskVoiceMail");
  vmMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(vmMod);

  // Required attribute
  LDAPMod contextMod;
  contextMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  contextMod.mod_type = const_cast<char *>("AstVoicemailContext");
  contextMod.mod_vals.modv_strvals = new char *[2];
  contextMod.mod_vals.modv_strvals[0] = const_cast<char *>("default");
  contextMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(contextMod);

  // Optional attributes
  if (password.has_value()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    passwordMod.mod_type = const_cast<char *>("AstVoicemailPassword");
    passwordMod.mod_vals.modv_strvals = new char *[2];
    passwordMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(password->c_str());
    passwordMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (fullname.has_value()) {
    LDAPMod fullnameMod;
    fullnameMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    fullnameMod.mod_type = const_cast<char *>("AstVoicemailFullname");
    fullnameMod.mod_vals.modv_strvals = new char *[2];
    fullnameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(fullname->c_str());
    fullnameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(fullnameMod);
  }
  if (email.has_value()) {
    LDAPMod emailMod;
    emailMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    emailMod.mod_type = const_cast<char *>("AstVoicemailEmail");
    emailMod.mod_vals.modv_strvals = new char *[2];
    emailMod.mod_vals.modv_strvals[0] = const_cast<char *>(email->c_str());
    emailMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(emailMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(mailboxDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Voicemail box created successfully!");
  return true;
}

bool AsteriskManager::updateVoicemailBox(
    const std::string &mailbox, const std::string &baseDN,
    const std::optional<std::string> &password,
    const std::optional<std::string> &fullname,
    const std::optional<std::string> &email) {
  std::string mailboxDN = getMailboxDN(mailbox, baseDN);

  console::e("Updating Asterisk voicemail box:");
  console::e("  Mailbox: {}", mailbox);
  console::e("  Mailbox DN: {}", mailboxDN);

  // Create LDAP mods for AsteriskVoiceMail object class
  std::vector<LDAPMod> mods;

  // Optional attributes
  if (password.has_value()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    passwordMod.mod_type = const_cast<char *>("AstVoicemailPassword");
    passwordMod.mod_vals.modv_strvals = new char *[2];
    passwordMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(password->c_str());
    passwordMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (fullname.has_value()) {
    LDAPMod fullnameMod;
    fullnameMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    fullnameMod.mod_type = const_cast<char *>("AstVoicemailFullname");
    fullnameMod.mod_vals.modv_strvals = new char *[2];
    fullnameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(fullname->c_str());
    fullnameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(fullnameMod);
  }
  if (email.has_value()) {
    LDAPMod emailMod;
    emailMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    emailMod.mod_type = const_cast<char *>("AstVoicemailEmail");
    emailMod.mod_vals.modv_strvals = new char *[2];
    emailMod.mod_vals.modv_strvals[0] = const_cast<char *>(email->c_str());
    emailMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(emailMod);
  }

  if (mods.empty()) {
    console::e("No attributes to update.");
    return false;
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(mailboxDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Voicemail box updated successfully!");
  return true;
}

bool AsteriskManager::deleteVoicemailBox(const std::string &mailbox,
                                         const std::string &baseDN) {
  std::string mailboxDN = getMailboxDN(mailbox, baseDN);

  console::e("Deleting Asterisk voicemail box:");
  console::e("  Mailbox: {}", mailbox);
  console::e("  Mailbox DN: {}", mailboxDN);

  if (!m_connection.deleteEntry(mailboxDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Voicemail box deleted successfully!");
  return true;
}

bool AsteriskManager::listVoicemailBoxes(const std::string &baseDN) {
  console::e("Listing Asterisk voicemail boxes:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=AsteriskVoiceMail)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No voicemail boxes found.");
    return true;
  }

  // Convert results to table format for display
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

std::string AsteriskManager::getAccountDN(const std::string &accountName,
                                          const std::string &baseDN) const {
  return "cn=" + accountName + "," + baseDN;
}

std::string AsteriskManager::getMailboxDN(const std::string &mailbox,
                                          const std::string &baseDN) const {
  return "cn=" + mailbox + "," + baseDN;
}
