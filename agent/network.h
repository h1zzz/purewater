/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _NETWORK_H
#define _NETWORK_H

#include "socket.h"

socket_t tcp_connect(const char *domain, uint16_t port);
socket_t udp_connect(const char *domain, uint16_t port);

#endif /* network.h */
