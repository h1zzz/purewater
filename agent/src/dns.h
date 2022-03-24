/* MIT License Copyright (c) 2022, h1zzz */

#ifndef _DNS_H
#define _DNS_H

#include <stdint.h>
#include <stddef.h>

/*
 * TYPE values
 *
 * TYPE fields are used in resource records.  Note that these types are a
 * subset of QTYPEs.
 */
#define DNS_A 1    /* 1 a host address */
#define DNS_TXT 16 /* 16 text strings */

struct dns_node {
    struct dns_node *next;
    int type;
    char data[256];
    size_t data_len;
};

struct dns_ns {
    struct dns_ns *next;
    char *host;
    uint16_t port;
};

typedef struct {
    struct dns_ns *ns_head;
    struct dns_ns *ns_tail;
} dns_context;

void dns_init(dns_context *ctx);
int dns_add_ns(dns_context *ctx, const char *host, uint16_t port);
struct dns_node *dns_query(dns_context *ctx, const char *domain, int type);
void dns_free(dns_context *ctx);

void dns_node_destroy(struct dns_node *dns_node);
struct dns_node *dns_query_ret(const char *domain, int type);

#endif /* dns.h */
