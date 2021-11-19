/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "net.h"

#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "dns.h"
#include "llist.h"
#include "util.h"
#include "proxy.h"

int net_connect(struct net_handle *net, const char *host, uint16_t port,
                const struct proxy *proxy)
{
    struct lnode *node;
    struct llist dns;
    int ret;

    memset(net, 0, sizeof(struct net_handle));

    ret = dns_resolve(&dns, proxy ? proxy->host : host);
    if (ret == -1) {
        debugf("resolve %s error", host);
        return -1;
    }

    for (node = dns.head; node; node = node->next) {
        memcpy(&net->addr, ((struct dns_node *)node)->addr, sizeof(net->addr));
        net->addr.sin_family = AF_INET;
        net->addr.sin_port = htons(proxy ? proxy->port : port);

        ret = socket_open(&net->sock, PF_INET, SOCK_STREAM, IPPROTO_IP);
        if (ret == -1)
            continue;

        ret = socket_connect(&net->sock, (struct sockaddr *)&net->addr,
                             sizeof(net->addr));
        if (ret == -1) {
            socket_close(&net->sock);
            continue;
        }
        break;
    }

    dns_destroy(&dns);

    if (ret == -1) /* Did not successfully connect to the server */
        return -1;

    if (proxy) {
        ret =
            proxy->handshake(net, host, port, proxy->username, proxy->password);
        if (ret == -1) {
            debug("proxy handshake error");
            socket_close(&net->sock);
            return -1;
        }
    }

    net->hostname = xstrdup(host);
    if (!net->hostname) {
        socket_close(&net->sock);
        debugf("xstrdup(%s) error", host);
        ret = -1;
    }

    return ret;
}

static void tls_free(struct net_handle *net)
{
    mbedtls_ssl_free(&net->ssl);
    mbedtls_ssl_config_free(&net->conf);
    mbedtls_ctr_drbg_free(&net->ctr_drbg);
    mbedtls_entropy_free(&net->entropy);
}

static int tls_init(struct net_handle *net)
{
    static const char *pers = "ssl_client";
    int ret;

    mbedtls_ssl_init(&net->ssl);
    mbedtls_ssl_config_init(&net->conf);
    mbedtls_ctr_drbg_init(&net->ctr_drbg);
    mbedtls_entropy_init(&net->entropy);

    ret = mbedtls_ctr_drbg_seed(&net->ctr_drbg, mbedtls_entropy_func,
                                &net->entropy, (const unsigned char *)pers,
                                strlen(pers));
    if (ret != 0) {
        debugf("failed  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto err;
    }

    ret = mbedtls_ssl_config_defaults(&net->conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        debugf("failed  ! mbedtls_ssl_config_defaults returned %d", ret);
        goto err;
    }

    mbedtls_ssl_conf_authmode(&net->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&net->conf, mbedtls_ctr_drbg_random, &net->ctr_drbg);

    return 0;
err:
    tls_free(net);
    return -1;
}

static int tls_recv(void *ctx, unsigned char *buf, size_t len)
{
    return socket_recv(&((struct net_handle *)ctx)->sock, buf, len);
}

static int tls_send(void *ctx, const unsigned char *buf, size_t len)
{
    return socket_send(&((struct net_handle *)ctx)->sock, buf, len);
}

int net_tls_handshake(struct net_handle *net)
{
    int ret;

    if (tls_init(net) == -1) {
        debug("tls_init error");
        return -1;
    }

    ret = mbedtls_ssl_setup(&net->ssl, &net->conf);
    if (ret != 0) {
        debugf("failed  ! mbedtls_ssl_setup returned %d", ret);
        goto err;
    }

    mbedtls_ssl_set_bio(&net->ssl, net, tls_send, tls_recv, NULL);

    while ((ret = mbedtls_ssl_handshake(&net->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            debugf("failed  ! mbedtls_ssl_handshake returned -0x%x", -ret);
            goto err;
        }
    }
    net->tls = 1;

    return 0;
err:
    tls_free(net);
    return -1;
}

int net_read(struct net_handle *net, void *buf, size_t size)
{
    if (net->tls)
        return mbedtls_ssl_read(&net->ssl, (unsigned char *)buf, size);
    return socket_recv(&net->sock, buf, size);
}

int net_readn(struct net_handle *net, void *buf, size_t n)
{
    size_t nleft = n;
    char *ptr = buf;
    int nread;

    while (nleft > 0) {
        nread = net_read(net, ptr, nleft);
        if (nread == -1) {
            debug("net_read error");
            return -1;
        }
        nleft -= nread;
        ptr += nread;
    }

    return n - nleft;
}

static int _net_write(struct net_handle *net, const void *data, size_t n)
{
    if (net->tls)
        return mbedtls_ssl_write(&net->ssl, (const unsigned char *)data, n);
    return socket_send(&net->sock, data, n);
}

int net_write(struct net_handle *net, const void *data, size_t n)
{
    const char *ptr = data;
    size_t nleft = n;
    int nwrite;

    while (nleft > 0) {
        nwrite = _net_write(net, ptr, nleft);
        if (nwrite == -1) {
            debug("_net_write error");
            return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }

    return n - nleft;
}

void net_close(struct net_handle *net)
{
    if (net->tls)
        tls_free(net);
    free(net->hostname);
    socket_close(&net->sock);
}
