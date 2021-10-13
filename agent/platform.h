/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <stdarg.h>

const char *xbasename(const char *file);
char *xstrdup(const char *str);

int xvasprintf(char **str, const char *fmt, va_list ap);
int xasprintf(char **str, const char *fmt, ...);

int xsscanf(const char *str, const char *fmt, ...);

#endif /* platform.h */
