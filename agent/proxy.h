/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PROXY_H
#define _PROXY_H

#include <stdint.h>

#include "net.h"

struct proxy {
    int (*handshake)(net_t *net, const char *host, uint16_t port,
                     const char *username, const char *password);
    const char *host;
    uint16_t port;
    const char *username;
    const char *password;
};

int proxy_socks5_handshake(net_t *net, const char *host, uint16_t port,
                           const char *username, const char *password);

int proxy_https_handshake(net_t *net, const char *host, uint16_t port,
                          const char *username, const char *password);

#endif /* proxy.h */
