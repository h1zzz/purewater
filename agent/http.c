/* MIT License Copyright (c) 2021, h1zzz */

#include "http.h"

#include <stdlib.h>

#include "debug.h"

struct http_client {
    struct url_struct *proxy;
};

/*
 * CONNECT h1zzz.net:443 HTTP/1.1
 * Host: h1zzz.net:443
 * Proxy-Authorization: Basic YWRtaW46MTIzNDU2
 * User-Agent: client
 * Proxy-Connection: Keep-Alive
 */

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

int http_client_set_proxy(http_client_t *client, const char *proxy)
{
    struct url_struct *ptr;

    assert(client);
    assert(proxy);

    ptr = url_parse(proxy);
    if (!ptr) {
        debug("url_parse fail");
        return -1;
    }

    if (client->proxy)
        url_cleanup(client->proxy);

    client->proxy = ptr;

    return 0;
}

struct http_response *http_client_do(http_client_t *client, const char *method,
                                     const char *url,
                                     const struct linklist *headers,
                                     const char *data, size_t len);

void http_client_free(http_client_t *client);

void http_response_free(struct http_response *resp);
