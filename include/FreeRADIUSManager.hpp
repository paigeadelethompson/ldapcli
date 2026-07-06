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