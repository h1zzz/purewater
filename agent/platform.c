/* MIT License Copyright (c) 2021, h1zzz */

#include "platform.h"

#ifdef _WIN32
#include <windows.h>
#else /* No define _WIN32 */
#include <unistd.h>
#endif /* _WIN32 */

void xsleep(int seconds)
{
#ifdef _WIN32
    Sleep(seconds * 1000);
#else  /* No defined _WIN32 */
    sleep(seconds);
#endif /* _WIN32 */
}