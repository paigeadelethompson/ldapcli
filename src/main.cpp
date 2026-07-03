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
#include "AsteriskManager.hpp"
#include "Config.hpp"
#include "Console.hpp"
#include "FreeRADIUSManager.hpp"
#include "KerberosManager.hpp"
#include "LDAPConnection.hpp"
#include "OpenDKIMManager.hpp"
#include "OpenLDAPManager.hpp"
#include "PowerDNSManager.hpp"
#include "SendmailManager.hpp"
#include <memory>
#include <stdexcept>

void printUsage(const char *programName) {
  console::e("{}{}{} - LDAP Service Management Tool{}", console::colors::BOLD,
             console::colors::CYAN, "LDAP CLI", console::colors::RESET);
  console::e("Usage: {} <{}service{}> [{}command{}] [{}arguments...{}]",
             programName, console::colors::ITALIC, console::colors::RESET,
             console::colors::ITALIC, console::colors::RESET,
             console::colors::ITALIC, console::colors::RESET);
  console::e("");
  console::e("{}Services:{}", console::colors::BOLD, console::colors::RESET);
  console::e("  {}dns{}          - PowerDNS management", console::colors::GREEN,
             console::colors::RESET);
  console::e("  {}asterisk{}     - Asterisk management", console::colors::GREEN,
             console::colors::RESET);
  console::e("  {}freeradius{}   - FreeRADIUS management",
             console::colors::GREEN, console::colors::RESET);
  console::e("  {}kerberos{}     - Kerberos management", console::colors::GREEN,
             console::colors::RESET);
  console::e("  {}opendkim{}     - OpenDKIM management", console::colors::GREEN,
             console::colors::RESET);
  console::e("  {}sendmail{}     - Sendmail management", console::colors::GREEN,
             console::colors::RESET);
  console::e("  {}openldap{}     - OpenLDAP management", console::colors::GREEN,
             console::colors::RESET);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  std::string service = argv[1];

  try {
    // Load configuration
    Config::getInstance().load();

    // Connect to LDAP
    LDAPConnection connection;
    std::string ldapURI = Config::getInstance().getLDAPURI();
    std::string bindDN = Config::getInstance().getBindDN();
    std::string bindPassword = Config::getInstance().getBindPassword();

    if (!connection.connect(ldapURI, bindDN, bindPassword)) {
      console::e("{}Failed to connect to LDAP: {}{}", console::colors::BOLD,
                 console::colors::RED, connection.getError());
      return 1;
    }

    // Create appropriate manager
    std::unique_ptr<LDAPManagerBase> manager;

    if (service == "dns") {
      manager = std::make_unique<PowerDNSManager>(connection);
    } else if (service == "asterisk") {
      manager = std::make_unique<AsteriskManager>(connection);
    } else if (service == "freeradius") {
      manager = std::make_unique<FreeRADIUSManager>(connection);
    } else if (service == "kerberos") {
      manager = std::make_unique<KerberosManager>(connection);
    } else if (service == "opendkim") {
      manager = std::make_unique<OpenDKIMManager>(connection);
    } else if (service == "sendmail") {
      manager = std::make_unique<SendmailManager>(connection);
    } else if (service == "openldap") {
      manager = std::make_unique<OpenLDAPManager>(connection);
    } else {
      console::e("{}Unknown service: {}{}", console::colors::BOLD,
                 console::colors::RED, service);
      printUsage(argv[0]);
      return 1;
    }

    // Execute command
    try {
      if (!manager->execute(argc, argv)) {
        return 1;
      }
    } catch (const std::exception &e) {
      console::e("{}Error: {}{}", console::colors::BOLD, console::colors::RED,
                 e.what());
      return 1;
    }

  } catch (const std::exception &e) {
    console::e("{}Error: {}{}", console::colors::BOLD, console::colors::RED,
               e.what());
    return 1;
  }

  return 0;
}