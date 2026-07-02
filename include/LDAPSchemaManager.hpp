#ifndef LDAP_SCHEMA_MANAGER_HPP
#define LDAP_SCHEMA_MANAGER_HPP

#include "LDAPConnection.hpp"
#include <map>
#include <string>
#include <vector>

class LDAPSchemaManager {
public:
  LDAPSchemaManager(LDAPConnection &connection);

  bool listServices();
  bool createService(const std::string &serviceName, const std::string &baseDN);
  bool updateService(const std::string &serviceName, const std::string &baseDN);
  bool deleteService(const std::string &serviceName, const std::string &baseDN);

  bool listServiceEntries(const std::string &serviceName,
                          const std::string &baseDN);
  bool createServiceEntry(const std::string &serviceName,
                          const std::string &baseDN, const std::string &entryDN,
                          const std::string &objectClass);
  bool updateServiceEntry(const std::string &serviceName,
                          const std::string &baseDN, const std::string &entryDN,
                          const std::string &objectClass);
  bool deleteServiceEntry(const std::string &serviceName,
                          const std::string &baseDN,
                          const std::string &entryDN);

  std::string getSchemaPath(const std::string &serviceName) const;

private:
  LDAPConnection &m_connection;
  std::map<std::string, std::string> m_serviceSchemas;

  bool loadServiceSchemas();
  bool validateService(const std::string &serviceName);
};

#endif