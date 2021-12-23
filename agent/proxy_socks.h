/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PROXY_SOCKS_H
#define _PROXY_SOCKS_H

#include "socket.h"

socket_t proxy_socks(const char *host, uint16_t port, const char *proxy_host,
                     uint16_t proxy_port, const char *proxy_user,
                     const char *proxy_passwd);

#endif /* proxy_socks.h */
