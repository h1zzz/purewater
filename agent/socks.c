/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "socks.h"

#include <string.h>

#include "debug.h"
#include "dns.h"
#include "llist.h"
#include "util.h"

int socks5_handshake(struct socket_handler *handler, const char *host,
                     uint16_t port, const struct proxy *proxy)
{
    int ret;

    (void)host;
    (void)port;

    ret = util_connect_tcp(handler, proxy->host, proxy->port);
    if (ret == -1) {
        debug("util_connect_tcp error");
        return -1;
    }

    return 0;
}
