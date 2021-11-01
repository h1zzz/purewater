/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _HTTP_H
#define _HTTP_H

#include "connection.h"
#include "llist.h"

/* https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.1 */
enum http_method {
    HTTP_OPTIONS,
    HTTP_GET,
    HTTP_HEAD,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_TRACE,
    HTTP_CONNECT,
};

struct http_header_node {
    struct lnode _node;
    char *key;
    char *val;
};

struct http_request {
    enum http_method method;
    const char *url;
    const void *body;
    size_t n;
    struct llist headers;
};

#define http_header_destroy(l) llist_destroy(l)

void http_header_init(struct llist *headers);
int http_header_add(struct llist *headers, const char *key, const char *val);
int http_header_set(struct llist *headers, const char *key, const char *val);
const char *http_header_get(struct llist *headers, const char *key);
int http_header_delete(struct llist *headers, const char *key);

#endif /* http.h */
