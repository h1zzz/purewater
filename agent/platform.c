/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "platform.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#    define PATH_SEPARATOR '\\'
#else /* No defined _WIN32 */
#    define PATH_SEPARATOR '/'
#endif /* _WIN32 */

const char *xbasename(const char *file)
{
    const char *ptr = NULL;

    while (*file) {
        if (*file++ == PATH_SEPARATOR)
            ptr = file;
    }
    return ptr;
}

char *xstrdup(const char *str)
{
    char *ptr, *s;
    size_t n = 0;

    while ('\0' != str[n++]) {}

    ptr = calloc(1, n);
    if (!ptr)
        return NULL;

    s = ptr;
    while (n--)
        *s++ = *str++;

    return ptr;
}

int xvasprintf(char **str, const char *fmt, va_list ap)
{
    va_list tmp;
    int size;

    va_copy(tmp, ap);
    size = vsnprintf(NULL, 0, fmt, tmp);
    va_end(tmp);

    *str = calloc(1, (size_t)size + 1);
    if (!(*str))
        return -1;

    return vsnprintf(*str, (size_t)size + 1, fmt, ap);
}

int xasprintf(char **str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = xvasprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}

int xsscanf(const char *str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsscanf(str, fmt, ap);
    va_end(ap);

    return ret;
}
