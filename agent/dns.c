/* MIT License Copyright (c) 2021, h1zzz */

#include "dns.h"

#ifdef _WIN32
#include <iphlpapi.h>
#endif /* _WIN32 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "debug.h"
#include "socket.h"
#include "util.h"
#include "llist.h"

#ifdef _MSC_VER
#pragma comment(lib, "iphlpapi.lib")
#endif /* _MSC_VER */

#define DNS_PORT 53

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

struct nameserver_node {
    struct lnode _node;
    struct sockaddr *addr;
    socklen_t addrlen;
};

static const char *default_nameserver[] = {
    "8.8.8.8",
    "9.9.9.9",
    "1.1.1.1",
    "1.2.4.8",
};

#if 0
static struct dns_node *dns_node_new(dns_type_t type, void *result)
{
    struct dns_node *node;

    node = calloc(1, sizeof(struct dns_node));
    if (!node) {
        debug("calloc error");
        return NULL;
    }

    node->type = type;

    return node;
}

static void dns_node_free(struct dns_node *node)
{
    switch (node->type) {
    case DNS_A:
        free(node->addr);
        break;
    case DNS_TXT:
        free(node->txt);
        break;
    default:
        debug("nosupported dns type");
    }
    free(node);
}
#endif

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

static int send_question(socket_handle_t *sock, const char *name, int type)
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

    question->qtype = htons(type);
    question->qclass = htons(CLASS_IN);

    if (socket_send(sock, buf, (int)n) == -1) {
        debug("send dns question error");
        return -1;
    }

    return 0;
}

static struct nameserver_node *nameserver_node_new(struct sockaddr *addr,
                                                   socklen_t addrlen)
{
    struct nameserver_node *node;

    node = calloc(1, sizeof(struct nameserver_node));
    if (!node) {
        debug("calloc error");
        return NULL;
    }

    node->addr = addr;
    node->addrlen = addrlen;

    return node;
}

static void nameserver_node_free(struct nameserver_node *node)
{
    free(node->addr);
    free(node);
}

static int append_nameserver(llist_t *list, const char *ip)
{
    struct nameserver_node *node;
    struct sockaddr_in *addr;
    int ret;

    addr = calloc(1, sizeof(struct sockaddr_in));
    if (!addr) {
        debug("calloc error");
        return -1;
    }

    ret = inet_pton(AF_INET, ip, &addr->sin_addr);
    if (ret <= 0) {
        debug("inet_pton error");
        free(addr);
        return -1;
    }

    addr->sin_family = AF_INET;
    addr->sin_port = htons(DNS_PORT);
    node = nameserver_node_new((struct sockaddr *)addr,
                               (socklen_t)sizeof(struct sockaddr_in));
    if (!node) {
        debug("nameserver_node_new error");
        free(addr);
        return -1;
    }

    llist_insert_next(list, list->tail, (struct lnode *)node);
    return 0;
}

#ifdef _WIN32
static int nameserver_add_local_dns_nameserver(llist_t *list)
{
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
        if (!check_is_ipv4(ipAddr->IpAddress.String)) {
            debug("Currently only supports the use of IPv4 DNS servers");
            ipAddr = ipAddr->Next;
            continue;
        }
        if (append_nameserver(list, ipAddr->IpAddress.String) == -1) {
            debug("append_nameserver error");
            goto err;
        }
        ipAddr = ipAddr->Next;
    }

    free(fInfo);
    return 0;
err:
    free(fInfo);
    return -1;
}
#else  /* No defined _WIN32 */
static int nameserver_add_local_dns_nameserver(llist_t *list)
{
    FILE *fp;
    char buf[256], *ptr;

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

        if (append_nameserver(list, ptr) == -1) {
            debug("append_nameserver error");
            goto err;
        }
    }

    fclose(fp);
    return 0;
err:
    fclose(fp);
    return -1;
}
#endif /* _WIN32 */

static void nameserver_destroy(llist_t *list)
{
    llist_destroy(list);
}

static int nameserver_init(llist_t *list)
{
    size_t i, n;

    llist_init(list, NULL, (lnode_free_t *)nameserver_node_free);
    nameserver_add_local_dns_nameserver(list);

    n = sizeof(default_nameserver) / sizeof(default_nameserver[0]);
    for (i = 0; i < n; i++)
        if (append_nameserver(list, default_nameserver[i]) == -1) {
            debug("append_nameserver error");
            goto err;
        }

    return 0;
err:
    nameserver_destroy(list);
    return -1;
}

static int dns_read_name(char *data, char *ptr, char *name, size_t size)
{
    unsigned int offset, count = 1, n, jumped = 0, i = 0;
    char ch;

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
            ch = *ptr++;
            if (name && i < size)
                name[i++] = ch;
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

static int parse_answer(struct dns_node **res, char *data, int n)
{
    uint16_t i, an_count, type, class, rd_length;
    struct dns_header *header;
    struct dns_rrs *answer;
    struct dns_node *dns_node;
    int pos, ret;

    pos = sizeof(struct dns_header);
    if (pos >= n)
        return -1;

    header = (struct dns_header *)data;
    an_count = ntohs(header->an_count);

    ret = dns_read_name(data, data + pos, NULL, 0);
    if (ret >= n - pos)
        return -1;

    pos += ret;
    if (sizeof(struct dns_question) > (size_t)n - pos)
        return -1;
    pos += sizeof(struct dns_question);

    for (i = 0; i < an_count; i++) {
        ret = dns_read_name(data, data + pos, NULL, 0);
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

        if (class != CLASS_IN) {
            debugf("nosuppoted class %d.", class);
            return 0;
        }

        dns_node = calloc(1, sizeof(struct dns_node));
        if (!dns_node) {
            debug("calloc error");
            return -1;
        }

        /* assert(rd_length <= sizeof(dns_node->data)); */

        switch (type) {
        case DNS_A:
            dns_node->data_len = rd_length;
            memcpy(dns_node->data, data + pos + 1, dns_node->data_len);
            break;
        case DNS_TXT:
            dns_node->data_len = data[pos];
            memcpy(dns_node->data, data + pos + 1, dns_node->data_len);
            break;
        default:
            debugf("nosupported type: %d", type);
            free(dns_node);
            dns_node = NULL;
            break;
        }

        if (dns_node) {
            dns_node->next = *res;
            *res = dns_node;
        }
        pos += rd_length;
    }

    return 0;
}

int dns_lookup(struct dns_node **res, const char *name, dns_type_t type)
{
    socket_handle_t sock;
    int ret;
    llist_t nslist;
    struct lnode *node;
    struct nameserver_node *nameserv;
    char buf[10240];
    struct dns_node *dns_node;

    *res = NULL;

    /* If it is an IP address, it is directly resolved to an address. */
    if (check_is_ipv4(name)) {
        dns_node = calloc(1, sizeof(struct dns_node));
        if (!dns_node) {
            debug("dns_node_new error");
            return -1;
        }

        dns_node->type = type;
        dns_node->data_len = sizeof(struct in_addr);

        ret = inet_pton(AF_INET, name, dns_node->data);
        if (ret <= 0) {
            free(dns_node);
            return -1;
        }

        dns_node->next = *res;
        *res = dns_node;

        return 0;
    }

    /* If there are records in the hosts file, get the address from the
       hosts file, unsupported. */

    if (nameserver_init(&nslist) == -1) {
        debug("nameserver_init error");
        return -1;
    }

    for (node = nslist.head; node; node = node->next) {
        nameserv = (struct nameserver_node *)node;
        ret = socket_open(&sock, PF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (ret == -1) {
            debug("open socket error");
            perror("socket_open");
            continue;
        }

        /* TODO: supported IPv6 name server. */
        ret = socket_connect(&sock, nameserv->addr, nameserv->addrlen);
        if (ret == -1) {
            debug("socket_connect error");
            goto cleanup;
        }

        ret = send_question(&sock, name, type);
        if (ret == -1) {
            debug("send question error");
            goto cleanup;
        }

        ret = socket_recv(&sock, buf, sizeof(buf));
        if (ret == -1) {
            debug("recv answer error");
            goto cleanup;
        }

        ret = parse_answer(res, buf, ret);
        if (ret == -1) {
            debug("parse answer error");
            dns_cleanup(*res);
            goto cleanup;
        }
    cleanup:
        socket_close(&sock);
        if (ret != -1)
            break;
    }

    nameserver_destroy(&nslist);

    if (*res)
        return 0;

    return -1;
}

void dns_cleanup(struct dns_node *head)
{
    struct dns_node *next, *curr = head;

    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}
