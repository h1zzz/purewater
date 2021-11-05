/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _CONNECT_H
#define _CONNECT_H

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>

#include "socket.h"

struct net_handle {
    struct socket_handle sock;
    struct sockaddr_in addr;
    int tls;
    char *hostname;

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
};

int net_connect(struct net_handle *net, const char *host, uint16_t port);
int net_tls_handshake(struct net_handle *net);
int net_read(struct net_handle *net, void *buf, size_t size);
int net_write(struct net_handle *net, const void *data, size_t n);
void net_close(struct net_handle *net);

#endif /* connect.h */
