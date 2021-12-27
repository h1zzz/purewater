/* MIT License Copyright (c) 2021, h1zzz */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "url.h"

int main(void)
{
    struct url_struct *url;

    url = url_parse(
        "https://admin:passwd@h1zzz.net:8080/helloworld?demo=key#hello");

    debugf("%s %s %s %s %hd %s %s %s", url->scheme, url->user, url->passwd,
           url->host, url->port, url->path, url->query, url->fragment);

    url_cleanup(url);

    return 0;
}
