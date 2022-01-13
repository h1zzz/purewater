/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "http.h"
#include "dns.h"

int main(int argc, char *argv[])
{
    struct dns_node *node;

    (void)argc;
    (void)argv;

    node = dns_lookup_ret("h1zzz.net", DNS_A);

    while (node) {
        debugf("%s", node->data);
        node = node->next;
    }

    return 0;
}
