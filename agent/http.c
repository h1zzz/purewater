/* MIT License Copyright (c) 2021, h1zzz */

#include "http.h"

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "debug.h"
#include "util.h"

#define USER_AGENT "curl/7.77.0" /* masquerading as curl useragent */

struct http_client {
    struct url_struct *proxy;
};

static const char *http_methods[] = {
    [HTTP_GET] = "GET",
    [HTTP_POST] = "POST",
    [HTTP_CONNECT] = "CONNECT",
};

int http_headers_append(struct http_headers *headers, const char *name,
                        const char *value)
{
    struct http_header *header;

    assert(headers);
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

    if (headers->tail) {
        headers->tail->next = header;
    } else {
        headers->head = header;
    }

    headers->tail = header;

    return 0;

err_dup_value:
    free(header->name);
err_dup_name:
    free(header);
    return -1;
}

const char *http_headers_get(const struct http_headers *headers,
                             const char *name)
{
    const struct http_header *curr;

    assert(headers);
    assert(name);

    for (curr = headers->head; curr; curr = curr->next) {
        if (strcmp(curr->name, name) == 0) {
            return curr->value;
        }
    }

    return NULL;
}

void http_headers_cleanup(struct http_headers *headers)
{
    struct http_header *curr, *next;

    assert(headers);

    curr = headers->head;
    while (curr) {
        next = curr->next;
        free(curr->name);
        free(curr->value);
        free(curr);
        curr = next;
    }

    headers->head = NULL;
    headers->tail = NULL;
}

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

void http_request_free(struct http_request *req)
{
    assert(req);

    http_headers_cleanup(&req->headers);
    url_cleanup(req->url);
    free(req->data);
    free(req);
}

dynbuf_t *http_request_build(const struct http_request *req)
{
    const struct http_header *curr;
    dynbuf_t *buf;

    buf = dynbuf_new(1024);
    if (!buf) {
        debug("dynbuf_new error");
        return NULL;
    }

    dynbuf_appendf(buf, "%s ", http_methods[req->method]);
    dynbuf_appendf(buf, "/%s", req->url->path ? req->url->path : "");

    /* Setup query */
    if (req->url->query) {
        dynbuf_appendf(buf, "?%s", req->url->query);
    }

    /* Setup fragment */
    if (req->url->fragment) {
        dynbuf_appendf(buf, "#%s", req->url->fragment);
    }

    /* Setup HTTP Version */
    dynbuf_appendf(buf, " HTTP/1.1\r\n");

    dynbuf_appendf(buf, "Host: %s\r\n", req->url->host);
    dynbuf_appendf(buf, "User-Agent: %s\r\n", USER_AGENT);
    dynbuf_appendf(buf, "Accept: */*\r\n");

    if (req->data && req->length) {
        dynbuf_appendf(buf, "Content-Length: %zu\r\n", req->length);
    }

    /* Setup headers */
    for (curr = req->headers.head; curr; curr = curr->next) {
        dynbuf_appendf(buf, "%s: %s\r\n", curr->name, curr->value);
    }

    dynbuf_appendf(buf, "\r\n");

    return buf;
}

void http_response_free(struct http_response *resp)
{
    assert(resp);
    free(resp->data);
    http_headers_cleanup(&resp->headers);
    free(resp);
}

http_client_t *http_client_new(void)
{
    http_client_t *client;

    client = calloc(1, sizeof(http_client_t));
    if (!client) {
        debug("calloc error");
        return NULL;
    }

    return client;
}

int http_client_set_proxy(http_client_t *client, const char *url)
{
    assert(client);
    assert(url);

    if (client->proxy) {
        url_cleanup(client->proxy);
        free(client->proxy);
    }

    client->proxy = url_parse(url);
    if (!client->proxy) {
        debug("url_parse error");
        return -1;
    }

    return 0;
}

static int http_response_readline(tcpconn_t *conn, void *buf, size_t size)
{
    char *ptr = buf, ch;
    int n = 0, ret;

    while ((size_t)n < size) {
        ret = tcpconn_recv(conn, &ch, 1);
        if (ret == -1) {
            debug("tcpconn_recv error");
            return -1;
        }
        if (ret == 0) { /* read of end */
            break;
        }
        if (ch == '\n') {
            ptr[--n] = '\0';
            break;
        }
        ptr[n++] = ch;
    }

    return n;
}

static int http_response_readn(tcpconn_t *conn, void *buf, size_t size)
{
    size_t nleft = size;
    int nread;
    char *ptr = buf;

    while (nleft > 0) {
        nread = tcpconn_recv(conn, ptr, nleft);
        if (nread == -1) {
            debug("tcpconn_recv error");
            return -1;
        }
        if (nread == 0) {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }

    return (int)(size - nleft);
}

static int http_header_parse(struct http_response *resp, const char *str)
{
    char *buf, *value;
    int ret;

    buf = xstrdup(str);
    if (!buf) {
        debug("xstrdup error");
        return -1;
    }

    value = strchr(buf, ':');
    if (!value) {
        debugf("invalid header format: %s", str);
        goto err;
    }

    *value = '\0';     /* ':' to '\0' */
    value = value + 2; /* skip "\0 " */

    ret = http_headers_append(&resp->headers, buf, value);
    if (ret == -1) {
        debug("http_header_append error");
        goto err;
    }

    free(buf);

    return 0;
err:
    free(buf);
    return -1;
}

static struct http_response *parse_http_response(tcpconn_t *conn)
{
    struct http_response *resp;
    char line[2048], *ptr;
    int ret;
    const char *value;

    resp = calloc(1, sizeof(struct http_response));
    if (!resp) {
        debug("calloc error");
        goto err_new_resp;
    }

    /* Parse http status */
    ret = http_response_readline(conn, line, sizeof(line));
    if (ret == -1) {
        debug("http_readline error");
        goto err_parse_status;
    }

    ptr = strchr(line, ' ');
    if (!ptr) {
        debugf("invalid http format: %s", line);
        goto err_parse_status;
    }

    resp->status_code = (int)strtol(ptr, NULL, 10);

    /* debugf("status_code: %d\n", resp->status_code); */

    /* Parse http response header */
    while (1) {
        ret = http_response_readline(conn, line, sizeof(line));
        if (ret == -1) {
            debug("http_readline error");
            goto err_parse_header;
        }
        if (ret == 0) { /* header read of end */
            break;
        }
        if (http_header_parse(resp, line) == -1) {
            debug("http_header_parse error");
        }
    }

    /* Parse data */
    /* Transfer-Encoding: chunked */
    value = http_headers_get(&resp->headers, "Transfer-Encoding");
    if (value) {
        debug("nosupport Transfer-Encoding");
        goto err_read_data;
    }

    value = http_headers_get(&resp->headers, "Content-Length");
    if (!value) {
        debug("no content length");
        goto err_read_data;
    }

    resp->content_length = (size_t)strtol(value, NULL, 10);
    if (resp->content_length >= 1024 * 512) {
        debug("content length exceeds limit");
        goto err_read_data;
    }

    resp->data = calloc(1, resp->content_length + 1);
    if (!resp->data) {
        debug("calloc error");
        goto err_read_data;
    }

    ret = http_response_readn(conn, resp->data, resp->content_length);
    if (ret == -1) {
        debug("http_response_readn error");
        free(resp->data);
        goto err_read_data;
    }

    return resp;

err_read_data:
err_parse_header:
    http_headers_cleanup(&resp->headers);
err_parse_status:
    free(resp);
err_new_resp:
    tcpconn_free(conn);
    return NULL;
}

struct http_response *http_client_do(http_client_t *client,
                                     const struct http_request *req,
                                     tcpconn_t *ret_conn)
{
    struct http_response *resp;
    tcpconn_t *conn;
    dynbuf_t *buf;
    int ret;

    assert(client);
    assert(req);

    conn = tcpconn_new();
    if (!conn) {
        debug("tcpconn_new error");
        return NULL;
    }

    if (client->proxy) {}

    if (strcmp(req->url->scheme, "https") == 0) {
        ret = tcpconn_ssl_connect(conn, req->url->host, req->url->port);
        if (ret == -1) {
            debugf("tcpconn_ssl_connect %s:%d error", req->url->host,
                   req->url->port);
            goto err_connect;
        }
    } else if (strcmp(req->url->scheme, "http") == 0) {
        ret = tcpconn_connect(conn, req->url->host, req->url->port);
        if (ret == -1) {
            debugf("tcpconn_connect %s:%d error", req->url->host,
                   req->url->port);
            goto err_connect;
        }
    } else {
        debugf("invalid scheme, %s", req->url->scheme);
        goto err_connect;
    }

    buf = http_request_build(req);
    if (!buf) {
        debug("http_request_build error");
        goto err_build_req;
    }

    /* debugf("request: %s", dynbuf_ptr(buf)); */

    ret = tcpconn_send(conn, dynbuf_ptr(buf), dynbuf_length(buf));
    if (ret == -1) {
        debug("tcpconn_send error");
        goto err_send_req;
    }

    if (req->data && req->length) {
        ret = tcpconn_send(conn, req->data, req->length);
        if (ret == -1) {
            debug("tcpconn_send error");
            goto err_send_req;
        }
    }

    dynbuf_free(buf);

    resp = parse_http_response(conn);

    if (ret_conn) {
        ret_conn = conn;
    } else {
        tcpconn_free(conn);
    }

    return resp;

err_send_req:
    dynbuf_free(buf);
err_build_req:
err_connect:
    tcpconn_free(conn);
    return NULL;
}

void http_client_free(http_client_t *client)
{
    assert(client);
    if (client->proxy) {
        url_cleanup(client->proxy);
    }
    free(client);
}
