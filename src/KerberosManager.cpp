#include "KerberosManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

KerberosManager::KerberosManager(LDAPConnection &connection)
    : m_connection(connection) {}

void KerberosManager::printUsage() const {
  console::e("Kerberos Commands:");
  console::e("  create-principal <principal> [base-dn]");
  console::e("  delete-principal <principal> [base-dn]");
  console::e("  update-principal <principal> [base-dn]");
  console::e("  list-principals [base-dn]");
}

std::string KerberosManager::getServiceName() const { return "kerberos"; }

bool KerberosManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-principal") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"fullname", required_argument, 0, 'f'},
        {"email", required_argument, 0, 'e'},
        {"canonicalname", required_argument, 0, 'c'},
        {"principaltype", required_argument, 0, 't'},
        {"principalexpiration", required_argument, 0, 'x'},
        {"passwordexpiration", required_argument, 0, 'X'},
        {"ticketflags", required_argument, 0, 'F'},
        {"maxticketlife", required_argument, 0, 'l'},
        {"maxrenewableage", required_argument, 0, 'r'},
        {"lastpwdchange", required_argument, 0, 'P'},
        {"lastsuccessfulauth", required_argument, 0, 'S'},
        {"lastfailedauth", required_argument, 0, 'L'},
        {"loginfailedcount", required_argument, 0, 'C'},
        {"principalaliases", required_argument, 0, 'a'},
        {"allowedtodelegateto", required_argument, 0, 'd'},
        {"principalauthind", required_argument, 0, 'i'},
        {0, 0, 0, 0}};

    std::string principal;
    std::string password, fullname, email, canonicalName;
    std::string principalType, principalExpiration, passwordExpiration;
    std::string ticketFlags, maxTicketLife, maxRenewableAge;
    std::string lastPwdChange, lastSuccessfulAuth, lastFailedAuth;
    std::string loginFailedCount, principalAliases, allowedToDelegateTo;
    std::string principalAuthInd;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "p:f:e:c:t:x:X:F:l:r:P:S:L:C:a:d:i:",
                              long_options, &option_index)) != -1) {
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
      case 'c':
        canonicalName = optarg;
        break;
      case 't':
        principalType = optarg;
        break;
      case 'x':
        principalExpiration = optarg;
        break;
      case 'X':
        passwordExpiration = optarg;
        break;
      case 'F':
        ticketFlags = optarg;
        break;
      case 'l':
        maxTicketLife = optarg;
        break;
      case 'r':
        maxRenewableAge = optarg;
        break;
      case 'P':
        lastPwdChange = optarg;
        break;
      case 'S':
        lastSuccessfulAuth = optarg;
        break;
      case 'L':
        lastFailedAuth = optarg;
        break;
      case 'C':
        loginFailedCount = optarg;
        break;
      case 'a':
        principalAliases = optarg;
        break;
      case 'd':
        allowedToDelegateTo = optarg;
        break;
      case 'i':
        principalAuthInd = optarg;
        break;
      default:
        console::e("Usage: ldapcli create-principal <principal> [options]");
        console::e("Options:");
        console::e("  -p, --password <secret>        Kerberos password");
        console::e("  -f, --fullname <name>          Full name");
        console::e("  -e, --email <email>            Email address");
        console::e("  -c, --canonicalname <name>     Canonical principal name");
        console::e("  -t, --principaltype <type>     Principal type");
        console::e("  -x, --principalexpiration <date> Principal expiration");
        console::e("  -X, --passwordexpiration <date> Password expiration");
        console::e("  -F, --ticketflags <flags>      Ticket flags");
        console::e("  -l, --maxticketlife <seconds>  Max ticket lifetime");
        console::e("  -r, --maxrenewableage <seconds> Max renewable age");
        console::e("  -P, --lastpwdchange <date>     Last password change");
        console::e("  -S, --lastsuccessfulauth <date> Last successful auth");
        console::e("  -L, --lastfailedauth <date>    Last failed auth");
        console::e("  -C, --loginfailedcount <count> Login failed count");
        console::e("  -a, --principalaliases <aliases> Principal aliases");
        console::e(
            "  -d, --allowedtodelegateto <services> Allowed to delegate to");
        console::e(
            "  -i, --principalauthind <indicators> Principal auth indicators");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-principal <principal> [options]");
      return false;
    }

    principal = argv[optind];

    auto optStr = [](const std::string &s) -> std::optional<std::string> {
      return s.empty() ? std::nullopt : std::optional<std::string>(s);
    };

    return createPrincipal(
        principal, baseDN, optStr(password), optStr(fullname), optStr(email),
        optStr(canonicalName), optStr(principalType),
        optStr(principalExpiration), optStr(passwordExpiration),
        optStr(ticketFlags), optStr(maxTicketLife), optStr(maxRenewableAge),
        optStr(lastPwdChange), optStr(lastSuccessfulAuth),
        optStr(lastFailedAuth), optStr(loginFailedCount),
        optStr(principalAliases), optStr(allowedToDelegateTo),
        optStr(principalAuthInd));
  } else if (command == "delete-principal") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-principal <principal>");
      return false;
    }

    std::string principal = argv[optind];

    return deletePrincipal(principal, baseDN);
  } else if (command == "update-principal") {
    static struct option long_options[] = {
        {"canonicalname", required_argument, 0, 'c'},
        {"principaltype", required_argument, 0, 't'},
        {"principalexpiration", required_argument, 0, 'x'},
        {"passwordexpiration", required_argument, 0, 'X'},
        {"ticketflags", required_argument, 0, 'F'},
        {"maxticketlife", required_argument, 0, 'l'},
        {"maxrenewableage", required_argument, 0, 'r'},
        {"lastpwdchange", required_argument, 0, 'P'},
        {"lastsuccessfulauth", required_argument, 0, 'S'},
        {"lastfailedauth", required_argument, 0, 'L'},
        {"loginfailedcount", required_argument, 0, 'C'},
        {"principalaliases", required_argument, 0, 'a'},
        {"allowedtodelegateto", required_argument, 0, 'd'},
        {"principalauthind", required_argument, 0, 'i'},
        {0, 0, 0, 0}};

    std::string principal;
    std::string canonicalName, principalType, principalExpiration,
        passwordExpiration;
    std::string ticketFlags, maxTicketLife, maxRenewableAge;
    std::string lastPwdChange, lastSuccessfulAuth, lastFailedAuth;
    std::string loginFailedCount, principalAliases, allowedToDelegateTo;
    std::string principalAuthInd;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "c:t:x:X:F:l:r:P:S:L:C:a:d:i:",
                              long_options, &option_index)) != -1) {
      switch (opt) {
      case 'c':
        canonicalName = optarg;
        break;
      case 't':
        principalType = optarg;
        break;
      case 'x':
        principalExpiration = optarg;
        break;
      case 'X':
        passwordExpiration = optarg;
        break;
      case 'F':
        ticketFlags = optarg;
        break;
      case 'l':
        maxTicketLife = optarg;
        break;
      case 'r':
        maxRenewableAge = optarg;
        break;
      case 'P':
        lastPwdChange = optarg;
        break;
      case 'S':
        lastSuccessfulAuth = optarg;
        break;
      case 'L':
        lastFailedAuth = optarg;
        break;
      case 'C':
        loginFailedCount = optarg;
        break;
      case 'a':
        principalAliases = optarg;
        break;
      case 'd':
        allowedToDelegateTo = optarg;
        break;
      case 'i':
        principalAuthInd = optarg;
        break;
      default:
        console::e("Usage: ldapcli update-principal <principal> [options]");
        console::e("Options:");
        console::e("  -c, --canonicalname <name>     Canonical principal name");
        console::e("  -t, --principaltype <type>     Principal type");
        console::e("  -x, --principalexpiration <date> Principal expiration");
        console::e("  -X, --passwordexpiration <date> Password expiration");
        console::e("  -F, --ticketflags <flags>      Ticket flags");
        console::e("  -l, --maxticketlife <seconds>  Max ticket lifetime");
        console::e("  -r, --maxrenewableage <seconds> Max renewable age");
        console::e("  -P, --lastpwdchange <date>     Last password change");
        console::e("  -S, --lastsuccessfulauth <date> Last successful auth");
        console::e("  -L, --lastfailedauth <date>    Last failed auth");
        console::e("  -C, --loginfailedcount <count> Login failed count");
        console::e("  -a, --principalaliases <aliases> Principal aliases");
        console::e(
            "  -d, --allowedtodelegateto <services> Allowed to delegate to");
        console::e(
            "  -i, --principalauthind <indicators> Principal auth indicators");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli update-principal <principal> [options]");
      return false;
    }

    principal = argv[optind];

    auto optStr = [](const std::string &s) -> std::optional<std::string> {
      return s.empty() ? std::nullopt : std::optional<std::string>(s);
    };

    return updatePrincipal(
        principal, baseDN, optStr(canonicalName), optStr(principalType),
        optStr(principalExpiration), optStr(passwordExpiration),
        optStr(ticketFlags), optStr(maxTicketLife), optStr(maxRenewableAge),
        optStr(lastPwdChange), optStr(lastSuccessfulAuth),
        optStr(lastFailedAuth), optStr(loginFailedCount),
        optStr(principalAliases), optStr(allowedToDelegateTo),
        optStr(principalAuthInd));
  } else if (command == "list-principals") {
    return listPrincipals(baseDN);
  }

  printUsage();
  return false;
}

bool KerberosManager::createPrincipal(
    const std::string &principal, const std::string &baseDN,
    const std::optional<std::string> &password,
    const std::optional<std::string> &fullname,
    const std::optional<std::string> &email,
    const std::optional<std::string> &canonicalName,
    const std::optional<std::string> &principalType,
    const std::optional<std::string> &principalExpiration,
    const std::optional<std::string> &passwordExpiration,
    const std::optional<std::string> &ticketFlags,
    const std::optional<std::string> &maxTicketLife,
    const std::optional<std::string> &maxRenewableAge,
    const std::optional<std::string> &lastPwdChange,
    const std::optional<std::string> &lastSuccessfulAuth,
    const std::optional<std::string> &lastFailedAuth,
    const std::optional<std::string> &loginFailedCount,
    const std::optional<std::string> &principalAliases,
    const std::optional<std::string> &allowedToDelegateTo,
    const std::optional<std::string> &principalAuthInd) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  console::e("Creating Kerberos principal:");
  console::e("  Principal: {}", principal);
  console::e("  Principal DN: {}", principalDN);

  // Create LDAP mods for Kerberos principal
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(principal.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[3];
  objectClassMod.mod_vals.modv_strvals[0] = const_cast<char *>("krbPrincipal");
  objectClassMod.mod_vals.modv_strvals[1] =
      const_cast<char *>("krbPrincipalAux");
  objectClassMod.mod_vals.modv_strvals[2] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod principalNameMod;
  principalNameMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  principalNameMod.mod_type = const_cast<char *>("krbPrincipalName");
  principalNameMod.mod_vals.modv_strvals = new char *[2];
  principalNameMod.mod_vals.modv_strvals[0] =
      const_cast<char *>(principal.c_str());
  principalNameMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(principalNameMod);

  // Optional attributes
  if (password.has_value()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    passwordMod.mod_type = const_cast<char *>("krbPrincipalKey");
    passwordMod.mod_vals.modv_strvals = new char *[2];
    passwordMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(password->c_str());
    passwordMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (canonicalName.has_value()) {
    LDAPMod canonicalNameMod;
    canonicalNameMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    canonicalNameMod.mod_type = const_cast<char *>("krbCanonicalName");
    canonicalNameMod.mod_vals.modv_strvals = new char *[2];
    canonicalNameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(canonicalName->c_str());
    canonicalNameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(canonicalNameMod);
  } else if (fullname.has_value()) {
    LDAPMod fullnameMod;
    fullnameMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    fullnameMod.mod_type = const_cast<char *>("krbCanonicalName");
    fullnameMod.mod_vals.modv_strvals = new char *[2];
    fullnameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(fullname->c_str());
    fullnameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(fullnameMod);
  }
  if (principalType.has_value()) {
    LDAPMod principalTypeMod;
    principalTypeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    principalTypeMod.mod_type = const_cast<char *>("krbPrincipalType");
    principalTypeMod.mod_vals.modv_strvals = new char *[2];
    principalTypeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(principalType->c_str());
    principalTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(principalTypeMod);
  }
  if (principalExpiration.has_value()) {
    LDAPMod principalExpirationMod;
    principalExpirationMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    principalExpirationMod.mod_type =
        const_cast<char *>("krbPrincipalExpiration");
    principalExpirationMod.mod_vals.modv_strvals = new char *[2];
    principalExpirationMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(principalExpiration->c_str());
    principalExpirationMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(principalExpirationMod);
  }
  if (passwordExpiration.has_value()) {
    LDAPMod passwordExpirationMod;
    passwordExpirationMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    passwordExpirationMod.mod_type =
        const_cast<char *>("krbPasswordExpiration");
    passwordExpirationMod.mod_vals.modv_strvals = new char *[2];
    passwordExpirationMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(passwordExpiration->c_str());
    passwordExpirationMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordExpirationMod);
  }
  if (ticketFlags.has_value()) {
    LDAPMod ticketFlagsMod;
    ticketFlagsMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    ticketFlagsMod.mod_type = const_cast<char *>("krbTicketFlags");
    ticketFlagsMod.mod_vals.modv_strvals = new char *[2];
    ticketFlagsMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(ticketFlags->c_str());
    ticketFlagsMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(ticketFlagsMod);
  }
  if (maxTicketLife.has_value()) {
    LDAPMod maxTicketLifeMod;
    maxTicketLifeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    maxTicketLifeMod.mod_type = const_cast<char *>("krbMaxTicketLife");
    maxTicketLifeMod.mod_vals.modv_strvals = new char *[2];
    maxTicketLifeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(maxTicketLife->c_str());
    maxTicketLifeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(maxTicketLifeMod);
  }
  if (maxRenewableAge.has_value()) {
    LDAPMod maxRenewableAgeMod;
    maxRenewableAgeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    maxRenewableAgeMod.mod_type = const_cast<char *>("krbMaxRenewableAge");
    maxRenewableAgeMod.mod_vals.modv_strvals = new char *[2];
    maxRenewableAgeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(maxRenewableAge->c_str());
    maxRenewableAgeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(maxRenewableAgeMod);
  }
  if (lastPwdChange.has_value()) {
    LDAPMod lastPwdChangeMod;
    lastPwdChangeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    lastPwdChangeMod.mod_type = const_cast<char *>("krbLastPwdChange");
    lastPwdChangeMod.mod_vals.modv_strvals = new char *[2];
    lastPwdChangeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(lastPwdChange->c_str());
    lastPwdChangeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(lastPwdChangeMod);
  }
  if (lastSuccessfulAuth.has_value()) {
    LDAPMod lastSuccessfulAuthMod;
    lastSuccessfulAuthMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    lastSuccessfulAuthMod.mod_type =
        const_cast<char *>("krbLastSuccessfulAuth");
    lastSuccessfulAuthMod.mod_vals.modv_strvals = new char *[2];
    lastSuccessfulAuthMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(lastSuccessfulAuth->c_str());
    lastSuccessfulAuthMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(lastSuccessfulAuthMod);
  }
  if (lastFailedAuth.has_value()) {
    LDAPMod lastFailedAuthMod;
    lastFailedAuthMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    lastFailedAuthMod.mod_type = const_cast<char *>("krbLastFailedAuth");
    lastFailedAuthMod.mod_vals.modv_strvals = new char *[2];
    lastFailedAuthMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(lastFailedAuth->c_str());
    lastFailedAuthMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(lastFailedAuthMod);
  }
  if (loginFailedCount.has_value()) {
    LDAPMod loginFailedCountMod;
    loginFailedCountMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    loginFailedCountMod.mod_type = const_cast<char *>("krbLoginFailedCount");
    loginFailedCountMod.mod_vals.modv_strvals = new char *[2];
    loginFailedCountMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(loginFailedCount->c_str());
    loginFailedCountMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(loginFailedCountMod);
  }
  if (principalAliases.has_value()) {
    LDAPMod principalAliasesMod;
    principalAliasesMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    principalAliasesMod.mod_type = const_cast<char *>("krbPrincipalAliases");
    principalAliasesMod.mod_vals.modv_strvals = new char *[2];
    principalAliasesMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(principalAliases->c_str());
    principalAliasesMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(principalAliasesMod);
  }
  if (allowedToDelegateTo.has_value()) {
    LDAPMod allowedToDelegateToMod;
    allowedToDelegateToMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    allowedToDelegateToMod.mod_type =
        const_cast<char *>("krbAllowedToDelegateTo");
    allowedToDelegateToMod.mod_vals.modv_strvals = new char *[2];
    allowedToDelegateToMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(allowedToDelegateTo->c_str());
    allowedToDelegateToMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(allowedToDelegateToMod);
  }
  if (principalAuthInd.has_value()) {
    LDAPMod principalAuthIndMod;
    principalAuthIndMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    principalAuthIndMod.mod_type = const_cast<char *>("krbPrincipalAuthInd");
    principalAuthIndMod.mod_vals.modv_strvals = new char *[2];
    principalAuthIndMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(principalAuthInd->c_str());
    principalAuthIndMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(principalAuthIndMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(principalDN, modPtrs.data())) {
    throw std::runtime_error(m_connection.getError());
  }

  console::e("Principal created successfully!");
  return true;
}

bool KerberosManager::updatePrincipal(
    const std::string &principal, const std::string &baseDN,
    const std::optional<std::string> &canonicalName,
    const std::optional<std::string> &principalType,
    const std::optional<std::string> &principalExpiration,
    const std::optional<std::string> &passwordExpiration,
    const std::optional<std::string> &ticketFlags,
    const std::optional<std::string> &maxTicketLife,
    const std::optional<std::string> &maxRenewableAge,
    const std::optional<std::string> &lastPwdChange,
    const std::optional<std::string> &lastSuccessfulAuth,
    const std::optional<std::string> &lastFailedAuth,
    const std::optional<std::string> &loginFailedCount,
    const std::optional<std::string> &principalAliases,
    const std::optional<std::string> &allowedToDelegateTo,
    const std::optional<std::string> &principalAuthInd) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  console::e("Updating Kerberos principal:");
  console::e("  Principal: {}", principal);
  console::e("  Principal DN: {}", principalDN);

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

  addReplace("krbCanonicalName", canonicalName);
  addReplace("krbPrincipalType", principalType);
  addReplace("krbPrincipalExpiration", principalExpiration);
  addReplace("krbPasswordExpiration", passwordExpiration);
  addReplace("krbTicketFlags", ticketFlags);
  addReplace("krbMaxTicketLife", maxTicketLife);
  addReplace("krbMaxRenewableAge", maxRenewableAge);
  addReplace("krbLastPwdChange", lastPwdChange);
  addReplace("krbLastSuccessfulAuth", lastSuccessfulAuth);
  addReplace("krbLastFailedAuth", lastFailedAuth);
  addReplace("krbLoginFailedCount", loginFailedCount);
  addReplace("krbPrincipalAliases", principalAliases);
  addReplace("krbAllowedToDelegateTo", allowedToDelegateTo);
  addReplace("krbPrincipalAuthInd", principalAuthInd);

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(principalDN, modPtrs.data())) {
    throw std::runtime_error(m_connection.getError());
  }

  console::e("Principal updated successfully!");
  return true;
}

bool KerberosManager::deletePrincipal(const std::string &principal,
                                      const std::string &baseDN) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  console::e("Deleting Kerberos principal:");
  console::e("  Principal: {}", principal);
  console::e("  Principal DN: {}", principalDN);

  if (!m_connection.deleteEntry(principalDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Principal deleted successfully!");
  return true;
}

bool KerberosManager::listPrincipals(const std::string &baseDN) {
  console::e("Listing Kerberos principals:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=krbPrincipal)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No principals found.");
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

std::string KerberosManager::getPrincipalDN(const std::string &principal,
                                            const std::string &baseDN) const {
  return "cn=" + principal + "," + baseDN;
}
