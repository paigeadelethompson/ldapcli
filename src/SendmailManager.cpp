#include "SendmailManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>

SendmailManager::SendmailManager(LDAPConnection &connection)
    : m_connection(connection) {}

void SendmailManager::printUsage() const {
  console::e("Sendmail Commands:");
  console::e("  create-mta <mta> [base-dn]");
  console::e("  delete-mta <mta> [base-dn]");
  console::e("  update-mta <mta> [base-dn]");
  console::e("  list-mtas [base-dn]");
}

std::string SendmailManager::getServiceName() const { return "sendmail"; }

bool SendmailManager::execute(int argc, char *argv[]) {
  if (argc < 3) {
    printUsage();
    return false;
  }

  std::string command = argv[2];
  std::string baseDN = Config::getInstance().getBaseDN();

  if (command == "create-mta") {
    static struct option long_options[] = {
        {"cluster", required_argument, 0, 'c'},
        {"host", required_argument, 0, 'h'},
        {0, 0, 0, 0}};

    std::string mtaName;
    std::string cluster;
    std::string host;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "c:h:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 'c':
        cluster = optarg;
        break;
      case 'h':
        host = optarg;
        break;
      default:
        console::e("Usage: ldapcli create-mta <mta-name> [-c cluster] [-h host]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli create-mta <mta-name> [-c cluster] [-h host]");
      return false;
    }

    mtaName = argv[optind];

    return createMTA(mtaName, baseDN);
  } else if (command == "delete-mta") {
    if (optind >= argc) {
      console::e("Usage: ldapcli delete-mta <mta-name>");
      return false;
    }

    std::string mtaName = argv[optind];

    return deleteMTA(mtaName, baseDN);
  } else if (command == "update-mta") {
    if (optind >= argc) {
      console::e("Usage: ldapcli update-mta <mta-name>");
      return false;
    }

    std::string mtaName = argv[optind];

    return updateMTA(mtaName, baseDN, argc, argv);
  } else if (command == "list-mtas") {
    return listMTAs(baseDN);
  } else {
    console::e("Unknown Sendmail command: {}", command);
    printUsage();
    return false;
  }
}

bool SendmailManager::listMTAs(const std::string &baseDN) {
  console::e("Listing Sendmail MTAs:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=sendmailMTA)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No MTAs found.");
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
    flatData.data(), results.size() + 1, 2
  );

  console::printTable(tableData);
  return true;
}

bool SendmailManager::createMTA(const std::string &mtaName,
                                const std::string &baseDN) {
  std::string mtaDN = getMTADN(mtaName, baseDN);

  console::e("Creating Sendmail MTA:");
  console::e("  MTA Name: {}", mtaName);
  console::e("  MTA DN: {}", mtaDN);

  // Create LDAP mods for sendmailMTA object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_bvals = new struct berval *[2];
  cnMod.mod_vals.modv_bvals[0] = new struct berval;
  cnMod.mod_vals.modv_bvals[0]->bv_len = mtaName.length();
  cnMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(mtaName.c_str());
  cnMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_bvals = new struct berval *[2];
  objectClassMod.mod_vals.modv_bvals[0] = new struct berval;
  objectClassMod.mod_vals.modv_bvals[0]->bv_len = 12;
  objectClassMod.mod_vals.modv_bvals[0]->bv_val =
      const_cast<char *>("sendmailMTA");
  objectClassMod.mod_vals.modv_bvals[1] = nullptr;
  mods.push_back(objectClassMod);

  // Optional attributes
  std::string cluster;
  std::string host;
  if (!cluster.empty()) {
    LDAPMod clusterMod;
    clusterMod.mod_op = LDAP_MOD_ADD;
    clusterMod.mod_type = const_cast<char *>("sendmailMTACluster");
    clusterMod.mod_vals.modv_bvals = new struct berval *[2];
    clusterMod.mod_vals.modv_bvals[0] = new struct berval;
    clusterMod.mod_vals.modv_bvals[0]->bv_len = cluster.length();
    clusterMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(cluster.c_str());
    clusterMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(clusterMod);
  }
  if (!host.empty()) {
    LDAPMod hostMod;
    hostMod.mod_op = LDAP_MOD_ADD;
    hostMod.mod_type = const_cast<char *>("sendmailMTAHost");
    hostMod.mod_vals.modv_bvals = new struct berval *[2];
    hostMod.mod_vals.modv_bvals[0] = new struct berval;
    hostMod.mod_vals.modv_bvals[0]->bv_len = host.length();
    hostMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(host.c_str());
    hostMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(hostMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(mtaDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("MTA created successfully!");
  return true;
}

bool SendmailManager::updateMTA(const std::string &mtaName,
                                const std::string &baseDN, int argc,
                                char *argv[]) {
  std::string mtaDN = getMTADN(mtaName, baseDN);

  console::e("Updating Sendmail MTA:");
  console::e("  MTA Name: {}", mtaName);
  console::e("  MTA DN: {}", mtaDN);

  static struct option long_options[] = {{"cluster", required_argument, 0, 'c'},
                                         {"host", required_argument, 0, 'h'},
                                         {0, 0, 0, 0}};

  std::string cluster;
  std::string host;

  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "c:h:", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 'c':
      cluster = optarg;
      break;
    case 'h':
      host = optarg;
      break;
    default:
      console::e("Usage: ldapcli update-mta <mta-name> [-c cluster] [-h host]");
      return false;
    }
  }

  if (optind >= argc) {
    console::e("Usage: ldapcli update-mta <mta-name> [-c cluster] [-h host]");
    return false;
  }

  // Get current MTA data
  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(cn=" + mtaName + ")";

  if (!m_connection.search(mtaDN, LDAP_SCOPE_BASE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("Error: MTA not found");
    return false;
  }

  // Create LDAP mods for updates
  std::vector<LDAPMod> mods;

  if (!cluster.empty()) {
    LDAPMod clusterMod;
    clusterMod.mod_op = LDAP_MOD_REPLACE;
    clusterMod.mod_type = const_cast<char *>("sendmailMTACluster");
    clusterMod.mod_vals.modv_bvals = new struct berval *[2];
    clusterMod.mod_vals.modv_bvals[0] = new struct berval;
    clusterMod.mod_vals.modv_bvals[0]->bv_len = cluster.length();
    clusterMod.mod_vals.modv_bvals[0]->bv_val =
        const_cast<char *>(cluster.c_str());
    clusterMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(clusterMod);
  }
  if (!host.empty()) {
    LDAPMod hostMod;
    hostMod.mod_op = LDAP_MOD_REPLACE;
    hostMod.mod_type = const_cast<char *>("sendmailMTAHost");
    hostMod.mod_vals.modv_bvals = new struct berval *[2];
    hostMod.mod_vals.modv_bvals[0] = new struct berval;
    hostMod.mod_vals.modv_bvals[0]->bv_len = host.length();
    hostMod.mod_vals.modv_bvals[0]->bv_val = const_cast<char *>(host.c_str());
    hostMod.mod_vals.modv_bvals[1] = nullptr;
    mods.push_back(hostMod);
  }

  if (mods.empty()) {
    console::e("Error: No attributes specified for update");
    return false;
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(mtaDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("MTA updated successfully!");
  return true;
}

bool SendmailManager::deleteMTA(const std::string &mtaName,
                                const std::string &baseDN) {
  std::string mtaDN = getMTADN(mtaName, baseDN);

  console::e("Deleting Sendmail MTA:");
  console::e("  MTA Name: {}", mtaName);
  console::e("  MTA DN: {}", mtaDN);

  if (!m_connection.deleteEntry(mtaDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("MTA deleted successfully!");
  return true;
}

std::string SendmailManager::getMTADN(const std::string &mtaName,
                                      const std::string &baseDN) const {
  return "cn=" + mtaName + "," + baseDN;
}