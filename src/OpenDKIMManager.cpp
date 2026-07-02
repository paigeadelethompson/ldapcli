#include "OpenDKIMManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>

OpenDKIMManager::OpenDKIMManager(LDAPConnection &connection)
    : m_connection(connection) {}

void OpenDKIMManager::printUsage() const {
  console::e("OpenDKIM Commands:");
  console::e("  create-identity <identity> [base-dn]");
  console::e("  delete-identity <identity> [base-dn]");
  console::e("  update-identity <identity> [base-dn]");
  console::e("  list-identities [base-dn]");
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
    std::optional<std::string> selector;
    std::optional<std::string> key;
    std::optional<std::string> domain;

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
        console::e("Usage: ldapcli create-identity <identity> [-s selector] "
                   "[-k key] [-d domain]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-identity <identity> [-s selector] [-k "
                 "key] [-d domain]");
      return false;
    }

    identity = argv[optind];

    return createIdentity(identity, baseDN, selector, key, domain);
  } else if (command == "delete-identity") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-identity <identity>");
      return false;
    }

    std::string identity = argv[optind];

    return deleteIdentity(identity, baseDN);
  } else if (command == "update-identity") {
    static struct option long_options[] = {
        {"selector", required_argument, 0, 's'},
        {"key", required_argument, 0, 'k'},
        {"domain", required_argument, 0, 'd'},
        {0, 0, 0, 0}};

    std::string identity;
    std::optional<std::string> selector;
    std::optional<std::string> key;
    std::optional<std::string> domain;

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
        console::e(
            "Usage: ldapcli update-identity <identity> [-s selector] [-k "
            "key] [-d domain]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli update-identity <identity> [-s selector] [-k "
                 "key] [-d domain]");
      return false;
    }

    identity = argv[optind];

    return updateIdentity(identity, baseDN, selector, key, domain);
  } else if (command == "list-identities") {
    return listIdentities(baseDN);
  } else {
    console::e("Unknown OpenDKIM command: {}", command);
    printUsage();
    return false;
  }
}

bool OpenDKIMManager::listIdentities(const std::string &baseDN) {
  console::e("Listing OpenDKIM identities:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=DKIM)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No identities found.");
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

  std::mdspan<std::string, std::dextents<size_t, 2>> tableData(
      flatData.data(), results.size() + 1, 2);

  console::printTable(tableData);
  return true;
}

bool OpenDKIMManager::createIdentity(const std::string &identity,
                                     const std::string &baseDN,
                                     const std::optional<std::string> &selector,
                                     const std::optional<std::string> &key,
                                     const std::optional<std::string> &domain) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  console::e("Creating OpenDKIM identity:");
  console::e("  Identity: {}", identity);
  console::e("  Identity DN: {}", identityDN);

  std::vector<LDAPMod> mods;

  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(identity.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[2];
  objectClassMod.mod_vals.modv_strvals[0] = const_cast<char *>("DKIM");
  objectClassMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod identityMod;
  identityMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  identityMod.mod_type = const_cast<char *>("DKIMIdentity");
  identityMod.mod_vals.modv_strvals = new char *[2];
  identityMod.mod_vals.modv_strvals[0] = const_cast<char *>(identity.c_str());
  identityMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(identityMod);

  if (selector.has_value()) {
    LDAPMod selectorMod;
    selectorMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    selectorMod.mod_type = const_cast<char *>("DKIMSelector");
    selectorMod.mod_vals.modv_strvals = new char *[2];
    selectorMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(selector->c_str());
    selectorMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(selectorMod);
  }
  if (key.has_value()) {
    LDAPMod keyMod;
    keyMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    keyMod.mod_type = const_cast<char *>("DKIMKey");
    keyMod.mod_vals.modv_strvals = new char *[2];
    keyMod.mod_vals.modv_strvals[0] = const_cast<char *>(key->c_str());
    keyMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(keyMod);
  }
  if (domain.has_value()) {
    LDAPMod domainMod;
    domainMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    domainMod.mod_type = const_cast<char *>("DKIMDomain");
    domainMod.mod_vals.modv_strvals = new char *[2];
    domainMod.mod_vals.modv_strvals[0] = const_cast<char *>(domain->c_str());
    domainMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(domainMod);
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(identityDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Identity created successfully!");
  return true;
}

bool OpenDKIMManager::updateIdentity(const std::string &identity,
                                     const std::string &baseDN,
                                     const std::optional<std::string> &selector,
                                     const std::optional<std::string> &key,
                                     const std::optional<std::string> &domain) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  console::e("Updating OpenDKIM identity:");
  console::e("  Identity: {}", identity);
  console::e("  Identity DN: {}", identityDN);

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

  addReplace("DKIMSelector", selector);
  addReplace("DKIMKey", key);
  addReplace("DKIMDomain", domain);

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(identityDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Identity updated successfully!");
  return true;
}

bool OpenDKIMManager::deleteIdentity(const std::string &identity,
                                     const std::string &baseDN) {
  std::string identityDN = getIdentityDN(identity, baseDN);

  console::e("Deleting OpenDKIM identity:");
  console::e("  Identity: {}", identity);
  console::e("  Identity DN: {}", identityDN);

  if (!m_connection.deleteEntry(identityDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Identity deleted successfully!");
  return true;
}

std::string OpenDKIMManager::getIdentityDN(const std::string &identity,
                                           const std::string &baseDN) const {
  return "cn=" + identity + "," + baseDN;
}
