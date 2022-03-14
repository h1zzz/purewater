/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>

#include "debug.h"
#include "util.h"

struct options {
    const char *program;
    const char *socks5;
    const char *user;
    const char *passwd;
    const char *proto;
};

struct options opts = {
    .socks5 = NULL,
    .user = NULL,
    .passwd = NULL,
};

static void readopts(int argc, char *argv[]) {
    opts.program = xbasename(*argv);

    argc--;
    argv++;
}

/* static void usage(void) {} */

int main(int argc, char *argv[]) {
    readopts(argc, argv);
    debugf("%s", opts.program);

    return 0;
}
