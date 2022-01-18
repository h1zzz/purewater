/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "http.h"

int main(int argc, char *argv[])
{
    struct http_request *req;
    http_client_t *client;
    struct http_response *resp;
    struct http_header *header;

    (void)argc;
    (void)argv;

    client = http_client_new();

    req = http_request_new(HTTP_GET, "https://h1zzz.net", NULL, 0);
    if (!req) {
        return -1;
    }

    http_headers_append(&req->headers, "Connection", "close");

    resp = http_client_do(client, req);
    if (!resp) {
        return -1;
    }

    http_request_free(req);
    http_client_free(client);

    for (header = resp->headers.head; header; header = header->next) {
        debugf("%s: %s", header->name, header->value);
    }

    debugf("%d", resp->status_code);
    debugf("%lu", resp->content_length);
    debugf("%s", resp->data);

    return 0;
}
