/* MIT License Copyright (c) 2021, h1zzz */

#include "proxy_socks.h"

#include <string.h>

#include "debug.h"
#include "dns.h"
#include "network.h"
#include "socket.h"
#include "socks.h"
#include "util.h"

socket_t proxy_socks5(const char *host, uint16_t port, const char *proxy_host,
                      uint16_t proxy_port, const char *proxy_user,
                      const char *proxy_passwd)
{
    uint8_t atyp, nmethods, methods[255];
    socket_t sock;
    int ret;

    sock = tcp_connect(proxy_host, proxy_port);
    if (sock == SOCK_INVAL) {
        debug("connect socks5 proxy server error");
        return SOCK_INVAL;
    }

    nmethods = 0;
    methods[nmethods++] = SOCKS5_NO_AUTHENTICATION_REQUIRED;

    if (proxy_user && proxy_passwd)
        methods[nmethods++] = SOCKS5_USERNAME_PASSWORD;

    ret = socks5_client_negotiate_auth_method(sock, methods, nmethods);
    if (ret == -1) {
        debug("socks5 negotiate auth method fail");
        goto err;
    }

    /* method */
    switch (ret) {
    case SOCKS5_NO_AUTHENTICATION_REQUIRED:
        break;
    case SOCKS5_USERNAME_PASSWORD:
        ret = socks5_client_username_password_auth(sock, proxy_user,
                                                   proxy_passwd);
        if (ret == -1) {
            debug("username password auth fail");
            goto err;
        }
        break;
    default:
        debugf("unsupported authentication method: %d", ret);
        goto err;
    }

    if (is_ipv4(host)) {
        atyp = SOCKS5_IPV4_ADDRESS;
    } else if (strchr(host, ':')) {
        /*
         * Whether there is a ':' character in the host to determine whether it
         * is an IPv6 address, it needs to be optimized
         * TODO: Support IPv6
         */
        debugf("does not support IPv6 proxy: %s", host);
        goto err;
    } else {
        /* TODO: Need to check if it is a legal domain name */
        atyp = SOCKS5_DOMAINNAME;
    }

    ret = socks5_client_request(sock, SOCKS5_CONNECT, atyp, host, port);
    if (ret != SOCKS5_SUCCEEDED) {
        debug("socks5 connect proxy error");
        goto err;
    }

    return sock;

err:
    xclose(sock);
    return SOCK_INVAL;
}
