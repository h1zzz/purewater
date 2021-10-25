/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _BUFFER_H
#define _BUFFER_H

struct buffer {
    unsigned int size;
    unsigned int len;
    char *ptr;
};

void buffer_init(struct buffer *buf);
void buffer_destroy(struct buffer *buf);

int buffer_append(struct buffer *buf, const void *data, unsigned int len);
int buffer_appendf(struct buffer *buf, const char *fmt, ...);

#endif /* buffer.h */
