#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>

class Config {
public:
  static Config &getInstance();

  void load(const std::string &configPath = "config/ldapcli.conf");

  std::string getBaseDN() const;
  std::string getLDAPURI() const;
  std::string getBindDN() const;
  std::string getBindPassword() const;

private:
  Config() = default;
  ~Config() = default;

  std::map<std::string, std::string> m_config;
};

#endif