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
#include "Config.hpp"
#include <fstream>
#include <sstream>

Config &Config::getInstance() {
  static Config instance;
  return instance;
}

void Config::load(const std::string &configPath) {
  // Check for config file in order:
  // 1. ./config/ldapcli.conf
  // 2. /etc/ldapcli/config
  // 3. Use defaults if neither exists

  std::string configFilePath;

  // First, check if configPath was provided
  if (!configPath.empty()) {
    configFilePath = configPath;
  }
  // Check ./config/ldapcli.conf
  else {
    std::ifstream localConfig("./config/ldapcli.conf");
    if (localConfig.is_open()) {
      configFilePath = "./config/ldapcli.conf";
      localConfig.close();
    }
    // Check /etc/ldapcli/config
    else {
      std::ifstream systemConfig("/etc/ldapcli/config");
      if (systemConfig.is_open()) {
        configFilePath = "/etc/ldapcli/config";
        systemConfig.close();
      }
    }
  }

  std::ifstream file(configFilePath);

  if (!file.is_open()) {
    // Use default values if config file doesn't exist
    m_config["baseDN"] = "dc=example,dc=com";
    m_config["ldapURI"] = "ldap://localhost:389";
    m_config["bindDN"] = "";
    m_config["bindPassword"] = "";
    return;
  }

  std::string line;
  while (std::getline(file, line)) {
    // Skip comments and empty lines
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Parse key=value
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      // Trim whitespace
      key.erase(0, key.find_first_not_of(" \t"));
      key.erase(key.find_last_not_of(" \t") + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);

      m_config[key] = value;
    }
  }
}

std::string Config::getBaseDN() const {
  return m_config.count("baseDN") ? m_config.at("baseDN") : "dc=example,dc=com";
}

std::string Config::getLDAPURI() const {
  return m_config.count("ldapURI") ? m_config.at("ldapURI")
                                   : "ldap://localhost:389";
}

std::string Config::getBindDN() const {
  return m_config.count("bindDN") ? m_config.at("bindDN") : "";
}

std::string Config::getBindPassword() const {
  return m_config.count("bindPassword") ? m_config.at("bindPassword") : "";
}