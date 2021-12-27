/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _URL_H
#define _URL_H

#include <stdint.h>

struct url_struct {
    const char *scheme;
    const char *user;
    const char *passwd;
    const char *host;
    uint16_t port;
    const char *path;
    const char *query;
    const char *fragment;
    char *buf;
};

struct url_struct *url_parse(const char *url);
void url_cleanup(struct url_struct *url);

#endif /* url.h */
