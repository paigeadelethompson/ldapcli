#ifndef POWERDNS_MANAGER_HPP
#define POWERDNS_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <string>
#include <vector>

class PowerDNSManager : public LDAPManagerBase {
public:
  PowerDNSManager(LDAPConnection &connection);

  bool listZones(const std::string &baseDN);
  bool createZone(const std::string &zoneName, const std::string &baseDN,
                  const std::string &type = "master");
  bool updateZone(const std::string &zoneName, const std::string &baseDN);
  bool deleteZone(const std::string &zoneName, const std::string &baseDN);
  bool addRecord(const std::string &zoneName, const std::string &baseDN,
                 const std::string &recordName, const std::string &recordType,
                 const std::string &recordValue, int ttl = 3600);
  bool deleteRecord(const std::string &zoneName, const std::string &baseDN,
                    const std::string &recordName,
                    const std::string &recordType);
  bool listRecords(const std::string &zoneName, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getZoneDN(const std::string &zoneName,
                        const std::string &baseDN) const;
  bool validateZoneName(const std::string &zoneName) const;
};

#endif