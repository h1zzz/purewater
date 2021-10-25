/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "buffer.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "platform.h"
#include "debug.h"

void buffer_init(struct buffer *buf)
{
    memset(buf, 0, sizeof(struct buffer));
}

void buffer_destroy(struct buffer *buf)
{
    free(buf->ptr);
}

int buffer_append(struct buffer *buf, const void *data, unsigned int len)
{
    char *ptr;

    if (!buf->ptr) {
        buf->ptr = calloc(1, len + 1);
        if (!buf->ptr)
            return -1;
        buf->size = len + 1;
        buf->ptr[len] = '\0';
    }

    if (buf->size <= buf->len + len) {
        ptr = realloc(buf->ptr, buf->len + len + 1);
        if (!ptr)
            return -1;
        buf->ptr = ptr;
        buf->size = buf->len + len + 1;
        buf->ptr[buf->len + len] = '\0';
    }

    memcpy(buf->ptr + buf->len, data, len);
    buf->len += len;

    return 0;
}

int buffer_appendf(struct buffer *buf, const char *fmt, ...)
{
    va_list ap;
    char *str;
    int ret;

    va_start(ap, fmt);
    ret = xvasprintf(&str, fmt, ap);
    va_end(ap);

    if (ret == -1)
        return -1;

    ret = buffer_append(buf, str, ret);
    free(str);

    return ret;
}
