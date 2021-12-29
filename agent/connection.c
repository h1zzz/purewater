/* MIT License Copyright (c) 2021, h1zzz */

#include "connection.h"

#include <stdlib.h>
#include <string.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ssl.h>

#include "debug.h"
#include "dns.h"
#include "network.h"
#include "util.h"

struct tcpconn {
    socket_t sock;                  /* Basic socket */
    proxy_connect_t *proxy_connect; /* Use proxy connect callback function */
    char *proxy_host;               /* Proxy server ip address or domain */
    uint16_t proxy_port;            /* Proxy server port */
    char *proxy_user;               /* Proxy username */
    char *proxy_passwd;             /* Proxy password */

    int connected;     /* Successfully established connection */
    int ssl_connected; /* SSL connection established successfully */

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
};

tcpconn_t *tcpconn_new(void)
{
    tcpconn_t *conn;

    conn = calloc(1, sizeof(tcpconn_t));
    if (!conn) {
        debug("calloc error");
        return NULL;
    }
    conn->sock = SOCK_INVAL;

    return conn;
}

int tcpconn_set_proxy(tcpconn_t *conn, proxy_connect_t *proxy_connect,
                      const char *host, uint16_t port, const char *user,
                      const char *passwd)
{
    conn->proxy_connect = proxy_connect;

    /* Prevent memory leaks and release memory occupied by old data */
    if (conn->proxy_host) {
        free(conn->proxy_host);
        conn->proxy_host = NULL;
    }

    if (conn->proxy_user) {
        free(conn->proxy_user);
        conn->proxy_user = NULL;
    }

    if (conn->proxy_passwd) {
        free(conn->proxy_passwd);
        conn->proxy_passwd = NULL;
    }

    conn->proxy_host = xstrdup(host);
    if (!conn->proxy_host) {
        debug("xstrdup error");
        goto err_dup_host;
    }

    conn->proxy_port = port;

    if (user && passwd) {
        conn->proxy_user = xstrdup(user);
        if (!conn->proxy_user) {
            debug("xstrdup error");
            goto err_dup_user;
        }
        conn->proxy_passwd = xstrdup(passwd);
        if (!conn->proxy_passwd) {
            debug("xstrdup error");
            goto err_dup_passwd;
        }
    }

    return 0;

err_dup_passwd:
    free(conn->proxy_user);
    conn->proxy_user = NULL;
err_dup_user:
    free(conn->proxy_host);
    conn->proxy_host = NULL;
err_dup_host:
    return -1;
}

int tcpconn_connect(tcpconn_t *conn, const char *host, uint16_t port)
{
    /* If there is a proxy, use the proxy to establish a connection */
    if (conn->proxy_connect) {
        conn->sock =
            conn->proxy_connect(host, port, conn->proxy_host, conn->proxy_port,
                                conn->proxy_user, conn->proxy_passwd);
        if (conn->sock == SOCK_INVAL) {
            debug("conn->pconnect error");
            return -1;
        }
        return 0;
    }

    conn->sock = tcp_connect(host, port);
    if (conn->sock == SOCK_INVAL)
        return -1;

    conn->connected = 1;

    return 0;
}

static int tcpconn_ssl_recv(tcpconn_t *conn, unsigned char *buf, size_t len)
{
    return xrecv(conn->sock, buf, len);
}

static int tcpconn_ssl_send(tcpconn_t *conn, const unsigned char *buf,
                            size_t len)
{
    return xsend(conn->sock, buf, len);
}

static void tcpconn_mbedtls_init(tcpconn_t *conn)
{
    mbedtls_ssl_init(&conn->ssl);
    mbedtls_ssl_config_init(&conn->conf);
    mbedtls_ctr_drbg_init(&conn->ctr_drbg);
    mbedtls_entropy_init(&conn->entropy);
}

static void tcpconn_mbedtls_free(tcpconn_t *conn)
{
    mbedtls_ssl_free(&conn->ssl);
    mbedtls_ssl_config_free(&conn->conf);
    mbedtls_ctr_drbg_free(&conn->ctr_drbg);
    mbedtls_entropy_free(&conn->entropy);
}

int tcpconn_ssl_connect(tcpconn_t *conn, const char *host, uint16_t port)
{
    static const char *pers = "ssl_client";
    int ret;

    if (tcpconn_connect(conn, host, port) == -1) {
        debug("tcpconn_connect error");
        return -1;
    }

    tcpconn_mbedtls_init(conn);

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

    mbedtls_ssl_set_bio(&conn->ssl, conn,
                        (mbedtls_ssl_send_t *)tcpconn_ssl_send,
                        (mbedtls_ssl_recv_t *)tcpconn_ssl_recv, NULL);

again:
    ret = mbedtls_ssl_handshake(&conn->ssl);
    if (ret != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            debugf("failed  ! mbedtls_ssl_handshake returned -0x%x", -ret);
            goto err;
        }
        goto again;
    }

    conn->ssl_connected = 1;
    return 0;

err:
    tcpconn_mbedtls_free(conn);
    return -1;
}

int tcpconn_recv(tcpconn_t *conn, void *buf, size_t size)
{
    int ret;

    if (conn->ssl_connected) {
    again:
        ret = mbedtls_ssl_read(&conn->ssl, (unsigned char *)buf, size);
        if (ret < 0) {
            if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                goto again;
            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
                return 0;
            debugf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
            return -1;
        }
        return ret;
    }

    return xrecv(conn->sock, buf, size);
}

int tcpconn_send(tcpconn_t *conn, const void *data, size_t len)
{
    int ret;

    if (conn->ssl_connected) {
    again:
        ret = mbedtls_ssl_write(&conn->ssl, (const unsigned char *)data, len);
        if (ret <= 0) {
            if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                goto again;
            debugf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
            return -1;
        }
        return ret;
    }

    return xsend(conn->sock, data, len);
}

void tcpconn_close(tcpconn_t *conn)
{
    if (conn->connected) {
        if (conn->ssl_connected) {
            mbedtls_ssl_close_notify(&conn->ssl);
            tcpconn_mbedtls_free(conn);
        }
        xclose(conn->sock);
    }
}

void tcpconn_free(tcpconn_t *conn)
{
    tcpconn_close(conn);

    if (conn->proxy_host)
        free(conn->proxy_host);

    if (conn->proxy_user)
        free(conn->proxy_user);

    if (conn->proxy_passwd)
        free(conn->proxy_passwd);

    free(conn);
}
