#include "OpenLDAPManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

OpenLDAPManager::OpenLDAPManager(LDAPConnection &connection)
    : m_connection(connection) {}

void OpenLDAPManager::printUsage() const {
  console::e("OPENLDAP Commands:");
  console::e("  create-ou <ou-name> [base-dn]");
  console::e("  delete-ou <ou-name> [base-dn]");
  console::e("  update-ou <ou-name> [base-dn]");
  console::e("  list-ous [base-dn]");
  console::e("  list-people [base-dn]");
  console::e("  create-person <cn> [base-dn]");
  console::e("  update-person <cn> [base-dn]");
  console::e("  delete-person <cn> [base-dn]");
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
        {"telephonenumber", required_argument, 0, 'p'},
        {"street", required_argument, 0, 's'},
        {"postalcode", required_argument, 0, 'z'},
        {"st", required_argument, 0, 'S'},
        {"l", required_argument, 0, 'L'},
        {"description", required_argument, 0, 'd'},
        {0, 0, 0, 0}};

    std::string ouName;
    std::optional<std::string> telephoneNumber;
    std::optional<std::string> street;
    std::optional<std::string> postalCode;
    std::optional<std::string> st;
    std::optional<std::string> l;
    std::optional<std::string> description;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "p:s:z:S:L:d:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'p':
        telephoneNumber = optarg;
        break;
      case 's':
        street = optarg;
        break;
      case 'z':
        postalCode = optarg;
        break;
      case 'S':
        st = optarg;
        break;
      case 'L':
        l = optarg;
        break;
      case 'd':
        description = optarg;
        break;
      default:
        console::e("Usage: ldapcli create-ou <ou-name> [-p telephone] "
                   "[-s street] [-z postalcode] [-S state] [-L city] "
                   "[-d description]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-ou <ou-name> [-p telephone] "
                 "[-s street] [-z postalcode] [-S state] [-L city] "
                 "[-d description]");
      return false;
    }

    ouName = argv[optind];

    return createOrganizationalUnit(ouName, baseDN, telephoneNumber, street,
                                    postalCode, st, l, description);
  } else if (command == "delete-ou") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-ou <ou-name>");
      return false;
    }

    std::string ouName = argv[optind];

    return deleteOrganizationalUnit(ouName, baseDN);
  } else if (command == "update-ou") {
    if (optind >= argc) {
      console::e("Usage: ldapcli update-ou <ou-name>");
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
    std::optional<std::string> uid;
    std::optional<std::string> givenName;
    std::optional<std::string> sn;
    std::optional<std::string> mail;
    std::optional<std::string> displayName;
    std::optional<std::string> employeeNumber;
    std::optional<std::string> employeeType;
    std::optional<std::string> departmentNumber;
    std::optional<std::string> mobile;
    std::optional<std::string> homePhone;
    std::optional<std::string> pager;
    std::optional<std::string> title;
    std::optional<std::string> telephoneNumber;
    std::optional<std::string> street;
    std::optional<std::string> postalCode;
    std::optional<std::string> l;
    std::optional<std::string> st;
    std::optional<std::string> c;

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
        console::e("Usage: ldapcli create-person <cn> [options]");
        console::e("Options:");
        console::e("  -u, --uid <uid>           User ID");
        console::e("  -g, --givenname <name>   Given name");
        console::e("  -s, --sn <surname>       Surname");
        console::e("  -m, --mail <email>       Email address");
        console::e("  -d, --displayname <name> Display name");
        console::e("  -e, --employeenumber <num> Employee number");
        console::e("  -t, --employeetype <type> Employee type");
        console::e("  -n, --departmentnumber <num> Department number");
        console::e("  -M, --mobile <phone>     Mobile phone");
        console::e("  -H, --homephone <phone>  Home phone");
        console::e("  -P, --pager <phone>      Pager");
        console::e("  -i, --title <title>      Job title");
        console::e("  -l, --telephonenumber <phone> Telephone number");
        console::e("  -r, --street <address>   Street address");
        console::e("  -z, --postalcode <code>  Postal code");
        console::e("  -L, --l <city>           City");
        console::e("  -S, --st <state>         State");
        console::e("  -C, --c <country>        Country");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-person <cn> [options]");
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
    std::optional<std::string> uid;
    std::optional<std::string> givenName;
    std::optional<std::string> sn;
    std::optional<std::string> mail;
    std::optional<std::string> displayName;
    std::optional<std::string> employeeNumber;
    std::optional<std::string> employeeType;
    std::optional<std::string> departmentNumber;
    std::optional<std::string> mobile;
    std::optional<std::string> homePhone;
    std::optional<std::string> pager;
    std::optional<std::string> title;
    std::optional<std::string> telephoneNumber;
    std::optional<std::string> street;
    std::optional<std::string> postalCode;
    std::optional<std::string> l;
    std::optional<std::string> st;
    std::optional<std::string> c;

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
        console::e("Usage: ldapcli update-person <cn> [options]");
        console::e("Options:");
        console::e("  -u, --uid <uid>           User ID");
        console::e("  -g, --givenname <name>   Given name");
        console::e("  -s, --sn <surname>       Surname");
        console::e("  -m, --mail <email>       Email address");
        console::e("  -d, --displayname <name> Display name");
        console::e("  -e, --employeenumber <num> Employee number");
        console::e("  -t, --employeetype <type> Employee type");
        console::e("  -n, --departmentnumber <num> Department number");
        console::e("  -M, --mobile <phone>     Mobile phone");
        console::e("  -H, --homephone <phone>  Home phone");
        console::e("  -P, --pager <phone>      Pager");
        console::e("  -i, --title <title>      Job title");
        console::e("  -l, --telephonenumber <phone> Telephone number");
        console::e("  -r, --street <address>   Street address");
        console::e("  -z, --postalcode <code>  Postal code");
        console::e("  -L, --l <city>           City");
        console::e("  -S, --st <state>         State");
        console::e("  -C, --c <country>        Country");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli update-person <cn> [options]");
      return false;
    }

    personName = argv[optind];

    return updatePerson(personName, baseDN, uid, givenName, sn, mail,
                        displayName, employeeNumber, employeeType,
                        departmentNumber, mobile, homePhone, pager, title,
                        telephoneNumber, street, postalCode, l, st, c);
  } else if (command == "delete-person") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-person <cn>");
      return false;
    }

    std::string personName = argv[optind];

    return deletePerson(personName, baseDN);
  }

  printUsage();
  return false;
}

bool OpenLDAPManager::listPeople(const std::string &baseDN) {
  console::e("Listing People:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=inetOrgPerson)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No people found.");
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

bool OpenLDAPManager::createPerson(
    const std::string &personName, const std::string &baseDN,
    const std::optional<std::string> &uid,
    const std::optional<std::string> &givenName,
    const std::optional<std::string> &sn,
    const std::optional<std::string> &mail,
    const std::optional<std::string> &displayName,
    const std::optional<std::string> &employeeNumber,
    const std::optional<std::string> &employeeType,
    const std::optional<std::string> &departmentNumber,
    const std::optional<std::string> &mobile,
    const std::optional<std::string> &homePhone,
    const std::optional<std::string> &pager,
    const std::optional<std::string> &title,
    const std::optional<std::string> &telephoneNumber,
    const std::optional<std::string> &street,
    const std::optional<std::string> &postalCode,
    const std::optional<std::string> &l, const std::optional<std::string> &st,
    const std::optional<std::string> &c) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  console::e("Creating Person:");
  console::e("  CN: {}", personName);
  console::e("  DN: {}", personDN);

  std::vector<LDAPMod> mods;

  auto addAttr = [&](const char *attr, const std::optional<std::string> &val) {
    if (!val.has_value()) {
      return;
    }
    LDAPMod mod;
    mod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mod.mod_type = const_cast<char *>(attr);
    mod.mod_vals.modv_strvals = new char *[2];
    mod.mod_vals.modv_strvals[0] = const_cast<char *>(val->c_str());
    mod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mod);
  };

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
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
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(personName.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  addAttr("uid", uid);
  addAttr("givenName", givenName);
  addAttr("sn", sn);
  addAttr("mail", mail);
  addAttr("displayName", displayName);
  addAttr("employeeNumber", employeeNumber);
  addAttr("employeeType", employeeType);
  addAttr("departmentNumber", departmentNumber);
  addAttr("mobile", mobile);
  addAttr("homePhone", homePhone);
  addAttr("pager", pager);
  addAttr("title", title);
  addAttr("telephoneNumber", telephoneNumber);
  addAttr("street", street);
  addAttr("postalCode", postalCode);
  addAttr("l", l);
  addAttr("st", st);
  addAttr("c", c);

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(personDN, modPtrs.data())) {
    throw std::runtime_error("failed to add entry");
  }

  console::e("Person created successfully!");
  return true;
}

bool OpenLDAPManager::updatePerson(
    const std::string &personName, const std::string &baseDN,
    const std::optional<std::string> &uid,
    const std::optional<std::string> &givenName,
    const std::optional<std::string> &sn,
    const std::optional<std::string> &mail,
    const std::optional<std::string> &displayName,
    const std::optional<std::string> &employeeNumber,
    const std::optional<std::string> &employeeType,
    const std::optional<std::string> &departmentNumber,
    const std::optional<std::string> &mobile,
    const std::optional<std::string> &homePhone,
    const std::optional<std::string> &pager,
    const std::optional<std::string> &title,
    const std::optional<std::string> &telephoneNumber,
    const std::optional<std::string> &street,
    const std::optional<std::string> &postalCode,
    const std::optional<std::string> &l, const std::optional<std::string> &st,
    const std::optional<std::string> &c) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  console::e("Updating Person:");
  console::e("  CN: {}", personName);
  console::e("  DN: {}", personDN);

  std::vector<LDAPMod> mods;

  auto addReplace = [&](const char *attr, const std::optional<std::string> &val) {
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

  addReplace("uid", uid);
  addReplace("givenName", givenName);
  addReplace("sn", sn);
  addReplace("mail", mail);
  addReplace("displayName", displayName);
  addReplace("employeeNumber", employeeNumber);
  addReplace("employeeType", employeeType);
  addReplace("departmentNumber", departmentNumber);
  addReplace("mobile", mobile);
  addReplace("homePhone", homePhone);
  addReplace("pager", pager);
  addReplace("title", title);
  addReplace("telephoneNumber", telephoneNumber);
  addReplace("street", street);
  addReplace("postalCode", postalCode);
  addReplace("l", l);
  addReplace("st", st);
  addReplace("c", c);

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(personDN, modPtrs.data())) {
    throw std::runtime_error("failed to modify entry");
  }

  console::e("Person updated successfully!");
  return true;
}

bool OpenLDAPManager::deletePerson(const std::string &personName,
                                   const std::string &baseDN) {
  std::string personDN = "cn=" + personName + "," + baseDN;

  console::e("Deleting Person:");
  console::e("  CN: {}", personName);
  console::e("  DN: {}", personDN);

  if (!m_connection.deleteEntry(personDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Person deleted successfully!");
  return true;
}

bool OpenLDAPManager::createOrganizationalUnit(
    const std::string &ouName, const std::string &baseDN,
    const std::optional<std::string> &telephoneNumber,
    const std::optional<std::string> &street,
    const std::optional<std::string> &postalCode,
    const std::optional<std::string> &st, const std::optional<std::string> &l,
    const std::optional<std::string> &description) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  console::e("Creating Organizational Unit:");
  console::e("  OU Name: {}", ouName);
  console::e("  OU DN: {}", ouDN);

  std::vector<LDAPMod> mods;

  LDAPMod ouMod;
  ouMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  ouMod.mod_type = const_cast<char *>("ou");
  ouMod.mod_vals.modv_strvals = new char *[2];
  ouMod.mod_vals.modv_strvals[0] = const_cast<char *>(ouName.c_str());
  ouMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(ouMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[2];
  objectClassMod.mod_vals.modv_strvals[0] =
      const_cast<char *>("organizationalUnit");
  objectClassMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(objectClassMod);

  auto addAttr = [&](const char *attr, const std::optional<std::string> &val) {
    if (!val.has_value()) {
      return;
    }
    LDAPMod mod;
    mod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mod.mod_type = const_cast<char *>(attr);
    mod.mod_vals.modv_strvals = new char *[2];
    mod.mod_vals.modv_strvals[0] = const_cast<char *>(val->c_str());
    mod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(mod);
  };

  addAttr("telephoneNumber", telephoneNumber);
  addAttr("street", street);
  addAttr("postalCode", postalCode);
  addAttr("st", st);
  addAttr("l", l);
  addAttr("description", description);

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(ouDN, modPtrs.data())) {
    throw std::runtime_error("failed to add entry");
  }

  console::e("Organizational Unit created successfully!");
  return true;
}

bool OpenLDAPManager::updateOrganizationalUnit(const std::string &ouName,
                                               const std::string &baseDN) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  console::e("Updating Organizational Unit:");
  console::e("  OU Name: {}", ouName);
  console::e("  OU DN: {}", ouDN);

  console::e("Update functionality not yet implemented");
  return false;
}

bool OpenLDAPManager::deleteOrganizationalUnit(const std::string &ouName,
                                               const std::string &baseDN) {
  std::string ouDN = "ou=" + ouName + "," + baseDN;

  console::e("Deleting Organizational Unit:");
  console::e("  OU Name: {}", ouName);
  console::e("  OU DN: {}", ouDN);

  if (!m_connection.deleteEntry(ouDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Organizational Unit deleted successfully!");
  return true;
}

bool OpenLDAPManager::listOrganizationalUnits(const std::string &baseDN) {
  console::e("Listing Organizational Units:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=organizationalUnit)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No organizational units found.");
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
