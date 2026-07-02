#ifndef FREERADIUS_MANAGER_HPP
#define FREERADIUS_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <optional>
#include <string>
#include <vector>

class FreeRADIUSManager : public LDAPManagerBase {
public:
  FreeRADIUSManager(LDAPConnection &connection);

  bool listClients(const std::string &baseDN);
  bool createClient(const std::string &clientName, const std::string &baseDN,
                    const std::optional<std::string> &secret,
                    const std::optional<std::string> &shortname,
                    const std::optional<std::string> &virtualServer,
                    const std::optional<std::string> &type,
                    const std::optional<bool> &requireMa,
                    const std::optional<std::string> &comment);
  bool updateClient(const std::string &clientName, const std::string &baseDN,
                    const std::optional<std::string> &secret,
                    const std::optional<std::string> &shortname,
                    const std::optional<std::string> &virtualServer,
                    const std::optional<std::string> &type,
                    const std::optional<bool> &requireMa,
                    const std::optional<std::string> &comment);
  bool deleteClient(const std::string &clientName, const std::string &baseDN);
  bool listUsers(const std::string &baseDN);
  bool createUser(const std::string &username, const std::string &baseDN,
                  const std::optional<std::string> &password,
                  const std::optional<std::string> &serviceType,
                  const std::optional<std::string> &framedProtocol);
  bool updateUser(const std::string &username, const std::string &baseDN,
                  const std::optional<std::string> &password,
                  const std::optional<std::string> &serviceType,
                  const std::optional<std::string> &framedProtocol);
  bool deleteUser(const std::string &username, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getClientDN(const std::string &clientName,
                          const std::string &baseDN) const;
  std::string getUserDN(const std::string &username,
                        const std::string &baseDN) const;
};

#endif