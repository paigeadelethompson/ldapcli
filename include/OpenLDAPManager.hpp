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
#ifndef OPENLDAP_MANAGER_HPP
#define OPENLDAP_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
#include <optional>
#include <string>
#include <vector>

class OpenLDAPManager : public LDAPManagerBase {
public:
  OpenLDAPManager(LDAPConnection &connection);

  bool listOrganizations(const std::string &baseDN);
  bool createOrganization(const std::string &orgName,
                          const std::string &baseDN);
  bool updateOrganization(const std::string &orgName,
                          const std::string &baseDN);
  bool deleteOrganization(const std::string &orgName,
                          const std::string &baseDN);
  bool listOrganizationalUnits(const std::string &baseDN);
  bool
  createOrganizationalUnit(const std::string &ouName, const std::string &baseDN,
                           const std::optional<std::string> &telephoneNumber,
                           const std::optional<std::string> &street,
                           const std::optional<std::string> &postalCode,
                           const std::optional<std::string> &st,
                           const std::optional<std::string> &l,
                           const std::optional<std::string> &description);
  bool deleteOrganizationalUnit(const std::string &ouName,
                                const std::string &baseDN);
  bool updateOrganizationalUnit(const std::string &ouName,
                                const std::string &baseDN);
  bool listPeople(const std::string &baseDN);
  bool createPerson(const std::string &personName, const std::string &baseDN,
                    const std::optional<std::string> &uid,
                    const std::optional<std::string> &givenName,
                    const std::optional<std::string> &sn,
                    const std::optional<std::string> &mail,
                    const std::optional<std::string> &displayName,
                    const std::optional<std::string> &employeeNumber,
                    const std::optional<std::string> &employeeType,
                    const std::optional<std::string> &departmentNumber,
                    const std::optional<std::string> &mobile,
                    const std::optional<std::string> &homePhone,
                    const std::optional<std::string> &pager,
                    const std::optional<std::string> &title,
                    const std::optional<std::string> &telephoneNumber,
                    const std::optional<std::string> &street,
                    const std::optional<std::string> &postalCode,
                    const std::optional<std::string> &l,
                    const std::optional<std::string> &st,
                    const std::optional<std::string> &c);
  bool updatePerson(const std::string &personName, const std::string &baseDN,
                    const std::optional<std::string> &uid,
                    const std::optional<std::string> &givenName,
                    const std::optional<std::string> &sn,
                    const std::optional<std::string> &mail,
                    const std::optional<std::string> &displayName,
                    const std::optional<std::string> &employeeNumber,
                    const std::optional<std::string> &employeeType,
                    const std::optional<std::string> &departmentNumber,
                    const std::optional<std::string> &mobile,
                    const std::optional<std::string> &homePhone,
                    const std::optional<std::string> &pager,
                    const std::optional<std::string> &title,
                    const std::optional<std::string> &telephoneNumber,
                    const std::optional<std::string> &street,
                    const std::optional<std::string> &postalCode,
                    const std::optional<std::string> &l,
                    const std::optional<std::string> &st,
                    const std::optional<std::string> &c);
  bool deletePerson(const std::string &personName, const std::string &baseDN);

  void printUsage() const override;
  std::string getServiceName() const override;

  bool execute(int argc, char *argv[]) override;

private:
  LDAPConnection &m_connection;

  std::string getOrganizationDN(const std::string &orgName,
                                const std::string &baseDN) const;
  std::string getOrganizationalUnitDN(const std::string &ouName,
                                      const std::string &baseDN) const;
  std::string getPersonDN(const std::string &personName,
                          const std::string &baseDN) const;
};

#endif
