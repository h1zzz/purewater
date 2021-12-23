/* MIT License Copyright (c) 2021, h1zzz */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "connection.h"

int main(void)
{
    tcpconn_t *conn;
    char buf[1024];
    int ret;
    const char *host = "h1zzz.net";
    uint16_t port = 443;

    conn = tcpconn_new();
    if (!conn) {
        debug("tcpconn_new error");
        return -1;
    }

    ret = tcpconn_ssl_connect(conn, host, port);
    if (ret == -1) {
        debug("tcpconn_ssl_connect error");
        goto err;
    }

    ret = snprintf(buf, sizeof(buf),
                   "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                   host);

    ret = tcpconn_send(conn, buf, ret);
    if (ret == -1) {
        debug("tcpconn_send error");
        goto err;
    }

    while (1) {
        ret = tcpconn_recv(conn, buf, sizeof(buf));
        if (ret == -1) {
            debug("tcpconn_recv error");
            goto err;
        }
        if (ret == 0) {
            break;
        }
        fwrite(buf, 1, ret, stdout);
        fflush(stdout);
    }

    tcpconn_free(conn);
    return 0;

err:
    tcpconn_free(conn);
    return -1;
}
