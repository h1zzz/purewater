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

#define assert(expr)
#define debug(str)
#define debugf(fmt, ...)
#define dbgerr(str)

#else /* No define NDEBUG */

#define debug(str)                                                             \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" str "\n", xbasename(__FILE__), \
            __LINE__);

#define debugf(fmt, ...)                                                       \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" fmt "\n", xbasename(__FILE__), \
            __LINE__, __VA_ARGS__);

#define dbgerr(str) perror(str)

#endif /* NDEBUG */

#endif /* debug.h */
