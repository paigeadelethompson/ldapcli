cmake_minimum_required(VERSION 3.28)
project(ldapcli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy config file to build directory
file(COPY ${CMAKE_SOURCE_DIR}/config/ldapcli.conf DESTINATION ${CMAKE_BINARY_DIR})

# Find LDAP libraries using pkg-config
find_package(PkgConfig REQUIRED)

# Check for LDAP via pkg-config
pkg_check_modules(LDAP REQUIRED IMPORTED_TARGET ldap)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
    PkgConfig::LDAP
)

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/LDAPConnection.cpp
    src/PowerDNSManager.cpp
    src/AsteriskManager.cpp
    src/FreeRADIUSManager.cpp
    src/KerberosManager.cpp
    src/OpenLDAPManager.cpp
    src/OpenDKIMManager.cpp
    src/SendmailManager.cpp
)

# Create executable
add_executable(ldapcli ${SOURCES})

# Link libraries
target_link_libraries(ldapcli PkgConfig::LDAP)

# Install rules
install(FILES ${CMAKE_SOURCE_DIR}/config/ldapcli.conf DESTINATION /etc/ldapcli)
