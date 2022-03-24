/* MIT License Copyright (c) 2022, h1zzz */

#ifndef _NET_H
#define _NET_H

#include <stdint.h>
#include <stddef.h>

#define NET_TCP 0
#define NET_UDP 1

typedef struct {
#ifdef _WIN32
    uint64_t fd;
#else  /* No define _WIN32 */
    int fd;
#endif /* _WIN32 */
} net_context;

void net_init(net_context *ctx);
int net_connect(net_context *ctx, const char *host, uint16_t port, int proto);
int net_recv(net_context *ctx, void *buf, size_t size);
int net_send(net_context *ctx, const void *data, size_t len);
void net_close(net_context *ctx);
void net_free(net_context *ctx);

#endif /* net.h */
