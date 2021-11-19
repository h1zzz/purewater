/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _PROXY_H
#define _PROXY_H

#include <stdint.h>

struct net_handle;

struct proxy {
    int (*handshake)(struct net_handle *net, const char *host, uint16_t port,
                     const char *username, const char *password);
    const char *host;
    uint16_t port;
    const char *username;
    const char *password;
};

int proxy_socks5_handshake(struct net_handle *net, const char *host,
                           uint16_t port, const char *username,
                           const char *password);

int proxy_https_handshake(struct net_handle *net, const char *host,
                          uint16_t port, const char *username,
                          const char *password);

#endif /* proxy.h */

