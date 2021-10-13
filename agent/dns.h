/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _DNS_H
#define _DNS_H

#include <stdint.h>

#include "llist.h"
#include "socket.h"

#define dns_destroy(list) llist_destroy(list)

struct dns_node {
    struct lnode _node;
    int family;
    int socktype;
    int protocol;
    struct sockaddr *addr;
    socklen_t addrlen;
};

int dns_resolve(struct llist *list, const char *name, uint16_t port,
                const struct dns_node *hints);

#endif /* dns.h */
