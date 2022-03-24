/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _DEBUG_H
#define _DEBUG_H

#ifndef NDEBUG
#include <assert.h>
#include <stdio.h>

#include "config.h"
#include "util.h"
#endif /* NDEBUG */

#ifdef NDEBUG

#define ASSERT(expr)
#define DBG(str)
#define DBGF(fmt, ...)
#define DBGERR(str)

#else /* No define NDEBUG */

#define ASSERT(expr) assert(expr)

#define DBG(str)                                                               \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" str "\n", xbasename(__FILE__), \
            __LINE__);

#define DBGF(fmt, ...)                                                         \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" fmt "\n", xbasename(__FILE__), \
            __LINE__, __VA_ARGS__);

#define DBGERR(str)                                                       \
    do {                                                                  \
        fprintf(stderr, "\033[1;90m[%s:%d] \033[0m", xbasename(__FILE__), \
                __LINE__);                                                \
        perror(str);                                                      \
    } while (0)

#endif /* NDEBUG */

#endif /* debug.h */
