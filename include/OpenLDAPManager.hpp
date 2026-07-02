#ifndef OPENLDAP_MANAGER_HPP
#define OPENLDAP_MANAGER_HPP

#include "LDAPConnection.hpp"
#include "LDAPManagerBase.hpp"
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
  bool createOrganizationalUnit(const std::string &ouName,
                                const std::string &baseDN,
                                const std::string &password = "",
                                const std::string &fullname = "",
                                const std::string &email = "");
  bool deleteOrganizationalUnit(const std::string &ouName,
                                const std::string &baseDN);
  bool updateOrganizationalUnit(const std::string &ouName,
                                const std::string &baseDN);
  bool listPeople(const std::string &baseDN);
  bool createPerson(
      const std::string &personName, const std::string &baseDN,
      const std::string &uid = "", const std::string &givenName = "",
      const std::string &sn = "", const std::string &mail = "",
      const std::string &displayName = "",
      const std::string &employeeNumber = "",
      const std::string &employeeType = "",
      const std::string &departmentNumber = "", const std::string &mobile = "",
      const std::string &homePhone = "", const std::string &pager = "",
      const std::string &title = "", const std::string &telephoneNumber = "",
      const std::string &street = "", const std::string &postalCode = "",
      const std::string &l = "", const std::string &st = "",
      const std::string &c = "");
  bool updatePerson(
      const std::string &personName, const std::string &baseDN,
      const std::string &uid = "", const std::string &givenName = "",
      const std::string &sn = "", const std::string &mail = "",
      const std::string &displayName = "",
      const std::string &employeeNumber = "",
      const std::string &employeeType = "",
      const std::string &departmentNumber = "", const std::string &mobile = "",
      const std::string &homePhone = "", const std::string &pager = "",
      const std::string &title = "", const std::string &telephoneNumber = "",
      const std::string &street = "", const std::string &postalCode = "",
      const std::string &l = "", const std::string &st = "",
      const std::string &c = "");
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