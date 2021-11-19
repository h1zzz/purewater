/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "debug.h"
#include "socket.h"

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

    while ('\0' != str[n++]) {}

    ptr = calloc(1, n);
    if (!ptr)
        return NULL;

    s = ptr;
    while (n--)
        *s++ = *str++;

    return ptr;
}

int check_is_ipv4(const char *ip)
{
    const char *s = ip;
    size_t i;
    int n;

    for (i = 0; i < sizeof(struct in_addr); i++) {
        if (*s == '\0')
            return 0;

        n = 0;

        while (*s) {
            if ('0' <= *s && *s <= '9') {
                n = n * 10 + (*s - '0');
                s++;
            } else {
                s++;
                break;
            }
        }

        if (n > 255)
            return 0;
    }

    if (*s != '\0' || i != sizeof(struct in_addr))
        return 0;

    return 1;
}
