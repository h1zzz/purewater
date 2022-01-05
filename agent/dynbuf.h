/* MIT License Copyright (c) 2022, h1zzz */

#ifndef _DYNBUF_H
#define _DYNBUF_H

#include <stddef.h>

typedef struct dynbuf dynbuf_t;

dynbuf_t *dynbuf_new(size_t size);
int dynbuf_resize(dynbuf_t *buf, size_t size);
int dynbuf_append(dynbuf_t *buf, const void *data, size_t size);
int dynbuf_appendf(dynbuf_t *buf, const char *fmt, ...);
int dynbuf_memset(dynbuf_t *buf);
char *dynbuf_ptr(dynbuf_t *buf);
size_t dynbuf_length(dynbuf_t *buf);
size_t dynbuf_size(dynbuf_t *buf);
void dynbuf_free(dynbuf_t *buf);

#endif /* dynbuf.h */
