#ifndef LDAP_MANAGER_BASE_HPP
#define LDAP_MANAGER_BASE_HPP

#include "LDAPConnection.hpp"
#include <string>
#include <vector>

class LDAPManagerBase {
public:
  virtual ~LDAPManagerBase() = default;

  virtual bool execute(int argc, char *argv[]) = 0;
  virtual void printUsage() const = 0;
  virtual std::string getServiceName() const = 0;
};

#endif