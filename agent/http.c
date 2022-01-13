/* MIT License Copyright (c) 2021, h1zzz */

#include "http.h"

#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "debug.h"
#include "util.h"

#define USER_AGENT "purewater-agent/" PROJECT_VERSION

static const char *http_methods[] = {
    [HTTP_GET] = "GET",
    [HTTP_POST] = "POST",
    [HTTP_CONNECT] = "CONNECT",
};

struct http_request *http_request_new(enum http_method method, const char *url,
                                      const char *data, size_t length)
{
    struct http_request *req;

    assert(url);

    req = calloc(1, sizeof(struct http_response));
    if (!req) {
        debug("calloc error");
        return NULL;
    }

    req->url = url_parse(url);
    if (!req->url) {
        debugf("url_parse %s error", url);
        goto err_url_parse;
    }

    req->data = calloc(1, length);
    if (!req->data) {
        debug("calloc error");
        goto err_copy_data;
    }

    memcpy(req->data, data, length);

    req->method = method;
    req->length = length;

    return req;

err_copy_data:
    url_cleanup(req->url);
err_url_parse:
    free(req);
    return NULL;
}

int http_request_add_header(struct http_request *req, const char *name,
                            const char *value)
{
    struct http_header *header;

    assert(req);
    assert(name);
    assert(value);

    header = calloc(1, sizeof(struct http_header));
    if (!header) {
        debug("calloc error");
        return -1;
    }

    header->name = xstrdup(name);
    if (!header->name) {
        debug("xstrdup error");
        goto err_dup_name;
    }

    header->value = xstrdup(value);
    if (!header->value) {
        debug("xstrdup error");
        goto err_dup_value;
    }

    if (req->headers.header_tail)
        req->headers.header_tail->next = header;
    else
        req->headers.header_head = header;

    req->headers.header_tail = header;

    return 0;

err_dup_value:
    free(header->name);
err_dup_name:
    free(header);
    return -1;
}

void http_request_free(struct http_request *req)
{
    struct http_header *curr, *next;

    assert(req);

    curr = req->headers.header_head;

    while (curr) {
        next = curr->next;
        free(curr->name);
        free(curr->value);
        free(curr);
        curr = next;
    }

    url_cleanup(req->url);
    free(req->data);
    free(req);
}

dynbuf_t *http_request_build(struct http_request *req)
{
    struct http_header *curr;
    dynbuf_t *buf;

    buf = dynbuf_new(1024);
    if (!buf) {
        debug("dynbuf_new error");
        return NULL;
    }

    dynbuf_appendf(buf, "%s ", http_methods[req->method]);
    dynbuf_appendf(buf, "/%s", req->url->path ? req->url->path : "");

    if (req->url->query)
        dynbuf_appendf(buf, "?%s", req->url->query);

    if (req->url->fragment)
        dynbuf_appendf(buf, "#%s", req->url->fragment);

    dynbuf_appendf(buf, " HTTP/1.1\r\n");

    curr = req->headers.header_head;

    while (curr) {
        dynbuf_appendf(buf, "%s: %s\r\n", curr->name, curr->value);
        curr = curr->next;
    }

    dynbuf_appendf(buf, "\r\n");

    return buf;
}

void http_response_free(struct http_response *resp);

http_client_t *http_client_new(void);

int http_client_set_proxy(http_client_t *client, const char *url);

struct http_response *http_client_do(http_client_t *client,
                                     const struct http_request *req);

void http_client_free(http_client_t *client);
