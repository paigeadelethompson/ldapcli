#include "LDAPSchemaManager.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

LDAPSchemaManager::LDAPSchemaManager(LDAPConnection &connection)
    : m_connection(connection) {
  loadServiceSchemas();
}

bool LDAPSchemaManager::loadServiceSchemas() {
  // Load schemas from schemas directory
  // This will be populated based on the actual schema files
  m_serviceSchemas["asterisk"] = "asterisk";
  m_serviceSchemas["freeradius"] = "freeradius";
  m_serviceSchemas["freeradius-clients"] = "freeradius-clients";
  m_serviceSchemas["kerberos"] = "kerberos";
  m_serviceSchemas["opendkim"] = "opendkim";
  m_serviceSchemas["powerdns"] = "powerdns";
  m_serviceSchemas["sendmail"] = "sendmail";
  m_serviceSchemas["openldap"] = "openldap";
  m_serviceSchemas["core"] = "core";
  m_serviceSchemas["inetorgperson"] = "inetorgperson";
  return true;
}

std::string
LDAPSchemaManager::getSchemaPath(const std::string &serviceName) const {
  // Check local ./schemas directory first
  std::string localPath = "./schemas/" + serviceName;

  // Check for .schema file first
  std::string schemaFile = localPath + ".schema";
  std::ifstream check(schemaFile);
  if (check.good()) {
    return schemaFile;
  }

  // Check for .ldif file
  std::string ldifFile = localPath + ".ldif";
  check.open(ldifFile);
  if (check.good()) {
    return ldifFile;
  }

  // Check installed schemas directory
  const char *installPrefix = std::getenv("CMAKE_INSTALL_PREFIX");
  std::string installPath = installPrefix ? installPrefix : "/usr/local";
  std::string installedPath =
      installPath + "/share/ldapcli/schemas/" + serviceName;

  // Check for .schema file
  schemaFile = installedPath + ".schema";
  check.open(schemaFile);
  if (check.good()) {
    return schemaFile;
  }

  // Check for .ldif file
  ldifFile = installedPath + ".ldif";
  check.open(ldifFile);
  if (check.good()) {
    return ldifFile;
  }

  return "";
}

bool LDAPSchemaManager::listServices() {
  std::cout << "Available LDAP Services:" << std::endl;
  std::cout << "-----------------------" << std::endl;

  for (const auto &[name, schema] : m_serviceSchemas) {
    std::cout << "  - " << name << std::endl;
  }

  return true;
}

bool LDAPSchemaManager::createService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Creating service base entry for: " << serviceName << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  // Create base entry with appropriate object class
  // This is a placeholder - actual implementation depends on schema
  std::string entryDN = baseDN + "," + serviceName;

  // TODO: Implement based on specific schema requirements
  std::cout << "Entry DN: " << entryDN << std::endl;

  return true;
}

bool LDAPSchemaManager::updateService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Updating service: " << serviceName << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  // TODO: Implement update logic
  return true;
}

bool LDAPSchemaManager::deleteService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Deleting service: " << serviceName << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  // TODO: Implement delete logic
  return true;
}

bool LDAPSchemaManager::listServiceEntries(const std::string &serviceName,
                                           const std::string &baseDN) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Listing entries for service: " << serviceName << std::endl;
  std::cout << "Base DN: " << baseDN << std::endl;

  // Search for entries
  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=*)";

  if (m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    std::cout << "Found " << results.size() << " entries:" << std::endl;

    for (size_t i = 0; i < results.size(); i++) {
      std::cout << "\nEntry " << (i + 1) << ":" << std::endl;
      for (const auto &[attr, value] : results[i]) {
        std::cout << "  " << attr << ": " << value << std::endl;
      }
    }

    return true;
  }

  std::cerr << "Error: " << m_connection.getError() << std::endl;
  return false;
}

bool LDAPSchemaManager::createServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN,
                                           const std::string &objectClass) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Creating entry for service: " << serviceName << std::endl;
  std::cout << "Entry DN: " << entryDN << std::endl;
  std::cout << "Object Class: " << objectClass << std::endl;

  // TODO: Implement based on specific schema requirements
  return true;
}

bool LDAPSchemaManager::updateServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN,
                                           const std::string &objectClass) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Updating entry for service: " << serviceName << std::endl;
  std::cout << "Entry DN: " << entryDN << std::endl;
  std::cout << "Object Class: " << objectClass << std::endl;

  // TODO: Implement update logic
  return true;
}

bool LDAPSchemaManager::deleteServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN) {
  if (!validateService(serviceName)) {
    std::cerr << "Error: Unknown service '" << serviceName << "'" << std::endl;
    return false;
  }

  std::cout << "Deleting entry for service: " << serviceName << std::endl;
  std::cout << "Entry DN: " << entryDN << std::endl;

  // TODO: Implement delete logic
  return true;
}

bool LDAPSchemaManager::validateService(const std::string &serviceName) {
  return m_serviceSchemas.find(serviceName) != m_serviceSchemas.end();
}