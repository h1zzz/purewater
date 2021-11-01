/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdint.h>

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>

#include "socket.h"
#include "proxy.h"

struct connection {
    struct socket_handler handler;
    struct sockaddr_in addr;
    int tls;
    char *hostname;

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
};

struct connection *connection_open(const char *host, uint16_t port,
                                   const struct proxy *proxy);
int connection_write(struct connection *conn, const void *data, size_t n);
int connection_read(struct connection *conn, void *buf, size_t size);
int connection_tls_handshake(struct connection *conn);
void connection_close(struct connection *conn);

#endif /* connection.h */
