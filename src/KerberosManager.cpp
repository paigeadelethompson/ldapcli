#include "KerberosManager.hpp"
#include "Config.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

KerberosManager::KerberosManager(LDAPConnection &connection)
    : m_connection(connection) {}

void KerberosManager::printUsage() const {
  std::cout << "Kerberos Commands:" << std::endl;
  std::cout << "  create-principal <principal> [base-dn]" << std::endl;
  std::cout << "  delete-principal <principal> [base-dn]" << std::endl;
  std::cout << "  update-principal <principal> [base-dn]" << std::endl;
  std::cout << "  list-principals [base-dn]" << std::endl;
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
        std::cerr << "Usage: ldapcli create-principal <principal> [options]"
                  << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -p, --password <secret>        Kerberos password"
                  << std::endl;
        std::cerr << "  -f, --fullname <name>          Full name" << std::endl;
        std::cerr << "  -e, --email <email>            Email address"
                  << std::endl;
        std::cerr << "  -c, --canonicalname <name>     Canonical principal name"
                  << std::endl;
        std::cerr << "  -t, --principaltype <type>     Principal type"
                  << std::endl;
        std::cerr << "  -x, --principalexpiration <date> Principal expiration"
                  << std::endl;
        std::cerr << "  -X, --passwordexpiration <date> Password expiration"
                  << std::endl;
        std::cerr << "  -F, --ticketflags <flags>      Ticket flags"
                  << std::endl;
        std::cerr << "  -l, --maxticketlife <seconds>  Max ticket lifetime"
                  << std::endl;
        std::cerr << "  -r, --maxrenewableage <seconds> Max renewable age"
                  << std::endl;
        std::cerr << "  -P, --lastpwdchange <date>     Last password change"
                  << std::endl;
        std::cerr << "  -S, --lastsuccessfulauth <date> Last successful auth"
                  << std::endl;
        std::cerr << "  -L, --lastfailedauth <date>    Last failed auth"
                  << std::endl;
        std::cerr << "  -C, --loginfailedcount <count> Login failed count"
                  << std::endl;
        std::cerr << "  -a, --principalaliases <aliases> Principal aliases"
                  << std::endl;
        std::cerr
            << "  -d, --allowedtodelegateto <services> Allowed to delegate to"
            << std::endl;
        std::cerr
            << "  -i, --principalauthind <indicators> Principal auth indicators"
            << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-principal <principal> [options]"
                << std::endl;
      return false;
    }

    principal = argv[optind];

    return createPrincipal(principal, baseDN, password, fullname, email,
                           canonicalName, principalType, principalExpiration,
                           passwordExpiration, ticketFlags, maxTicketLife,
                           maxRenewableAge, lastPwdChange, lastSuccessfulAuth,
                           lastFailedAuth, loginFailedCount, principalAliases,
                           allowedToDelegateTo, principalAuthInd);
  } else if (command == "delete-principal") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-principal <principal>" << std::endl;
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
        std::cerr << "Usage: ldapcli update-principal <principal> [options]"
                  << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -c, --canonicalname <name>     Canonical principal name"
                  << std::endl;
        std::cerr << "  -t, --principaltype <type>     Principal type"
                  << std::endl;
        std::cerr << "  -x, --principalexpiration <date> Principal expiration"
                  << std::endl;
        std::cerr << "  -X, --passwordexpiration <date> Password expiration"
                  << std::endl;
        std::cerr << "  -F, --ticketflags <flags>      Ticket flags"
                  << std::endl;
        std::cerr << "  -l, --maxticketlife <seconds>  Max ticket lifetime"
                  << std::endl;
        std::cerr << "  -r, --maxrenewableage <seconds> Max renewable age"
                  << std::endl;
        std::cerr << "  -P, --lastpwdchange <date>     Last password change"
                  << std::endl;
        std::cerr << "  -S, --lastsuccessfulauth <date> Last successful auth"
                  << std::endl;
        std::cerr << "  -L, --lastfailedauth <date>    Last failed auth"
                  << std::endl;
        std::cerr << "  -C, --loginfailedcount <count> Login failed count"
                  << std::endl;
        std::cerr << "  -a, --principalaliases <aliases> Principal aliases"
                  << std::endl;
        std::cerr
            << "  -d, --allowedtodelegateto <services> Allowed to delegate to"
            << std::endl;
        std::cerr
            << "  -i, --principalauthind <indicators> Principal auth indicators"
            << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-principal <principal> [options]"
                << std::endl;
      return false;
    }

    principal = argv[optind];

    return updatePrincipal(
        principal, baseDN, canonicalName, principalType, principalExpiration,
        passwordExpiration, ticketFlags, maxTicketLife, maxRenewableAge,
        lastPwdChange, lastSuccessfulAuth, lastFailedAuth, loginFailedCount,
        principalAliases, allowedToDelegateTo, principalAuthInd);
  } else if (command == "list-principals") {
    return listPrincipals(baseDN);
  }

  printUsage();
  return false;
}

bool KerberosManager::createPrincipal(
    const std::string &principal, const std::string &baseDN,
    const std::string &password, const std::string &fullname,
    const std::string &email, const std::string &canonicalName,
    const std::string &principalType, const std::string &principalExpiration,
    const std::string &passwordExpiration, const std::string &ticketFlags,
    const std::string &maxTicketLife, const std::string &maxRenewableAge,
    const std::string &lastPwdChange, const std::string &lastSuccessfulAuth,
    const std::string &lastFailedAuth, const std::string &loginFailedCount,
    const std::string &principalAliases, const std::string &allowedToDelegateTo,
    const std::string &principalAuthInd) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  std::cout << "Creating Kerberos principal:" << std::endl;
  std::cout << "  Principal: " << principal << std::endl;
  std::cout << "  Principal DN: " << principalDN << std::endl;

  // Create LDAP mods for Kerberos principal
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = principal.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(principal.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[3];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 12;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("krb5Principal");
  objectClassMod.mod_vals.modv_bvals[1] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[1]->bv_len = 15;
  objectClassMod.mod_vals.modv_bvals[1]->bv_val =
      const_cast<char *>("krbPrincipalAux");
  objectClassMod.mod_vals.modv_bvals[2] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[2]->bv_len = 11;
  objectClassMod.mod_vals.modv_bvals[2]->bv_val =
      const_cast<char *>("krbPrincipal");
  objectClassMod.mod_vals.modv_bvals[3] = nullptr;
  mods.push_back(objectClassMod);

  // Optional attributes
  if (!password.empty()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD;
    passwordMod.mod_type = const_cast<char *>("krbPrincipalKey");
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
    fullnameMod.mod_type = const_cast<char *>("krbPrincipalName");
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
    emailMod.mod_type = const_cast<char *>("mail");
    emailMod.mod_vals.modv_bvals = new struct berval *[2];
    emailMod.mod_vals.modv_bvals[0] = new struct berval;
    emailMod.mod_vals.modv_bvals[0]->bv_len = email.length();
    emailMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(email.c_str());
    emailMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(emailMod);
  }
  if (!canonicalName.empty()) {
    LDAPMod canonicalNameMod;
    canonicalNameMod.mod_op = LDAP_MOD_ADD;
    canonicalNameMod.mod_type = const_cast<char *>("krbCanonicalName");
    canonicalNameMod.mod_vals.modv_bvals = new struct berval *[2];
    canonicalNameMod.mod_vals.modv_bvals[0] = new struct berval;
    canonicalNameMod.mod_vals.modv_bvals[0]->bv_len = canonicalName.length();
    canonicalNameMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(canonicalName.c_str());
    canonicalNameMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(canonicalNameMod);
  }
  if (!principalType.empty()) {
    LDAPMod principalTypeMod;
    principalTypeMod.mod_op = LDAP_MOD_ADD;
    principalTypeMod.mod_type = const_cast<char *>("krbPrincipalType");
    principalTypeMod.mod_vals.modv_bvals = new struct berval *[2];
    principalTypeMod.mod_vals.modv_bvals[0] = new struct berval;
    principalTypeMod.mod_vals.modv_bvals[0]->bv_len = principalType.length();
    principalTypeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalType.c_str());
    principalTypeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalTypeMod);
  }
  if (!principalExpiration.empty()) {
    LDAPMod principalExpirationMod;
    principalExpirationMod.mod_op = LDAP_MOD_ADD;
    principalExpirationMod.mod_type =
        const_cast<char *>("krbPrincipalExpiration");
    principalExpirationMod.mod_vals.modv_bvals = new struct berval *[2];
    principalExpirationMod.mod_vals.modv_bvals[0] = new struct berval;
    principalExpirationMod.mod_vals.modv_bvals[0]->bv_len =
        principalExpiration.length();
    principalExpirationMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalExpiration.c_str());
    principalExpirationMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalExpirationMod);
  }
  if (!passwordExpiration.empty()) {
    LDAPMod passwordExpirationMod;
    passwordExpirationMod.mod_op = LDAP_MOD_ADD;
    passwordExpirationMod.mod_type =
        const_cast<char *>("krbPasswordExpiration");
    passwordExpirationMod.mod_vals.modv_bvals = new struct berval *[2];
    passwordExpirationMod.mod_vals.modv_bvals[0] = new struct berval;
    passwordExpirationMod.mod_vals.modv_bvals[0]->bv_len =
        passwordExpiration.length();
    passwordExpirationMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(passwordExpiration.c_str());
    passwordExpirationMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(passwordExpirationMod);
  }
  if (!ticketFlags.empty()) {
    LDAPMod ticketFlagsMod;
    ticketFlagsMod.mod_op = LDAP_MOD_ADD;
    ticketFlagsMod.mod_type = const_cast<char *>("krbTicketFlags");
    ticketFlagsMod.mod_vals.modv_bvals = new struct berval *[2];
    ticketFlagsMod.mod_vals.modv_bvals[0] = new struct berval;
    ticketFlagsMod.mod_vals.modv_bvals[0]->bv_len = ticketFlags.length();
    ticketFlagsMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(ticketFlags.c_str());
    ticketFlagsMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(ticketFlagsMod);
  }
  if (!maxTicketLife.empty()) {
    LDAPMod maxTicketLifeMod;
    maxTicketLifeMod.mod_op = LDAP_MOD_ADD;
    maxTicketLifeMod.mod_type = const_cast<char *>("krbMaxTicketLife");
    maxTicketLifeMod.mod_vals.modv_bvals = new struct berval *[2];
    maxTicketLifeMod.mod_vals.modv_bvals[0] = new struct berval;
    maxTicketLifeMod.mod_vals.modv_bvals[0]->bv_len = maxTicketLife.length();
    maxTicketLifeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(maxTicketLife.c_str());
    maxTicketLifeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(maxTicketLifeMod);
  }
  if (!maxRenewableAge.empty()) {
    LDAPMod maxRenewableAgeMod;
    maxRenewableAgeMod.mod_op = LDAP_MOD_ADD;
    maxRenewableAgeMod.mod_type = const_cast<char *>("krbMaxRenewableAge");
    maxRenewableAgeMod.mod_vals.modv_bvals = new struct berval *[2];
    maxRenewableAgeMod.mod_vals.modv_bvals[0] = new struct berval;
    maxRenewableAgeMod.mod_vals.modv_bvals[0]->bv_len =
        maxRenewableAge.length();
    maxRenewableAgeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(maxRenewableAge.c_str());
    maxRenewableAgeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(maxRenewableAgeMod);
  }
  if (!lastPwdChange.empty()) {
    LDAPMod lastPwdChangeMod;
    lastPwdChangeMod.mod_op = LDAP_MOD_ADD;
    lastPwdChangeMod.mod_type = const_cast<char *>("krbLastPwdChange");
    lastPwdChangeMod.mod_vals.modv_bvals = new struct berval *[2];
    lastPwdChangeMod.mod_vals.modv_bvals[0] = new struct berval;
    lastPwdChangeMod.mod_vals.modv_bvals[0]->bv_len = lastPwdChange.length();
    lastPwdChangeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastPwdChange.c_str());
    lastPwdChangeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastPwdChangeMod);
  }
  if (!lastSuccessfulAuth.empty()) {
    LDAPMod lastSuccessfulAuthMod;
    lastSuccessfulAuthMod.mod_op = LDAP_MOD_ADD;
    lastSuccessfulAuthMod.mod_type =
        const_cast<char *>("krbLastSuccessfulAuth");
    lastSuccessfulAuthMod.mod_vals.modv_bvals = new struct berval *[2];
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0] = new struct berval;
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0]->bv_len =
        lastSuccessfulAuth.length();
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastSuccessfulAuth.c_str());
    lastSuccessfulAuthMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastSuccessfulAuthMod);
  }
  if (!lastFailedAuth.empty()) {
    LDAPMod lastFailedAuthMod;
    lastFailedAuthMod.mod_op = LDAP_MOD_ADD;
    lastFailedAuthMod.mod_type = const_cast<char *>("krbLastFailedAuth");
    lastFailedAuthMod.mod_vals.modv_bvals = new struct berval *[2];
    lastFailedAuthMod.mod_vals.modv_bvals[0] = new struct berval;
    lastFailedAuthMod.mod_vals.modv_bvals[0]->bv_len = lastFailedAuth.length();
    lastFailedAuthMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastFailedAuth.c_str());
    lastFailedAuthMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastFailedAuthMod);
  }
  if (!loginFailedCount.empty()) {
    LDAPMod loginFailedCountMod;
    loginFailedCountMod.mod_op = LDAP_MOD_ADD;
    loginFailedCountMod.mod_type = const_cast<char *>("krbLoginFailedCount");
    loginFailedCountMod.mod_vals.modv_bvals = new struct berval *[2];
    loginFailedCountMod.mod_vals.modv_bvals[0] = new struct berval;
    loginFailedCountMod.mod_vals.modv_bvals[0]->bv_len =
        loginFailedCount.length();
    loginFailedCountMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(loginFailedCount.c_str());
    loginFailedCountMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(loginFailedCountMod);
  }
  if (!principalAliases.empty()) {
    LDAPMod principalAliasesMod;
    principalAliasesMod.mod_op = LDAP_MOD_ADD;
    principalAliasesMod.mod_type = const_cast<char *>("krbPrincipalAliases");
    principalAliasesMod.mod_vals.modv_bvals = new struct berval *[2];
    principalAliasesMod.mod_vals.modv_bvals[0] = new struct berval;
    principalAliasesMod.mod_vals.modv_bvals[0]->bv_len =
        principalAliases.length();
    principalAliasesMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalAliases.c_str());
    principalAliasesMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalAliasesMod);
  }
  if (!allowedToDelegateTo.empty()) {
    LDAPMod allowedToDelegateToMod;
    allowedToDelegateToMod.mod_op = LDAP_MOD_ADD;
    allowedToDelegateToMod.mod_type =
        const_cast<char *>("krbAllowedToDelegateTo");
    allowedToDelegateToMod.mod_vals.modv_bvals = new struct berval *[2];
    allowedToDelegateToMod.mod_vals.modv_bvals[0] = new struct berval;
    allowedToDelegateToMod.mod_vals.modv_bvals[0]->bv_len =
        allowedToDelegateTo.length();
    allowedToDelegateToMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(allowedToDelegateTo.c_str());
    allowedToDelegateToMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(allowedToDelegateToMod);
  }
  if (!principalAuthInd.empty()) {
    LDAPMod principalAuthIndMod;
    principalAuthIndMod.mod_op = LDAP_MOD_ADD;
    principalAuthIndMod.mod_type = const_cast<char *>("krbPrincipalAuthInd");
    principalAuthIndMod.mod_vals.modv_bvals = new struct berval *[2];
    principalAuthIndMod.mod_vals.modv_bvals[0] = new struct berval;
    principalAuthIndMod.mod_vals.modv_bvals[0]->bv_len =
        principalAuthInd.length();
    principalAuthIndMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalAuthInd.c_str());
    principalAuthIndMod.mod_vals.modv_bvals[1] = nullptr;
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

  std::cout << "Principal created successfully!" << std::endl;
  return true;
}

bool KerberosManager::updatePrincipal(
    const std::string &principal, const std::string &baseDN,
    const std::string &canonicalName, const std::string &principalType,
    const std::string &principalExpiration,
    const std::string &passwordExpiration, const std::string &ticketFlags,
    const std::string &maxTicketLife, const std::string &maxRenewableAge,
    const std::string &lastPwdChange, const std::string &lastSuccessfulAuth,
    const std::string &lastFailedAuth, const std::string &loginFailedCount,
    const std::string &principalAliases, const std::string &allowedToDelegateTo,
    const std::string &principalAuthInd) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  std::cout << "Updating Kerberos principal:" << std::endl;
  std::cout << "  Principal: " << principal << std::endl;
  std::cout << "  Principal DN: " << principalDN << std::endl;

  std::vector<LDAPMod> mods;

  // Update optional attributes
  if (!canonicalName.empty()) {
    LDAPMod canonicalNameMod;
    canonicalNameMod.mod_op = LDAP_MOD_REPLACE;
    canonicalNameMod.mod_type = const_cast<char *>("krbCanonicalName");
    canonicalNameMod.mod_vals.modv_bvals = new struct berval *[2];
    canonicalNameMod.mod_vals.modv_bvals[0] = new struct berval;
    canonicalNameMod.mod_vals.modv_bvals[0]->bv_len = canonicalName.length();
    canonicalNameMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(canonicalName.c_str());
    canonicalNameMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(canonicalNameMod);
  }

  if (!principalType.empty()) {
    LDAPMod principalTypeMod;
    principalTypeMod.mod_op = LDAP_MOD_REPLACE;
    principalTypeMod.mod_type = const_cast<char *>("krbPrincipalType");
    principalTypeMod.mod_vals.modv_bvals = new struct berval *[2];
    principalTypeMod.mod_vals.modv_bvals[0] = new struct berval;
    principalTypeMod.mod_vals.modv_bvals[0]->bv_len = principalType.length();
    principalTypeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalType.c_str());
    principalTypeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalTypeMod);
  }

  if (!principalExpiration.empty()) {
    LDAPMod principalExpirationMod;
    principalExpirationMod.mod_op = LDAP_MOD_REPLACE;
    principalExpirationMod.mod_type =
        const_cast<char *>("krbPrincipalExpiration");
    principalExpirationMod.mod_vals.modv_bvals = new struct berval *[2];
    principalExpirationMod.mod_vals.modv_bvals[0] = new struct berval;
    principalExpirationMod.mod_vals.modv_bvals[0]->bv_len =
        principalExpiration.length();
    principalExpirationMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalExpiration.c_str());
    principalExpirationMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalExpirationMod);
  }

  if (!passwordExpiration.empty()) {
    LDAPMod passwordExpirationMod;
    passwordExpirationMod.mod_op = LDAP_MOD_REPLACE;
    passwordExpirationMod.mod_type =
        const_cast<char *>("krbPasswordExpiration");
    passwordExpirationMod.mod_vals.modv_bvals = new struct berval *[2];
    passwordExpirationMod.mod_vals.modv_bvals[0] = new struct berval;
    passwordExpirationMod.mod_vals.modv_bvals[0]->bv_len =
        passwordExpiration.length();
    passwordExpirationMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(passwordExpiration.c_str());
    passwordExpirationMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(passwordExpirationMod);
  }

  if (!ticketFlags.empty()) {
    LDAPMod ticketFlagsMod;
    ticketFlagsMod.mod_op = LDAP_MOD_REPLACE;
    ticketFlagsMod.mod_type = const_cast<char *>("krbTicketFlags");
    ticketFlagsMod.mod_vals.modv_bvals = new struct berval *[2];
    ticketFlagsMod.mod_vals.modv_bvals[0] = new struct berval;
    ticketFlagsMod.mod_vals.modv_bvals[0]->bv_len = ticketFlags.length();
    ticketFlagsMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(ticketFlags.c_str());
    ticketFlagsMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(ticketFlagsMod);
  }

  if (!maxTicketLife.empty()) {
    LDAPMod maxTicketLifeMod;
    maxTicketLifeMod.mod_op = LDAP_MOD_REPLACE;
    maxTicketLifeMod.mod_type = const_cast<char *>("krbMaxTicketLife");
    maxTicketLifeMod.mod_vals.modv_bvals = new struct berval *[2];
    maxTicketLifeMod.mod_vals.modv_bvals[0] = new struct berval;
    maxTicketLifeMod.mod_vals.modv_bvals[0]->bv_len = maxTicketLife.length();
    maxTicketLifeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(maxTicketLife.c_str());
    maxTicketLifeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(maxTicketLifeMod);
  }

  if (!maxRenewableAge.empty()) {
    LDAPMod maxRenewableAgeMod;
    maxRenewableAgeMod.mod_op = LDAP_MOD_REPLACE;
    maxRenewableAgeMod.mod_type = const_cast<char *>("krbMaxRenewableAge");
    maxRenewableAgeMod.mod_vals.modv_bvals = new struct berval *[2];
    maxRenewableAgeMod.mod_vals.modv_bvals[0] = new struct berval;
    maxRenewableAgeMod.mod_vals.modv_bvals[0]->bv_len =
        maxRenewableAge.length();
    maxRenewableAgeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(maxRenewableAge.c_str());
    maxRenewableAgeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(maxRenewableAgeMod);
  }

  if (!lastPwdChange.empty()) {
    LDAPMod lastPwdChangeMod;
    lastPwdChangeMod.mod_op = LDAP_MOD_REPLACE;
    lastPwdChangeMod.mod_type = const_cast<char *>("krbLastPwdChange");
    lastPwdChangeMod.mod_vals.modv_bvals = new struct berval *[2];
    lastPwdChangeMod.mod_vals.modv_bvals[0] = new struct berval;
    lastPwdChangeMod.mod_vals.modv_bvals[0]->bv_len = lastPwdChange.length();
    lastPwdChangeMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastPwdChange.c_str());
    lastPwdChangeMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastPwdChangeMod);
  }

  if (!lastSuccessfulAuth.empty()) {
    LDAPMod lastSuccessfulAuthMod;
    lastSuccessfulAuthMod.mod_op = LDAP_MOD_REPLACE;
    lastSuccessfulAuthMod.mod_type =
        const_cast<char *>("krbLastSuccessfulAuth");
    lastSuccessfulAuthMod.mod_vals.modv_bvals = new struct berval *[2];
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0] = new struct berval;
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0]->bv_len =
        lastSuccessfulAuth.length();
    lastSuccessfulAuthMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastSuccessfulAuth.c_str());
    lastSuccessfulAuthMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastSuccessfulAuthMod);
  }

  if (!lastFailedAuth.empty()) {
    LDAPMod lastFailedAuthMod;
    lastFailedAuthMod.mod_op = LDAP_MOD_REPLACE;
    lastFailedAuthMod.mod_type = const_cast<char *>("krbLastFailedAuth");
    lastFailedAuthMod.mod_vals.modv_bvals = new struct berval *[2];
    lastFailedAuthMod.mod_vals.modv_bvals[0] = new struct berval;
    lastFailedAuthMod.mod_vals.modv_bvals[0]->bv_len = lastFailedAuth.length();
    lastFailedAuthMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(lastFailedAuth.c_str());
    lastFailedAuthMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(lastFailedAuthMod);
  }

  if (!loginFailedCount.empty()) {
    LDAPMod loginFailedCountMod;
    loginFailedCountMod.mod_op = LDAP_MOD_REPLACE;
    loginFailedCountMod.mod_type = const_cast<char *>("krbLoginFailedCount");
    loginFailedCountMod.mod_vals.modv_bvals = new struct berval *[2];
    loginFailedCountMod.mod_vals.modv_bvals[0] = new struct berval;
    loginFailedCountMod.mod_vals.modv_bvals[0]->bv_len =
        loginFailedCount.length();
    loginFailedCountMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(loginFailedCount.c_str());
    loginFailedCountMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(loginFailedCountMod);
  }

  if (!principalAliases.empty()) {
    LDAPMod principalAliasesMod;
    principalAliasesMod.mod_op = LDAP_MOD_REPLACE;
    principalAliasesMod.mod_type = const_cast<char *>("krbPrincipalAliases");
    principalAliasesMod.mod_vals.modv_bvals = new struct berval *[2];
    principalAliasesMod.mod_vals.modv_bvals[0] = new struct berval;
    principalAliasesMod.mod_vals.modv_bvals[0]->bv_len =
        principalAliases.length();
    principalAliasesMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalAliases.c_str());
    principalAliasesMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalAliasesMod);
  }

  if (!allowedToDelegateTo.empty()) {
    LDAPMod allowedToDelegateToMod;
    allowedToDelegateToMod.mod_op = LDAP_MOD_REPLACE;
    allowedToDelegateToMod.mod_type =
        const_cast<char *>("krbAllowedToDelegateTo");
    allowedToDelegateToMod.mod_vals.modv_bvals = new struct berval *[2];
    allowedToDelegateToMod.mod_vals.modv_bvals[0] = new struct berval;
    allowedToDelegateToMod.mod_vals.modv_bvals[0]->bv_len =
        allowedToDelegateTo.length();
    allowedToDelegateToMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(allowedToDelegateTo.c_str());
    allowedToDelegateToMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(allowedToDelegateToMod);
  }

  if (!principalAuthInd.empty()) {
    LDAPMod principalAuthIndMod;
    principalAuthIndMod.mod_op = LDAP_MOD_REPLACE;
    principalAuthIndMod.mod_type = const_cast<char *>("krbPrincipalAuthInd");
    principalAuthIndMod.mod_vals.modv_bvals = new struct berval *[2];
    principalAuthIndMod.mod_vals.modv_bvals[0] = new struct berval;
    principalAuthIndMod.mod_vals.modv_bvals[0]->bv_len =
        principalAuthInd.length();
    principalAuthIndMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(principalAuthInd.c_str());
    principalAuthIndMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(principalAuthIndMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(principalDN, modPtrs.data())) {
    throw std::runtime_error(m_connection.getError());
  }

  std::cout << "Principal updated successfully!" << std::endl;
  return true;
}

bool KerberosManager::deletePrincipal(const std::string &principal,
                                      const std::string &baseDN) {
  std::string principalDN = getPrincipalDN(principal, baseDN);

  std::cout << "Deleting Kerberos principal:" << std::endl;
  std::cout << "  Principal: " << principal << std::endl;
  std::cout << "  Principal DN: " << principalDN << std::endl;

  if (!m_connection.deleteEntry(principalDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Principal deleted successfully!" << std::endl;
  return true;
}

bool KerberosManager::listPrincipals(const std::string &baseDN) {
  std::cout << "Listing Kerberos principals:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=krb5Principal)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " principals:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\\nPrincipal " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

std::string KerberosManager::getPrincipalDN(const std::string &principal,
                                            const std::string &baseDN) const {
  return "cn=" + principal + "," + baseDN;
}
