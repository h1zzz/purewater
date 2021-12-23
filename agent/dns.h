/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _DNS_H
#define _DNS_H

#include <stddef.h>
#include <stdint.h>

/*
 * TYPE values
 *
 * TYPE fields are used in resource records.  Note that these types are a
 * subset of QTYPEs.
 */
#define DNS_A 1    /* 1 a host address */
#define DNS_TXT 16 /* 16 text strings */

typedef struct dns dns_t;

struct dns_node {
    struct dns_node *next;
    int type;
    char data[256];
    size_t data_len;
};

dns_t *dns_new(void);
int dns_add_dns_server(dns_t *dns, const char *host, uint16_t port);
struct dns_node *dns_lookup(dns_t *dns, const char *name, int type);
void dns_node_cleanup(struct dns_node *dns_node);
void dns_free(dns_t *dns);

/* Use local DNS server and built-in DNS server query */
struct dns_node *dns_lookup_ret(const char *host, int type);

#endif /* dns.h */
