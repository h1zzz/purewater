/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "dns.h"

#ifdef _WIN32
#    include <iphlpapi.h>
#endif /* _WIN32 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "debug.h"
#include "socket.h"
#include "util.h"

#ifdef _WIN32
#    pragma comment(lib, "iphlpapi.lib")
#endif /* _WIN32 */

#define DNS_PORT               53
#define nameserv_destroy(list) llist_destroy(list)

/*
 * TYPE values
 *
 * TYPE fields are used in resource records.  Note that these types are a
 * subset of QTYPEs.
 */
#define TYPE_A     1 /* 1 a host address */
#define TYPE_CNAME 5 /* 5 the canonical name for an alias */
/* https://datatracker.ietf.org/doc/html/rfc3596#section-2.1 */
#define TYPE_AAAA 28

#define CLASS_IN 1

/* https://datatracker.ietf.org/doc/html/rfc1035#section-4.1.1 */
struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
};

/* https://datatracker.ietf.org/doc/html/rfc1035#section-4.1.2 */
struct dns_question {
    /* char qname[]; */
    uint16_t qtype;
    uint16_t qclass;
};

/* https://datatracker.ietf.org/doc/html/rfc1035#section-3.2 */
#pragma pack(push, 1)
struct dns_rrs {
    /* char qname[]; */
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rd_length;
    /* char rd_data[]; */
};
#pragma pack(pop)

struct nameserv_node {
    struct lnode _node;
    struct sockaddr *addr;
    socklen_t addrlen;
};

static struct dns_node *dns_node_new(const struct dns_node *hints,
                                     struct sockaddr *addr, socklen_t addrlen);
static void dns_node_free(struct dns_node *node);
static int send_question(struct socket_handler *handler, int family,
                         const char *name);
static void format_dns_name(char *name);
static struct nameserv_node *nameserv_node_new(struct sockaddr *addr,
                                               socklen_t addrlen);
static void nameserv_node_free(struct nameserv_node *node);
static int nameserv_add_local_dns(struct llist *list);
static int nameserv_init(struct llist *list);
static int parse_answer(struct llist *list, int family,
                        const struct dns_node *hints, uint16_t port, char *data,
                        int n);
static int dns_read_name(char *data, char *ptr, char *name, size_t size);

static const char *defnameserv[] = {"8.8.8.8", "9.9.9.9", "1.1.1.1", "1.2.4.8"};

int dns_resolve(struct llist *list, const char *name, uint16_t port,
                const struct dns_node *hints)
{
    struct socket_handler handler;
    int family = AF_INET, ret;
    struct llist nslist;
    struct lnode *node;
    struct nameserv_node *nameserv;
    char buf[10240];
    struct sockaddr_in *si4;
    struct sockaddr_in6 *si6;
    struct dns_node *dns_node;

    llist_init(list, NULL, (lnode_free_t *)dns_node_free);

    if (hints) {
        family = hints->family;
    }

    /* If it is an IP address, it is directly resolved to an address. */
    switch (family) {
    case AF_INET:
        if (!check_is_ipv4(name))
            break;

        si4 = calloc(1, sizeof(struct sockaddr_in));
        if (!si4) {
            debug("calloc error");
            return -1;
        }

        ret = inet_pton(family, name, &si4->sin_addr);
        if (ret <= 0) {
            free(si4);
            break;
        }

        si4->sin_family = AF_INET;
        si4->sin_port = htons(port);
        dns_node = dns_node_new(hints, (struct sockaddr *)si4,
                                (socklen_t)sizeof(struct sockaddr_in));
        if (!dns_node) {
            debug("dns_node_new error");
            free(si4);
            return -1;
        }
        llist_insert_next(list, list->tail, (struct lnode *)dns_node);
        return 0;
    case AF_INET6:
        if (!check_is_ipv6(name))
            break;
        si6 = calloc(1, sizeof(struct sockaddr_in6));
        if (!si6) {
            debug("calloc error");
            return -1;
        }
        ret = inet_pton(family, name, &si6->sin6_addr);
        if (ret <= 0) {
            free(si6);
            break;
        }
        si6->sin6_family = AF_INET6;
        si6->sin6_port = htons(port);
        dns_node = dns_node_new(hints, (struct sockaddr *)si6,
                                (socklen_t)sizeof(struct sockaddr_in6));
        if (!dns_node) {
            debug("dns_node_new error");
            free(si6);
            return -1;
        }
        llist_insert_next(list, list->tail, (struct lnode *)dns_node);
        return 0;
    default:
        return -1;
    }

    /* If there are records in the hosts file, get the address from the
       hosts file, unsupported. */

    if (nameserv_init(&nslist) == -1) {
        debug("nameserv_init error");
        return -1;
    }

    for (node = nslist.head; node; node = node->next) {
        nameserv = (struct nameserv_node *)node;
        ret = socket_open(&handler, PF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (ret == -1) {
            debug("open socket error");
            perror("socket_open");
            continue;
        }
        /* TODO: supported IPv6 name server. */
        ret = socket_connect(&handler, nameserv->addr, nameserv->addrlen);
        if (ret == -1) {
            debug("socket_connect error");
            goto cleanup;
        }
        ret = send_question(&handler, family, name);
        if (ret == -1) {
            debug("send question error");
            goto cleanup;
        }
        ret = socket_read(&handler, buf, sizeof(buf));
        if (ret == -1) {
            debug("recv answer error");
            goto cleanup;
        }
        ret = parse_answer(list, family, hints, port, buf, ret);
        if (ret == -1) {
            debug("parse answer error");
            goto cleanup;
        }
    cleanup:
        socket_close(&handler);
        if (ret != -1)
            break;
    }

    nameserv_destroy(&nslist);

    if (list->head)
        return 0;

    return -1;
}

static struct dns_node *dns_node_new(const struct dns_node *hints,
                                     struct sockaddr *addr, socklen_t addrlen)
{
    struct dns_node *node;

    node = calloc(1, sizeof(struct dns_node));
    if (!node) {
        debug("calloc error");
        return NULL;
    }

    if (hints)
        memcpy(node, hints, sizeof(struct dns_node));

    node->addr = addr;
    node->addrlen = addrlen;

    return node;
}

static void dns_node_free(struct dns_node *node)
{
    free(node->addr);
    free(node);
}

static int send_question(struct socket_handler *handler, int family,
                         const char *name)
{
    struct dns_header *header;
    struct dns_question *question;
    char buf[512], *qname;
    size_t n;

    memset(buf, 0, sizeof(buf));

    /* dns header. */
    header = (struct dns_header *)buf;
    n = sizeof(struct dns_header);

    header->id = htons(rand() % 0xffff);
    header->flags = htons(1 << 8);
    header->qd_count = htons(1);
    header->an_count = 0;
    header->ns_count = 0;
    header->ar_count = 0;

    /* dns question. */
    /* format "www.h1zzz.net" to "www.h1zzz.net." */
    if (strlen(name) + 1 >= sizeof(buf) - n)
        return -1;

    qname = buf + n;
    n += snprintf(qname, sizeof(buf) - n, "%s.", name);
    format_dns_name(qname);

    if (n >= sizeof(buf))
        return -1;

    buf[n++] = '\0';

    question = (struct dns_question *)(buf + n);
    n += sizeof(struct dns_question);

    if (n > sizeof(buf))
        return -1;

    switch (family) {
    case AF_INET:
        question->qtype = htons(TYPE_A);
        break;
    case AF_INET6:
        question->qtype = htons(TYPE_AAAA);
        break;
    default:
        return -1;
    }

    question->qclass = htons(CLASS_IN);

    if (socket_write(handler, buf, (int)n) == -1) {
        debug("send dns question error");
        return -1;
    }

    return 0;
}

/*
 * TODO: Format multiple domain names.
 * https://datatracker.ietf.org/doc/html/rfc1035#section-4.1.4
 * format "www.h1zzz.net." to "\003www\005h1zzz\003net"
 */
static void format_dns_name(char *name)
{
    char *ptr;
    int i, n;

    ptr = name;
    n = 0;

    /* www.h1zzz.net. */
    while (*ptr) {
        if (ptr[n] == '.') {
            for (i = n - 1; i >= 0; i--)
                ptr[i + 1] = ptr[i];
            *ptr = n;
            ptr = ptr + n + 1; /* +1 skip '.' */
            n = 0;
        } else {
            n++;
        }
    }
}

static struct nameserv_node *nameserv_node_new(struct sockaddr *addr,
                                               socklen_t addrlen)
{
    struct nameserv_node *node;

    node = calloc(1, sizeof(struct nameserv_node));
    if (!node) {
        debug("calloc error");
        return NULL;
    }

    node->addr = addr;
    node->addrlen = addrlen;

    return node;
}

static void nameserv_node_free(struct nameserv_node *node)
{
    free(node->addr);
    free(node);
}

#ifdef _WIN32
static int nameserv_add_local_dns(struct llist *list)
{
    struct nameserv_node *node;
    struct sockaddr_in *si4;
    FIXED_INFO *fInfo;
    ULONG fInfoLen;
    IP_ADDR_STRING *ipAddr;

    fInfo = calloc(1, sizeof(FIXED_INFO));
    if (!fInfo) {
        debug("calloc error");
        return -1;
    }
    fInfoLen = sizeof(FIXED_INFO);

    if (GetNetworkParams(fInfo, &fInfoLen) == ERROR_BUFFER_OVERFLOW) {
        free(fInfo);
        fInfo = calloc(1, fInfoLen);
        if (!fInfo) {
            debug("calloc error");
            return -1;
        }
    }

    if (GetNetworkParams(fInfo, &fInfoLen) != NO_ERROR) {
        free(fInfo);
        return -1;
    }

    ipAddr = &fInfo->DnsServerList;

    while (ipAddr) {
        if (!util_is_ipv4(ipAddr->IpAddress.String)) {
            debug("Currently only supports the use of IPv4 DNS servers");
            ipAddr = ipAddr->Next;
            continue;
        }

        si4 = calloc(1, sizeof(struct sockaddr_in));
        if (!si4) {
            debug("calloc error");
            goto err;
        }

        if (inet_pton(AF_INET, ipAddr->IpAddress.String, &si4->sin_addr) <= 0) {
            debug("inet_pton error");
            free(si4);
            goto err;
        }

        si4->sin_family = AF_INET;
        si4->sin_port = htons(DNS_PORT);
        node = nameserv_node_new((struct sockaddr *)si4,
                                 (socklen_t)sizeof(struct sockaddr_in));
        if (!node) {
            debug("nameserv_node_new error");
            free(si4);
            goto err;
        }
        llist_insert_next(list, list->tail, (struct lnode *)node);
        ipAddr = ipAddr->Next;
    }

    free(fInfo);
    return 0;
err:
    free(fInfo);
    return -1;
}
#else  /* No defined _WIN32 */
static int nameserv_add_local_dns(struct llist *list)
{
    struct nameserv_node *node;
    struct sockaddr_in *si4;
    FILE *fp;
    char buf[256], *ptr;
    int ret;

    fp = fopen("/etc/resolv.conf", "r");
    if (!fp) {
        debug("fopen /etc/resolv.conf error");
        return -1;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, sizeof(buf) - 1, fp) == NULL)
            break;

        if (buf[0] == '#') /* skip comment */
            continue;

        ptr = strchr(buf, '\n');
        if (!ptr)
            continue;

        *ptr = '\0';

        if (strncmp(buf, "nameserver ", 11) != 0)
            continue;

        ptr = &buf[11];

        if (!check_is_ipv4(ptr)) {
            debug("Currently only supports the use of IPv4 DNS servers");
            continue;
        }

        si4 = calloc(1, sizeof(struct sockaddr_in));
        if (!si4) {
            debug("calloc error");
            goto err;
        }

        ret = inet_pton(AF_INET, ptr, &si4->sin_addr);
        if (ret <= 0) {
            if (ret == 0)
                continue;
            debug("inet_pton error");
            free(si4);
            goto err;
        }

        si4->sin_family = AF_INET;
        si4->sin_port = htons(DNS_PORT);
        node = nameserv_node_new((struct sockaddr *)si4,
                                 (socklen_t)sizeof(struct sockaddr_in));
        if (!node) {
            debug("nameserv_node_new error");
            free(si4);
            goto err;
        }
        llist_insert_next(list, list->tail, (struct lnode *)node);
    }

    fclose(fp);
    return 0;
err:
    fclose(fp);
    return -1;
}
#endif /* _WIN32 */

static int nameserv_init(struct llist *list)
{
    struct nameserv_node *node;
    struct sockaddr_in *si4;
    size_t i, n;

    llist_init(list, NULL, (lnode_free_t *)nameserv_node_free);
    nameserv_add_local_dns(list);

    n = sizeof(defnameserv) / sizeof(defnameserv[0]);
    for (i = 0; i < n; i++) {
        si4 = calloc(1, sizeof(struct sockaddr_in));
        if (!si4) {
            debug("calloc error");
            goto err;
        }
        if (inet_pton(AF_INET, defnameserv[i], &si4->sin_addr) <= 0) {
            debug("inet_pton error");
            free(si4);
            goto err;
        }
        si4->sin_family = AF_INET;
        si4->sin_port = htons(DNS_PORT);
        node = nameserv_node_new((struct sockaddr *)si4,
                                 (socklen_t)sizeof(struct sockaddr_in));
        if (!node) {
            debug("nameserv_node_new error");
            free(si4);
            goto err;
        }
        llist_insert_next(list, list->tail, (struct lnode *)node);
    }

    return 0;
err:
    nameserv_destroy(list);
    return -1;
}

static int parse_answer(struct llist *list, int family,
                        const struct dns_node *hints, uint16_t port, char *data,
                        int n)
{
    uint16_t i, an_count, type, class, rd_length;
    struct dns_header *header;
    struct dns_rrs *answer;
    struct sockaddr_in *si4;
    struct sockaddr_in6 *si6;
    struct dns_node *dns_node;
    int pos, ret;
    char name[256];

    pos = sizeof(struct dns_header);
    if (pos >= n)
        return -1;

    header = (struct dns_header *)data;
    an_count = ntohs(header->an_count);

    memset(name, 0, sizeof(name));
    ret = dns_read_name(data, data + pos, name, sizeof(name));
    if (ret >= n - pos)
        return -1;

    pos += ret;
    if (sizeof(struct dns_question) > (size_t)n - pos)
        return -1;
    pos += sizeof(struct dns_question);

    for (i = 0; i < an_count; i++) {
        memset(name, 0, sizeof(name));
        ret = dns_read_name(data, data + pos, name, sizeof(name));
        if (ret >= n - pos)
            return -1;

        pos += ret;
        if (sizeof(struct dns_rrs) >= (size_t)n - pos)
            return -1;

        answer = (struct dns_rrs *)(data + pos);
        pos += sizeof(struct dns_rrs);

        rd_length = ntohs(answer->rd_length);
        if (rd_length > n - pos)
            return -1;

        type = ntohs(answer->type);
        class = ntohs(answer->class);

        switch (class) {
        case CLASS_IN: {
            switch (type) {
            case TYPE_A: { /* IPv4 */
                if (family != AF_INET) {
                    break;
                }
                si4 = calloc(1, sizeof(struct sockaddr_in));
                if (!si4) {
                    debug("calloc error");
                    break;
                }
                memcpy(&si4->sin_addr, data + pos, rd_length);
                si4->sin_family = AF_INET;
                si4->sin_port = htons(port);
                dns_node = dns_node_new(hints, (struct sockaddr *)si4,
                                        sizeof(struct sockaddr_in));
                if (!dns_node) {
                    debug("dns_node_new error");
                    free(si4);
                    break;
                }
                llist_insert_next(list, list->tail, (struct lnode *)dns_node);
                break;
            }
            case TYPE_AAAA: { /* IPv6 */
                if (rd_length == 1 && data[pos] == 0) {
                    break;
                }
                if (family != AF_INET6) {
                    break;
                }
                si6 = calloc(1, sizeof(struct sockaddr_in6));
                if (!si6) {
                    debug("calloc error");
                    break;
                }
                memcpy(&si6->sin6_addr, data + pos, rd_length);
                si6->sin6_family = AF_INET6;
                si6->sin6_port = htons(port);
                dns_node = dns_node_new(hints, (struct sockaddr *)si6,
                                        sizeof(struct sockaddr_in6));
                if (!dns_node) {
                    debug("dns_node_new error");
                    free(si6);
                    break;
                }
                llist_insert_next(list, list->tail, (struct lnode *)dns_node);
                break;
            }
            case TYPE_CNAME: {
                memset(name, 0, sizeof(name));
                dns_read_name(data, data + pos, name, sizeof(name));
                break;
            }
            /* ... */
            default:
                debug("nosupported type.");
                break;
            }
            break;
        }
        default:
            debug("nosuppoted class type.");
            break;
        }
        pos += rd_length;
    }

    return 0;
}

static int dns_read_name(char *data, char *ptr, char *name, size_t size)
{
    unsigned int offset, count = 1, n, jumped = 0, i = 0;

    while (*ptr) {

        if (*(uint8_t *)ptr >= 0xc0) {
            offset = ntohs(*(uint16_t *)ptr) - 0xc000;
            ptr = data + offset;
            jumped = 1;
        }

        n = *ptr++;

        if (!jumped)
            count += n + 1;

        while (n--) {
            if (name && i < size)
                name[i++] = *ptr++;
        }

        if (name && i < size)
            name[i++] = '.';
    }

    if (jumped)
        count++;

    if (name && i < size)
        name[i - 1] = '\0';

    return count;
}
