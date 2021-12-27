/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _SOCKS_H
#define _SOCKS_H

#include <stdint.h>

#include "socket.h"

/* Auth method */
#define SOCKS5_NO_AUTHENTICATION_REQUIRED 0x00
/* #define SOCKS5_GSSAPI 0x01 */
#define SOCKS5_USERNAME_PASSWORD 0x02
/* #define SOCKS5_IANA_ASSIGNED '03' to X '7F' */
/* #define SOCKS5_RESERVED_FOR_PRIVATE_METHODS '80' to X 'FE' */
#define SOCKS5_NO_ACCEPTABLE_METHODS 0xff

/* Socks5 CMD */
#define SOCKS5_CONNECT 0x01
/* #define SOCKS5_BIND 0x02 */
/* #define SOCKS5_UDP_ASSOCIATE_UDP 0x03 */

/* Sock5 Address type */
#define SOCKS5_IPV4_ADDRESS 0x01
#define SOCKS5_DOMAINNAME 0x03
/* #define SOCKS5_ATYP_IPV6_ADDRESS 0x04 */

/* Socks5 Replies code */
#define SOCKS5_SUCCEEDED 0x00
#define SOCKS5_GENERAL_SOCKS_SERVER_FAILURE 0x01
#define SOCKS5_CONNECTION_NOT_ALLOWED_BY_RULESET 0x02
#define SOCKS5_NETWORK_UNREACHABLE 0x03
#define SOCKS5_HOST_UNREACHABLE 0x04
#define SOCKS5_CONNECTION_REFUSED 0x05
#define SOCKS5_TTL_EXPIRED 0x06
#define SOCKS5_COMMAND_NOT_SUPPORTED 0x07
#define SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED 0x08
/*  0x09 to X'FF' unassigned   0x09 */

/* Negotiate the authentication method with the proxy server, return -1 if it
   fails, and return the authentication method if it succeeds */
int socks5_client_negotiate_auth_method(socket_t sock, const uint8_t *methods,
                                        uint8_t nmethods);

/* Use username and password to authenticate, return 0 on success, return -1 on
   failure */
int socks5_client_username_password_auth(socket_t sock, const char *user,
                                         const char *passwd);

/* Send a request to the socks5 proxy server, return 0 on success, return an
   error code on failure */
int socks5_client_request(socket_t sock, uint8_t cmd, uint8_t atyp,
                          const char *addr, uint16_t port);

#endif /* socks.h */
