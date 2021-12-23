/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _HTTP_H
#define _HTTP_H

struct http_response {
    int status_code;
};

struct http_response *http_get();
struct http_response *http_post();
struct http_response *http_connect();

#endif /* http.h */
