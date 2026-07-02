#include "OpenLDAPManager.hpp"
#include "Config.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

OpenLDAPManager::OpenLDAPManager(LDAPConnection &connection)
    : m_connection(connection) {}

void OpenLDAPManager::printUsage() const {
  std::cout << "OPENLDAP Commands:" << std::endl;
  std::cout << "  create-ou <ou-name> [base-dn]" << std::endl;
  std::cout << "  delete-ou <ou-name> [base-dn]" << std::endl;
  std::cout << "  update-ou <ou-name> [base-dn]" << std::endl;
  std::cout << "  list-ous [base-dn]" << std::endl;
  std::cout << "  list-people [base-dn]" << std::endl;
  std::cout << "  create-person <cn> [base-dn]" << std::endl;
  std::cout << "  update-person <cn> [base-dn]" << std::endl;
  std::cout << "  delete-person <cn> [base-dn]" << std::endl;
}

std::string OpenLDAPManager::getServiceName() const { return "openldap"; }

bool OpenLDAPManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-ou") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"fullname", required_argument, 0, 'f'},
        {"email", required_argument, 0, 'e'},
        {0, 0, 0, 0}};

    std::string ouName;
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
        std::cerr << "Usage: ldapcli create-ou <ou-name> [-p password] "
                     "[-f fullname] [-e email]"
                  << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-ou <ou-name> [-p password] "
                   "[-f fullname] [-e email]"
                << std::endl;
      return false;
    }

    ouName = argv[optind];

    return createOrganizationalUnit(ouName, baseDN);
  } else if (command == "delete-ou") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-ou <ou-name>" << std::endl;
      return false;
    }

    std::string ouName = argv[optind];

    return deleteOrganizationalUnit(ouName, baseDN);
  } else if (command == "update-ou") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-ou <ou-name>" << std::endl;
      return false;
    }

    std::string ouName = argv[optind];

    return false;
  } else if (command == "list-ous") {
    return listOrganizationalUnits(baseDN);
  } else if (command == "list-people") {
    return listPeople(baseDN);
  } else if (command == "create-person") {
    static struct option long_options[] = {
        {"uid", required_argument, 0, 'u'},
        {"givenname", required_argument, 0, 'g'},
        {"sn", required_argument, 0, 's'},
        {"mail", required_argument, 0, 'm'},
        {"displayname", required_argument, 0, 'd'},
        {"employeenumber", required_argument, 0, 'e'},
        {"employeetype", required_argument, 0, 't'},
        {"departmentnumber", required_argument, 0, 'n'},
        {"mobile", required_argument, 0, 'M'},
        {"homephone", required_argument, 0, 'H'},
        {"pager", required_argument, 0, 'P'},
        {"title", required_argument, 0, 'i'},
        {"telephonenumber", required_argument, 0, 'l'},
        {"street", required_argument, 0, 'r'},
        {"postalcode", required_argument, 0, 'z'},
        {"l", required_argument, 0, 'L'},
        {"st", required_argument, 0, 'S'},
        {"c", required_argument, 0, 'C'},
        {0, 0, 0, 0}};

    std::string personName;
    std::string uid, givenName, sn, mail, displayName;
    std::string employeeNumber, employeeType, departmentNumber;
    std::string mobile, homePhone, pager, title, telephoneNumber;
    std::string street, postalCode, l, st, c;

    int opt;
    int option_index = 0;

    while (
        (opt = getopt_long(argc, argv, "u:g:s:m:d:e:t:n:M:H:P:i:l:r:z:L:S:C:",
                           long_options, &option_index)) != -1) {
      switch (opt) {
      case 'u':
        uid = optarg;
        break;
      case 'g':
        givenName = optarg;
        break;
      case 's':
        sn = optarg;
        break;
      case 'm':
        mail = optarg;
        break;
      case 'd':
        displayName = optarg;
        break;
      case 'e':
        employeeNumber = optarg;
        break;
      case 't':
        employeeType = optarg;
        break;
      case 'n':
        departmentNumber = optarg;
        break;
      case 'M':
        mobile = optarg;
        break;
      case 'H':
        homePhone = optarg;
        break;
      case 'P':
        pager = optarg;
        break;
      case 'i':
        title = optarg;
        break;
      case 'l':
        telephoneNumber = optarg;
        break;
      case 'r':
        street = optarg;
        break;
      case 'z':
        postalCode = optarg;
        break;
      case 'L':
        l = optarg;
        break;
      case 'S':
        st = optarg;
        break;
      case 'C':
        c = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli create-person <cn> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -u, --uid <uid>           User ID" << std::endl;
        std::cerr << "  -g, --givenname <name>   Given name" << std::endl;
        std::cerr << "  -s, --sn <surname>       Surname" << std::endl;
        std::cerr << "  -m, --mail <email>       Email address" << std::endl;
        std::cerr << "  -d, --displayname <name> Display name" << std::endl;
        std::cerr << "  -e, --employeenumber <num> Employee number"
                  << std::endl;
        std::cerr << "  -t, --employeetype <type> Employee type" << std::endl;
        std::cerr << "  -n, --departmentnumber <num> Department number"
                  << std::endl;
        std::cerr << "  -M, --mobile <phone>     Mobile phone" << std::endl;
        std::cerr << "  -H, --homephone <phone>  Home phone" << std::endl;
        std::cerr << "  -P, --pager <phone>      Pager" << std::endl;
        std::cerr << "  -i, --title <title>      Job title" << std::endl;
        std::cerr << "  -l, --telephonenumber <phone> Telephone number"
                  << std::endl;
        std::cerr << "  -r, --street <address>   Street address" << std::endl;
        std::cerr << "  -z, --postalcode <code>  Postal code" << std::endl;
        std::cerr << "  -L, --l <city>           City" << std::endl;
        std::cerr << "  -S, --st <state>         State" << std::endl;
        std::cerr << "  -C, --c <country>        Country" << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli create-person <cn> [options]" << std::endl;
      return false;
    }

    personName = argv[optind];

    return createPerson(personName, baseDN, uid, givenName, sn, mail,
                        displayName, employeeNumber, employeeType,
                        departmentNumber, mobile, homePhone, pager, title,
                        telephoneNumber, street, postalCode, l, st, c);
  } else if (command == "update-person") {
    static struct option long_options[] = {
        {"uid", required_argument, 0, 'u'},
        {"givenname", required_argument, 0, 'g'},
        {"sn", required_argument, 0, 's'},
        {"mail", required_argument, 0, 'm'},
        {"displayname", required_argument, 0, 'd'},
        {"employeenumber", required_argument, 0, 'e'},
        {"employeetype", required_argument, 0, 't'},
        {"departmentnumber", required_argument, 0, 'n'},
        {"mobile", required_argument, 0, 'M'},
        {"homephone", required_argument, 0, 'H'},
        {"pager", required_argument, 0, 'P'},
        {"title", required_argument, 0, 'i'},
        {"telephonenumber", required_argument, 0, 'l'},
        {"street", required_argument, 0, 'r'},
        {"postalcode", required_argument, 0, 'z'},
        {"l", required_argument, 0, 'L'},
        {"st", required_argument, 0, 'S'},
        {"c", required_argument, 0, 'C'},
        {0, 0, 0, 0}};

    std::string personName;
    std::string uid, givenName, sn, mail, displayName;
    std::string employeeNumber, employeeType, departmentNumber;
    std::string mobile, homePhone, pager, title, telephoneNumber;
    std::string street, postalCode, l, st, c;

    int opt;
    int option_index = 0;

    while (
        (opt = getopt_long(argc, argv, "u:g:s:m:d:e:t:n:M:H:P:i:l:r:z:L:S:C:",
                           long_options, &option_index)) != -1) {
      switch (opt) {
      case 'u':
        uid = optarg;
        break;
      case 'g':
        givenName = optarg;
        break;
      case 's':
        sn = optarg;
        break;
      case 'm':
        mail = optarg;
        break;
      case 'd':
        displayName = optarg;
        break;
      case 'e':
        employeeNumber = optarg;
        break;
      case 't':
        employeeType = optarg;
        break;
      case 'n':
        departmentNumber = optarg;
        break;
      case 'M':
        mobile = optarg;
        break;
      case 'H':
        homePhone = optarg;
        break;
      case 'P':
        pager = optarg;
        break;
      case 'i':
        title = optarg;
        break;
      case 'l':
        telephoneNumber = optarg;
        break;
      case 'r':
        street = optarg;
        break;
      case 'z':
        postalCode = optarg;
        break;
      case 'L':
        l = optarg;
        break;
      case 'S':
        st = optarg;
        break;
      case 'C':
        c = optarg;
        break;
      default:
        std::cerr << "Usage: ldapcli update-person <cn> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -u, --uid <uid>           User ID" << std::endl;
        std::cerr << "  -g, --givenname <name>   Given name" << std::endl;
        std::cerr << "  -s, --sn <surname>       Surname" << std::endl;
        std::cerr << "  -m, --mail <email>       Email address" << std::endl;
        std::cerr << "  -d, --displayname <name> Display name" << std::endl;
        std::cerr << "  -e, --employeenumber <num> Employee number"
                  << std::endl;
        std::cerr << "  -t, --employeetype <type> Employee type" << std::endl;
        std::cerr << "  -n, --departmentnumber <num> Department number"
                  << std::endl;
        std::cerr << "  -M, --mobile <phone>     Mobile phone" << std::endl;
        std::cerr << "  -H, --homephone <phone>  Home phone" << std::endl;
        std::cerr << "  -P, --pager <phone>      Pager" << std::endl;
        std::cerr << "  -i, --title <title>      Job title" << std::endl;
        std::cerr << "  -l, --telephonenumber <phone> Telephone number"
                  << std::endl;
        std::cerr << "  -r, --street <address>   Street address" << std::endl;
        std::cerr << "  -z, --postalcode <code>  Postal code" << std::endl;
        std::cerr << "  -L, --l <city>           City" << std::endl;
        std::cerr << "  -S, --st <state>         State" << std::endl;
        std::cerr << "  -C, --c <country>        Country" << std::endl;
        return false;
      }
    }

    if (optind >= argc) {
      std::cerr << "Usage: ldapcli update-person <cn> [options]" << std::endl;
      return false;
    }

    personName = argv[optind];

    return updatePerson(personName, baseDN, uid, givenName, sn, mail,
                        displayName, employeeNumber, employeeType,
                        departmentNumber, mobile, homePhone, pager, title,
                        telephoneNumber, street, postalCode, l, st, c);
  } else if (command == "delete-person") {
    if (optind >= argc) {
      std::cerr << "Usage: ldapcli delete-person <cn>" << std::endl;
      return false;
    }

    std::string personName = argv[optind];

    return deletePerson(personName, baseDN);
  }

  printUsage();
  return false;
}

bool OpenLDAPManager::listPeople(const std::string &baseDN) {
  std::cout << "Listing People:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=inetOrgPerson)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size() << " People:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\\nPerson " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}

bool OpenLDAPManager::createPerson(
    const std::string &personName, const std::string &baseDN,
    const std::string &uid, const std::string &givenName, const std::string &sn,
    const std::string &mail, const std::string &displayName,
    const std::string &employeeNumber, const std::string &employeeType,
    const std::string &departmentNumber, const std::string &mobile,
    const std::string &homePhone, const std::string &pager,
    const std::string &title, const std::string &telephoneNumber,
    const std::string &street, const std::string &postalCode,
    const std::string &l, const std::string &st, const std::string &c) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  std::cout << "Creating Person:" << std::endl;
  std::cout << "  CN: " << personName << std::endl;
  std::cout << "  DN: " << personDN << std::endl;

  std::vector<LDAPMod> mods;

  // Required attributes for inetOrgPerson
  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[5];
  objectClassMod.mod_vals.modv_strvals[0] = const_cast<char *>("top");
  objectClassMod.mod_vals.modv_strvals[1] = const_cast<char *>("person");
  objectClassMod.mod_vals.modv_strvals[2] =
      const_cast<char *>("organizationalPerson");
  objectClassMod.mod_vals.modv_strvals[3] = const_cast<char *>("inetOrgPerson");
  objectClassMod.mod_vals.modv_strvals[4] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(personName.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod uidMod;
  uidMod.mod_op = LDAP_MOD_ADD;
  uidMod.mod_type = const_cast<char *>("uid");
  uidMod.mod_vals.modv_strvals = new char *[2];
  uidMod.mod_vals.modv_strvals[0] = const_cast<char *>(uid.c_str());
  uidMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(uidMod);

  // Optional inetOrgPerson attributes
  if (!givenName.empty()) {
    LDAPMod givenNameMod;
    givenNameMod.mod_op = LDAP_MOD_ADD;
    givenNameMod.mod_type = const_cast<char *>("givenName");
    givenNameMod.mod_vals.modv_strvals = new char *[2];
    givenNameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(givenName.c_str());
    givenNameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(givenNameMod);
  }

  if (!sn.empty()) {
    LDAPMod snMod;
    snMod.mod_op = LDAP_MOD_ADD;
    snMod.mod_type = const_cast<char *>("sn");
    snMod.mod_vals.modv_strvals = new char *[2];
    snMod.mod_vals.modv_strvals[0] = const_cast<char *>(sn.c_str());
    snMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(snMod);
  }

  if (!mail.empty()) {
    LDAPMod mailMod;
    mailMod.mod_op = LDAP_MOD_ADD;
    mailMod.mod_type = const_cast<char *>("mail");
    mailMod.mod_vals.modv_strvals = new char *[2];
    mailMod.mod_vals.modv_strvals[0] = const_cast<char *>(mail.c_str());
    mailMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mailMod);
  }

  if (!displayName.empty()) {
    LDAPMod displayNameMod;
    displayNameMod.mod_op = LDAP_MOD_ADD;
    displayNameMod.mod_type = const_cast<char *>("displayName");
    displayNameMod.mod_vals.modv_strvals = new char *[2];
    displayNameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(displayName.c_str());
    displayNameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(displayNameMod);
  }

  if (!employeeNumber.empty()) {
    LDAPMod employeeNumberMod;
    employeeNumberMod.mod_op = LDAP_MOD_ADD;
    employeeNumberMod.mod_type = const_cast<char *>("employeeNumber");
    employeeNumberMod.mod_vals.modv_strvals = new char *[2];
    employeeNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(employeeNumber.c_str());
    employeeNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(employeeNumberMod);
  }

  if (!employeeType.empty()) {
    LDAPMod employeeTypeMod;
    employeeTypeMod.mod_op = LDAP_MOD_ADD;
    employeeTypeMod.mod_type = const_cast<char *>("employeeType");
    employeeTypeMod.mod_vals.modv_strvals = new char *[2];
    employeeTypeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(employeeType.c_str());
    employeeTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(employeeTypeMod);
  }

  if (!departmentNumber.empty()) {
    LDAPMod departmentNumberMod;
    departmentNumberMod.mod_op = LDAP_MOD_ADD;
    departmentNumberMod.mod_type = const_cast<char *>("departmentNumber");
    departmentNumberMod.mod_vals.modv_strvals = new char *[2];
    departmentNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(departmentNumber.c_str());
    departmentNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(departmentNumberMod);
  }

  if (!mobile.empty()) {
    LDAPMod mobileMod;
    mobileMod.mod_op = LDAP_MOD_ADD;
    mobileMod.mod_type = const_cast<char *>("mobile");
    mobileMod.mod_vals.modv_strvals = new char *[2];
    mobileMod.mod_vals.modv_strvals[0] = const_cast<char *>(mobile.c_str());
    mobileMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mobileMod);
  }

  if (!homePhone.empty()) {
    LDAPMod homePhoneMod;
    homePhoneMod.mod_op = LDAP_MOD_ADD;
    homePhoneMod.mod_type = const_cast<char *>("homePhone");
    homePhoneMod.mod_vals.modv_strvals = new char *[2];
    homePhoneMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(homePhone.c_str());
    homePhoneMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(homePhoneMod);
  }

  if (!pager.empty()) {
    LDAPMod pagerMod;
    pagerMod.mod_op = LDAP_MOD_ADD;
    pagerMod.mod_type = const_cast<char *>("pager");
    pagerMod.mod_vals.modv_strvals = new char *[2];
    pagerMod.mod_vals.modv_strvals[0] = const_cast<char *>(pager.c_str());
    pagerMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(pagerMod);
  }

  if (!title.empty()) {
    LDAPMod titleMod;
    titleMod.mod_op = LDAP_MOD_ADD;
    titleMod.mod_type = const_cast<char *>("title");
    titleMod.mod_vals.modv_strvals = new char *[2];
    titleMod.mod_vals.modv_strvals[0] = const_cast<char *>(title.c_str());
    titleMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(titleMod);
  }

  if (!telephoneNumber.empty()) {
    LDAPMod telephoneNumberMod;
    telephoneNumberMod.mod_op = LDAP_MOD_ADD;
    telephoneNumberMod.mod_type = const_cast<char *>("telephoneNumber");
    telephoneNumberMod.mod_vals.modv_strvals = new char *[2];
    telephoneNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(telephoneNumber.c_str());
    telephoneNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(telephoneNumberMod);
  }

  if (!street.empty()) {
    LDAPMod streetMod;
    streetMod.mod_op = LDAP_MOD_ADD;
    streetMod.mod_type = const_cast<char *>("street");
    streetMod.mod_vals.modv_strvals = new char *[2];
    streetMod.mod_vals.modv_strvals[0] = const_cast<char *>(street.c_str());
    streetMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(streetMod);
  }

  if (!postalCode.empty()) {
    LDAPMod postalCodeMod;
    postalCodeMod.mod_op = LDAP_MOD_ADD;
    postalCodeMod.mod_type = const_cast<char *>("postalCode");
    postalCodeMod.mod_vals.modv_strvals = new char *[2];
    postalCodeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(postalCode.c_str());
    postalCodeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(postalCodeMod);
  }

  if (!l.empty()) {
    LDAPMod lMod;
    lMod.mod_op = LDAP_MOD_ADD;
    lMod.mod_type = const_cast<char *>("l");
    lMod.mod_vals.modv_strvals = new char *[2];
    lMod.mod_vals.modv_strvals[0] = const_cast<char *>(l.c_str());
    lMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(lMod);
  }

  if (!st.empty()) {
    LDAPMod stMod;
    stMod.mod_op = LDAP_MOD_ADD;
    stMod.mod_type = const_cast<char *>("st");
    stMod.mod_vals.modv_strvals = new char *[2];
    stMod.mod_vals.modv_strvals[0] = const_cast<char *>(st.c_str());
    stMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(stMod);
  }

  if (!c.empty()) {
    LDAPMod cMod;
    cMod.mod_op = LDAP_MOD_ADD;
    cMod.mod_type = const_cast<char *>("c");
    cMod.mod_vals.modv_strvals = new char *[2];
    cMod.mod_vals.modv_strvals[0] = const_cast<char *>(c.c_str());
    cMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(cMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(personDN, modPtrs.data())) {
    throw std::runtime_error("failed to add entry");
  }

  std::cout << "Person created successfully!" << std::endl;
  return true;
}

bool OpenLDAPManager::updatePerson(
    const std::string &personName, const std::string &baseDN,
    const std::string &uid, const std::string &givenName, const std::string &sn,
    const std::string &mail, const std::string &displayName,
    const std::string &employeeNumber, const std::string &employeeType,
    const std::string &departmentNumber, const std::string &mobile,
    const std::string &homePhone, const std::string &pager,
    const std::string &title, const std::string &telephoneNumber,
    const std::string &street, const std::string &postalCode,
    const std::string &l, const std::string &st, const std::string &c) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  std::cout << "Updating Person:" << std::endl;
  std::cout << "  CN: " << personName << std::endl;
  std::cout << "  DN: " << personDN << std::endl;

  std::vector<LDAPMod> mods;

  // Update uid if provided
  if (!uid.empty()) {
    LDAPMod uidMod;
    uidMod.mod_op = LDAP_MOD_REPLACE;
    uidMod.mod_type = const_cast<char *>("uid");
    uidMod.mod_vals.modv_strvals = new char *[2];
    uidMod.mod_vals.modv_strvals[0] = const_cast<char *>(uid.c_str());
    uidMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(uidMod);
  }

  // Update givenName if provided
  if (!givenName.empty()) {
    LDAPMod givenNameMod;
    givenNameMod.mod_op = LDAP_MOD_REPLACE;
    givenNameMod.mod_type = const_cast<char *>("givenName");
    givenNameMod.mod_vals.modv_strvals = new char *[2];
    givenNameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(givenName.c_str());
    givenNameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(givenNameMod);
  }

  // Update sn if provided
  if (!sn.empty()) {
    LDAPMod snMod;
    snMod.mod_op = LDAP_MOD_REPLACE;
    snMod.mod_type = const_cast<char *>("sn");
    snMod.mod_vals.modv_strvals = new char *[2];
    snMod.mod_vals.modv_strvals[0] = const_cast<char *>(sn.c_str());
    snMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(snMod);
  }

  // Update mail if provided
  if (!mail.empty()) {
    LDAPMod mailMod;
    mailMod.mod_op = LDAP_MOD_REPLACE;
    mailMod.mod_type = const_cast<char *>("mail");
    mailMod.mod_vals.modv_strvals = new char *[2];
    mailMod.mod_vals.modv_strvals[0] = const_cast<char *>(mail.c_str());
    mailMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mailMod);
  }

  // Update displayName if provided
  if (!displayName.empty()) {
    LDAPMod displayNameMod;
    displayNameMod.mod_op = LDAP_MOD_REPLACE;
    displayNameMod.mod_type = const_cast<char *>("displayName");
    displayNameMod.mod_vals.modv_strvals = new char *[2];
    displayNameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(displayName.c_str());
    displayNameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(displayNameMod);
  }

  // Update employeeNumber if provided
  if (!employeeNumber.empty()) {
    LDAPMod employeeNumberMod;
    employeeNumberMod.mod_op = LDAP_MOD_REPLACE;
    employeeNumberMod.mod_type = const_cast<char *>("employeeNumber");
    employeeNumberMod.mod_vals.modv_strvals = new char *[2];
    employeeNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(employeeNumber.c_str());
    employeeNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(employeeNumberMod);
  }

  // Update employeeType if provided
  if (!employeeType.empty()) {
    LDAPMod employeeTypeMod;
    employeeTypeMod.mod_op = LDAP_MOD_REPLACE;
    employeeTypeMod.mod_type = const_cast<char *>("employeeType");
    employeeTypeMod.mod_vals.modv_strvals = new char *[2];
    employeeTypeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(employeeType.c_str());
    employeeTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(employeeTypeMod);
  }

  // Update departmentNumber if provided
  if (!departmentNumber.empty()) {
    LDAPMod departmentNumberMod;
    departmentNumberMod.mod_op = LDAP_MOD_REPLACE;
    departmentNumberMod.mod_type = const_cast<char *>("departmentNumber");
    departmentNumberMod.mod_vals.modv_strvals = new char *[2];
    departmentNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(departmentNumber.c_str());
    departmentNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(departmentNumberMod);
    departmentNumberMod.mod_vals.modv_bvals[0]->bv_len =
        departmentNumber.length();
    departmentNumberMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(departmentNumber.c_str());
    departmentNumberMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(departmentNumberMod);
  }

  // Update mobile if provided
  if (!mobile.empty()) {
    LDAPMod mobileMod;
    mobileMod.mod_op = LDAP_MOD_REPLACE;
    mobileMod.mod_type = const_cast<char *>("mobile");
    mobileMod.mod_vals.modv_strvals = new char *[2];
    mobileMod.mod_vals.modv_strvals[0] = const_cast<char *>(mobile.c_str());
    mobileMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mobileMod);
  }

  // Update homePhone if provided
  if (!homePhone.empty()) {
    LDAPMod homePhoneMod;
    homePhoneMod.mod_op = LDAP_MOD_REPLACE;
    homePhoneMod.mod_type = const_cast<char *>("homePhone");
    homePhoneMod.mod_vals.modv_strvals = new char *[2];
    homePhoneMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(homePhone.c_str());
    homePhoneMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(homePhoneMod);
  }

  // Update pager if provided
  if (!pager.empty()) {
    LDAPMod pagerMod;
    pagerMod.mod_op = LDAP_MOD_REPLACE;
    pagerMod.mod_type = const_cast<char *>("pager");
    pagerMod.mod_vals.modv_strvals = new char *[2];
    pagerMod.mod_vals.modv_strvals[0] = const_cast<char *>(pager.c_str());
    pagerMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(pagerMod);
  }

  // Update title if provided
  if (!title.empty()) {
    LDAPMod titleMod;
    titleMod.mod_op = LDAP_MOD_REPLACE;
    titleMod.mod_type = const_cast<char *>("title");
    titleMod.mod_vals.modv_strvals = new char *[2];
    titleMod.mod_vals.modv_strvals[0] = const_cast<char *>(title.c_str());
    titleMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(titleMod);
  }

  // Update telephoneNumber if provided
  if (!telephoneNumber.empty()) {
    LDAPMod telephoneNumberMod;
    telephoneNumberMod.mod_op = LDAP_MOD_REPLACE;
    telephoneNumberMod.mod_type = const_cast<char *>("telephoneNumber");
    telephoneNumberMod.mod_vals.modv_strvals = new char *[2];
    telephoneNumberMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(telephoneNumber.c_str());
    telephoneNumberMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(telephoneNumberMod);
  }

  // Update street if provided
  if (!street.empty()) {
    LDAPMod streetMod;
    streetMod.mod_op = LDAP_MOD_REPLACE;
    streetMod.mod_type = const_cast<char *>("street");
    streetMod.mod_vals.modv_strvals = new char *[2];
    streetMod.mod_vals.modv_strvals[0] = const_cast<char *>(street.c_str());
    streetMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(streetMod);
    streetMod.mod_vals.modv_bvals[0]->bv_len = street.length();
    streetMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(street.c_str());
    streetMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(streetMod);
  }

  // Update postalCode if provided
  if (!postalCode.empty()) {
    LDAPMod postalCodeMod;
    postalCodeMod.mod_op = LDAP_MOD_REPLACE;
    postalCodeMod.mod_type = const_cast<char *>("postalCode");
    postalCodeMod.mod_vals.modv_strvals = new char *[2];
    postalCodeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(postalCode.c_str());
    postalCodeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(postalCodeMod);
  }

  // Update l if provided
  if (!l.empty()) {
    LDAPMod lMod;
    lMod.mod_op = LDAP_MOD_REPLACE;
    lMod.mod_type = const_cast<char *>("l");
    lMod.mod_vals.modv_strvals = new char *[2];
    lMod.mod_vals.modv_strvals[0] = const_cast<char *>(l.c_str());
    lMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(lMod);
  }

  // Update st if provided
  if (!st.empty()) {
    LDAPMod stMod;
    stMod.mod_op = LDAP_MOD_REPLACE;
    stMod.mod_type = const_cast<char *>("st");
    stMod.mod_vals.modv_strvals = new char *[2];
    stMod.mod_vals.modv_strvals[0] = const_cast<char *>(st.c_str());
    stMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(stMod);
  }

  // Update c if provided
  if (!c.empty()) {
    LDAPMod cMod;
    cMod.mod_op = LDAP_MOD_REPLACE;
    cMod.mod_type = const_cast<char *>("c");
    cMod.mod_vals.modv_strvals = new char *[2];
    cMod.mod_vals.modv_strvals[0] = const_cast<char *>(c.c_str());
    cMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(cMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(personDN, modPtrs.data())) {
    throw std::runtime_error("failed to modify entry");
  }

  std::cout << "Person updated successfully!" << std::endl;
  return true;
}

bool OpenLDAPManager::deletePerson(const std::string &personName,
                                   const std::string &baseDN) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  std::cout << "Deleting Person:" << std::endl;
  std::cout << "  CN: " << personName << std::endl;
  std::cout << "  DN: " << personDN << std::endl;

  if (!m_connection.deleteEntry(personDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Person deleted successfully!" << std::endl;
  return true;
}

bool OpenLDAPManager::createOrganizationalUnit(const std::string &ouName,
                                               const std::string &baseDN,
                                               const std::string &password,
                                               const std::string &fullname,
                                               const std::string &email) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  std::cout << "Creating Organizational Unit:" << std::endl;
  std::cout << "  OU Name: " << ouName << std::endl;
  std::cout << "  OU DN: " << ouDN << std::endl;

  std::vector<LDAPMod> mods;

  LDAPMod ouMod;
  ouMod.mod_op = LDAP_MOD_ADD;
  ouMod.mod_type = const_cast<char *>("ou");
  ouMod.mod_vals.modv_bvals = new struct berval *[2];
  ouMod.mod_vals.modv_bvals[0] = new struct berval;
  ouMod.mod_vals.modv_bvals[0]->bv_len = ouName.length();
  ouMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(ouName.c_str());
  ouMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(ouMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[2];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 18;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("organizationalUnit");
  objectClassMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(objectClassMod);

  if (!password.empty()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD;
    passwordMod.mod_type = const_cast<char *>("userPassword");
    passwordMod.mod_vals.modv_bvals = new struct berval *[2];
    passwordMod.mod_vals.modv_bvals[0] = new struct berval;
    passwordMod.mod_vals.modv_bvals[0]->bv_len = password.length();
    passwordMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(password.c_str());
    passwordMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (!fullname.empty()) {
    LDAPMod cnMod;
    cnMod.mod_op = LDAP_MOD_ADD;
    cnMod.mod_type = const_cast<char *>("cn");
    cnMod.mod_vals.modv_bvals = new struct berval *[2];
    cnMod.mod_vals.modv_bvals[0] = new struct berval;
    cnMod.mod_vals.modv_bvals[0]->bv_len = fullname.length();
    cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(fullname.c_str());
    cnMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(cnMod);
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

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(ouDN, modPtrs.data())) {
    throw std::runtime_error("failed to add entry");
  }

  std::cout << "Organizational Unit created successfully!" << std::endl;
  return true;
}

bool OpenLDAPManager::updateOrganizationalUnit(const std::string &ouName,
                                               const std::string &baseDN) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  std::cout << "Updating Organizational Unit:" << std::endl;
  std::cout << "  OU Name: " << ouName << std::endl;
  std::cout << "  OU DN: " << ouDN << std::endl;

  std::cerr << "Update functionality not yet implemented" << std::endl;
  return false;
}

bool OpenLDAPManager::deleteOrganizationalUnit(const std::string &ouName,
                                               const std::string &baseDN) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  std::cout << "Deleting Organizational Unit:" << std::endl;
  std::cout << "  OU Name: " << ouName << std::endl;
  std::cout << "  OU DN: " << ouDN << std::endl;

  if (!m_connection.deleteEntry(ouDN)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Organizational Unit deleted successfully!" << std::endl;
  return true;
}

bool OpenLDAPManager::listOrganizationalUnits(const std::string &baseDN) {
  std::cout << "Listing Organizational Units:" << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=organizationalUnit)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cerr << "Error: " << m_connection.getError() << std::endl;
    return false;
  }

  std::cout << "Found " << results.size()
            << " Organizational Units:" << std::endl;

  for (size_t i = 0; i < results.size(); i++) {
    std::cout << "\\nOrganizational Unit " << (i + 1) << ":" << std::endl;
    for (const auto &[attr, value] : results[i]) {
      std::cout << "  " << attr << ": " << value << std::endl;
    }
  }

  return true;
}
