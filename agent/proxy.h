/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PROXY_H
#define _PROXY_H

#include "socket.h"

/* Use the callback function to establish a connection with the proxy, return a
   new socket successfully, and return SOCK_INVAL if it fails */
typedef socket_t proxy_connect_t(const char *host, uint16_t port,
                                 const char *proxy_host, uint16_t proxy_port,
                                 const char *proxy_user,
                                 const char *proxy_passwd);

#endif /* proxy.h */
