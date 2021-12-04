/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _SOCKS5_H
#define _SOCKS5_H

#include "net.h"

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

/*
 * Send a negotiated authentication request to the socks5 server, if it fails,
 * it will return -1, if it succeeds, it will return the authentication method
 */
int socks5_client_send_method(net_handle_t *net, int use_password);

/*
 * Use user name and password to authenticate to the socks5 server, return 0 if
 * the authentication succeeds, and return -1 if it fails
 */
int socks5_client_send_password_auth(net_handle_t *net, const char *uname,
                                     const char *passwd);

/*
 * Send a detailed proxy request to the socks5 server, return socks5 replies
 * code
 */
int socks5_client_request(net_handle_t *net, int cmd, int atyp,
                          const char *dst_addr, uint16_t dst_port);

#endif /* socks5.h */
