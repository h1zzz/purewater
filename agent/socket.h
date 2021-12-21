/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
typedef int socklen_t;
#else  /* No define _WIN32 */
#endif /* _WIN32 */

typedef struct socket_handle {
#ifdef _WIN32
    SOCKET fd;
#else  /* No define _WIN32 */
    int fd;
#endif /* _WIN32 */
} socket_t;

int socket_open(socket_t *sock, int af, int type, int protocol);
int socket_connect(socket_t *sock, const struct sockaddr *addr,
                   socklen_t addrlen);
int socket_listen(socket_t *sock, const struct sockaddr *addr,
                  socklen_t addrlen);
int socket_accept(socket_t *sock, socket_t *conn, struct sockaddr *addr,
                  socklen_t *addrlen);
int socket_recv(socket_t *sock, void *buf, size_t size);
int socket_send(socket_t *sock, const void *data, size_t n);
int socket_set_blocking(socket_t *sock);
int socket_set_nonblocking(socket_t *sock);
void socket_shutdown(socket_t *sock);
void socket_close(socket_t *sock);

#endif /* socket.h */
