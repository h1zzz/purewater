/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "util.h"

#include <string.h>

#include "dns.h"
#include "debug.h"
#include "llist.h"

int util_connect_tcp(struct socket_handler *handler, const char *host,
                     uint16_t port)
{
    struct dns_node hints, *dns_node;
    struct llist dns;
    struct lnode *node;
    int ret;

    memset(&hints, 0, sizeof(struct dns_node));
    hints.family = AF_INET;
    hints.socktype = SOCK_STREAM;
    hints.protocol = IPPROTO_IP;

    ret = dns_resolve(&dns, host, port, &hints);
    if (ret == -1) {
        debugf("dns_resolve %s error", host);
        return -1;
    }

    for (node = dns.head; node; node = node->next) {
        dns_node = (struct dns_node *)node;
        ret =
            socket_open(handler, hints.family, hints.socktype, hints.protocol);
        if (ret == -1) {
            debug("socket open error");
            continue;
        }
        ret = socket_connect(handler, dns_node->addr, dns_node->addrlen);
        if (ret != -1) {
            break;
        }
        debugf("connect %s:%d error", host, port);
        socket_close(handler);
    }

    dns_destroy(&dns);

    return ret;
}
