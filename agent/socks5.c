/*
 * MIT License Copyright (c) 2021, h1zzz
 *
 * https://datatracker.ietf.org/doc/html/rfc1928
 * https://datatracker.ietf.org/doc/html/rfc1929
 */

#include "socks5.h"

#include <string.h>

#include "debug.h"
#include "llist.h"
#include "util.h"

#define SOCKS5_VERSION 5

int socks5_client_send_method(struct net_handle *net, int use_password)
{
    unsigned char buf[32] = {0};
    uint8_t nmethods = 1; /* NO AUTHENTICATION REQUIRED */
    int ret, n = 0;

    /*
     * The client connects to the server, and sends a version
     * identifier/method selection message:
     * +----+----------+----------+
     * |VER | NMETHODS | METHODS  |
     * +----+----------+----------+
     * | 1  |    1     | 1 to 255 |
     * +----+----------+----------+
     */

    if (use_password)
        nmethods++;

    buf[n++] = SOCKS5_VERSION;                    /* VER */
    buf[n++] = nmethods;                          /* NMETHODS */
    buf[n++] = SOCKS5_NO_AUTHENTICATION_REQUIRED; /* METHOD */

    if (use_password)
        buf[n++] = SOCKS5_USERNAME_PASSWORD; /* METHOD */

    ret = net_write(net, buf, n);
    if (ret == -1) {
        debug("net_write error");
        return -1;
    }

    /*
     * The server selects from one of the methods given in METHODS, and
     * sends a METHOD selection message:
     * +----+--------+
     * |VER | METHOD |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    ret = net_read(net, buf, sizeof(buf));
    if (ret == -1) {
        debug("net_read error");
        return -1;
    }

    /* Verify message length */
    if (ret != 2) {
        debugf("packet length error: %d", ret);
        return -1;
    }

    /* Verify the socks version */
    if (buf[0] != SOCKS5_VERSION) {
        debugf("incorrect protocol version: %d", buf[0]);
        return -1;
    }

    return buf[1];
}

int socks5_client_send_password_auth(struct net_handle *net, const char *uname,
                                     const char *passwd)
{
    /*
     * +----+------+----------+------+----------+
     * |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
     * +----+------+----------+------+----------+
     * | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
     * +----+------+----------+------+----------+
     */
    unsigned char buf[1024] = {0};
    int n = 0, len, ret;

    buf[n++] = SOCKS5_VERSION; /* VER */

    len = strlen(uname);
    /* The size of the username and password ranges from 0 to 255 */
    if (len == 0 || len > 255) {
        debugf("incorrect username format: %s", uname);
        return -1;
    }

    buf[n++] = len; /* ULEN */
    while (len--)
        buf[n++] = *uname++; /* UNAME */

    len = strlen(passwd);
    if (len == 0 || len > 255) {
        debugf("incorrect password format: %s", passwd);
        return -1;
    }

    buf[n++] = len; /* PLEN */
    while (len--)
        buf[n++] = *passwd++; /* PASSWD */

    ret = net_write(net, buf, n);
    if (ret == -1) {
        debug("net_write error");
        return -1;
    }

    /*
     * The server verifies the supplied UNAME and PASSWD, and sends the
     * following response:
     * +----+--------+
     * |VER | STATUS |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    ret = net_read(net, buf, sizeof(buf));
    if (ret == -1) {
        debug("net_read error");
        return -1;
    }

    /* Verify message length */
    if (ret != 2) {
        debugf("packet length error: %d", ret);
        return -1;
    }

    /* Verify the socks version */
    if (buf[0] != SOCKS5_VERSION) {
        debugf("incorrect protocol version: %d", buf[0]);
        return -1;
    }

    /*
     * A STATUS field of X'00' indicates success. If the server returns a
     * `failure' (STATUS value other than X'00') status, it MUST close the
     * connection.
     */
    return buf[1] == 0;
}

int socks5_client_request(struct net_handle *net, int cmd, int atyp,
                          const char *dst_addr, uint16_t dst_port)
{
    /*
     * The SOCKS request is formed as follows:
     * +----+-----+-------+------+----------+----------+
     * |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
     * +----+-----+-------+------+----------+----------+
     * | 1  |  1  | X'00' |  1   | Variable |    2     |
     * +----+-----+-------+------+----------+----------+
     */
    unsigned char buf[1024] = {0};
    int n = 0, ret, len;

    buf[n++] = SOCKS5_VERSION; /* VER */
    buf[n++] = cmd;            /* CMD */
    buf[n++] = 0;              /* RSV */
    buf[n++] = atyp;           /* ATYP */

    switch (atyp) {
    case SOCKS5_IPV4_ADDRESS: {
        ret = inet_pton(AF_INET, dst_addr, buf + n);
        if (ret <= 0) {
            debugf("inet_pton error: %s", dst_addr);
            return -1;
        }
        /* the address is a version-4 IP address, with a length of 4 octets */
        n += 4;
        break;
    }
    case SOCKS5_DOMAINNAME: {
        /*
         * the address field contains a fully-qualified domain name.  The first
         * octet of the address field contains the number of octets of name that
         * follow, there is no terminating NUL octet.
         */
        len = strlen(dst_addr);
        if (len == 0 || len > 255) {
            debugf("incorrect domain format: %s", dst_addr);
            return -1;
        }
        buf[n++] = len; /* DST.ADDR Length */
        while (len--)
            buf[n++] = *dst_addr++; /* DST.ADDR */
        break;
    }
    default:
        debug("unsupported address type");
        return -1;
    }

    /* desired destination port in network octet order, 2 bytes */
    *(uint16_t *)(&buf[n]) = htons(dst_port);
    n += 2;

    ret = net_write(net, buf, n);
    if (ret == -1) {
        debug("net_write error");
        return -1;
    }

    /*
     * The SOCKS request information is sent by the client as soon as it has
     * established a connection to the SOCKS server, and completed the
     * authentication negotiations.  The server evaluates the request, and
     * returns a reply formed as follows:
     * +----+-----+-------+------+----------+----------+
     * |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
     * +----+-----+-------+------+----------+----------+
     * | 1  |  1  | X'00' |  1   | Variable |    2     |
     * +----+-----+-------+------+----------+----------+
     */
    ret = net_read(net, buf, sizeof(buf));
    if (ret == -1) {
        debug("net_read error");
        return -1;
    }

    /* Verify message length */
    if (ret <= 4) {
        debugf("packet length error: %d", ret);
        return -1;
    }

    /* Verify the socks version */
    if (buf[0] != SOCKS5_VERSION) {
        debugf("incorrect protocol version: %d", buf[0]);
        return -1;
    }

    return 0;
}

#if 0
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
#endif