/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "connection.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "dns.h"
#include "llist.h"
#include "platform.h"
#include "util.h"

static int connection_read_mbedtls_cb(void *ctx, unsigned char *buf,
                                      size_t len);
static int connection_write_mbedtls_cb(void *ctx, const unsigned char *buf,
                                       size_t len);

struct connection *connection_open(const char *host, uint16_t port,
                                   const struct proxy *proxy)
{
    struct connection *conn;
    int ret;

    conn = calloc(1, sizeof(struct connection));
    if (!conn) {
        debug("calloc error");
        return NULL;
    }

    conn->hostname = xstrdup(host);
    if (!conn->hostname) {
        debugf("xstrdup %s error", host);
        free(conn);
        return NULL;
    }

    if (proxy) {
        ret = proxy->handshake(&conn->handler, host, port, proxy);
        if (ret == -1) {
            debugf("proxy handshake error %s:%d %s:%s", proxy->host,
                   proxy->port, proxy->user, proxy->passwd);
            free(conn->hostname);
            free(conn);
            return NULL;
        }
        return conn;
    }

    ret = util_connect_tcp(&conn->handler, host, port);
    if (ret == -1) {
        free(conn->hostname);
        free(conn);
        return NULL;
    }

    return conn;
}

int connection_write(struct connection *conn, const void *data, size_t n)
{
    if (conn->tls)
        return mbedtls_ssl_write(&conn->ssl, data, n);
    return socket_write(&conn->handler, data, n);
}

int connection_read(struct connection *conn, void *buf, size_t size)
{
    if (conn->tls)
        return mbedtls_ssl_read(&conn->ssl, buf, size);
    return socket_read(&conn->handler, buf, size);
}

int connection_tls_handshake(struct connection *conn)
{
    static const char *pers = "ssl_client";
    int ret;

    mbedtls_ssl_init(&conn->ssl);
    mbedtls_ssl_config_init(&conn->conf);
    mbedtls_ctr_drbg_init(&conn->ctr_drbg);
    mbedtls_entropy_init(&conn->entropy);

    ret = mbedtls_ctr_drbg_seed(&conn->ctr_drbg, mbedtls_entropy_func,
                                &conn->entropy, (const unsigned char *)pers,
                                strlen(pers));
    if (ret != 0) {
        debugf("failed  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto err;
    }

    ret = mbedtls_ssl_config_defaults(&conn->conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        debugf("failed  ! mbedtls_ssl_config_defaults returned %d", ret);
        goto err;
    }

    mbedtls_ssl_conf_authmode(&conn->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&conn->conf, mbedtls_ctr_drbg_random, &conn->ctr_drbg);

    ret = mbedtls_ssl_setup(&conn->ssl, &conn->conf);
    if (ret != 0) {
        debugf("failed  ! mbedtls_ssl_setup returned %d", ret);
        goto err;
    }

    mbedtls_ssl_set_bio(&conn->ssl, conn, connection_write_mbedtls_cb,
                        connection_read_mbedtls_cb, NULL);

    while ((ret = mbedtls_ssl_handshake(&conn->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            debugf("failed  ! mbedtls_ssl_handshake returned -0x%x", -ret);
            goto err;
        }
    }

    conn->tls = 1;

    return 0;

err:
    mbedtls_ssl_free(&conn->ssl);
    mbedtls_ssl_config_free(&conn->conf);
    mbedtls_ctr_drbg_free(&conn->ctr_drbg);
    mbedtls_entropy_free(&conn->entropy);

    return -1;
}

void connection_close(struct connection *conn)
{
    if (conn->tls) {
        mbedtls_ssl_close_notify(&conn->ssl);
        mbedtls_ssl_free(&conn->ssl);
        mbedtls_ssl_config_free(&conn->conf);
        mbedtls_ctr_drbg_free(&conn->ctr_drbg);
        mbedtls_entropy_free(&conn->entropy);
    }
    socket_shutdown(&conn->handler);
    socket_close(&conn->handler);
    free(conn->hostname);
    free(conn);
}

static int connection_read_mbedtls_cb(void *ctx, unsigned char *buf, size_t len)
{
    struct connection *conn = ctx;
    return socket_read(&conn->handler, buf, len);
}

static int connection_write_mbedtls_cb(void *ctx, const unsigned char *buf,
                                       size_t len)
{
    struct connection *conn = ctx;
    return socket_write(&conn->handler, buf, len);
}
