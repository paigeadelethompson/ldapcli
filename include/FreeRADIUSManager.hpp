#ifndef FREERADIUS_MANAGER_HPP
#define FREERADIUS_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <string>
#include <vector>

class FreeRADIUSManager : public LDAPManagerBase {
public:
  FreeRADIUSManager(LDAPConnection &connection);

  bool listClients(const std::string &baseDN);
  bool createClient(const std::string &clientName, const std::string &baseDN,
                    const std::string &secret, const std::string &shortname,
                    const std::string &type);
  bool updateClient(const std::string &clientName, const std::string &baseDN);
  bool deleteClient(const std::string &clientName, const std::string &baseDN);
  bool listUsers(const std::string &baseDN);
  bool createUser(const std::string &username, const std::string &baseDN,
                  const std::string &password, const std::string &serviceType,
                  const std::string &framedProtocol);
  bool updateUser(const std::string &username, const std::string &baseDN);
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