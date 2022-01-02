/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _HTTP_HEADER_H
#define _HTTP_HEADER_H

#include "linklist.h"

struct http_header {
    struct linknode node;
    char *name;
    char *value;
};

struct linklist *http_header_new(void);
int http_header_add(struct linklist *headers, const char *name,
                    const char *value);
int http_header_del(struct linklist *headers, const char *name);
int http_header_set(struct linklist *headers, const char *name,
                    const char *value);
const char *http_header_get(struct linklist *headers, const char *name);
void http_header_free(struct linklist *headers);

#endif /* http_header.h */
