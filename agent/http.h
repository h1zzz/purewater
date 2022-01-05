/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _HTTP_H
#define _HTTP_H

#include <stddef.h>
#include <stdint.h>

#include "connection.h"
#include "linklist.h"
#include "url.h"

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_CONNECT "CONNECT"

typedef struct http_client http_client_t;

struct http_response {
    tcpconn_t *conn;
    int status_code;
    struct linklist *headers;
    char *data;
    size_t length;
};

http_client_t *http_client_new(void);
int http_client_set_proxy(http_client_t *client, const char *proxy);
struct http_response *http_client_do(http_client_t *client, const char *method,
                                     const char *url,
                                     const struct linklist *headers,
                                     const char *data, size_t len);
void http_client_free(http_client_t *client);

void http_response_free(struct http_response *resp);

#endif /* http.h */
