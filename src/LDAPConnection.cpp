#include "LDAPConnection.hpp"
#include "ca.h"
#include <cstring>
#include <stdexcept>

std::string ModsToString(const std::string &dn, LDAPMod **mods,
                         const std::string &changeType) {
  if (!mods)
    return "";

  std::string ldif = "dn: " + dn + "\nchangetype: " + changeType + "\n";

  for (int i = 0; mods[i] != nullptr; ++i) {
    LDAPMod *mod = mods[i];
    std::string op;

    // Map LDAP operation bits to LDIF change actions
    switch (mod->mod_op & ~LDAP_MOD_BVALUES) {
    case LDAP_MOD_ADD:
      op = "add";
      break;
    case LDAP_MOD_DELETE:
      op = "delete";
      break;
    case LDAP_MOD_REPLACE:
      op = "replace";
      break;
    default:
      op = "replace";
      break;
    }

    ldif += op + ": " + mod->mod_type + "\n";

    // Handle string values (mod_values) or binary values (mod_bvalues)
    if (mod->mod_op & LDAP_MOD_BVALUES) {
      if (mod->mod_bvalues) {
        for (int j = 0; mod->mod_bvalues[j] != nullptr; ++j) {
          // Note: True binary values should be Base64 encoded in production
          ldif += std::string(mod->mod_type) + ":: " +
                  std::string(mod->mod_bvalues[j]->bv_val,
                              mod->mod_bvalues[j]->bv_len) +
                  "\n";
        }
      }
    } else if (mod->mod_values) {
      for (int j = 0; mod->mod_values[j] != nullptr; ++j) {
        ldif += std::string(mod->mod_type) + ": " + mod->mod_values[j] + "\n";
      }
    }
    ldif += "-\n";
  }
  return ldif;
}

LDAPConnection::LDAPConnection() : m_ldap(nullptr) {}

LDAPConnection::~LDAPConnection() { disconnect(); }

void LDAPConnection::disconnect() {
  if (m_ldap) {
    int rc = ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
      m_error = "LDAP unbind failed: " + std::string(ldap_err2string(rc));
    }
    m_ldap = nullptr;
  }
}

bool LDAPConnection::connect(const std::string &uri, const std::string &bindDN,
                             const std::string &password) {
  /*
   * LDAP Connection Configuration
   *
   * TLS Options (from ldap.h):
   * - LDAP_OPT_X_TLS (0x6000): Base option for TLS configuration
   * - LDAP_OPT_X_TLS_CTX (0x6001): OpenSSL CTX* for TLS context
   * - LDAP_OPT_X_TLS_CACERTFILE (0x6002): CA certificate file path
   * - LDAP_OPT_X_TLS_CACERTDIR (0x6003): CA certificate directory path
   * - LDAP_OPT_X_TLS_CERTFILE (0x6004): Client certificate file path
   * - LDAP_OPT_X_TLS_KEYFILE (0x6005): Client key file path
   * - LDAP_OPT_X_TLS_REQUIRE_CERT (0x6006): Certificate requirement level
   *   - LDAP_OPT_X_TLS_NEVER (0): Never require certificate
   *   - LDAP_OPT_X_TLS_HARD (1): Always require certificate
   *   - LDAP_OPT_X_TLS_DEMAND (2): Require certificate on demand
   *   - LDAP_OPT_X_TLS_ALLOW (3): Allow certificate if provided
   *   - LDAP_OPT_X_TLS_TRY (4): Try to use certificate if available
   * - LDAP_OPT_X_TLS_PROTOCOL_MIN (0x6007): Minimum TLS protocol version
   *   - LDAP_OPT_X_TLS_PROTOCOL_TLS1_0 ((3 << 8) + 1): TLS 1.0
   *   - LDAP_OPT_X_TLS_PROTOCOL_TLS1_1 ((3 << 8) + 2): TLS 1.1
   *   - LDAP_OPT_X_TLS_PROTOCOL_TLS1_2 ((3 << 8) + 3): TLS 1.2
   *   - LDAP_OPT_X_TLS_PROTOCOL_TLS1_3 ((3 << 8) + 4): TLS 1.3
   * - LDAP_OPT_X_TLS_CIPHER_SUITE (0x6008): TLS cipher suite string
   *   - TLS_AES_256_GCM_SHA384: AES-256-GCM with SHA-384
   * - LDAP_OPT_X_TLS_RANDOM_FILE (0x6009): Random file for TLS entropy
   * - LDAP_OPT_X_TLS_SSL_CTX (0x600a): OpenSSL SSL* for TLS session
   * - LDAP_OPT_X_TLS_CRLCHECK (0x600b): Certificate revocation list check
   *   - LDAP_OPT_X_TLS_CRL_NONE (0): No CRL check
   *   - LDAP_OPT_X_TLS_CRL_PEER (1): Check peer certificate CRL
   *   - LDAP_OPT_X_TLS_CRL_ALL (2): Check all certificates CRL
   * - LDAP_OPT_X_TLS_VERSION (0x6013): Current TLS version (read-only)
   * - LDAP_OPT_X_TLS_CIPHER (0x6014): Current cipher (read-only)
   * - LDAP_OPT_X_TLS_PEERCERT (0x6015): Peer certificate (read-only)
   *
   * Protocol Versions:
   * - LDAP_VERSION1 (1): LDAPv1 (deprecated)
   * - LDAP_VERSION2 (2): LDAPv2 (deprecated)
   * - LDAP_VERSION3 (3): LDAPv3 (current standard)
   *
   * Ports:
   * - LDAP_PORT (389): Unencrypted LDAP
   * - LDAPS_PORT (636): LDAPS (LDAP over SSL/TLS)
   */

  if (m_ldap) {
    m_error = "Already connected";
    return false;
  }

  int rc = ldap_initialize(&m_ldap, uri.c_str());
  if (rc != LDAP_SUCCESS) {
    m_error = "LDAP initialize failed: " + std::string(ldap_err2string(rc));
    m_ldap = nullptr;
    return false;
  }

  int version = LDAP_VERSION3;
  rc = ldap_set_option(m_ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
  /*
   * Set LDAP protocol version to LDAPv3 (current standard)
   * LDAPv1 and LDAPv2 are deprecated and not recommended
   */
  if (rc != LDAP_SUCCESS) {
    m_error = "Failed to set LDAP version: " + std::string(ldap_err2string(rc));
    ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    m_ldap = nullptr;
    return false;
  }

  int tls_require_cert = LDAP_OPT_X_TLS_HARD;
  rc = ldap_set_option(m_ldap, LDAP_OPT_X_TLS_REQUIRE_CERT, &tls_require_cert);
  /*
   * Set TLS certificate requirement to HARD
   * Always require certificate for secure connections
   * Options: NEVER, HARD, DEMAND, ALLOW, TRY
   */
  if (rc != LDAP_SUCCESS) {
    m_error =
        "Failed to set TLS require cert: " + std::string(ldap_err2string(rc));
    ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    m_ldap = nullptr;
    return false;
  }

  int tls_protocol_min = LDAP_OPT_X_TLS_PROTOCOL_TLS1_3;
  rc = ldap_set_option(m_ldap, LDAP_OPT_X_TLS_PROTOCOL_MIN, &tls_protocol_min);
  /*
   * Set minimum TLS protocol version to TLS 1.3
   * Ensures secure, modern TLS encryption
   */
  if (rc != LDAP_SUCCESS) {
    m_error =
        "Failed to set TLS protocol min: " + std::string(ldap_err2string(rc));
    ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    m_ldap = nullptr;
    return false;
  }

  const char *tls_cipher_suite = "TLS_AES_256_GCM_SHA384";
  rc = ldap_set_option(m_ldap, LDAP_OPT_X_TLS_CIPHER_SUITE, &tls_cipher_suite);
  /*
   * Set TLS cipher suite to TLS_AES_256_GCM_SHA384
   * Uses AES-256-GCM with SHA-384 for strong encryption
   */
  if (rc != LDAP_SUCCESS) {
    m_error =
        "Failed to set TLS cipher suite: " + std::string(ldap_err2string(rc));
    ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    m_ldap = nullptr;
    return false;
  }

 
  rc = ldap_set_option(m_ldap, LDAP_OPT_X_TLS_CACERTFILE, netcrave_cert);
  /*
   * Embed CA certificate as byte array for TLS verification
   * Certificate is from Easy-RSA with 2048-bit RSA key
   * Used for LDAPS connections with self-signed certificates
   */

  if (rc != LDAP_SUCCESS) {
    m_error = "Failed to set TLS CA cert: " + std::string(ldap_err2string(rc));
    ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
    m_ldap = nullptr;
    return false;
  }

  if (!bindDN.empty() && !password.empty()) {
    if (!bind(bindDN, password)) {
      throw std::runtime_error("LDAP bind failed: " + m_error + " " + uri +
                               " " + bindDN + " " + password);
    }
  }

  return true;
}

bool LDAPConnection::isConnected() const { return m_ldap != nullptr; }

LDAP *LDAPConnection::getHandle() const { return m_ldap; }

std::string LDAPConnection::getError() const { return m_error; }

bool LDAPConnection::addEntry(const std::string &dn, LDAPMod **mods) {
  if (!m_ldap) {
    throw std::runtime_error("LDAP connection not initialized");
  }

  int result = ldap_add_ext_s(m_ldap, dn.c_str(), mods, nullptr, nullptr);
  if (result != LDAP_SUCCESS) {
    throw std::runtime_error(
        "LDAP add failed: " + std::string(ldap_err2string(result)) + " " +
        ModsToString(dn.c_str(), mods, "add"));
  }
  return true;
}

bool LDAPConnection::modifyEntry(const std::string &dn, LDAPMod **mods) {
  if (!m_ldap)
    return false;

  int result = ldap_modify_ext_s(m_ldap, dn.c_str(), mods, nullptr, nullptr);
  if (result != LDAP_SUCCESS) {
    throw std::runtime_error(
        "LDAP modify failed: " + std::string(ldap_err2string(result)) + " " +
        ModsToString(dn.c_str(), mods, "modify"));
  }
  return true;
}

bool LDAPConnection::deleteEntry(const std::string &dn) {
  if (!m_ldap)
    return false;

  int result = ldap_delete_ext_s(m_ldap, dn.c_str(), nullptr, nullptr);
  if (result != LDAP_SUCCESS) {
    throw std::runtime_error(
        "LDAP delete failed: " + std::string(ldap_err2string(result)) + " " +
        ModsToString(dn.c_str(), nullptr, "delete"));
  }
  return true;
}

bool LDAPConnection::search(
    const std::string &base, int scope, const std::string &filter,
    std::vector<std::vector<std::pair<std::string, std::string>>> &results) {
  if (!m_ldap)
    return false;

  LDAPMessage *result = nullptr;
  LDAPMessage *entry = nullptr;

  int rc = ldap_search_ext_s(m_ldap, base.c_str(), scope, filter.c_str(),
                             nullptr, 0, nullptr, nullptr, nullptr, 0, &result);

  if (rc != LDAP_SUCCESS) {
    throw std::runtime_error("LDAP search failed: " +
                             std::string(ldap_err2string(rc)));
  }

  for (entry = ldap_first_entry(m_ldap, result); entry != nullptr;
       entry = ldap_next_entry(m_ldap, entry)) {
    BerElement *ber = nullptr;
    char *attr = ldap_first_attribute(m_ldap, entry, &ber);

    std::vector<std::pair<std::string, std::string>> entryData;

    while (attr != nullptr) {
      struct berval **vals = ldap_get_values_len(m_ldap, entry, attr);
      if (vals != nullptr) {
        for (int i = 0; vals[i] != nullptr; i++) {
          entryData.emplace_back(attr, vals[i]->bv_val);
        }
        ldap_value_free_len(vals);
      }
      ldap_memfree(attr);
      attr = ldap_next_attribute(m_ldap, entry, ber);
    }

    if (ber) {
      ber_free(ber, 0);
    }

    results.push_back(entryData);
  }

  ldap_msgfree(result);
  return true;
}

bool LDAPConnection::bind(const std::string &bindDN,
                          const std::string &password) {
  if (!m_ldap)
    return false;

  /*
   * LDAP Bind with Simple Authentication
   *
   * ldap_sasl_bind_s() Parameters:
   * - m_ldap: LDAP session handle
   * - bindDN.c_str(): Distinguished Name to bind as
   * - LDAP_SASL_SIMPLE: Simple authentication mechanism
   * - cred: Credentials as berval structure
   * - serverctrls: Server controls (nullptr for simple auth)
   * - clientctrls: Client controls (nullptr for simple auth)
   *
   * Simple Authentication:
   * - No SASL, no TLS required
   * - Credentials sent in clear text (insecure)
   * - Use only for testing or non-critical environments
   *
   * Common Failure Causes:
   * 1. Invalid bindDN format (must match LDAP DN syntax)
   * 2. Empty or null password
   * 3. Authentication credentials rejected by server
   * 4. User not found on server
   * 5. Insufficient permissions
   *
   * LDAP Error Codes (from ldap.h):
   * - LDAP_SUCCESS (0): Bind successful
   * - LDAP_INVALID_CREDENTIALS (49): Bad username/password
   * - LDAP_INSUFFICIENT_ACCESS (50): Insufficient permissions
   * - LDAP_NO_SUCH_OBJECT (32): User not found
   * - LDAP_OPERATIONS_ERROR (1): General error
   */

  struct berval cred = {0};
  cred.bv_len = password.length();
  cred.bv_val = const_cast<char *>(password.c_str());

  int result = ldap_sasl_bind_s(m_ldap, bindDN.c_str(), LDAP_SASL_SIMPLE, &cred,
                                nullptr, nullptr, nullptr);
  if (result != LDAP_SUCCESS) {
    m_error = "LDAP bind failed: " + std::string(ldap_err2string(result));
    return false;
  }
  return true;
}

bool LDAPConnection::unbind() {
  if (!m_ldap)
    return false;

  int result = ldap_unbind_ext_s(m_ldap, nullptr, nullptr);
  if (result != LDAP_SUCCESS) {
    m_error = "LDAP unbind failed: " + std::string(ldap_err2string(result));
    return false;
  }
  m_ldap = nullptr;
  return true;
}