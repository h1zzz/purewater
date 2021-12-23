/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef NDEBUG
#else /* No define NDEBUG */
#include <stdio.h>

#include "config.h"
#include "util.h"
#endif /* NDEBUG */

#ifdef NDEBUG
#define debug(str)
#define debugf(fmt, ...)
#else /* No define NDEBUG */
#define debug(str)                                                             \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" str "\n", xbasename(__FILE__), \
            __LINE__);
#define debugf(fmt, ...)                                                       \
    fprintf(stdout, "\033[1;90m[%s:%d] \033[0m" fmt "\n", xbasename(__FILE__), \
            __LINE__, __VA_ARGS__);
#endif /* NDEBUG */

#endif /* debug.h */
