/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _DNS_H
#define _DNS_H

#include <stddef.h>

/*
 * TYPE values
 *
 * TYPE fields are used in resource records.  Note that these types are a
 * subset of QTYPEs.
 */
typedef enum dns_type {
    DNS_A = 1,    /* 1 a host address */
    DNS_TXT = 16, /* 16 text strings */
} dns_type_t;

struct dns_node {
    struct dns_node *next;
    dns_type_t type;
    char data[256];
    size_t data_len;
};

int dns_lookup(struct dns_node **res, const char *name, dns_type_t type);
void dns_cleanup(struct dns_node *head);

#endif /* dns.h */
