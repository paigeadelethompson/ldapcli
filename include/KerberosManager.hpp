#ifndef KERBEROS_MANAGER_HPP
#define KERBEROS_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <optional>
#include <string>
#include <vector>

class KerberosManager : public LDAPManagerBase {
public:
  KerberosManager(LDAPConnection &connection);

  bool listPrincipals(const std::string &baseDN);
  bool createPrincipal(
      const std::string &principal, const std::string &baseDN,
      const std::optional<std::string> &password,
      const std::optional<std::string> &fullname,
      const std::optional<std::string> &email,
      const std::optional<std::string> &canonicalName,
      const std::optional<std::string> &principalType,
      const std::optional<std::string> &principalExpiration,
      const std::optional<std::string> &passwordExpiration,
      const std::optional<std::string> &ticketFlags,
      const std::optional<std::string> &maxTicketLife,
      const std::optional<std::string> &maxRenewableAge,
      const std::optional<std::string> &lastPwdChange,
      const std::optional<std::string> &lastSuccessfulAuth,
      const std::optional<std::string> &lastFailedAuth,
      const std::optional<std::string> &loginFailedCount,
      const std::optional<std::string> &principalAliases,
      const std::optional<std::string> &allowedToDelegateTo,
      const std::optional<std::string> &principalAuthInd);
  bool updatePrincipal(
      const std::string &principal, const std::string &baseDN,
      const std::optional<std::string> &canonicalName,
      const std::optional<std::string> &principalType,
      const std::optional<std::string> &principalExpiration,
      const std::optional<std::string> &passwordExpiration,
      const std::optional<std::string> &ticketFlags,
      const std::optional<std::string> &maxTicketLife,
      const std::optional<std::string> &maxRenewableAge,
      const std::optional<std::string> &lastPwdChange,
      const std::optional<std::string> &lastSuccessfulAuth,
      const std::optional<std::string> &lastFailedAuth,
      const std::optional<std::string> &loginFailedCount,
      const std::optional<std::string> &principalAliases,
      const std::optional<std::string> &allowedToDelegateTo,
      const std::optional<std::string> &principalAuthInd);
  bool deletePrincipal(const std::string &principal, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getPrincipalDN(const std::string &principal,
                             const std::string &baseDN) const;
};

#endif