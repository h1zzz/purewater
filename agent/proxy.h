/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _PROXY_H
#define _PROXY_H

#include "socket.h"

struct proxy;

typedef int proxy_handshake_t(struct socket_handler *handler, const char *host,
                              uint16_t port, const struct proxy *proxy);

struct proxy {
    proxy_handshake_t *handshake;
    const char *host;
    uint16_t port;
    const char *user;
    const char *passwd;
};

#endif /* proxy.h */
