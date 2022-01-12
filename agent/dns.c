/* MIT License Copyright (c) 2021, h1zzz */

#include "dns.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "debug.h"
#include "linklist.h"
#include "socket.h"
#include "util.h"

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

struct dns_server {
    struct linknode node;
    char *host;
    uint16_t port;
};

struct dns {
    struct linklist dns_servers;
};

static struct dns_server *dns_server_new(const char *host, uint16_t port)
{
    struct dns_server *server;

    server = calloc(1, sizeof(struct dns_server));
    if (!server) {
        debug("calloc error");
        return NULL;
    }

    server->host = xstrdup(host);
    if (!server->host) {
        debug("xstrdup error");
        free(server);
        return NULL;
    }

    server->port = port;
    return server;
}

static void dns_server_free(struct dns_server *server)
{
    free(server->host);
    free(server);
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

static int send_question(socket_t sock, const char *name, int type)
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

    if (xsend(sock, buf, (int)n) == -1) {
        debug("xsend error");
        return -1;
    }

    return 0;
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

static int append_dns_node(struct dns_node **res, int type, const char *data,
                           size_t len)
{
    struct dns_node *dns_node;

    dns_node = calloc(1, sizeof(struct dns_node));
    if (!dns_node) {
        debug("calloc error");
        return -1;
    }

    assert(len <= sizeof(dns_node->data));

    switch (type) {
    case DNS_A:
        dns_node->data_len = len;
        if (!inet_ntop(AF_INET, data, dns_node->data, sizeof(dns_node->data))) {
            debug("inet_ntop error");
            free(dns_node);
            dns_node = NULL;
        }
        break;
    case DNS_TXT:
        dns_node->data_len = data[0];
        memcpy(dns_node->data, data + 1, dns_node->data_len);
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

    return 0;
}

static int parse_answer(struct dns_node **res, char *data, int n)
{
    uint16_t i, an_count, type, class, rd_length;
    struct dns_header *header;
    struct dns_rrs *answer;
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

        if (append_dns_node(res, type, data + pos, rd_length) == -1) {
            debug("append_dns_node error");
            return -1;
        }

        pos += rd_length;
    }

    return 0;
}

dns_t *dns_new(void)
{
    dns_t *dns;

    dns = calloc(1, sizeof(dns_t));
    if (!dns) {
        debug("calloc error");
        return NULL;
    }

    linklist_init(&dns->dns_servers, NULL, (linknode_free_t *)dns_server_free);

    return dns;
}

int dns_add_dns_server(dns_t *dns, const char *host, uint16_t port)
{
    struct dns_server *server;

    assert(dns);
    assert(host);
    assert(port != 0);

    server = dns_server_new(host, port);
    if (!server) {
        debug("dns_server_new error");
        return -1;
    }

    linklist_insert_next(&dns->dns_servers, dns->dns_servers.tail,
                         (struct linknode *)server);
    return 0;
}

struct dns_node *dns_lookup(dns_t *dns, const char *name, int type)
{
    struct dns_node *dns_node, *new_node;
    struct dns_server *server;
    struct linknode *node;
    char buf[10240];
    socket_t sock;
    int ret;

    dns_node = NULL;

    if (is_ipv4(name)) {
        new_node = calloc(1, sizeof(struct dns_node));
        if (!new_node) {
            debug("calloc error");
            return NULL;
        }

        new_node->type = type;
        new_node->data_len = strlen(name);

        memcpy(new_node->data, name, new_node->data_len);

        new_node->next = dns_node;
        dns_node = new_node;

        return dns_node;
    }

    if (!dns->dns_servers.head) {
        debug("no dns_server");
        return NULL;
    }

    for (node = dns->dns_servers.head; node; node = node->next) {
        server = (struct dns_server *)node;

        sock = xsocket(SOCK_UDP);
        if (sock == SOCK_INVAL) {
            debug("xsocket error");
            continue;
        }

        ret = xconnect(sock, server->host, server->port);
        if (ret == -1) {
            debug("xconnect error");
            goto cleanup;
        }

        ret = send_question(sock, name, type);
        if (ret == -1) {
            debug("send_question error");
            goto cleanup;
        }

        ret = xrecv(sock, buf, sizeof(buf));
        if (ret == -1) {
            debug("xrecv error");
            goto cleanup;
        }

        ret = parse_answer(&dns_node, buf, ret);
        if (ret == -1) {
            debug("parse_answer error");
            goto cleanup;
        }

    cleanup:
        xclose(sock);
        if (ret != -1)
            break;
    }

    return dns_node;
}

void dns_node_cleanup(struct dns_node *dns_node)
{
    struct dns_node *next, *curr = dns_node;

    assert(dns_node);

    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

void dns_free(dns_t *dns)
{
    assert(dns);
    linklist_destroy(&dns->dns_servers);
    free(dns);
}

#ifdef _WIN32
static int dns_add_dns_server_for_local(dns_t *dns)
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
        if (!is_ipv4(ipAddr->IpAddress.String)) {
            debug("Currently only supports the use of IPv4 DNS servers");
            ipAddr = ipAddr->Next;
            continue;
        }
        if (dns_add_dns_server(dns, ipAddr->IpAddress.String, DNS_PORT) == -1) {
            debug("dns_add_dns_server error");
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
static int dns_add_dns_server_for_local(dns_t *dns)
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

        if (!is_ipv4(ptr)) {
            debug("currently only supports the use of IPv4 DNS servers");
            continue;
        }

        if (dns_add_dns_server(dns, ptr, DNS_PORT) == -1) {
            debug("dns_add_dns_server error");
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

struct dns_node *dns_lookup_ret(const char *host, int type)
{
    static const char *dns_host[] = {"8.8.8.8", "9.9.9.9", "1.1.1.1", "1.2.4.8",
                                     NULL};
    struct dns_node *dns_node;
    dns_t *dns;
    int i;

    dns = dns_new();
    if (!dns) {
        debug("dns_new error");
        return NULL;
    }

    dns_add_dns_server_for_local(dns);

    for (i = 0; dns_host[i]; i++)
        dns_add_dns_server(dns, dns_host[i], DNS_PORT);

    dns_node = dns_lookup(dns, host, type);
    dns_free(dns);

    return dns_node;
}
