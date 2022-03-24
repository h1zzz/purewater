/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "debug.h"
#include "util.h"

struct options {
    const char *program; /* program name */
    const char *socks5;  /* socks5 proxy */
    const char *proxy;   /* http proxy */
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
    srand((unsigned int)time(NULL));
    return 0;
}
