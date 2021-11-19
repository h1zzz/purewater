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
    struct sockaddr_in *addr;
};

int dns_resolve(struct llist *list, const char *name);

#endif /* dns.h */

