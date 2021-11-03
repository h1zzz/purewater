/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _SOCKS_H
#define _SOCKS_H

#include "proxy.h"
#include "socket.h"

int socks5_handshake(struct socket_handler *handler, const char *host,
                     uint16_t port, const struct proxy *proxy);

#endif /* socks.h */
