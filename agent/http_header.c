/* MIT License Copyright (c) 2021, h1zzz */

#include "http_header.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "util.h"

static struct http_header *http_header_node_new(const char *name,
                                                const char *value)
{
    struct http_header *header;

    header = calloc(1, sizeof(struct http_header));
    if (!header) {
        debug("calloc error");
        return NULL;
    }

    header->name = xstrdup(name);
    if (!header->name) {
        debug("xstrdup error");
        goto err_dup_name;
    }

    header->value = xstrdup(value);
    if (!header->value) {
        debug("xstrdup error");
        goto err_dup_value;
    }

    return header;

err_dup_value:
    free(header->name);
err_dup_name:
    free(header);

    return NULL;
}

static int http_header_node_find(const struct http_header *header,
                                 const char *name)
{
    return strcmp(header->name, name) == 0;
}

static void http_header_node_free(struct http_header *header)
{
    free(header->value);
    free(header->name);
    free(header);
}

struct linklist *http_header_new(void)
{
    struct linklist *headers;

    headers = calloc(1, sizeof(struct linklist));
    if (!headers) {
        debug("calloc error");
        return NULL;
    }

    linklist_init(headers, (linknode_find_t *)http_header_node_find,
                  (linknode_free_t *)http_header_node_free);

    return headers;
}

int http_header_add(struct linklist *headers, const char *name,
                    const char *value)
{
    struct http_header *header;
    struct linknode *ptr;

    assert(headers);
    assert(name);
    assert(value);

    ptr = linklist_find(headers, name);
    if (ptr) {
        debug("add repeatedly");
        return -1;
    }

    header = http_header_node_new(name, value);
    if (!header) {
        debug("new http_header node fail");
        return -1;
    }

    linklist_insert_next(headers, headers->tail, (struct linknode *)header);

    return 0;
}

int http_header_del(struct linklist *headers, const char *name)
{
    struct linknode *ptr;

    assert(headers);
    assert(name);

    ptr = linklist_find(headers, name);
    if (!ptr) {
        debugf("%s not found", name);
        return -1;
    }

    linklist_remove(headers, ptr);

    return 0;
}

int http_header_set(struct linklist *headers, const char *name,
                    const char *value)
{
    struct http_header *header;
    struct linknode *ptr;
    char *str;

    assert(headers);
    assert(name);
    assert(value);

    ptr = linklist_find(headers, name);
    if (!ptr)
        return http_header_add(headers, name, value);

    str = xstrdup(value);
    if (!str) {
        debug("xstrdup error");
        return -1;
    }

    header = (struct http_header *)ptr;
    free(header->value);
    header->value = str;

    return 0;
}

const char *http_header_get(struct linklist *headers, const char *name)
{
    struct http_header *header;
    struct linknode *ptr;

    assert(headers);
    assert(name);

    ptr = linklist_find(headers, name);
    if (ptr) {
        debugf("%s not found", name);
        return NULL;
    }

    header = (struct http_header *)ptr;

    return header->value;
}

void http_header_free(struct linklist *headers)
{
    assert(headers);
    linklist_destroy(headers);
}
