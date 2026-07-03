# LDAP CLI

A comprehensive command-line interface tool for managing LDAP-based services and directory operations.

## Overview

LDAP CLI is a versatile management tool designed to interact with various LDAP-enabled services including DNS, telephony, authentication, email, and directory services. Built with modern C++ standards and designed for enterprise environments.

## Features

- **Multi-Service Support**: Manage multiple LDAP-based services from a single interface
  - PowerDNS - DNS management
  - Asterisk - Telephony management
  - FreeRADIUS - Authentication management
  - Kerberos - Security management
  - OpenDKIM - Email security
  - Sendmail - Email management
  - OpenLDAP - Directory operations

- **Modern C++ Implementation**: Built with C++26 standard
- **CMake Build System**: Cross-platform build configuration
- **LDAP Integration**: Full support for OpenLDAP C API
- **Configurable**: Easy configuration through configuration files
- **Error Handling**: Comprehensive error reporting and validation

## Requirements

- C++26 compatible compiler
- CMake 3.28 or higher
- OpenLDAP development libraries
- pkg-config

### System Dependencies

- OpenLDAP client libraries
- Standard C++ library

## Installation

### From Source

1. Clone the repository:
```bash
git clone <repository-url>
cd ldapcli
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Configure with CMake:
```bash
cmake ..
```

4. Build the project:
```bash
cmake --build .
```

5. Install (optional):
```bash
sudo make install
```

## Configuration

The tool uses a configuration file located at `config/ldapcli.conf` (or `/etc/ldapcli/ldapcli.conf` after installation).

### Configuration File Format

```ini
# LDAP CLI Configuration File
# Base DN for LDAP operations
baseDN=dc=netcrave,dc=local

# LDAP Server URI
ldapURI=ldap://ldap.netcrave.local:389

# LDAP Bind DN (optional)
bindDN=cn=manager,dc=netcrave,dc=local

# LDAP Bind Password (optional)
bindPassword=your_password
```

### Configuration Parameters

- **baseDN**: The base distinguished name for LDAP operations
- **ldapURI**: LDAP server URI (e.g., `ldap://localhost:389`)
- **bindDN**: Distinguished name for binding to LDAP server (optional)
- **bindPassword**: Password for LDAP binding (optional)

## Usage

### Basic Syntax

```bash
./ldapcli <service> <command> [arguments...]
```

### Available Services

#### DNS Management (PowerDNS)
```bash
./ldapcli dns
```

**Output:**
```
DNS Commands:
  create-zone <zone> [--type|--t TYPE]
  delete-zone <zone> [--zone ZONE]
  update-zone <zone> [--notified-serial|--n N] [--last-check|--l N] [--master|--m HOST]
  list-zones
  add-record [--zone|--z ZONE] [--name|--n NAME] [--ttl|--t SEC] (--type|--y TYPE --value|--v VALUE | --a ADDR | --aaaa ADDR | ...)
  update-record [--zone|--z ZONE] [--name|--n NAME] [--type|--y TYPE] [--value|--v VALUE | --a ADDR | ...] [--ttl|--t SEC]
  delete-record [--zone|--z ZONE] [--name|--n NAME] [--type|--y TYPE]
  list-records [--zone|--z ZONE]
Record type long options (each sets type and value):
  --a <value>  (A record)
  --aaaa <value>  (AAAA record)
  --mx <value>  (MX record)
  --ns <value>  (NS record)
  --cname <value>  (CNAME record)
  --soa <value>  (SOA record)
  --txt <value>  (TXT record)
  --ptr <value>  (PTR record)
  --srv <value>  (SRV record)
  --caa <value>  (CAA record)
  --hinfo <value>  (HINFO record)
  --minfo <value>  (MINFO record)
  --rp <value>  (RP record)
  --afsdb <value>  (AFSDB record)
  --sig <value>  (SIG record)
  --key <value>  (KEY record)
  --gpos <value>  (GPOS record)
  --loc <value>  (LOC record)
  --nxt <value>  (NXT record)
  --naptr <value>  (NAPTR record)
  --kx <value>  (KX record)
  --cert <value>  (CERT record)
  --ds <value>  (DS record)
  --sshfp <value>  (SSHFP record)
  --tlsa <value>  (TLSA record)
  --spf <value>  (SPF record)
  --uri <value>  (URI record)
  --alias <value>  (ALIAS record)
  --wks <value>  (WKS record)
  --a6 <value>  (A6 record)
  --dname <value>  (DNAME record)
  --apl <value>  (APL record)
  --rrsig <value>  (RRSIG record)
  --nsec <value>  (NSEC record)
  --dnskey <value>  (DNSKEY record)
  --dhcid <value>  (DHCID record)
  --nsec3 <value>  (NSEC3 record)
  --nsec3param <value>  (NSEC3PARAM record)
  --cds <value>  (CDS record)
  --cdnskey <value>  (CDNSKEY record)
  --openpgpkey <value>  (OPENPGPKEY record)
  --svcb <value>  (SVCB record)
  --https <value>  (HTTPS record)
  --eui48 <value>  (EUI48 record)
  --eui64 <value>  (EUI64 record)
  --tkey <value>  (TKEY record)
  --dlv <value>  (DLV record)
```

#### Asterisk Management
```bash
./ldapcli asterisk
```

**Output:**
```
Asterisk Commands:
  create-account <account> [-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]
  update-account <account> [-s|--secret SECRET] [-c|--caller-id ID] [-m|--mailbox BOX]
  delete-account <account> [--account ACCOUNT]
  list-accounts
  create-voicemail <mailbox> [-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]
  update-voicemail <mailbox> [-p|--password PASS] [-f|--fullname NAME] [-e|--email ADDR]
  delete-voicemail <mailbox> [--mailbox BOX]
  list-voicemail
```

#### FreeRADIUS Management
```bash
./ldapcli freeradius
```

**Output:**
```
FreeRADIUS Commands:
  create-client <client> [-s|--secret SECRET] [-n|--shortname NAME] [-t|--type TYPE]
  update-client <client> [-s|--secret SECRET] [-n|--shortname NAME] [-v|--virtual-server SERVER] [-t|--type TYPE] [-m|--require-ma BOOL] [-c|--comment TEXT]
  delete-client <client> [--client CLIENT]
  list-clients
  create-user <username> [-p|--password PASS] [-s|--service-type TYPE] [-f|--framed-protocol PROTO]
  update-user <username> [-p|--password PASS] [-s|--service-type TYPE] [-f|--framed-protocol PROTO]
  delete-user <username> [--user USER]
  list-users
```

#### Kerberos Management
```bash
./ldapcli kerberos
```

**Output:**
```
Kerberos Commands:
  create-principal <principal> [options]
  delete-principal <principal> [--principal PRINCIPAL]
  update-principal <principal> [options]
  list-principals
```

#### OpenDKIM Management
```bash
./ldapcli opendkim
```

**Output:**
```
OpenDKIM Commands:
  create-identity <identity> [-s|--selector SEL] [-k|--key KEY] [-d|--domain DOMAIN]
  delete-identity <identity> [--identity IDENTITY]
  update-identity <identity> [-s|--selector SEL] [-k|--key KEY] [-d|--domain DOMAIN]
  list-identities
```

#### Sendmail Management
```bash
./ldapcli sendmail
```

**Output:**
```
Sendmail Commands:
  create-mta <mta> [-c|--cluster NAME] [-h|--host HOST] [-d|--description TEXT]
  delete-mta <mta> [--mta MTA]
  update-mta <mta> [-c|--cluster NAME] [-h|--host HOST] [-d|--description TEXT]
  list-mtas
```

#### OpenLDAP Management
```bash
./ldapcli openldap
```

**Output:**
```
OpenLDAP Commands:
  create-ou <ou> [-p|--telephonenumber NUM] [-s|--street ADDR] [-z|--postalcode CODE] [-S|--st STATE] [-L|--l CITY] [-d|--description TEXT]
  delete-ou <ou> [--ou OU]
  update-ou <ou>
  list-ous
  list-people
  create-person <cn> [options]
  update-person <cn> [options]
  delete-person <cn> [--cn CN]
```

### Getting Help

```bash
./ldapcli
```

## Building from Source

### Prerequisites

- CMake 3.28 or higher
- C++26 compatible compiler (GCC, Clang, or MSVC)
- OpenLDAP development libraries
- pkg-config

### Build Steps

1. **Clone the repository**:
```bash
git clone <repository-url>
cd ldapcli
```

2. **Create build directory**:
```bash
mkdir build && cd build
```

3. **Configure with CMake**:
```bash
cmake ..
```

4. **Build the project**:
```bash
cmake --build .
```

5. **Run the executable**:
```bash
./ldapcli
```

## Project Structure

```
ldapcli/
├── CMakeLists.txt          # CMake build configuration
├── LICENSE.md              # BSD 3-Clause License
├── README.md               # This file
├── config/
│   └── ldapcli.conf        # Configuration file
├── include/
│   ├── LDAPConnection.hpp  # LDAP connection management
│   ├── LDAPManagerBase.hpp # Base manager class
│   ├── PowerDNSManager.hpp # PowerDNS manager
│   ├── AsteriskManager.hpp # Asterisk manager
│   ├── FreeRADIUSManager.hpp # FreeRADIUS manager
│   ├── KerberosManager.hpp # Kerberos manager
│   ├── OpenDKIMManager.hpp # OpenDKIM manager
│   ├── SendmailManager.hpp # Sendmail manager
│   ├── OpenLDAPManager.hpp # OpenLDAP manager
│   ├── Config.hpp          # Configuration management
│   └── Console.hpp         # Console output utilities
├── schemas/                # LDAP schema definitions
│   ├── asterisk/
│   ├── freeradius/
│   ├── krb5/
│   ├── netcrave/
│   ├── opendkim/
│   └── openldap/
├── src/
│   ├── Main.cpp            # Main entry point
│   ├── Config.cpp          # Configuration implementation
│   ├── LDAPConnection.cpp  # LDAP connection implementation
│   ├── PowerDNSManager.cpp # PowerDNS implementation
│   ├── AsteriskManager.cpp # Asterisk implementation
│   ├── FreeRADIUSManager.cpp # FreeRADIUS implementation
│   ├── KerberosManager.cpp # Kerberos implementation
│   ├── OpenDKIMManager.cpp # OpenDKIM implementation
│   ├── SendmailManager.cpp # Sendmail implementation
│   └── OpenLDAPManager.cpp # OpenLDAP implementation
└── build/                  # Build output directory
```

## License

BSD 3-Clause License

Copyright (c) 2026, RavenHammer Research Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## Contributing

Contributions are welcome! Please ensure your code follows the project's coding standards and includes appropriate tests.

## Support

For issues, questions, or contributions, please contact the development team at RavenHammer Research Inc.