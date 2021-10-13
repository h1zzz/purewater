/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _URL_H
#define _URL_H

#include "llist.h"

#define url_value_destroy(list) llist_destroy(list)

struct url {
    const char *scheme;
    const char *user;
    const char *passwd;
    const char *host;
    const char *port;
    const char *path;
    const char *query;
    const char *fragment;
    unsigned short port_num;
    char *temp;
};

struct url_value_node {
    struct lnode _node;
    char *name;
    char *value;
};

void url_init(struct url *url);
int url_parse(struct url *url, const char *str);
void url_destroy(struct url *url);

int url_value_init(struct llist *list, const char *s);
int url_value_set(struct llist *list, const char *name, const char *value);
const char *url_value_get(struct llist *list, const char *name);
int url_value_del(struct llist *list, const char *name);
int url_value_encode(struct llist *list, char **ret);

#endif /* url.h */
