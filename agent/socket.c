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

#ifdef _WIN32
#    pragma comment(lib, "ws2_32.lib")
#endif /* _WIN32 */

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

int socket_open(struct socket_handler *handler, int af, int type, int protocol)
{
#ifdef _WIN32
    if (socket_init() == -1) {
        debug("socket_init error");
        return -1;
    }
#endif /* _WIN32 */
    handler->fd = socket(af, type, protocol);
#ifdef _WIN32
    if (handler->fd == INVALID_SOCKET) {
        debug("create socket error");
        return -1;
    }
#else  /* No defined _WIN32 */
    if (handler->fd == -1) {
        debug("create socket error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_connect(struct socket_handler *handler, const struct sockaddr *addr,
                   socklen_t addrlen)
{
    int ret;

again:
    ret = connect(handler->fd, addr, addrlen);
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

int socket_listen(struct socket_handler *handler, const struct sockaddr *addr,
                  socklen_t addrlen)
{
    int ret;

    ret = bind(handler->fd, addr, addrlen);
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

    ret = listen(handler->fd, SOMAXCONN);
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

int socket_accept(struct socket_handler *handler, struct socket_handler *conn,
                  struct sockaddr *addr, socklen_t *addrlen)
{
again:
    conn->fd = accept(handler->fd, addr, addrlen);
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

int socket_read(struct socket_handler *handler, void *buf, size_t size)
{
    int nread;

again:
#ifdef _WIN32
    nread = recv(handler->fd, buf, (int)size, 0);
    if (nread == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("recv error");
        return -1;
    }
#else  /* No defined _WIN32 */
    nread = recv(handler->fd, buf, size, MSG_NOSIGNAL);
    if (nread == -1) {
        if (errno == EINTR)
            goto again;
        debug("recv error");
        return -1;
    }
#endif /* _WIN32 */
    return nread;
}

int socket_write(struct socket_handler *handler, const void *data, size_t n)
{
    int nwrite;

again:
#ifdef _WIN32
    nwrite = send(handler->fd, data, (int)n, 0);
    if (nwrite == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            goto again;
        debug("send error");
        return -1;
    }
#else  /* No defined _WIN32 */
    nwrite = send(handler->fd, data, n, MSG_NOSIGNAL);
    if (nwrite == -1) {
        if (errno == EINTR)
            goto again;
        debug("send error");
        return -1;
    }
#endif /* _WIN32 */
    return nwrite;
}

int socket_set_blocking(struct socket_handler *handler)
{
#ifdef _WIN32
    u_long opt = 0;
    if (ioctlsocket(handler->fd, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("set block error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(handler->fd, F_GETFL);
    if (flags == -1) {
        debug("set block GETFL error");
        return -1;
    }

    if (fcntl(handler->fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        debug("set block SETFL error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

int socket_set_nonblocking(struct socket_handler *handler)
{
#ifdef _WIN32
    u_long opt = 1;
    if (ioctlsocket(handler->fd, FIONBIO, &opt) == SOCKET_ERROR) {
        debug("set nonblock error");
        return -1;
    }
#else  /* No define _WIN32 */
    int flags = fcntl(handler->fd, F_GETFL);
    if (flags == -1) {
        debug("set nonblock GETFL error");
        return -1;
    }

    if (fcntl(handler->fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        debug("set nonblock SETFL error");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}

void socket_shutdown(struct socket_handler *handler)
{
#ifdef _WIN32
    shutdown(handler->fd, SD_BOTH);
#else  /* No defined _WIN32 */
    shutdown(handler->fd, SHUT_RDWR);
#endif /* _WIN32 */
}

void socket_close(struct socket_handler *handler)
{
#ifdef _WIN32
    closesocket(handler->fd);
#else  /* No defined _WIN32 */
    close(handler->fd);
#endif /* _WIN32 */
}
