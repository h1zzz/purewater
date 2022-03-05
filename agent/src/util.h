/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

void xsleep(int seconds);

const char *xbasename(const char *str);
int xrand(void);
char *xstrdup(const char *str);

#endif /* util.h */
