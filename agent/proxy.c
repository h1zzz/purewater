/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "proxy.h"

#include <string.h>

#include "debug.h"
#include "socket.h"
#include "socks5.h"
#include "util.h"

int proxy_socks5_handshake(struct net_handle *net, const char *host,
                           uint16_t port, const char *username,
                           const char *password)
{
    int ret, atyp;

    ret = socks5_client_send_method(net, username && password);
    if (ret == -1) {
        debug("socks5_client_send_method error");
        return -1;
    }

    switch (ret) {
    case SOCKS5_NO_AUTHENTICATION_REQUIRED:
        /* No authentication required */
        break;
    case SOCKS5_USERNAME_PASSWORD: {
        /* Use username and password to authenticate */
        ret = socks5_client_send_password_auth(net, username, password);
        if (ret == -1) {
            /* Authentication failed */
            debug("authentication failed");
            return -1;
        }
        break;
    }
    default:
        debugf("unsupported authentication method: %d", ret);
        return -1;
    }

    if (check_is_ipv4(host)) {
        atyp = SOCKS5_IPV4_ADDRESS;
    } else if (strchr(host, ':')) {
        /*
         * Whether there is a ':' character in the host to determine whether it
         * is an IPv6 address, it needs to be optimized
         * TODO: Support IPv6
         */
        debug("does not support IPv6 proxy");
        return -1;
    } else {
        /* TODO: Need to check if it is a legal domain name */
        atyp = SOCKS5_DOMAINNAME;
    }

    ret = socks5_client_request(net, SOCKS5_CONNECT, atyp, host, port);
    if (ret != SOCKS5_SUCCEEDED) {
        debug("socks5_client_request error");
        return -1;
    }

    return 0;
}

/* int proxy_https_handshake(struct net_handle *net, const char *host,
                          uint16_t port, const char *username,
                          const char *password)
{
    return 0;
} */
