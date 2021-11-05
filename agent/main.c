/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include <string.h>

#include "debug.h"
#include "dns.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    debug("hello world");

    char buf[INET6_ADDRSTRLEN];
    struct llist dns;
    int ret;
    struct lnode *node;
    struct dns_node hints, *dns_node;
    struct sockaddr_in *si4;
    struct sockaddr_in6 *si6;

    memset(&hints, 0, sizeof(hints));
    hints.family = AF_INET;
    /* hints.family = AF_INET6; */
    hints.socktype = SOCK_STREAM;
    hints.protocol = IPPROTO_IP;

    ret = dns_resolve(&dns, "h1zzz.net", 0, &hints);
    if (ret == -1) {
        debug("resolve name error.");
        return -1;
    }

    for (node = dns.head; node; node = node->next) {
        dns_node = (struct dns_node *)node;
        switch (dns_node->addr->sa_family) {
        case AF_INET: {
            si4 = (struct sockaddr_in *)dns_node->addr;
            memset(buf, 0, sizeof(buf));
            if (inet_ntop(AF_INET, (char *)&si4->sin_addr, buf,
                          (socklen_t)sizeof(buf)) == NULL) {
                debug("invalid addr.");
                break;
            }
            printf("%s\n", buf);
            break;
        }
        case AF_INET6: {
            si6 = (struct sockaddr_in6 *)dns_node->addr;
            memset(buf, 0, sizeof(buf));
            if (inet_ntop(AF_INET6, (char *)&si6->sin6_addr, buf,
                          (socklen_t)sizeof(buf)) == NULL) {
                debug("invalid addr.");
                break;
            }
            printf("%s\n", buf);
            break;
        }
        default:
            debug("invalid dns_node");
            break;
        }
    }

    dns_destroy(&dns);

    return 0;
}
