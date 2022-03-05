/* MIT License Copyright (c) 2022, h1zzz */

#include "socks.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"

#define SOCKS5_VERSION 0x05

/* Auth method */
#define SOCKS5_NO_AUTHENTICATION_REQUIRED 0x00
/* #define SOCKS5_GSSAPI 0x01 */
#define SOCKS5_USERNAME_PASSWORD 0x02
/* #define SOCKS5_IANA_ASSIGNED '03' to X '7F' */
/* #define SOCKS5_RESERVED_FOR_PRIVATE_METHODS '80' to X 'FE' */
#define SOCKS5_NO_ACCEPTABLE_METHODS 0xff

/* Sock5 Address type */
#define SOCKS5_IPV4_ADDRESS 0x01
#define SOCKS5_DOMAINNAME 0x03
/* #define SOCKS5_ATYP_IPV6_ADDRESS 0x04 */

/* Socks5 Replies code */
#define SOCKS5_SUCCEEDED 0x00
#define SOCKS5_GENERAL_SOCKS_SERVER_FAILURE 0x01
#define SOCKS5_CONNECTION_NOT_ALLOWED_BY_RULESET 0x02
#define SOCKS5_NETWORK_UNREACHABLE 0x03
#define SOCKS5_HOST_UNREACHABLE 0x04
#define SOCKS5_CONNECTION_REFUSED 0x05
#define SOCKS5_TTL_EXPIRED 0x06
#define SOCKS5_COMMAND_NOT_SUPPORTED 0x07
#define SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED 0x08
/*  0x09 to X'FF' unassigned   0x09 */

/* Socks5 CMD */
#define SOCKS5_CONNECT 0x01
/* #define SOCKS5_BIND 0x02 */
/* #define SOCKS5_UDP_ASSOCIATE_UDP 0x03 */

struct socks5_client {
    char host[256];
    uint16_t port;
    char user[256];
    char passwd[256];
};

struct socks5_client *socks5_client_new(const char *host, uint16_t port,
                                        const char *user, const char *passwd)
{
    struct socks5_client *client;

    assert(host);
    assert(port);

    client = calloc(1, sizeof(struct socks5_client));
    if (!client) {
        dbgerr("calloc");
        return NULL;
    }

    assert(strlen(host) < sizeof(client->host));

    strcpy(client->host, host);
    client->port = port;

    if (!user || !passwd) {
        return client;
    }

    assert(strlen(user) < sizeof(client->user));
    assert(strlen(passwd) < sizeof(client->passwd));

    strcpy(client->user, user);
    strcpy(client->passwd, passwd);

    return client;
}

static int socks5_client_negotiate_auth_method(mbedtls_net_context *ctx,
                                               const uint8_t *methods,
                                               uint8_t nmethods)
{
    unsigned char buf[512];
    int ret, len = 0;

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

    ret = mbedtls_net_send(ctx, (unsigned char *)buf, len);
    if (ret <= 0) {
        debug("mbedtls_net_send error");
        return -1;
    }

    ret = mbedtls_net_recv(ctx, (unsigned char *)buf, sizeof(buf));
    if (ret <= 0) {
        debug("mbedtls_net_recv error");
        return -1;
    }

    /*
     * +----+--------+
     * |VER | METHOD |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    assert(ret >= 2);

    /* version */
    if (buf[0] != SOCKS5_VERSION) {
        debug("wrong socks proxy server version");
        return -1;
    }

    return buf[1];
}

static int socks5_client_username_password_auth(struct socks5_client *client,
                                                mbedtls_net_context *ctx)
{
    unsigned char buf[1024];
    int ret, len = 0;

    /*
     * +----+------+----------+------+----------+
     * |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
     * +----+------+----------+------+----------+
     * | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
     * +----+------+----------+------+----------+
     */

    buf[len++] = SOCKS5_VERSION;

    ret = (int)strlen(client->user);
    buf[len++] = (uint8_t)ret; /* ulen */

    memcpy(buf + len, client->user, ret); /* uname */
    len += ret;

    ret = (int)strlen(client->passwd);
    buf[len++] = (uint8_t)ret; /* plen */

    memcpy(buf + len, client->passwd, ret); /* passwd */
    len += ret;

    ret = mbedtls_net_send(ctx, buf, len);
    if (ret <= 0) {
        debug("mbedtls_net_send error");
        return -1;
    }

    ret = mbedtls_net_recv(ctx, buf, sizeof(buf));
    if (ret <= 0) {
        debug("mbedtls_net_recv error");
        return -1;
    }

    /*
     * +----+--------+
     * |VER | STATUS |
     * +----+--------+
     * | 1  |   1    |
     * +----+--------+
     */
    assert(ret >= 2);

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

static int socks5_client_request(mbedtls_net_context *ctx, uint8_t cmd,
                                 uint8_t atyp, const char *addr, uint16_t port)
{
    unsigned char buf[512] = {0};
    int ret, len = 0;

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
        ret = (int)strlen(addr);
        assert(ret < 256);
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

    ret = mbedtls_net_send(ctx, (unsigned char *)buf, len);
    if (ret <= 0) {
        debug("mbedtls_net_send error");
        return -1;
    }

    ret = mbedtls_net_recv(ctx, (unsigned char *)buf, sizeof(buf));
    if (ret <= 0) {
        debug("mbedtls_net_recv error");
        return -1;
    }

    assert(ret >= 2);

    /* version */
    if (buf[0] != SOCKS5_VERSION) {
        debug("wrong socks proxy server version");
        return -1;
    }

    return buf[1];
}

int socks5_client_connect(struct socks5_client *client,
                          mbedtls_net_context *ctx, const char *host,
                          uint16_t port)
{
    uint8_t atyp, nmethods, methods[255];
    int ret;
    char sport[8], ip[4];

    assert(client);
    assert(ctx);
    assert(host);
    assert(port);

    sprintf(sport, "%hu", client->port);
    mbedtls_net_init(ctx);

    ret = mbedtls_net_connect(ctx, client->host, sport, MBEDTLS_NET_PROTO_TCP);
    if (ret != 0) {
        debug("mbedtls_net_connect error");
        goto err;
    }

    nmethods = 0;
    methods[nmethods++] = SOCKS5_NO_AUTHENTICATION_REQUIRED;

    if (client->user[0] != 0 && client->passwd[0] != 0) {
        methods[nmethods++] = SOCKS5_USERNAME_PASSWORD;
    }

    ret = socks5_client_negotiate_auth_method(ctx, methods, nmethods);
    if (ret == -1) {
        debug("socks5 negotiate auth method fail");
        goto err;
    }

    switch (ret) {
    case SOCKS5_NO_AUTHENTICATION_REQUIRED:
        break;
    case SOCKS5_USERNAME_PASSWORD:
        ret = socks5_client_username_password_auth(client, ctx);
        if (ret == -1) {
            debug("username password auth fail");
            goto err;
        }
        break;
    default:
        debugf("unsupported authentication method: %d", ret);
        goto err;
    }

    if (inet_pton(AF_INET, host, ip) == 1) {
        atyp = SOCKS5_IPV4_ADDRESS;
    } else {
        atyp = SOCKS5_DOMAINNAME;
    }

    ret = socks5_client_request(ctx, SOCKS5_CONNECT, atyp, host, port);
    if (ret != SOCKS5_SUCCEEDED) {
        debugf("socks5 CONNECT %s:%hu error: %d", host, port, ret);
        goto err;
    }

    return 0;

err:
    mbedtls_net_free(ctx);
    return -1;
}

void socks5_client_free(struct socks5_client *client)
{
    assert(client);
    free(client);
}
