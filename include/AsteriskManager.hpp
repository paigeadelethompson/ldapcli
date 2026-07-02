#ifndef ASTERISK_MANAGER_HPP
#define ASTERISK_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <string>
#include <vector>

class AsteriskManager : public LDAPManagerBase {
public:
  AsteriskManager(LDAPConnection &connection);

  bool listAccounts(const std::string &baseDN);
  bool createAccount(const std::string &accountName, const std::string &baseDN,
                     const std::string &secret, const std::string &callerId,
                     const std::string &mailbox);
  bool updateAccount(const std::string &accountName, const std::string &baseDN);
  bool deleteAccount(const std::string &accountName, const std::string &baseDN);
  bool listVoicemailBoxes(const std::string &baseDN);
  bool createVoicemailBox(const std::string &mailbox, const std::string &baseDN,
                          const std::string &password,
                          const std::string &fullname,
                          const std::string &email);
  bool deleteVoicemailBox(const std::string &mailbox,
                          const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getAccountDN(const std::string &accountName,
                           const std::string &baseDN) const;
  std::string getMailboxDN(const std::string &mailbox,
                           const std::string &baseDN) const;
};

#endif