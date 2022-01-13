/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "http.h"

int main(int argc, char *argv[])
{
    struct http_request *req;
    dynbuf_t *buf;

    (void)argc;
    (void)argv;

    req = http_request_new(HTTP_GET, "https://h1zzz.net/index?a=b#cc", NULL, 0);

    http_request_add_header(req, "Hello", "World");
    http_request_add_header(req, "Cookie", "cookie");

    buf = http_request_build(req);
    http_request_free(req);

    debugf("%s", dynbuf_ptr(buf));
    dynbuf_free(buf);

    return 0;
}
