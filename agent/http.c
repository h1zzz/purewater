/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "http.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "platform.h"

static struct http_header_node *http_header_node_new(const char *key,
                                                     const char *val);
static void http_header_node_free(struct http_header_node *node);
static int http_header_node_find(const struct http_header_node *node,
                                 const char *name);

/* clang-format off */
/* static const char *methods[] = {
    [HTTP_OPTIONS]  = "OPTIONS",
    [HTTP_GET]      = "GET",
    [HTTP_HEAD]     = "HEAD",
    [HTTP_POST]     = "POST",
    [HTTP_PUT]      = "PUT",
    [HTTP_DELETE]   = "DELETE",
    [HTTP_TRACE]    = "TRACE",
    [HTTP_CONNECT]  = "CONNECT",
}; */
/* clang-format on */

void http_header_init(struct llist *headers)
{
    llist_init(headers, (lnode_find_t *)http_header_node_find,
               (lnode_free_t *)http_header_node_free);
}

int http_header_add(struct llist *headers, const char *key, const char *val)
{
    struct http_header_node *node = http_header_node_new(key, val);
    if (!node) {
        debugf("http_header_node_new error: %s, %s", key, val);
        return -1;
    }
    llist_insert_next(headers, headers->tail, (struct lnode *)node);
    return 0;
}

int http_header_set(struct llist *headers, const char *key, const char *val)
{
    struct http_header_node *node;
    char *ptr;

    node = (struct http_header_node *)llist_find(headers, key);
    if (!node)
        return http_header_add(headers, key, val);

    ptr = xstrdup(val);
    if (!ptr) {
        debugf("xstrdup error: %s", val);
        return -1;
    }

    free(node->val);
    node->val = ptr;

    return 0;
}

const char *http_header_get(struct llist *headers, const char *key)
{
    struct http_header_node *node;

    node = (struct http_header_node *)llist_find(headers, key);
    if (!node)
        return NULL;
    return node->val;
}

int http_header_delete(struct llist *headers, const char *key)
{
    struct lnode *node;

    node = llist_find(headers, key);
    if (!node) {
        debugf("llist_find error: %s", key);
        return -1;
    }

    llist_remove(headers, node);
    return 0;
}

static struct http_header_node *http_header_node_new(const char *key,
                                                     const char *val)
{
    struct http_header_node *node;

    node = calloc(1, sizeof(struct http_header_node));
    if (!node) {
        debug("calloc error");
        return NULL;
    }

    node->key = xstrdup(key);
    if (!node->key) {
        debugf("xstrdup error: %s", key);
        free(node);
        return NULL;
    }

    node->val = xstrdup(val);
    if (!node->val) {
        debugf("xstrdup error: %s", key);
        free(node->key);
        free(node);
        return NULL;
    }

    return node;
}

static void http_header_node_free(struct http_header_node *node)
{
    free(node->key);
    free(node->val);
    free(node);
}

static int http_header_node_find(const struct http_header_node *node,
                                 const char *name)
{
    return strcmp(name, node->key) == 0;
}
