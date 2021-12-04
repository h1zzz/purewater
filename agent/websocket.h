/* MIT License Copyright (c) 2021, h1zzz */

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
#define WEBSOCKET_TEXT 0x1
#define WEBSOCKET_BINARY 0x2
#define WEBSOCKET_CLOSE 0x8
#define WEBSOCKET_PING 0x9
#define WEBSOCKET_PONG 0xa

typedef struct websocket {
    net_handle_t net;
    uint64_t remaining;
} websocket_t;

int websocket_connect(websocket_t *ws, const char *host, uint16_t port,
                      const char *path, int tls, const struct proxy *proxy);

/*
 * type: WebSocket message type
 * Read a frame of data, if it is not read, discard the unread data
 */
int websocket_recv(websocket_t *ws, int *type, void *buf, size_t n);

/*
 * Send data to the websocket server, return the number of bytes sent
 * successfully, return -1 on failure
 */
int websocket_send(websocket_t *ws, int type, const void *buf, size_t n);

void websocket_close(websocket_t *ws);

#endif /* websocket.h */
