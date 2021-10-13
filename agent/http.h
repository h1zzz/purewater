/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _HTTP_H
#define _HTTP_H

#include "connection.h"

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

struct http_request {
    enum http_method method;
};

#endif /* http.h */
