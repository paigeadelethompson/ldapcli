/**
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, RavenHammer Research Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "FreeRADIUSManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>

FreeRADIUSManager::FreeRADIUSManager(LDAPConnection &connection)
    : m_connection(connection) {}

void FreeRADIUSManager::printUsage() const {
  console::e("FreeRADIUS Commands:");
  console::e(
      "  create-client <client> [-s|--secret SECRET] [-n|--shortname NAME] "
      "[-t|--type TYPE]");
  console::e(
      "  update-client <client> [-s|--secret SECRET] [-n|--shortname NAME] "
      "[-v|--virtual-server SERVER] [-t|--type TYPE] "
      "[-m|--require-ma BOOL] [-c|--comment TEXT]");
  console::e("  delete-client <client> [--client CLIENT]");
  console::e("  list-clients");
  console::e(
      "  create-user <username> [-p|--password PASS] [-s|--service-type TYPE] "
      "[-f|--framed-protocol PROTO]");
  console::e(
      "  update-user <username> [-p|--password PASS] [-s|--service-type TYPE] "
      "[-f|--framed-protocol PROTO]");
  console::e("  delete-user <username> [--user USER]");
  console::e("  list-users");
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

    optind = 3;
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
        console::e(
            "Usage: ldapcli freeradius create-client <client> "
            "[-s|--secret SECRET] [-n|--shortname NAME] [-t|--type TYPE]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli freeradius create-client <client> "
                 "[-s|--secret SECRET] [-n|--shortname NAME] [-t|--type TYPE]");
      return false;
    }

    clientName = argv[optind];

    return createClient(clientName, baseDN, secret, shortname, std::nullopt,
                        type, std::nullopt, std::nullopt);
  } else if (command == "delete-client") {
    static struct option long_options[] = {
        {"client", required_argument, 0, 'c'}, {nullptr, 0, 0, 0}};

    std::string clientName;
    int opt;
    int option_index = 0;

    optind = 3;
    while ((opt = getopt_long(argc, argv, "c:", long_options, &option_index)) !=
           -1) {
      if (opt == 'c') {
        clientName = optarg;
      } else {
        console::e("Usage: ldapcli freeradius delete-client <client> "
                   "[--client CLIENT]");
        return false;
      }
    }

    if (clientName.empty() && optind < argc) {
      clientName = argv[optind];
    }
    if (clientName.empty()) {
      console::e("Usage: ldapcli freeradius delete-client <client> "
                 "[--client CLIENT]");
      return false;
    }

    return deleteClient(clientName, baseDN);
  } else if (command == "update-client") {
    static struct option long_options[] = {
        {"secret", required_argument, 0, 's'},
        {"shortname", required_argument, 0, 'n'},
        {"virtual-server", required_argument, 0, 'v'},
        {"type", required_argument, 0, 't'},
        {"require-ma", required_argument, 0, 'm'},
        {"comment", required_argument, 0, 'c'},
        {0, 0, 0, 0}};

    std::string clientName;
    std::optional<std::string> secret;
    std::optional<std::string> shortname;
    std::optional<std::string> virtualServer;
    std::optional<std::string> type;
    std::optional<bool> requireMa;
    std::optional<std::string> comment;

    int opt;
    int option_index = 0;

    optind = 3;
    while ((opt = getopt_long(argc, argv, "s:n:v:t:m:c:", long_options,
                              &option_index)) != -1) {
      switch (opt) {
      case 's':
        secret = optarg;
        break;
      case 'n':
        shortname = optarg;
        break;
      case 'v':
        virtualServer = optarg;
        break;
      case 't':
        type = optarg;
        break;
      case 'm':
        requireMa = (strcmp(optarg, "true") == 0);
        break;
      case 'c':
        comment = optarg;
        break;
      default:
        console::e("Usage: ldapcli freeradius update-client <client> "
                   "[-s|--secret SECRET] [-n|--shortname NAME] "
                   "[-v|--virtual-server SERVER] [-t|--type TYPE] "
                   "[-m|--require-ma BOOL] [-c|--comment TEXT]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli freeradius update-client <client> "
                 "[-s|--secret SECRET] [-n|--shortname NAME] "
                 "[-v|--virtual-server SERVER] [-t|--type TYPE] "
                 "[-m|--require-ma BOOL] [-c|--comment TEXT]");
      return false;
    }

    clientName = argv[optind];

    return updateClient(clientName, baseDN, secret, shortname, virtualServer,
                        type, requireMa, comment);
  } else if (command == "list-clients") {
    return listClients(baseDN);
  } else if (command == "create-user") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"service-type", required_argument, 0, 's'},
        {"framed-protocol", required_argument, 0, 'f'},
        {0, 0, 0, 0}};

    std::string username;
    std::optional<std::string> password;
    std::optional<std::string> serviceType = "Framed-User";
    std::optional<std::string> framedProtocol = "PPP";

    int opt;
    int option_index = 0;

    optind = 3;
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
        console::e("Usage: ldapcli freeradius create-user <username> "
                   "[-p|--password PASS] [-s|--service-type TYPE] "
                   "[-f|--framed-protocol PROTO]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli freeradius create-user <username> "
                 "[-p|--password PASS] [-s|--service-type TYPE] "
                 "[-f|--framed-protocol PROTO]");
      return false;
    }

    username = argv[optind];

    return createUser(username, baseDN, password, serviceType, framedProtocol);
  } else if (command == "update-user") {
    static struct option long_options[] = {
        {"password", required_argument, 0, 'p'},
        {"service-type", required_argument, 0, 's'},
        {"framed-protocol", required_argument, 0, 'f'},
        {0, 0, 0, 0}};

    std::string username;
    std::optional<std::string> password;
    std::optional<std::string> serviceType;
    std::optional<std::string> framedProtocol;

    int opt;
    int option_index = 0;

    optind = 3;
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
        console::e("Usage: ldapcli freeradius update-user <username> "
                   "[-p|--password PASS] [-s|--service-type TYPE] "
                   "[-f|--framed-protocol PROTO]");
        return false;
      }
    }

    if (optind >= argc) {
      console::e("Usage: ldapcli freeradius update-user <username> "
                 "[-p|--password PASS] [-s|--service-type TYPE] "
                 "[-f|--framed-protocol PROTO]");
      return false;
    }

    username = argv[optind];

    return updateUser(username, baseDN, password, serviceType, framedProtocol);
  } else if (command == "delete-user") {
    static struct option long_options[] = {{"user", required_argument, 0, 'u'},
                                           {nullptr, 0, 0, 0}};

    std::string username;
    int opt;
    int option_index = 0;

    optind = 3;
    while ((opt = getopt_long(argc, argv, "u:", long_options, &option_index)) !=
           -1) {
      if (opt == 'u') {
        username = optarg;
      } else {
        console::e("Usage: ldapcli freeradius delete-user <username> "
                   "[--user USER]");
        return false;
      }
    }

    if (username.empty() && optind < argc) {
      username = argv[optind];
    }
    if (username.empty()) {
      console::e("Usage: ldapcli freeradius delete-user <username> "
                 "[--user USER]");
      return false;
    }

    return deleteUser(username, baseDN);
  } else if (command == "list-users") {
    return listUsers(baseDN);
  }

  printUsage();
  return false;
}

bool FreeRADIUSManager::createClient(
    const std::string &clientName, const std::string &baseDN,
    const std::optional<std::string> &secret,
    const std::optional<std::string> &shortname,
    const std::optional<std::string> &virtualServer,
    const std::optional<std::string> &type,
    const std::optional<bool> &requireMa,
    const std::optional<std::string> &comment) {
  std::string clientDN = getClientDN(clientName, baseDN);

  console::e("Creating FreeRADIUS client:");
  console::e("  Client Name: {}", clientName);
  console::e("  Client DN: {}", clientDN);

  // Create LDAP mods for radiusClient object class
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod identifierMod;
  identifierMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  identifierMod.mod_type = const_cast<char *>("radiusClientIdentifier");
  identifierMod.mod_vals.modv_strvals = new char *[2];
  identifierMod.mod_vals.modv_strvals[0] =
      const_cast<char *>(clientName.c_str());
  identifierMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(identifierMod);

  LDAPMod secretMod;
  secretMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  secretMod.mod_type = const_cast<char *>("radiusClientSecret");
  secretMod.mod_vals.modv_strvals = new char *[2];
  secretMod.mod_vals.modv_strvals[0] = const_cast<char *>("default");
  secretMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(secretMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[2];
  objectClassMod.mod_vals.modv_strvals[0] = const_cast<char *>("radiusClient");
  objectClassMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(objectClassMod);

  // Optional attributes
  if (shortname.has_value()) {
    LDAPMod shortnameMod;
    shortnameMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    shortnameMod.mod_type = const_cast<char *>("radiusClientShortname");
    shortnameMod.mod_vals.modv_strvals = new char *[2];
    shortnameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(shortname->c_str());
    shortnameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(shortnameMod);
  }
  if (virtualServer.has_value()) {
    LDAPMod virtualServerMod;
    virtualServerMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    virtualServerMod.mod_type = const_cast<char *>("radiusClientVirtualServer");
    virtualServerMod.mod_vals.modv_strvals = new char *[2];
    virtualServerMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(virtualServer->c_str());
    virtualServerMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(virtualServerMod);
  }
  if (type.has_value()) {
    LDAPMod typeMod;
    typeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    typeMod.mod_type = const_cast<char *>("radiusClientType");
    typeMod.mod_vals.modv_strvals = new char *[2];
    typeMod.mod_vals.modv_strvals[0] = const_cast<char *>(type->c_str());
    typeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(typeMod);
  }
  if (requireMa.has_value()) {
    LDAPMod requireMaMod;
    requireMaMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    requireMaMod.mod_type = const_cast<char *>("radiusClientRequireMa");
    requireMaMod.mod_vals.modv_strvals = new char *[2];
    requireMaMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(requireMa.value() ? "TRUE" : "FALSE");
    requireMaMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(requireMaMod);
  }
  if (comment.has_value()) {
    LDAPMod commentMod;
    commentMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    commentMod.mod_type = const_cast<char *>("radiusClientComment");
    commentMod.mod_vals.modv_strvals = new char *[2];
    commentMod.mod_vals.modv_strvals[0] = const_cast<char *>(comment->c_str());
    commentMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(commentMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(clientDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Client created successfully!");
  return true;
}

bool FreeRADIUSManager::updateClient(
    const std::string &clientName, const std::string &baseDN,
    const std::optional<std::string> &secret,
    const std::optional<std::string> &shortname,
    const std::optional<std::string> &virtualServer,
    const std::optional<std::string> &type,
    const std::optional<bool> &requireMa,
    const std::optional<std::string> &comment) {
  std::string clientDN = getClientDN(clientName, baseDN);

  console::e("Updating FreeRADIUS client:");
  console::e("  Client Name: {}", clientName);
  console::e("  Client DN: {}", clientDN);

  // Create LDAP mods for radiusClient object class
  std::vector<LDAPMod> mods;

  // Optional attributes
  if (secret.has_value()) {
    LDAPMod secretMod;
    secretMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    secretMod.mod_type = const_cast<char *>("radiusClientSecret");
    secretMod.mod_vals.modv_strvals = new char *[2];
    secretMod.mod_vals.modv_strvals[0] = const_cast<char *>(secret->c_str());
    secretMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(secretMod);
  }
  if (shortname.has_value()) {
    LDAPMod shortnameMod;
    shortnameMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    shortnameMod.mod_type = const_cast<char *>("radiusClientShortname");
    shortnameMod.mod_vals.modv_strvals = new char *[2];
    shortnameMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(shortname->c_str());
    shortnameMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(shortnameMod);
  }
  if (virtualServer.has_value()) {
    LDAPMod virtualServerMod;
    virtualServerMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    virtualServerMod.mod_type = const_cast<char *>("radiusClientVirtualServer");
    virtualServerMod.mod_vals.modv_strvals = new char *[2];
    virtualServerMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(virtualServer->c_str());
    virtualServerMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(virtualServerMod);
  }
  if (type.has_value()) {
    LDAPMod typeMod;
    typeMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    typeMod.mod_type = const_cast<char *>("radiusClientType");
    typeMod.mod_vals.modv_strvals = new char *[2];
    typeMod.mod_vals.modv_strvals[0] = const_cast<char *>(type->c_str());
    typeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(typeMod);
  }
  if (requireMa.has_value()) {
    LDAPMod requireMaMod;
    requireMaMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    requireMaMod.mod_type = const_cast<char *>("radiusClientRequireMa");
    requireMaMod.mod_vals.modv_strvals = new char *[2];
    requireMaMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(requireMa.value() ? "TRUE" : "FALSE");
    requireMaMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(requireMaMod);
  }
  if (comment.has_value()) {
    LDAPMod commentMod;
    commentMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    commentMod.mod_type = const_cast<char *>("radiusClientComment");
    commentMod.mod_vals.modv_strvals = new char *[2];
    commentMod.mod_vals.modv_strvals[0] = const_cast<char *>(comment->c_str());
    commentMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(commentMod);
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

  if (!m_connection.modifyEntry(clientDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Client updated successfully!");
  return true;
}

bool FreeRADIUSManager::deleteClient(const std::string &clientName,
                                     const std::string &baseDN) {
  std::string clientDN = getClientDN(clientName, baseDN);

  console::e("Deleting FreeRADIUS client:");
  console::e("  Client Name: {}", clientName);
  console::e("  Client DN: {}", clientDN);

  if (!m_connection.deleteEntry(clientDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("Client deleted successfully!");
  return true;
}

bool FreeRADIUSManager::listClients(const std::string &baseDN) {
  console::e("Listing FreeRADIUS clients:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=FreeRADIUSClient)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No clients found.");
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

bool FreeRADIUSManager::createUser(
    const std::string &username, const std::string &baseDN,
    const std::optional<std::string> &password,
    const std::optional<std::string> &serviceType,
    const std::optional<std::string> &framedProtocol) {
  std::string userDN = getUserDN(username, baseDN);

  console::e("Creating FreeRADIUS user:");
  console::e("  Username: {}", username);
  console::e("  User DN: {}", userDN);

  // Create LDAP mods for radiusObjectProfile and radiusprofile object classes
  std::vector<LDAPMod> mods;

  // Required attributes
  LDAPMod cnMod;
  cnMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  cnMod.mod_type = const_cast<char *>("cn");
  cnMod.mod_vals.modv_strvals = new char *[2];
  cnMod.mod_vals.modv_strvals[0] = const_cast<char *>(username.c_str());
  cnMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(cnMod);

  LDAPMod objectClassMod;
  objectClassMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  objectClassMod.mod_type = const_cast<char *>("objectClass");
  objectClassMod.mod_vals.modv_strvals = new char *[2];
  objectClassMod.mod_vals.modv_strvals[0] =
      const_cast<char *>("radiusObjectProfile");
  objectClassMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(objectClassMod);

  LDAPMod profileMod;
  profileMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
  profileMod.mod_type = const_cast<char *>("objectClass");
  profileMod.mod_vals.modv_strvals = new char *[2];
  profileMod.mod_vals.modv_strvals[0] = const_cast<char *>("radiusprofile");
  profileMod.mod_vals.modv_strvals[1] = nullptr;
  mods.push_back(profileMod);

  // Optional attributes
  if (password.has_value()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    passwordMod.mod_type = const_cast<char *>("userPassword");
    passwordMod.mod_vals.modv_strvals = new char *[2];
    passwordMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(password->c_str());
    passwordMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (serviceType.has_value()) {
    LDAPMod serviceTypeMod;
    serviceTypeMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    serviceTypeMod.mod_type = const_cast<char *>("radiusServiceType");
    serviceTypeMod.mod_vals.modv_strvals = new char *[2];
    serviceTypeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(serviceType->c_str());
    serviceTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(serviceTypeMod);
  }
  if (framedProtocol.has_value()) {
    LDAPMod framedProtocolMod;
    framedProtocolMod.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    framedProtocolMod.mod_type = const_cast<char *>("radiusFramedProtocol");
    framedProtocolMod.mod_vals.modv_strvals = new char *[2];
    framedProtocolMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(framedProtocol->c_str());
    framedProtocolMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(framedProtocolMod);
  }

  // Convert mods to LDAPMod**
  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.addEntry(userDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("User created successfully!");
  return true;
}

bool FreeRADIUSManager::updateUser(
    const std::string &username, const std::string &baseDN,
    const std::optional<std::string> &password,
    const std::optional<std::string> &serviceType,
    const std::optional<std::string> &framedProtocol) {
  std::string userDN = getUserDN(username, baseDN);

  console::e("Updating FreeRADIUS user:");
  console::e("  Username: {}", username);
  console::e("  User DN: {}", userDN);

  std::vector<LDAPMod> mods;

  if (password.has_value()) {
    LDAPMod passwordMod;
    passwordMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    passwordMod.mod_type = const_cast<char *>("userPassword");
    passwordMod.mod_vals.modv_strvals = new char *[2];
    passwordMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(password->c_str());
    passwordMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(passwordMod);
  }
  if (serviceType.has_value()) {
    LDAPMod serviceTypeMod;
    serviceTypeMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    serviceTypeMod.mod_type = const_cast<char *>("radiusServiceType");
    serviceTypeMod.mod_vals.modv_strvals = new char *[2];
    serviceTypeMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(serviceType->c_str());
    serviceTypeMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(serviceTypeMod);
  }
  if (framedProtocol.has_value()) {
    LDAPMod framedProtocolMod;
    framedProtocolMod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    framedProtocolMod.mod_type = const_cast<char *>("radiusFramedProtocol");
    framedProtocolMod.mod_vals.modv_strvals = new char *[2];
    framedProtocolMod.mod_vals.modv_strvals[0] =
        const_cast<char *>(framedProtocol->c_str());
    framedProtocolMod.mod_vals.modv_strvals[1] = nullptr;
    mods.push_back(framedProtocolMod);
  }

  if (mods.empty()) {
    console::e("No attributes to update.");
    return false;
  }

  std::vector<LDAPMod *> modPtrs;
  for (auto &mod : mods) {
    modPtrs.push_back(&mod);
  }
  modPtrs.push_back(nullptr);

  if (!m_connection.modifyEntry(userDN, modPtrs.data())) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("User updated successfully!");
  return true;
}

bool FreeRADIUSManager::deleteUser(const std::string &username,
                                   const std::string &baseDN) {
  std::string userDN = getUserDN(username, baseDN);

  console::e("Deleting FreeRADIUS user:");
  console::e("  Username: {}", username);
  console::e("  User DN: {}", userDN);

  if (!m_connection.deleteEntry(userDN)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  console::e("User deleted successfully!");
  return true;
}

bool FreeRADIUSManager::listUsers(const std::string &baseDN) {
  console::e("Listing FreeRADIUS users:");
  console::e("Base DN: {}", baseDN);

  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=radiusObjectProfile)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No users found.");
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

std::string FreeRADIUSManager::getClientDN(const std::string &clientName,
                                           const std::string &baseDN) const {
  return "cn=" + clientName + "," + baseDN;
}

std::string FreeRADIUSManager::getUserDN(const std::string &username,
                                         const std::string &baseDN) const {
  return "cn=" + username + "," + baseDN;
}
