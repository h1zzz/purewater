/* MIT License Copyright (c) 2021, h1zzz */

#include "socket.h"

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

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#ifdef _WIN32
typedef int socklen_t;
#else /* No define _WIN32 */
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif /* _WIN32 */

#ifdef _WIN32
static int wsa_init(void)
{
    static int inited = 0;
    WSADATA wsaData;

    if (!inited) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            debug("WSAStartup error");
            return -1;
        }
        inited = 1;
    }
    return 0;
}
#endif /* _WIN32 */

static int init_sockaddr(struct sockaddr_in *addr, const char *host,
                         uint16_t port)
{
    memset(addr, 0, sizeof(struct sockaddr_in));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr->sin_addr) <= 0) {
        debug("inet_pton error");
        return -1;
    }
    return 0;
}

socket_t xsocket(int type)
{
    socket_t sock;

    assert(type == SOCK_TCP || type == SOCK_UDP);

#ifdef _WIN32
    if (wsa_init() == -1) {
        debug("wsa_init error");
        return SOCK_INVAL;
    }
#endif /* _WIN32 */

    type = (type == SOCK_TCP) ? SOCK_STREAM : SOCK_DGRAM;

    sock = socket(PF_INET, type, IPPROTO_IP);
    if (sock == SOCK_INVAL) {
        debug("socket error");
    }
    return sock;
}

int xconnect(socket_t sock, const char *host, uint16_t port)
{
    struct sockaddr_in addr;
    int ret;

    assert(sock != SOCK_INVAL);
    assert(host);
    assert(port != 0);

    if (init_sockaddr(&addr, host, port) == -1) {
        debug("init_sockaddr error");
        return -1;
    }
again:
    ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEINTR) {
            goto again;
        }
#else  /* No define _WIN32 */
        if (errno == EINTR) {
            goto again;
        }
        debug("connect error");
        return -1;
#endif /* _WIN32 */
    }
    return 0;
}

int xlisten(socket_t sock, const char *host, uint16_t port)
{
    struct sockaddr_in addr;
    int ret;

    assert(sock != SOCK_INVAL);
    assert(host);
    assert(port != 0);

    if (init_sockaddr(&addr, host, port) == -1) {
        debug("init_sockaddr error");
        return -1;
    }

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        debug("bind error");
        return -1;
    }

    ret = listen(sock, SOMAXCONN);
    if (ret == -1) {
        debug("listen error");
        return -1;
    }

    return 0;
}

socket_t xaccept(socket_t sock, struct xsockaddr *addr)
{
    struct sockaddr_in si;
    socklen_t silen;
    socket_t conn;

    assert(sock != SOCK_INVAL);
    assert(addr);

    memset(&si, 0, sizeof(si));
    silen = (socklen_t)sizeof(si);

again:
    conn = accept(sock, (struct sockaddr *)&si, &silen);
    if (conn == SOCK_INVAL) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEINTR) {
            goto again;
        }
#else  /* No define _WIN32 */
        if (errno == EINTR) {
            goto again;
        }
#endif /* _WIN32 */
        debug("accept error");
        return SOCK_INVAL;
    }

    if (addr) {
        if (inet_ntop(AF_INET, &si.sin_addr.s_addr, addr->host,
                      sizeof(addr->host)) == NULL) {
            debug("inet_ntop error");
            xclose(conn);
            return SOCK_INVAL;
        }
        addr->port = ntohs(si.sin_port);
    }
    return conn;
}

int xrecv(socket_t sock, void *buf, size_t size)
{
    int ret;

    assert(sock != SOCK_INVAL);
    assert(buf);
    assert(size != 0);

again:
#ifdef _WIN32
    ret = recv(sock, buf, (int)size, 0);
    if (ret == -1) {
        if (WSAGetLastError() == WSAEINTR) {
            goto again;
        }
        debug("recv error");
        return -1;
    }
#else  /* No define _WIN32 */
    ret = recv(sock, buf, size, MSG_NOSIGNAL);
    if (ret == -1) {
        if (errno == EINTR) {
            goto again;
        }
        debug("recv error");
        return -1;
    }
#endif /* _WIN32 */
    return ret;
}

int xsend(socket_t sock, const void *data, size_t len)
{
    int ret;

    assert(sock != SOCK_INVAL);
    assert(data);
    assert(len != 0);

again:
#ifdef _WIN32
    ret = send(sock, data, (int)len, 0);
    if (ret == -1) {
        if (WSAGetLastError() == WSAEINTR) {
            goto again;
        }
        debug("send error");
        return -1;
    }
#else  /* No define _WIN32 */
    ret = send(sock, data, len, MSG_NOSIGNAL);
    if (ret == -1) {
        if (errno == EINTR) {
            goto again;
        }
        debug("send error");
        return -1;
    }
#endif /* _WIN32 */
    return ret;
}

void xshutdown(socket_t sock)
{
    assert(sock != SOCK_INVAL);
#ifdef _WIN32
    shutdown(sock, SD_BOTH);
#else  /* No define _WIN32 */
    shutdown(sock, SHUT_RDWR);
#endif /* _WIN32 */
}

void xclose(socket_t sock)
{
    assert(sock != SOCK_INVAL);
#ifdef _WIN32
    closesocket(sock);
#else  /* No define _WIN32 */
    close(sock);
#endif /* _WIN32 */
}

int set_blocking(socket_t sock)
{
    assert(sock != SOCK_INVAL);
#ifdef _WIN32
    u_long opt = 0;
    if (ioctlsocket(sock, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("ioctlsocket error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(sock, F_GETFL);
    if (flags == -1) {
        debug("fcntl error");
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        debug("fcntl error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int set_nonblocking(socket_t sock)
{
    assert(sock != SOCK_INVAL);
#ifdef _WIN32
    u_long opt = 1;
    if (ioctlsocket(sock, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("ioctlsocket error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(sock, F_GETFL);
    if (flags == -1) {
        debug("fcntl error");
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        debug("fcntl error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}
