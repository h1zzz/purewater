/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include <string.h>

#include "debug.h""
#include "websocket.h"

int main(int argc, char *argv[])
{
    struct websocket ws;
    int ret;
    char buf[256] = {0};

    (void)argc;
    (void)argv;

    ret = websocket_connect(&ws, "127.0.0.1", 8080, "/ws", 1, NULL);
    if (ret == -1) {
        debug("websocket_connect error");
        return -1;
    }

    ret = websocket_recv(&ws, NULL, buf, 2);
    if (ret == -1) {
        debug("websocket_recv error");
        goto err;
    }

    debugf("%s", buf);

    ret = websocket_recv(&ws, NULL, buf, sizeof(buf) - 1);
    if (ret == -1) {
        debug("websocket_recv error");
        goto err;
    }

    debugf("%s", buf);

err:
    websokcet_close(&ws);
    return ret;
}
