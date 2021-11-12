/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32
#    include <winsock2.h>
#    include <ws2tcpip.h>
#else /* No define _WIN32 */
#    include <arpa/inet.h>
#    ifdef __linux__
#        include <endian.h>
#    endif /* __linux */
#endif     /* _WIN32 */

#include <stdint.h>
#include <stddef.h>

#ifdef __linux__
#    if __BYTE_ORDER == __BIG_ENDIAN
#        define ntohll(x) (x)
#        define htonll(x) (x)
#    elif __BYTE_ORDER == __LITTLE_ENDIAN
#        define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#        define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#    endif
#endif /* __linux__ */

#ifdef _WIN32
typedef int socklen_t;
#else  /* No define _WIN32 */
#endif /* _WIN32 */

struct socket_handle {
#ifdef _WIN32
    SOCKET fd;
#else  /* No define _WIN32 */
    int fd;
#endif /* _WIN32 */
};

int socket_open(struct socket_handle *sock, int af, int type, int protocol);
int socket_connect(struct socket_handle *sock, const struct sockaddr *addr,
                   socklen_t addrlen);
int socket_listen(struct socket_handle *sock, const struct sockaddr *addr,
                  socklen_t addrlen);
int socket_accept(struct socket_handle *sock, struct socket_handle *conn,
                  struct sockaddr *addr, socklen_t *addrlen);
int socket_recv(struct socket_handle *sock, void *buf, size_t size);
int socket_send(struct socket_handle *sock, const void *data, size_t n);
int socket_set_blocking(struct socket_handle *sock);
int socket_set_nonblocking(struct socket_handle *sock);
void socket_shutdown(struct socket_handle *sock);
void socket_close(struct socket_handle *sock);

#endif /* socket.h */
