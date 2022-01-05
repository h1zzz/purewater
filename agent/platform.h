/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <stdint.h>
#include <stdlib.h>

#define xfree(ptr)     \
    do {               \
        if (ptr)       \
            free(ptr); \
        (ptr) = NULL;  \
    } while (0)

void xsleep(int seconds);

#endif /* platform.h */
