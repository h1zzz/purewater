/* MIT License Copyright (c) 2021, h1zzz */

#include "socks.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <string.h>

#include "debug.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#define SOCKS5_VERSION 0x05

/*
 * https://datatracker.ietf.org/doc/html/rfc1928
 * https://datatracker.ietf.org/doc/html/rfc1929
 */

int socks5_client_negotiate_auth_method(socket_t sock, const uint8_t *methods,
                                        uint8_t nmethods)
{
    unsigned char buf[512];
    int ret, len = 0;

    assert(sock != SOCK_INVAL);
    assert(methods);
    assert(nmethods != 0);

    /*
     * +----+----------+----------+
     * |VER | NMETHODS | METHODS  |
     * +----+----------+----------+
     * | 1  |    1     | 1 to 255 |
     * +----+----------+----------+
     */
    buf[len++] = SOCKS5_VERSION;
    buf[len++] = nmethods;

    memcpy(buf + len, methods, nmethods);
    len += nmethods;

    ret = xsend(sock, buf, len);
    if (ret == -1) {
        debug("xsend error");
        return -1;
    }

    ret = xrecv(sock, buf, sizeof(buf));
    if (ret == -1) {
        debug("xrecv error");
        return -1;
    }

    /*
     * +----+--------+
     * |VER | METHOD |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    if (ret != 2) {
        debug("invalid negotiation result");
        return -1;
    }

    /* version */
    if (buf[0] != SOCKS5_VERSION) {
        debug("wrong socks proxy server version");
        return -1;
    }

    return buf[1]; /* method */
}

int socks5_client_username_password_auth(socket_t sock, const char *user,
                                         const char *passwd)
{
    unsigned char buf[1024];
    int ret, len = 0;

    assert(sock != SOCK_INVAL);
    assert(user);
    assert(passwd);

    /*
     * +----+------+----------+------+----------+
     * |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
     * +----+------+----------+------+----------+
     * | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
     * +----+------+----------+------+----------+
     */

    buf[len++] = SOCKS5_VERSION;

    ret = strlen(user);
    if (ret >= 256) {
        debugf("username exceeds length limit: %s", user);
        return -1;
    }

    buf[len++] = (uint8_t)ret; /* ulen */

    memcpy(buf + len, user, ret); /* uname */
    len += ret;

    ret = strlen(passwd);
    if (ret >= 256) {
        debugf("password exceeds length limit: %s", passwd);
        return -1;
    }

    buf[len++] = (uint8_t)ret; /* plen */

    memcpy(buf + len, user, ret); /* passwd */
    len += ret;

    ret = xsend(sock, buf, len);
    if (ret == -1) {
        debug("xsend error");
        return -1;
    }

    ret = xrecv(sock, buf, sizeof(buf));
    if (ret == -1) {
        debug("xrecv error");
        return -1;
    }

    /*
     * +----+--------+
     * |VER | STATUS |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    if (ret != 2) {
        debug("invalid authentication result message");
        return -1;
    }

    /* version */
    if (buf[0] != SOCKS5_VERSION) {
        debug("wrong socks proxy server version");
        return -1;
    }

    /* A STATUS field of X'00' indicates success. If the server returns a
      `failure' (STATUS value other than X'00') status, it MUST close the
       connection. */
    if (buf[1] != 0x00) {
        debug("authentication failed");
        return -1;
    }

    return 0;
}

int socks5_client_request(socket_t sock, uint8_t cmd, uint8_t atyp,
                          const char *addr, uint16_t port)
{
    unsigned char buf[512];
    int ret, len = 0;

    assert(sock != SOCK_INVAL);
    assert(addr);
    assert(atyp == SOCKS5_IPV4_ADDRESS || atyp == SOCKS5_DOMAINNAME);
    assert(port != 0);

    /*
     * +----+-----+-------+------+----------+----------+
     * |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
     * +----+-----+-------+------+----------+----------+
     * | 1  |  1  | X'00' |  1   | Variable |    2     |
     * +----+-----+-------+------+----------+----------+
     */

    buf[len++] = SOCKS5_VERSION;
    buf[len++] = cmd;
    buf[len++] = 0; /* RSV */
    buf[len++] = atyp;

    switch (atyp) {
    case SOCKS5_IPV4_ADDRESS:
        ret = inet_pton(AF_INET, addr, buf + len);
        if (ret <= 0) {
            debugf("inet_pton error: %s", addr);
            return -1;
        }
        len += 4; /* IPv4 address 32bit */
        break;
    case SOCKS5_DOMAINNAME:
        ret = strlen(addr);
        if (ret == 0 || ret >= 256) {
            debugf("The length of the domain name exceeds the limit: %s", addr);
            return -1;
        }
        buf[len++] = (uint8_t)ret; /* domain length */
        memcpy(buf + len, addr, ret);
        len += ret;
        break;
    default:
        debug("unsupported address type");
        return -1;
    }

    *((uint16_t *)(buf + len)) = htons(port);
    len += 2;

    ret = xsend(sock, buf, len);
    if (ret == -1) {
        debug("xsend error");
        return -1;
    }

    ret = xrecv(sock, buf, sizeof(buf));
    if (ret == -1) {
        debug("xrecv error");
        return -1;
    }

    if (ret < 2) {
        debug("invalid packet");
        return -1;
    }

    /* version */
    if (buf[0] != SOCKS5_VERSION) {
        debug("wrong socks proxy server version");
        return -1;
    }

    /* rep */
    switch (buf[1]) {
    case SOCKS5_SUCCEEDED:
        return 0;
    default:
        debugf("rep: %d", buf[1]);
        break;
    }

    return -1;
}
