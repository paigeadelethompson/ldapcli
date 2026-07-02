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
  console::e("Available LDAP Services:");
  console::e("-----------------------");

  for (const auto &[name, schema] : m_serviceSchemas) {
    console::e("  - {}", name);
  }

  return true;
}

bool LDAPSchemaManager::createService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Creating service base entry for: {}", serviceName);
  console::e("Base DN: {}", baseDN);

  // Create base entry with appropriate object class
  // This is a placeholder - actual implementation depends on schema
  std::string entryDN = baseDN + "," + serviceName;

  // TODO: Implement based on specific schema requirements
  console::e("Entry DN: {}", entryDN);

  return true;
}

bool LDAPSchemaManager::updateService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Updating service: {}", serviceName);
  console::e("Base DN: {}", baseDN);

  // TODO: Implement update logic
  return true;
}

bool LDAPSchemaManager::deleteService(const std::string &serviceName,
                                      const std::string &baseDN) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Deleting service: {}", serviceName);
  console::e("Base DN: {}", baseDN);

  // TODO: Implement delete logic
  return true;
}

bool LDAPSchemaManager::listServiceEntries(const std::string &serviceName,
                                           const std::string &baseDN) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Listing entries for service: {}", serviceName);
  console::e("Base DN: {}", baseDN);

  // Search for entries
  std::vector<std::vector<std::pair<std::string, std::string>>> results;
  std::string filter = "(objectClass=*)";

  if (!m_connection.search(baseDN, LDAP_SCOPE_SUBTREE, filter, results)) {
    console::e("Error: {}", m_connection.getError());
    return false;
  }

  if (results.empty()) {
    console::e("No entries found.");
    return true;
  }

  // Convert results to table format for display
  std::vector<std::vector<std::string>> tableData;
  for (const auto &entry : results) {
    std::vector<std::string> rowData;
    for (const auto &[attr, value] : entry) {
      rowData.push_back(attr);
      rowData.push_back(value);
    }
    tableData.push_back(rowData);
  }

  console::printTable(tableData);
  return true;
}

bool LDAPSchemaManager::createServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN,
                                           const std::string &objectClass) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Creating entry for service: {}", serviceName);
  console::e("Entry DN: {}", entryDN);
  console::e("Object Class: {}", objectClass);

  // TODO: Implement based on specific schema requirements
  return true;
}

bool LDAPSchemaManager::updateServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN,
                                           const std::string &objectClass) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Updating entry for service: {}", serviceName);
  console::e("Entry DN: {}", entryDN);
  console::e("Object Class: {}", objectClass);

  // TODO: Implement update logic
  return true;
}

bool LDAPSchemaManager::deleteServiceEntry(const std::string &serviceName,
                                           const std::string &baseDN,
                                           const std::string &entryDN) {
  if (!validateService(serviceName)) {
    console::e("Error: Unknown service '{}'", serviceName);
    return false;
  }

  console::e("Deleting entry for service: {}", serviceName);
  console::e("Entry DN: {}", entryDN);

  // TODO: Implement delete logic
  return true;
}

bool LDAPSchemaManager::validateService(const std::string &serviceName) {
  return m_serviceSchemas.find(serviceName) != m_serviceSchemas.end();
}