/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "socket.h"

#ifdef _WIN32
#    include <windows.h>
#else /* No define _WIN32 */
#    include <sys/socket.h>
#    include <unistd.h>
#    include <errno.h>
#    include <fcntl.h>
#endif /* _WIN32 */

#include "debug.h"

#ifdef _MSC_VER
#    pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#ifdef _WIN32
static int socket_init(void)
{
    static int inited = 0;
    WSADATA wsaData;
    int ret;

    if (inited)
        return 0;

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
        return -1;

    inited = 1;
    return 0;
}
#endif /* _WIN32 */

int socket_open(struct socket_handle *sock, int af, int type, int protocol)
{
#ifdef _WIN32
    if (socket_init() == -1) {
        debug("socket_init error");
        return -1;
    }
#endif /* _WIN32 */
    sock->fd = socket(af, type, protocol);
#ifdef _WIN32
    if (sock->fd == INVALID_SOCKET) {
        debug("create socket error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (sock->fd == -1) {
        debug("create socket error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_connect(struct socket_handle *sock, const struct sockaddr *addr,
                   socklen_t addrlen)
{
    int ret;

again:
    ret = connect(sock->fd, addr, addrlen);
#ifdef _WIN32
    if (ret == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("connect error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (ret == -1) {
        if (errno == EINTR)
            goto again;
        debug("connect error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_listen(struct socket_handle *sock, const struct sockaddr *addr,
                  socklen_t addrlen)
{
    int ret;

    ret = bind(sock->fd, addr, addrlen);
#ifdef _WIN32
    if (ret == SOCKET_ERROR) {
        debug("bind address error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (ret == -1) {
        debug("bind address error");
        return -1;
    }
#endif /* _WIN32 */

    ret = listen(sock->fd, SOMAXCONN);
#ifdef _WIN32
    if (ret == SOCKET_ERROR) {
        debug("listen error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (ret == -1) {
        debug("listen error");
        return -1;
    }
#endif /* _WIN32 */

    return 0;
}

int socket_accept(struct socket_handle *sock, struct socket_handle *conn,
                  struct sockaddr *addr, socklen_t *addrlen)
{
again:
    conn->fd = accept(sock->fd, addr, addrlen);
#ifdef _WIN32
    if (conn->fd == INVALID_SOCKET) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("accept error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (conn->fd == -1) {
        if (errno == EINTR)
            goto again;
        debug("accept error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_recv(struct socket_handle *sock, void *buf, size_t size)
{
    int nread;

again:
#ifdef _WIN32
    nread = recv(sock->fd, buf, (int)size, 0);
    if (nread == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("recv error");
        return -1;
    }
#else  /* No defined _WIN32 */
    nread = recv(sock->fd, buf, size, MSG_NOSIGNAL);
    if (nread == -1) {
        if (errno == EINTR)
            goto again;
        debug("recv error");
        return -1;
    }
#endif /* _WIN32 */
    return nread;
}

int socket_send(struct socket_handle *sock, const void *data, size_t n)
{
    int nwrite;

again:
#ifdef _WIN32
    nwrite = send(sock->fd, data, (int)n, 0);
    if (nwrite == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("send error");
        return -1;
    }
#else  /* No defined _WIN32 */
    nwrite = send(sock->fd, data, n, MSG_NOSIGNAL);
    if (nwrite == -1) {
        if (errno == EINTR)
            goto again;
        debug("send error");
        return -1;
    }
#endif /* _WIN32 */
    return nwrite;
}

int socket_set_blocking(struct socket_handle *sock)
{
#ifdef _WIN32
    u_long opt = 0;
    if (ioctlsocket(sock->fd, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("set block error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(sock->fd, F_GETFL);
    if (flags == -1) {
        debug("set block GETFL error");
        return -1;
    }

    if (fcntl(sock->fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        debug("set block SETFL error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_set_nonblocking(struct socket_handle *sock)
{
#ifdef _WIN32
    u_long opt = 1;
    if (ioctlsocket(sock->fd, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("set nonblock error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(sock->fd, F_GETFL);
    if (flags == -1) {
        debug("set nonblock GETFL error");
        return -1;
    }

    if (fcntl(sock->fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        debug("set nonblock SETFL error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

void socket_shutdown(struct socket_handle *sock)
{
#ifdef _WIN32
    shutdown(sock->fd, SD_BOTH);
#else  /* No defined _WIN32 */
    shutdown(sock->fd, SHUT_RDWR);
#endif /* _WIN32 */
}

void socket_close(struct socket_handle *sock)
{
#ifdef _WIN32
    closesocket(sock->fd);
#else  /* No defined _WIN32 */
    close(sock->fd);
#endif /* _WIN32 */
}
