/* MIT License Copyright (c) 2021, h1zzz */

#include "network.h"

#include "debug.h"
#include "dns.h"

static socket_t wrap_connect(const char *domain, uint16_t port, int type)
{
    struct dns_node *dns_node, *dns_ptr;
    socket_t sock;
    int ret;

    dns_node = dns_lookup_ret(domain, DNS_A);
    if (!dns_node) {
        debug("dns_lookup_ret error");
        return SOCK_INVAL;
    }

    for (dns_ptr = dns_node; dns_ptr; dns_ptr = dns_ptr->next) {
        sock = xsocket(type);
        if (sock == SOCK_INVAL) {
            debug("xsocket error");
            return SOCK_INVAL;
        }
        ret = xconnect(sock, dns_ptr->data, port);
        if (ret == -1) {
            debug("xconnect error");
            xclose(sock);
            sock = SOCK_INVAL;
            continue;
        }
        break;
    }

    dns_node_cleanup(dns_node);

    if (sock == SOCK_INVAL) {
        debug("connect socks5 proxy server error");
        return SOCK_INVAL;
    }

    return sock;
}

socket_t tcp_connect(const char *domain, uint16_t port)
{
    assert(domain);
    assert(port != 0);
    return wrap_connect(domain, port, SOCK_TCP);
}

socket_t udp_connect(const char *domain, uint16_t port)
{
    assert(domain);
    assert(port != 0);
    return wrap_connect(domain, port, SOCK_UDP);
}
