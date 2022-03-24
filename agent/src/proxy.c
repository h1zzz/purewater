/* MIT License Copyright (c) 2022, h1zzz */

#include "proxy.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mbedtls/base64.h>

#include "debug.h"

struct http_proxy {
    char host[256];
    uint16_t port;
    char user[256];
    char passwd[256];
};

struct http_proxy *http_proxy_new(const char *host, uint16_t port,
                                  const char *user, const char *passwd) {
    struct http_proxy *proxy;

    ASSERT(host);
    ASSERT(port);

    proxy = calloc(1, sizeof(struct http_proxy));
    if (!proxy) {
        DBGERR("calloc");
        return NULL;
    }

    ASSERT(strlen(host) < sizeof(proxy->host));

    memcpy(proxy->host, host, strlen(host));
    proxy->port = port;

    if (!user || !passwd) {
        return proxy;
    }

    ASSERT(strlen(user) < sizeof(proxy->user));
    ASSERT(strlen(passwd) < sizeof(proxy->passwd));

    memcpy(proxy->user, user, strlen(user));
    memcpy(proxy->passwd, passwd, strlen(passwd));

    return proxy;
}

/*
 * CONNECT h1zzz.net:443 HTTP/1.1
 * Proxy-Authorization: Basic YWRtaW46MTIzNDU2
 * Proxy-Connection: Keep-Alive
 */

int http_proxy_connect(struct http_proxy *proxy, net_context *ctx,
                       const char *host, uint16_t port) {
    unsigned char base64[512] = {0}, str[256] = {0};
    char buf[1024] = {0};
    int ret;
    size_t olen, len = 0;

    ASSERT(proxy);
    ASSERT(ctx);
    ASSERT(host);
    ASSERT(port);

    ret = snprintf(buf + len, sizeof(buf) - len, "CONNECT %s:%hu HTTP/1.1\r\n",
                   host, port);
    ASSERT(ret > 0);

    len += ret;
    ASSERT(len < sizeof(buf));

    if (proxy->user[0] && proxy->passwd[0]) {
        ret = snprintf((char *)str, sizeof(str), "%s:%s", proxy->user,
                       proxy->passwd);
        ASSERT(ret > 0 && (size_t)ret < sizeof(str));

        /* Base64 encode the username and password */
        ret = mbedtls_base64_encode(base64, sizeof(base64), &olen, str,
                                    (size_t)ret);
        if (ret != 0) {
            DBG("mbedtls_base64_encode error");
            return -1;
        }

        /* Add the username and password to the request message */
        ret = snprintf(buf + len, sizeof(buf) - len,
                       "Proxy-Authorization: Basic %s\r\n", base64);
        ASSERT(ret > 0);

        len += ret;
        ASSERT(len < sizeof(buf));
    }

    ret = snprintf(buf + len, sizeof(buf) - len, "Proxy-Connection: %s\r\n\r\n",
                   "Keep-Alive");
    ASSERT(ret > 0);

    len += ret;
    ASSERT(len < sizeof(buf));

    net_init(ctx);

    DBGF("%s:%hd", proxy->host, proxy->port);

    ret = net_connect(ctx, proxy->host, proxy->port, NET_TCP);
    if (ret != 0) {
        DBG("net_connect error");
        goto err;
    }

    ret = net_send(ctx, (unsigned char *)buf, len);
    if (ret <= 0) {
        DBG("net_send error");
        goto err;
    }

    ret = net_recv(ctx, (unsigned char *)buf, sizeof(buf));
    if (ret <= 0) {
        DBG("net_recv error");
        goto err;
    }

    if (!strstr(buf, " 200 Connection established\r\n")) {
        DBGF("fail: %s", buf);
        goto err;
    }

    return 0;

err:
    net_free(ctx);
    return -1;
}

void http_proxy_free(struct http_proxy *proxy) {
    ASSERT(proxy);
    free(proxy);
}
