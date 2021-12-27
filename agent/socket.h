/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _SOCKET_H
#define _SOCKET_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
typedef uint64_t socket_t;
#else  /* No define _WIN32 */
typedef int socket_t;
#endif /* _WIN32 */

#define SOCK_TCP 0
#define SOCK_UDP 1

#ifdef _WIN32
#define SOCK_INVAL (socket_t)(~0)
#else /* No define _WIN32 */
#define SOCK_INVAL (-1)
#endif /* _WIN32 */

struct xsockaddr {
    char host[64]; /* address */
    uint16_t port;
};

socket_t xsocket(int type);
int xconnect(socket_t sock, const char *host, uint16_t port);
int xlisten(socket_t sock, const char *host, uint16_t port);
socket_t xaccept(socket_t sock, struct xsockaddr *addr);
int xrecv(socket_t sock, void *buf, size_t size);
int xsend(socket_t sock, const void *data, size_t len);
void xshutdown(socket_t sock);
void xclose(socket_t sock);

int set_blocking(socket_t sock);
int set_nonblocking(socket_t sock);

#endif /* socket.h */
