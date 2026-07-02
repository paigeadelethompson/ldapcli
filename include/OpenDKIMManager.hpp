#ifndef OPENDKIM_MANAGER_HPP
#define OPENDKIM_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <optional>
#include <string>
#include <vector>

class OpenDKIMManager : public LDAPManagerBase {
public:
  OpenDKIMManager(LDAPConnection &connection);

  bool listIdentities(const std::string &baseDN);
  bool createIdentity(const std::string &identity, const std::string &baseDN,
                      const std::optional<std::string> &selector,
                      const std::optional<std::string> &key,
                      const std::optional<std::string> &domain);
  bool updateIdentity(const std::string &identity, const std::string &baseDN,
                      const std::optional<std::string> &selector,
                      const std::optional<std::string> &key,
                      const std::optional<std::string> &domain);
  bool deleteIdentity(const std::string &identity, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getIdentityDN(const std::string &identity,
                            const std::string &baseDN) const;
};

#endif