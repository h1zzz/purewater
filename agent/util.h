/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdlib.h>

#define xfree(ptr)     \
    do {               \
        if (ptr)       \
            free(ptr); \
        (ptr) = NULL;  \
    } while (0)

void xsleep(int seconds);

const char *xbasename(const char *str);
int xrand(void);
char *xstrdup(const char *str);
int is_ipv4(const char *ip);

#endif /* util.h */
