/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "socks.h"

#include <string.h>

#include "debug.h"
#include "dns.h"
#include "llist.h"
#include "util.h"

/* socks5 client */
int socks5_handshake(struct socket_handler *handler, const char *host,
                     uint16_t port, const struct proxy *proxy)
{
    unsigned char buf[256] = {0x05, 0x01, 0x00};
    size_t n = 3, len;
    int ret;

    (void)host;
    (void)port;

    ret = util_connect_tcp(handler, proxy->host, proxy->port);
    if (ret == -1) {
        debug("util_connect_tcp error");
        return -1;
    }

    if (proxy->user && proxy->passwd) {
        buf[1] = 2;    /* NMETHODS */
        buf[3] = 0x02; /* USERNAME/PASSWORD */
        n += 1;
    }

    ret = socket_write(handler, buf, n);
    if (ret == -1) {
        debug("socket_write error");
        goto err;
    }

    ret = socket_read(handler, buf, sizeof(buf));
    if (ret == -1) {
        debug("socket_read error");
        goto err;
    }

    if (ret != 2 && buf[0] != 0x05) {
        debug("unsupported protocol");
        goto err;
    }

    if (buf[1] == 0x02) {
        /* Username/Password Authentication for SOCKS V5 */
        n = 0;
        buf[n++] = 0x05;

        len = strlen(proxy->user);
        buf[n++] = len;
        memcpy(buf + n, proxy->user, len);
        n += len;

        len = strlen(proxy->passwd);
        buf[n++] = len;
        memcpy(buf + n, proxy->passwd, len);
        n += len;

        ret = socket_write(handler, buf, n);
        if (ret == -1) {
            debug("socket_write error");
            goto err;
        }

        ret = socket_read(handler, buf, sizeof(buf));
        if (ret == -1) {
            debug("socket_read error");
            goto err;
        }

        if (buf[1] != 0x00) {
            debug("authentication failure");
            goto err;
        }
    } else if (buf[1] != 0x00) {
        debug("unsupported authentication protocol");
        goto err;
    }

    n = 0;
    buf[n++] = 0x05; /* VER */
    buf[n++] = 0x01; /* CONNECT */
    buf[n++] = 0x00; /* RSV */

    if (util_is_ipv4(host)) {
        /* IP V4 address */
        buf[n++] = 0x01; /* ATYP */
        ret = inet_pton(AF_INET, host, buf + n);
        if (ret <= 0) {
            debug("inet_pton error");
            goto err;
        }
        n += 4;
    } else if (util_is_ipv6(host)) {
        /* IP V6 address */
        /* buf[n++] = 0x04; */ /* ATYP */
        debug("unsupported IPv6");
        goto err;
    } else {
        /* DOMAINNAME */
        buf[n++] = 0x03; /* ATYP */
        len = strlen(host);
        buf[n++] = len;
        memcpy(buf + n, host, len);
        n += len;
    }

    *(uint16_t *)(buf + n) = htons(port);
    n += 2;

    ret = socket_write(handler, buf, n);
    if (ret == -1) {
        debug("socket_write error");
        goto err;
    }

    ret = socket_read(handler, buf, sizeof(buf));
    if (ret == -1) {
        debug("socket_read error");
        goto err;
    }

    if (ret < 2 || buf[1] != 0x00) {
        debugf("socks5 connect error, %d", buf[1]);
        goto err;
    }

    return 0;
err:
    socket_close(handler);
    return -1;
}
