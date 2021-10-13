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
#endif /* _WIN32 */

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
typedef int socklen_t;
#else  /* No define _WIN32 */
#endif /* _WIN32 */

struct socket_handler {
#ifdef _WIN32
    SOCKET fd;
#else  /* No define _WIN32 */
    int fd;
#endif /* _WIN32 */
};

int socket_open(struct socket_handler *handler, int af, int type, int protocol);
int socket_connect(struct socket_handler *handler, const struct sockaddr *addr,
                   socklen_t addrlen);
int socket_listen(struct socket_handler *handler, const struct sockaddr *addr,
                  socklen_t addrlen);
int socket_accept(struct socket_handler *handler, struct socket_handler *conn,
                  struct sockaddr *addr, socklen_t *addrlen);
int socket_read(struct socket_handler *handler, void *buf, int size);
int socket_write(struct socket_handler *handler, const void *data, int n);
int socket_set_blocking(struct socket_handler *handler);
int socket_set_nonblocking(struct socket_handler *handler);
void socket_shutdown(struct socket_handler *handler);
void socket_close(struct socket_handler *handler);

#endif /* socket.h */
