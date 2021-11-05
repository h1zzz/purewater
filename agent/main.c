/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "debug.h"
#include "net.h"

int main(int argc, char *argv[])
{
    struct net_handle net;
    int ret;

    (void)argc;
    (void)argv;

    ret = net_connect(&net, "127.0.0.1", 2323);
    if (ret == -1) {
        debug("net_connect error");
        return -1;
    }

    net_tls_handshake(&net);

    net_write(&net, "hello\n", 6);
    net_close(&net);

    return 0;
}
