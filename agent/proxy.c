/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "proxy.h"

#include <string.h>
#include <stdio.h>

#include <mbedtls/base64.h>

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

/*
 * CONNECT h1zzz.net:443 HTTP/1.1
 * Host: h1zzz.net:443
 * Proxy-Authorization: Basic YWRtaW46MTIzNDU2
 * User-Agent: client
 * Proxy-Connection: Keep-Alive
 */
int proxy_https_handshake(struct net_handle *net, const char *host,
                          uint16_t port, const char *username,
                          const char *password)
{
    unsigned char dst[1024] = {0}, src[512] = {0};
    char buf[2048] = {0};
    int ret;
    size_t olen, len = 0;

    ret = snprintf(buf + len, sizeof(buf) - len,
                   "CONNECT %s:%hd HTTP/1.1\r\nHost: %s:%hd\r\n", host, port,
                   host, port);
    if (ret <= 0 || (size_t)ret >= sizeof(buf)) {
        debug("snprintf error");
        return -1;
    }
    len += ret;

    if (username && password) {
        /* format username and password */
        ret = snprintf((char *)src, sizeof(src), "%s:%s", username, password);
        if (ret <= 0 || (size_t)ret >= sizeof(src)) {
            debugf("username and password exceed size limit (%s:%s)", username,
                   password);
            return -1;
        }
        /* Base64 encode the username and password */
        ret = mbedtls_base64_encode(dst, sizeof(dst), &olen, src, (size_t)ret);
        if (ret != 0) {
            debug("mbedtls_base64_encode error");
            return -1;
        }
        /* Add the username and password to the request message */
        ret = snprintf(buf + len, sizeof(buf) - len,
                       "Proxy-Authorization: Basic %s\r\n", dst);
        if (ret <= 0 || (size_t)ret >= sizeof(buf)) {
            debug("snprintf error");
            return -1;
        }
        len += ret;
    }

    ret = snprintf(buf + len, sizeof(buf) - len,
                   "User-Agent: %s\r\nProxy-Connection: %s\r\n\r\n", "client",
                   "Keep-Alive");
    if (ret <= 0 || (size_t)ret >= sizeof(buf)) {
        debug("snprintf error");
        return -1;
    }
    len += ret;

    ret = net_write(net, buf, len);
    if (ret == -1) {
        debug("net_write error");
        return -1;
    }

    ret = net_read(net, buf, sizeof(buf));
    if (ret == -1) {
        debug("net_read error");
        return -1;
    }

    if (memcmp(buf, "HTTP/1.1 200", 12) != 0) {
        debugf("http proxy failed: %s", buf);
        return -1;
    }

    return 0;
}
