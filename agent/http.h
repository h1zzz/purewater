/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _HTTP_H
#define _HTTP_H

#include <stddef.h>
#include <stdint.h>

#include "connection.h"

struct http_response {
    int status_code;
    tcpconn_t *conn;
};

struct http_response *http_connect(const char *url, const char *host,
                                   uint16_t port);

#endif /* http.h */
