/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H

#include <stdint.h>

#include "net.h"
#include "proxy.h"

/*
 * %x0 denotes a continuation frame
 * %x1 denotes a text frame
 * %x2 denotes a binary frame
 * %x3-7 are reserved for further non-control frames
 * %x8 denotes a connection close
 * %x9 denotes a ping
 * %xA denotes a pong
 * %xB-F are reserved for further control frames
 */
#define WEBSOCKET_CONTINUATION 0x0
#define WEBSOCKET_TEXT         0x1
#define WEBSOCKET_BINARY       0x2
#define WEBSOCKET_CLOSE        0x8
#define WEBSOCKET_PING         0x9
#define WEBSOCKET_PONG         0xa

struct websocket {
    struct net_handle net;
    unsigned long long remaining;
};

int websocket_connect(struct websocket *ws, const char *host, uint16_t port,
                      const char *path, const struct proxy *proxy);

/*
 * type: WebSocket message type
 * Read a frame of data, if it is not read, discard the unread data
 */
int websocket_recv(struct websocket *ws, int *type, void *buf, size_t n);
void websokcet_close(struct websocket *ws);

#endif /* websocket.h */
