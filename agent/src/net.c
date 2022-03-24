/* MIT License Copyright (c) 2022, h1zzz */

#include "net.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else /* No define _WIN32 */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif /* _WIN32 */

#include <string.h>

#include "debug.h"
#include "dns.h"

#ifndef _WIN32
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket(fd) close(fd)
#endif /* _WIN32 */

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#ifdef _WIN32
static int wsa_init(void) {
    static int inited = 0;
    WSADATA wsaData;

    if (!inited) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            DBGERR("WSAStartup error");
            return -1;
        }
        inited = 1;
    }
    return 0;
}
#endif /* _WIN32 */

void net_init(net_context *ctx) {
    ASSERT(ctx);
    ctx->fd = INVALID_SOCKET;
}

int net_connect(net_context *ctx, const char *host, uint16_t port, int proto) {
    struct dns_node *dns_node, *curr;
    struct sockaddr_in addr;
    int ret = -1;

    ASSERT(ctx && ctx->fd == INVALID_SOCKET);
    ASSERT(host);
    ASSERT(port);
    ASSERT(proto == NET_TCP || proto == NET_UDP);

#ifdef _WIN32
    if (wsa_init() == -1) {
        DBG("wsa_init error");
        return -1;
    }
#endif /* _WIN32 */

    proto = (proto == NET_TCP) ? SOCK_STREAM : SOCK_DGRAM;

    dns_node = dns_query_ret(host, DNS_A);
    if (!dns_node) {
        DBG("dns_query_ret error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    for (curr = dns_node; curr; curr = curr->next) {
        if (curr->type != DNS_A) {
            continue;
        }

        ctx->fd = socket(PF_INET, proto, IPPROTO_IP);
        if (ctx->fd == INVALID_SOCKET) {
            DBGERR("socket error");
            continue;
        }

        ret = inet_pton(AF_INET, curr->data, &addr.sin_addr);
        if (ret != 1) {
            DBGERR("inet_pton error");
            closesocket(ctx->fd);
            continue;
        }

        ret = connect(ctx->fd, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == SOCKET_ERROR) {
            DBGERR("socket error");
            closesocket(ctx->fd);
            continue;
        }

        break;
    }

    dns_node_destroy(dns_node);

    if (ret != SOCKET_ERROR) {
        return 0;
    }

    return -1;
}

int net_recv(net_context *ctx, void *buf, size_t size) {
    int ret;

    ASSERT(ctx && ctx->fd != INVALID_SOCKET);
    ASSERT(buf);
    ASSERT(size);

again:
    ret = recv(ctx->fd, (char *)buf, (int)size, 0);
    if (ret <= 0) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEINTR) {
#else  /* No define _WIN32 */
        if (errno == EINTR) {
#endif /* _WIN32 */
            goto again;
        }
        DBGERR("send error");
        return -1;
    }

    return ret;
}

int net_send(net_context *ctx, const void *data, size_t len) {
    int ret;

    ASSERT(ctx && ctx->fd != INVALID_SOCKET);
    ASSERT(data);
    ASSERT(len);

again:
    ret = send(ctx->fd, (const char *)data, (int)len, 0);
    if (ret <= 0) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEINTR) {
#else  /* No define _WIN32 */
        if (errno == EINTR) {
#endif /* _WIN32 */
            goto again;
        }
        DBGERR("send error");
        return -1;
    }

    return ret;
}

void net_close(net_context *ctx) {
    ASSERT(ctx && ctx->fd != INVALID_SOCKET);
    closesocket(ctx->fd);
    ctx->fd = INVALID_SOCKET;
}

void net_free(net_context *ctx) {
    ASSERT(ctx);

    if (ctx->fd == INVALID_SOCKET) {
        return;
    }

#ifdef _WIN32
    shutdown(ctx->fd, SD_BOTH);
#else  /* No define _WIN32 */
    shutdown(ctx->fd, SHUT_RDWR);
#endif /* _WIN32 */

    closesocket(ctx->fd);
}
