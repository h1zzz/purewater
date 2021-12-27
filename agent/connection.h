/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "socket.h"
#include "proxy.h"

typedef struct tcpconn tcpconn_t;

/* Create a connection handle, return a pointer to the handle on success, return
   NULL on failure */
tcpconn_t *tcpconn_new(void);
/* Set the proxy, return 0 if successful, return -1 if failed */
int tcpconn_set_proxy(tcpconn_t *conn, proxy_connect_t *proxy_connect,
                      const char *host, uint16_t port, const char *user,
                      const char *passwd);
/* Connect to hos:port, return 0 on success, return -1 on failure */
int tcpconn_connect(tcpconn_t *conn, const char *host, uint16_t port);
/* Establish an ssl connection, return 0 on success, return -1 on failure */
int tcpconn_ssl_connect(tcpconn_t *conn, const char *host, uint16_t port);
/* Read data, return the number of bytes read successfully, return -1 on
   failure, return 0 when the peer is closed */
int tcpconn_recv(tcpconn_t *conn, void *buf, size_t size);
/* Send data, return the number of bytes on success, return -1 on failure */
int tcpconn_send(tcpconn_t *conn, const void *data, size_t len);
void tcpconn_close(tcpconn_t *conn);
/* Close the connection and release the occupied resources */
void tcpconn_free(tcpconn_t *conn);

#endif /* connection.h */
