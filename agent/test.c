/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "connection.h"
#include "debug.h"
#include "dns.h"
#include "llist.h"
#include "platform.h"
#include "url.h"

struct node {
    struct lnode _node;
    char *text;
};

static struct node *node_new(const char *text)
{
    struct node *node;

    node = calloc(1, sizeof(struct node));
    if (!node)
        return NULL;

    node->text = xstrdup(text);
    if (!node->text) {
        free(node);
        return NULL;
    }

    return node;
}

static void node_free(struct node *node)
{
    free(node->text);
    free(node);
}

void llist_test(void)
{
    struct llist list;
    struct lnode *iter;
    struct node *node;

    llist_init(&list, NULL, (lnode_free_t *)node_free);

    llist_insert_next(&list, list.tail, (struct lnode *)node_new("hello"));
    llist_insert_next(&list, list.tail, (struct lnode *)node_new("world"));
    llist_insert_next(&list, list.tail, (struct lnode *)node_new("h1zzz"));
    llist_insert_next(&list, list.tail, (struct lnode *)node_new("hi"));

    for (iter = list.head; iter; iter = iter->next) {
        node = (struct node *)iter;
        printf("%s\n", node->text);
    }

    llist_destroy(&list);
}

void dns_test(void)
{
    char buf[INET6_ADDRSTRLEN];
    struct llist dns;
    int ret;
    struct lnode *node;
    struct dns_node hints, *dns_node;
    struct sockaddr_in *si4;
    struct sockaddr_in6 *si6;

    memset(&hints, 0, sizeof(hints));
    hints.family = AF_INET;
    /* hints.family = AF_INET6; */
    hints.socktype = SOCK_STREAM;
    hints.protocol = IPPROTO_IP;

    ret = dns_resolve(&dns, "www.baidu.com", 0, &hints);
    if (ret == -1) {
        debug("resolve name error.");
        return;
    }

    for (node = dns.head; node; node = node->next) {
        dns_node = (struct dns_node *)node;
        switch (dns_node->addr->sa_family) {
        case AF_INET: {
            si4 = (struct sockaddr_in *)dns_node->addr;
            memset(buf, 0, sizeof(buf));
            if (inet_ntop(AF_INET, (char *)&si4->sin_addr, buf,
                          (socklen_t)sizeof(buf)) == NULL) {
                debug("invalid addr.");
                break;
            }
            printf("%s\n", buf);
            break;
        }
        case AF_INET6: {
            si6 = (struct sockaddr_in6 *)dns_node->addr;
            memset(buf, 0, sizeof(buf));
            if (inet_ntop(AF_INET6, (char *)&si6->sin6_addr, buf,
                          (socklen_t)sizeof(buf)) == NULL) {
                debug("invalid addr.");
                break;
            }
            printf("%s\n", buf);
            break;
        }
        default:
            debug("invalid dns_node");
            break;
        }
    }

    dns_destroy(&dns);
}

void platform_test(void)
{
    char *str;
    int ret;

    ret = xasprintf(&str, "name: %s", "h1zzz");
    printf("%d\n", ret);
    printf("%s\n", str);
}

void url_value_encode_test(void)
{
    struct llist list;
    char *str;

    if (url_value_init(&list, NULL) == -1)
        return;

    url_value_set(&list, "name", "h1zzz");
    url_value_set(&list, "age", "19");
    url_value_set(&list, "sex", "1");

    if (url_value_encode(&list, &str) != -1) {
        printf("%s\n", str);
        free(str);
    }

    url_value_destroy(&list);
}

void url_value_decode_test(void)
{
    /* clang-format off */
    static const char *s = "sl=en&tl=zh-CN&text=hello%20world%20%E4%BD%A0%E5%A5%BD%5%95%8A&op=translate&name=h1zzz";
    /* clang-format on */
    struct llist list;
    char *str;

    if (url_value_init(&list, s) == -1)
        return;

    url_value_del(&list, "sl");

    if (url_value_encode(&list, &str) != -1) {
        printf("%s\n", str);
        free(str);
    }
    url_value_destroy(&list);
}

void url_decode_test(void)
{
    struct url url;

    url_init(&url);
    /* url_parse(&url, "https://admin:123456@[::]:443/a.jsp?k=v&a=z#h1"); */
    url_parse(&url, "https://admin:123456@h1zzz.net:443/a.jsp?k=v&a=z#h1");
    printf("%s://%s:%s@%s:%d/%s?%s#%s\n", url.scheme, url.user, url.passwd,
           url.host, url.port_num, url.path, url.query, url.fragment);
    url_destroy(&url);
}

void connection_test(void)
{
    struct connection *conn;
    char buf[1024];
    int ret, n;
    char *request;
    const char *host = "www.h1zzz.net";
    uint16_t port = 443;

    conn = connection_open(host, port, NULL);
    if (!conn) {
        debugf("connection_open %s:%d error", host, port);
        return;
    }

    ret = connection_tls_handshake(conn);
    if (ret == -1) {
        debug("tls handshake error");
        connection_close(conn);
        return;
    }

    n = xasprintf(&request, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);

    ret = connection_write(conn, request, n);
    printf("ret: %d\n", ret);

    ret = connection_read(conn, buf, sizeof(buf));
    printf("ret: %d\n", ret);

    printf("buf: %s\n", buf);
    connection_close(conn);
}

int main(void)
{
    llist_test();
    dns_test();
    platform_test();
    url_value_decode_test();
    url_value_encode_test();
    url_decode_test();
    connection_test();
    return 0;
}
