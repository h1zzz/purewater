/* MIT License Copyright (c) 2021, h1zzz */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "websocket.h"
#include "config.h"
#include "debug.h"
#include "proxy.h"
#include "util.h"
#include "dns.h"
#include "llist.h"

int main(void)
{
    struct dns_node *dns, *ptr;

    dns_lookup(&dns, "txt.the-issues.org", DNS_TXT);
    for (ptr = dns; ptr; ptr = ptr->next) {
        debugf("%s", ptr->data);
    }
    dns_cleanup(dns);

    return 0;
}

#if 0
int main(int argc, char *argv[])
{
    websocket_t ws;
    int ret;

    (void)argc;
    (void)argv;

    ret = websocket_connect(&ws, "127.0.0.1", 2323, "/ws", 1, NULL);
    if (ret == -1) {
        debugf("connect to %s:%d error", "127.0.0.1", 2323);
        return -1;
    }

    return 0;
}
#endif
