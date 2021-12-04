/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _DNS_H
#define _DNS_H

#include "llist.h"
#include "socket.h"

struct dns_node {
    lnode_t _node;
    struct sockaddr_in *addr;
};

int dns_resolve(llist_t *list, const char *name);
void dns_destroy(llist_t *list);

#endif /* dns.h */
