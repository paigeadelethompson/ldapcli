#ifndef SENDMAIL_MANAGER_HPP
#define SENDMAIL_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <optional>
#include <string>
#include <vector>

class SendmailManager : public LDAPManagerBase {
public:
  SendmailManager(LDAPConnection &connection);

  bool listMTAs(const std::string &baseDN);
  bool createMTA(const std::string &mtaName, const std::string &baseDN,
                 const std::optional<std::string> &cluster,
                 const std::optional<std::string> &host,
                 const std::optional<std::string> &description);
  bool updateMTA(const std::string &mtaName, const std::string &baseDN,
                 const std::optional<std::string> &cluster,
                 const std::optional<std::string> &host,
                 const std::optional<std::string> &description);
  bool deleteMTA(const std::string &mtaName, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getMTADN(const std::string &mtaName,
                       const std::string &baseDN) const;
};

#endif