/**
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, RavenHammer Research Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
                 const std::string &recordValue, const std::optional<int> &ttl);
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