/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _CONNECT_H
#define _CONNECT_H

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>

#include "socket.h"

struct proxy;

typedef struct net_handle {
    socket_handle_t sock;
    struct sockaddr_in addr;
    int tls;
    char *hostname;

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
} net_handle_t;

int net_connect(net_handle_t *net, const char *host, uint16_t port,
                const struct proxy *proxy);
int net_tls_handshake(net_handle_t *net);
int net_read(net_handle_t *net, void *buf, size_t size);
int net_readn(net_handle_t *net, void *buf, size_t size);
int net_write(net_handle_t *net, const void *data, size_t n);
void net_close(net_handle_t *net);

#endif /* connect.h */
