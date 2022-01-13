/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _HTTP_H
#define _HTTP_H

#include <stddef.h>
#include <stdint.h>

#include "connect.h"
#include "dynbuf.h"
#include "url.h"

enum http_method {
    HTTP_GET = 0,
    HTTP_POST,
    HTTP_CONNECT,
};

struct http_header {
    struct http_header *next;
    char *name;
    char *value;
};

struct http_request {
    enum http_method method;
    struct url_struct *url;
    struct {
        struct http_header *header_head;
        struct http_header *header_tail;
    } headers;
    char *data;
    size_t length;
};

struct http_response {
    int status_code;
    struct {
        struct http_header *header_head;
        struct http_header *header_tail;
    } headers;
    char *data;
    size_t content_length;
    tcpconn_t *rw_conn;
};

typedef struct http_client http_client_t;

struct http_request *http_request_new(enum http_method method, const char *url,
                                      const char *data, size_t length);
int http_request_add_header(struct http_request *req, const char *name,
                            const char *value);
void http_request_free(struct http_request *req);

void http_response_free(struct http_response *resp);

dynbuf_t *http_request_build(struct http_request *req);

http_client_t *http_client_new(void);
int http_client_set_proxy(http_client_t *client, const char *url);
struct http_response *http_client_do(http_client_t *client,
                                     const struct http_request *req);
void http_client_free(http_client_t *client);

#endif /* http.h */
