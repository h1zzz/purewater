/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include <string.h>

#include "debug.h"
#include "net.h"
#include "proxy.h"

int main(int argc, char *argv[])
{
    struct net_handle net;
    int ret;
    struct proxy proxy;

    (void)argc;
    (void)argv;

    memset(&proxy, 0, sizeof(proxy));

    proxy.handshake = proxy_socks5_handshake;
    proxy.host = "127.0.0.1";
    proxy.port = 1080;
    proxy.username = "admin";
    proxy.password = "123456";

    ret = net_connect(&net, "127.0.0.1", 2323, &proxy);
    if (ret == -1) {
        debug("net_connect error");
        return -1;
    }

    /* net_tls_handshake(&net); */

    net_write(&net, "hello\n", 6);
    net_close(&net);

    return 0;
}
