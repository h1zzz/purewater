/* MIT License Copyright (c) 2021, h1zzz */

#include "url.h"

#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "util.h"

struct url_struct *url_parse(const char *url)
{
    struct url_struct *ret;
    char *ptr, *str;
    long port;

    ret = calloc(1, sizeof(struct url_struct));
    if (!ret) {
        debug("calloc error");
        return NULL;
    }

    ret->buf = xstrdup(url);
    if (!ret->buf) {
        debug("xstrdup error");
        free(ret);
        return NULL;
    }

    ptr = ret->buf;
    ret->scheme = ptr;

    str = strstr(ptr, "://");
    if (!str) {
        debug("scheme not found");
        goto err;
    }

    *str = '\0';

    ptr = str + 3;

    str = strchr(ptr, '@');
    if (str) {
        *str = '\0';
        ret->user = ptr;
        ptr = str + 1;
        str = (char *)ret->user;
        str = strchr(str, ':');
        if (!str) {
            debug("userinfo invalid");
            goto err;
        }
        *str = '\0';
        ret->passwd = str + 1;
    }

    ret->host = ptr;

    str = strchr(ptr, ':');
    if (str) {
        *str = '\0';
        ptr = str + 1;
        port = strtol(ptr, &str, 10);
        if (port == 0 || port > 65535) {
            debug("port invalid");
            goto err;
        }
        ret->port = (uint16_t)port;
        ptr = str;
    }

    str = strchr(ptr, '/');
    if (str) {
        *str = '\0';
        ret->path = str + 1;
        ptr = str + 1;
    }

    str = strchr(ptr, '?');
    if (str) {
        *str = '\0';
        ret->query = str + 1;
        ptr = str + 1;
    }

    str = strchr(ptr, '#');
    if (str) {
        *str = '\0';
        ret->fragment = str + 1;
    }

    return ret;

err:
    url_cleanup(ret);
    return NULL;
}

void url_cleanup(struct url_struct *url)
{
    free(url->buf);
    free(url);
}
