/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _UTIL_H
#define _UTIL_H

#ifdef _WIN32
#include <windows.h>
#else /* No define _WIN32 */
#include <unistd.h>
#endif /* _WIN32 */

#ifdef _WIN32
#define xsleep(x) Sleep((x) * 1000)
#else  /* No define _WIN32 */
#define xsleep(x) sleep(x)
#endif /* _WIN32 */

const char *xbasename(const char *str);
int xrand(void);
char *xstrdup(const char *str);
int check_is_ipv4(const char *ip);

#endif /* util.h */
