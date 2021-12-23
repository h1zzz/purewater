/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PROXY_H
#define _PROXY_H

#include "socket.h"

/* Use the callback function to establish a connection with the proxy, return a
   new socket successfully, and return SOCK_INVAL if it fails */
typedef socket_t proxy_connect(const char *host, uint16_t port,
                               const char *phost, uint16_t pport,
                               const char *puser, const char *ppasswd);

#endif /* proxy.h */
