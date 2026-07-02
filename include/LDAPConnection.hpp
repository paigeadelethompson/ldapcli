#ifndef LDAP_CONNECTION_HPP
#define LDAP_CONNECTION_HPP

#include <ldap.h>
#include <memory>
#include <string>
#include <vector>

class LDAPConnection {
public:
  LDAPConnection();
  ~LDAPConnection();

  bool connect(const std::string &uri, const std::string &bindDN = "",
               const std::string &password = "");
  void disconnect();
  bool isConnected() const;

  LDAP *getHandle() const;

  std::string getError() const;

  bool addEntry(const std::string &dn, LDAPMod **mods);
  bool modifyEntry(const std::string &dn, LDAPMod **mods);
  bool deleteEntry(const std::string &dn);
  bool search(
      const std::string &base, int scope, const std::string &filter,
      std::vector<std::vector<std::pair<std::string, std::string>>> &results);
  bool bind(const std::string &bindDN, const std::string &password);
  bool unbind();

private:
  LDAP *m_ldap;
  std::string m_error;
};

#endif