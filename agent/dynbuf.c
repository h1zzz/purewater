/* MIT License Copyright (c) 2022, h1zzz */

#include "dynbuf.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

struct dynbuf {
    char *ptr;
    size_t size;
    size_t length;
};

dynbuf_t *dynbuf_new(size_t size)
{
    dynbuf_t *buf;

    assert(size != 0);

    buf = calloc(1, sizeof(dynbuf_t) + 1); /* +1 '\0' */
    if (!buf) {
        debug("calloc error");
        return NULL;
    }

    buf->ptr = calloc(1, size);
    if (!buf->ptr) {
        debug("calloc error");
        free(buf);
        return NULL;
    }

    buf->size = size;

    return buf;
}

int dynbuf_resize(dynbuf_t *buf, size_t size)
{
    char *ptr;

    assert(buf);
    assert(size != 0);

    ptr = calloc(1, size + 1); /* +1 '\0' */
    if (!ptr) {
        debug("calloc error");
        return -1;
    }

    if (size > buf->size) {
        memcpy(ptr, buf->ptr, buf->size);
        buf->size = size;
        free(buf->ptr);
        buf->ptr = ptr;
    } else {
        memcpy(ptr, buf->ptr, size);
        buf->size = size;
        buf->length = size;
        free(buf->ptr);
        buf->ptr = ptr;
    }

    return 0;
}

static int dynbuf_extend_size(dynbuf_t *buf, size_t size)
{
    if (buf->size < buf->length + size)
        return dynbuf_resize(buf, buf->length + size);
    return 0;
}

int dynbuf_append(dynbuf_t *buf, const void *data, size_t size)
{
    assert(buf);
    assert(data);
    assert(size != 0);

    if (dynbuf_extend_size(buf, size) == -1) {
        debug("extend buffer size error");
        return -1;
    }

    memcpy(buf->ptr + buf->length, data, size);
    buf->length += size;

    return 0;
}

int dynbuf_appendf(dynbuf_t *buf, const char *fmt, ...)
{
    va_list ap, ap2;
    size_t size;

    assert(buf);
    assert(fmt);

    va_start(ap, fmt);
    va_copy(ap2, ap);

    size = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (dynbuf_extend_size(buf, size) == -1) {
        debug("extend buffer size error");
        va_end(ap2);
        return -1;
    }

    buf->length += vsnprintf(buf->ptr + buf->length, /* +1 '\0' */
                             buf->size - buf->length + 1, fmt, ap2);

    va_end(ap2);

    return 0;
}

int dynbuf_memset(dynbuf_t *buf)
{
    assert(buf);
    buf->length = 0;
    memset(buf->ptr, 0, buf->size);
    return 0;
}

char *dynbuf_ptr(dynbuf_t *buf)
{
    assert(buf);
    return buf->ptr;
}

size_t dynbuf_length(dynbuf_t *buf)
{
    assert(buf);
    return buf->length;
}

size_t dynbuf_size(dynbuf_t *buf)
{
    assert(buf);
    return buf->size;
}

void dynbuf_free(dynbuf_t *buf)
{
    assert(buf);
    free(buf->ptr);
    free(buf);
}
