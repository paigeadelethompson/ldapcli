#ifndef POWERDNS_MANAGER_HPP
#define POWERDNS_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <getopt.h>
#include <optional>
#include <string>
#include <vector>

class PowerDNSManager : public LDAPManagerBase {
public:
  PowerDNSManager(LDAPConnection &connection);

  bool listZones(const std::string &baseDN);
  bool createZone(const std::string &zoneName, const std::string &baseDN,
                  const std::optional<std::string> &type);
  bool updateZone(const std::string &zoneName, const std::string &baseDN,
                  const std::optional<std::string> &notifiedSerial,
                  const std::optional<std::string> &lastCheck,
                  const std::optional<std::string> &master);
  bool deleteZone(const std::string &zoneName, const std::string &baseDN);
  bool addRecord(const std::string &zoneName, const std::string &baseDN,
                 const std::string &recordName, const std::string &recordType,
                 const std::string &recordValue,
                 const std::optional<int> &ttl);
  bool updateRecord(const std::string &zoneName, const std::string &baseDN,
                    const std::string &recordName,
                    const std::string &recordType,
                    const std::optional<std::string> &recordValue,
                    const std::optional<int> &ttl);
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
  std::string recordTypeToAttribute(const std::string &recordType) const;
  bool validateZoneName(const std::string &zoneName) const;
  void printRecordTypeOptions() const;
};

#endif