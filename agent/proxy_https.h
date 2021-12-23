/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PROXY_HTTPS_H
#define _PROXY_HTTPS_H

#include "socket.h"

socket_t proxy_https(const char *host, uint16_t port, const char *proxy_host,
                     uint16_t proxy_port, const char *proxy_user,
                     const char *proxy_passwd);

#endif /* proxy_https.h */
