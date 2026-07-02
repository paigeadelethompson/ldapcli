#include "AsteriskManager.hpp"
#include "Config.hpp"
#include "FreeRADIUSManager.hpp"
#include "KerberosManager.hpp"
#include "LDAPConnection.hpp"
#include "OpenDKIMManager.hpp"
#include "OpenLDAPManager.hpp"
#include "PowerDNSManager.hpp"
#include "SendmailManager.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

void printUsage(const char *programName) {
  std::cout << "LDAP CLI - LDAP Service Management Tool" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Usage: " << programName << " <service> [command] [arguments...]"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Services:" << std::endl;
  std::cout << "  dns          - PowerDNS management" << std::endl;
  std::cout << "  asterisk     - Asterisk management" << std::endl;
  std::cout << "  freeradius   - FreeRADIUS management" << std::endl;
  std::cout << "  kerberos     - Kerberos management" << std::endl;
  std::cout << "  opendkim     - OpenDKIM management" << std::endl;
  std::cout << "  sendmail     - Sendmail management" << std::endl;
  std::cout << "  openldap     - OpenLDAP management" << std::endl;
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
      std::cerr << "Failed to connect to LDAP: " << connection.getError()
                << std::endl;
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
      std::cerr << "Unknown service: " << service << std::endl;
      printUsage(argv[0]);
      return 1;
    }

    // Execute command
    try {
      if (!manager->execute(argc, argv)) {
        return 1;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}