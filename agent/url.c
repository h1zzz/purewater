/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "url.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#include "platform.h"

static int url_value_parse(struct llist *list, const char *s);
static struct url_value_node *url_value_node_new(const char *name,
                                                 const char *value);
static void url_value_node_free(struct url_value_node *node);
static int url_value_node_find(const struct url_value_node *value,
                               const char *name);
static unsigned short getservport(const char *servname);

void url_init(struct url *url)
{
    memset(url, 0, sizeof(struct url));
}

int url_parse(struct url *url, const char *str)
{
    char *ptr, *s;

    if (url->temp) {
        url_destroy(url);
        url_init(url);
    }

    url->temp = xstrdup(str);
    if (!url->temp)
        return -1;

    ptr = url->temp;
    s = ptr;

    ptr = strchr(s, '#');
    if (ptr) {
        *ptr++ = '\0';
        url->fragment = ptr;
    }

    ptr = strstr(s, "://");
    if (ptr) {
        *ptr = '\0';
        url->scheme = s;
        s = ptr + 3;
    }

    ptr = strchr(s, '?');
    if (ptr) {
        *ptr++ = '\0';
        url->query = ptr;
    }

    ptr = strchr(s, '/');
    if (ptr) {
        *ptr++ = '\0';
        url->path = ptr;
    }

    ptr = strchr(s, '@');
    if (ptr) {
        *ptr++ = '\0';
        url->user = s;
        s = strchr(s, ':');
        if (s) {
            *s++ = '\0';
            url->passwd = s;
        }
        s = ptr;
    }

    if (*s == '[') { /* IPv6 address. */
        ptr = strstr(s, "]:");
        if (ptr) {
            *++ptr = '\0';
            ptr++;
            url->port = ptr;
        }
    } else {
        ptr = strchr(s, ':');
        if (ptr) {
            *ptr++ = '\0';
            url->port = ptr;
        }
    }

    url->host = s;
    url->port_num = getservport(url->port ? url->port : url->scheme);

    return 0;
}

void url_destroy(struct url *url)
{
    free(url->temp);
}

int url_value_init(struct llist *list, const char *s)
{
    llist_init(list, (lnode_find_t *)url_value_node_find,
               (lnode_free_t *)url_value_node_free);
    if (!s)
        return 0;

    if (url_value_parse(list, s) == -1) {
        llist_destroy(list);
        return -1;
    }

    return 0;
}

int url_value_set(struct llist *list, const char *name, const char *value)
{
    struct url_value_node *node;
    struct lnode *ptr;
    char *str;

    ptr = llist_find(list, name);
    if (!ptr) {
        node = url_value_node_new(name, value);
        if (!node)
            return -1;
        llist_insert_next(list, list->tail, (struct lnode *)node);
        return 0;
    }

    str = xstrdup(value);
    if (!str)
        return -1;

    node = (struct url_value_node *)ptr;
    free(node->value);
    node->value = str;

    return 0;
}

const char *url_value_get(struct llist *list, const char *name)
{
    struct url_value_node *node;
    struct lnode *ptr;

    ptr = llist_find(list, name);
    if (!ptr)
        return NULL;
    node = (struct url_value_node *)ptr;
    return node->value;
}

int url_value_del(struct llist *list, const char *name)
{
    struct lnode *ptr;

    ptr = llist_find(list, name);
    if (!ptr)
        return -1;
    llist_remove(list, ptr);
    return 0;
}

int url_value_encode(struct llist *list, char **ret)
{
    size_t size = 512, len = 0, n;
    char *str, *tmp, *s;
    struct lnode *ptr;
    struct url_value_node *node;

    str = malloc(size);
    if (!str)
        return -1;

    for (ptr = list->head; ptr; ptr = ptr->next) {

        node = (struct url_value_node *)ptr;
        n = (strlen(node->name) + strlen(node->value) * 3);
        n += 2; /* '=', '&' */

        while (size <= (len + n)) {
            tmp = realloc(str, size * 2);
            if (tmp) {
                size *= 2;
                str = tmp;
                break;
            }
            free(str);
            return -1;
        }

        s = node->name;
        while (*s) {
            if (len >= size) {
                free(str);
                return -1;
            }
            str[len++] = *s++;
        }

        if (len >= size) {
            free(str);
            return -1;
        }
        str[len++] = '=';

        s = node->value;
        while (*s) {
            if (isalnum(*(uint8_t *)s) || '-' == *s || '.' == *s || '_' == *s ||
                '~' == *s) {
                if (len >= size) {
                    free(str);
                    return -1;
                }
                str[len++] = *s++;
            } else {
                if (len + 3 >= size) {
                    free(str);
                    return -1;
                }
                len += snprintf(str + len, size - len, "%%%02X", *s);
                s++;
            }
        }

        if (len >= size) {
            free(str);
            return -1;
        }
        str[len++] = '&';
    }

    if (len != 0)
        str[len - 1] = '\0';

    *ret = str;

    return (int)len;
}

static int url_value_parse(struct llist *list, const char *str)
{
    char *buf, *ptr, *s;
    const char *k, *v;
    int flag = 0, ret = 0;
    unsigned int num;

    buf = xstrdup(str);
    if (!buf)
        return -1;

    ptr = buf;
    s = buf;
    k = s;
    v = "";

    while (*ptr) {
        switch (*ptr) {
        case '=':
            *s = '\0';
            v = s + 1;
            break;
        case '&':
            *s = '\0';
            flag = 1;
            break;
        case '+':
            *s = ' ';
            break;
        case '%': {
            if (xsscanf(ptr + 1, "%02X", &num) != 1) {
                ret = -1;
                goto end;
            }
            *s = (uint8_t)num;
            ptr += 2;
            break;
        }
        default:
            *s = *ptr;
            break;
        }
        s++;
        ptr++;
        if (*ptr == '\0' || flag) {
            if (*ptr == '\0')
                *s = '\0';
            if (url_value_set(list, k, v) == -1) {
                ret = -1;
                goto end;
            }
            flag = 0;
            k = s;
        }
    }

end:
    free(buf);
    return ret;
}

static struct url_value_node *url_value_node_new(const char *name,
                                                 const char *value)
{
    struct url_value_node *node;

    node = malloc(sizeof(struct url_value_node));
    if (!node)
        return NULL;

    node->name = xstrdup(name);
    if (!node->name) {
        free(node);
        return NULL;
    }

    node->value = xstrdup(value);
    if (!node->value) {
        free(node->name);
        free(node);
        return NULL;
    }

    return node;
}

static void url_value_node_free(struct url_value_node *node)
{
    free(node->name);
    free(node->value);
    free(node);
}

static int url_value_node_find(const struct url_value_node *node,
                               const char *name)
{
    return strcmp(node->name, name) == 0;
}

static unsigned short getservport(const char *servname)
{
    long port;

    port = strtol(servname, NULL, 10);
    if (port)
        return (unsigned short)port;

    if (strcmp(servname, "https") == 0 || strcmp(servname, "wss") == 0)
        return 443;
    if (strcmp(servname, "http") == 0 || strcmp(servname, "ws") == 0)
        return 80;

    return 0;
}
