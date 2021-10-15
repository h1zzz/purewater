/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _UTIL_H
#define _UTIL_H

#include "socket.h"

int util_connect_tcp(struct socket_handler *handler, const char *host,
                     uint16_t port);

#endif /* util.h */
