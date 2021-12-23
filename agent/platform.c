/* MIT License Copyright (c) 2021, h1zzz */

#include "platform.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#include <unistd.h>
#endif /* _WIN32 */

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

void xsleep(int seconds)
{
#ifdef _WIN32
    Sleep(seconds * 1000);
#else  /* No defined _WIN32 */
    sleep(seconds);
#endif /* _WIN32 */
}

uint16_t xhtons(uint16_t x)
{
    return htons(x);
}

uint16_t xntohs(uint16_t x)
{
    return ntohs(x);
}
