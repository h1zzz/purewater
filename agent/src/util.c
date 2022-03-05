/* MIT License Copyright (c) 2021, h1zzz */

#include "util.h"

#ifdef _WIN32
#include <windows.h>
#else /* No define _WIN32 */
#include <unistd.h>
#endif /* _WIN32 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "debug.h"

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else /* No define _WIN32 */
#define PATH_SEPARATOR '/'
#endif /* _WIN32 */

void xsleep(int seconds)
{
    assert(seconds > 0);
#ifdef _WIN32
    Sleep(seconds * 1000);
#else  /* No defined _WIN32 */
    sleep(seconds);
#endif /* _WIN32 */
}

const char *xbasename(const char *str)
{
    const char *s = NULL;

    assert(str);

    while (*str) {
        if (*str++ == PATH_SEPARATOR) {
            s = str;
        }
    }
    return s;
}

int xrand(void)
{
    static int sranded = 0;

    if (!sranded) {
        srand((unsigned int)time(NULL));
        sranded = 1;
    }
    return rand();
}

char *xstrdup(const char *str)
{
    char *ptr, *s;
    size_t n = 0;

    assert(str);

    while ('\0' != str[n++]) {}

    ptr = calloc(1, n);
    if (!ptr) {
        debug("calloc error");
        return NULL;
    }

    s = ptr;
    while (n--) {
        *s++ = *str++;
    }

    return ptr;
}