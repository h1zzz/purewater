/* MIT License Copyright (c) 2022, h1zzz */

#include "dns.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else /* No define _WIN32 */
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"
#include "net.h"
#include "util.h"

#ifdef _MSC_VER
#pragma comment(lib, "iphlpapi.lib")
#endif /* _MSC_VER */

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

static struct dns_ns *dns_ns_new(const char *host, uint16_t port);
static void dns_ns_free(struct dns_ns *ns);
static struct dns_node *_dns_query(net_context *ctx, const char *domain,
                                   int type);
static void dns_format_name(char *name);
static int dns_read_name(char *data, char *ptr, char *name, size_t size);
static int check_is_ipv4(const char *ip);
static int dns_add_local_ns(dns_context *ctx);

void dns_init(dns_context *ctx) {
    ctx->ns_head = NULL;
    ctx->ns_tail = NULL;
}

int dns_add_ns(dns_context *ctx, const char *host, uint16_t port) {
    struct dns_ns *ns;

    ASSERT(ctx);
    ASSERT(host);
    ASSERT(port);

    ns = dns_ns_new(host, port);
    if (!ns) {
        DBG("dns_ns_new error");
        return -1;
    }

    ASSERT(ns->next == NULL);

    if (ctx->ns_head) {
        ctx->ns_tail->next = ns;
        ctx->ns_tail = ns;
    } else {
        ctx->ns_head = ns;
        ctx->ns_tail = ns;
    }

    return 0;
}

struct dns_node *dns_query(dns_context *ctx, const char *domain, int type) {
    struct dns_node *dns_node = NULL;
    struct dns_ns *ns;
    net_context net_ctx;
    int ret;

    ASSERT(ctx);
    ASSERT(domain);
    ASSERT(type == DNS_A || type == DNS_TXT);
    ASSERT(ctx->ns_head);

    if (check_is_ipv4(domain)) {
        dns_node = calloc(1, sizeof(struct dns_node));
        if (!dns_node) {
            DBGERR("calloc error");
            return NULL;
        }

        memcpy(dns_node->data, domain, strlen(domain));
        dns_node->type = type;
        dns_node->data_len = strlen(domain);

        return dns_node;
    }

    for (ns = ctx->ns_head; ns; ns = ns->next) {
        net_init(&net_ctx);

        ret = net_connect(&net_ctx, ns->host, ns->port, NET_UDP);
        if (ret == -1) {
            net_free(&net_ctx);
            DBG("net_connect error");
            continue;
        }

        dns_node = _dns_query(&net_ctx, domain, type);
        if (dns_node) {
            net_free(&net_ctx);
            break;
        }

        net_free(&net_ctx);
    }

    return dns_node;
}

void dns_free(dns_context *ctx) {
    struct dns_ns *curr, *next;

    ASSERT(ctx);

    curr = ctx->ns_head;
    while (curr) {
        next = curr->next;
        dns_ns_free(curr);
        curr = next;
    }
}

void dns_node_destroy(struct dns_node *dns_node) {
    struct dns_node *curr, *next;

    ASSERT(dns_node);

    curr = dns_node;
    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

struct dns_node *dns_query_ret(const char *domain, int type) {
    struct dns_node *dns_node;
    dns_context ctx;

    ASSERT(domain);
    ASSERT(type == DNS_A || type == DNS_TXT);

    dns_init(&ctx);

    dns_add_local_ns(&ctx);

    dns_add_ns(&ctx, "8.8.8.8", 53);
    dns_add_ns(&ctx, "9.9.9.9", 53);
    dns_add_ns(&ctx, "1.1.1.1", 53);
    dns_add_ns(&ctx, "1.2.4.8", 53);

    dns_node = dns_query(&ctx, domain, type);
    dns_free(&ctx);

    return dns_node;
}

static struct dns_ns *dns_ns_new(const char *host, uint16_t port) {
    struct dns_ns *ns;

    ns = calloc(1, sizeof(struct dns_ns));
    if (!ns) {
        DBGERR("calloc error");
        return NULL;
    }

    ns->host = xstrdup(host);
    ns->port = port;
    ns->next = NULL;

    return ns;
}

static void dns_ns_free(struct dns_ns *ns) {
    ASSERT(ns);
    if (ns->host) {
        free(ns->host);
    }
    free(ns);
}

static struct dns_node *_dns_query(net_context *ctx, const char *domain,
                                   int type) {
    struct dns_node *dns_node = NULL, *dns_node_ptr;
    uint16_t i, an_count, _type, class, rd_length;
    struct dns_rrs *answer;
    struct dns_header *header;
    struct dns_question *question;
    char buf[10240] = {0}, *qname;
    size_t n;
    int ret, pos;

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
    ASSERT(strlen(domain) + 1 < sizeof(buf) - n);

    qname = buf + n;
    n += snprintf(qname, sizeof(buf) - n, "%s.", domain);
    dns_format_name(qname);

    ASSERT(n < sizeof(buf));

    buf[n++] = '\0';

    question = (struct dns_question *)(buf + n);
    n += sizeof(struct dns_question);

    ASSERT(n < sizeof(buf));

    question->qtype = htons(type);
    question->qclass = htons(CLASS_IN);

    ret = net_send(ctx, buf, n);
    if (ret <= 0) {
        DBG("net_send error");
        return NULL;
    }

    ret = net_recv(ctx, buf, sizeof(buf));
    if (ret <= 0) {
        DBG("net_recv error");
        return NULL;
    }

    n = (size_t)ret;

    pos = sizeof(struct dns_header);
    if ((size_t)pos >= n) {
        DBG("invalid message");
        return NULL;
    }

    header = (struct dns_header *)buf;
    an_count = ntohs(header->an_count);

    ret = dns_read_name(buf, buf + pos, NULL, 0);
    if ((size_t)ret >= n - pos) {
        DBG("invalid message");
        return NULL;
    }

    pos += ret;

    if (sizeof(struct dns_question) > (size_t)n - pos) {
        return NULL;
    }

    pos += sizeof(struct dns_question);

    for (i = 0; i < an_count; i++) {
        ret = dns_read_name(buf, buf + pos, NULL, 0);
        if ((size_t)ret >= n - pos) {
            DBG("invalid message");
            goto end;
        }

        pos += ret;
        if (sizeof(struct dns_rrs) >= (size_t)n - pos) {
            DBG("invalid message");
            goto end;
        }

        answer = (struct dns_rrs *)(buf + pos);
        pos += sizeof(struct dns_rrs);

        rd_length = ntohs(answer->rd_length);
        if (rd_length > n - pos) {
            DBG("invalid message");
            goto end;
        }

        _type = ntohs(answer->type);
        class = ntohs(answer->class);

        if (class != CLASS_IN) {
            DBGF("nosuppoted class %d.", class);
            continue;
        }

        dns_node_ptr = calloc(1, sizeof(struct dns_node));
        if (!dns_node_ptr) {
            DBGERR("calloc error");
            goto end;
        }

        dns_node_ptr->type = _type;

        ASSERT(rd_length <= sizeof(dns_node_ptr->data));

        switch (_type) {
        case DNS_A:
            if (!inet_ntop(AF_INET, buf + pos, dns_node_ptr->data,
                           sizeof(dns_node_ptr->data))) {
                DBGERR("inet_ntop error");
                free(dns_node_ptr);
                dns_node_ptr = NULL;
            }
            dns_node_ptr->data_len = strlen(dns_node_ptr->data);
            break;
        case DNS_TXT:
            dns_node_ptr->data_len = buf[pos];
            memcpy(dns_node_ptr->data, buf + pos + 1, dns_node_ptr->data_len);
            break;
        default:
            /* DBGF("nosupported type: %d", _type); */
            free(dns_node_ptr);
            dns_node_ptr = NULL;
            break;
        }

        if (dns_node_ptr) {
            dns_node_ptr->next = dns_node;
            dns_node = dns_node_ptr;
        }

        pos += rd_length;
    }

end:
    return dns_node;
}

static int dns_read_name(char *data, char *ptr, char *name, size_t size) {
    unsigned int offset, count = 1, n, jumped = 0, i = 0;
    char ch;

    while (*ptr) {
        if (*(uint8_t *)ptr >= 0xc0) {
            offset = ntohs(*(uint16_t *)ptr) - 0xc000;
            ptr = data + offset;
            jumped = 1;
        }

        n = *ptr++;

        if (!jumped) {
            count += n + 1;
        }

        while (n--) {
            ch = *ptr++;
            if (name && i < size) {
                name[i++] = ch;
            }
        }

        if (name && i < size) {
            name[i++] = '.';
        }
    }

    if (jumped) {
        count++;
    }

    if (name && i < size) {
        name[i - 1] = '\0';
    }

    return count;
}

/*
 * TODO: Format multiple domain names.
 * https://datatracker.ietf.org/doc/html/rfc1035#section-4.1.4
 * format "www.h1zzz.net." to "\003www\005h1zzz\003net"
 */
static void dns_format_name(char *name) {
    char *ptr;
    int i, n;

    ptr = name;
    n = 0;

    /* www.h1zzz.net. */
    while (*ptr) {
        if (ptr[n] == '.') {
            for (i = n - 1; i >= 0; i--) {
                ptr[i + 1] = ptr[i];
            }
            *ptr = n;
            ptr = ptr + n + 1; /* +1 skip '.' */
            n = 0;
        } else {
            n++;
        }
    }
}

static int check_is_ipv4(const char *ip) {
    const char *s = ip;
    size_t i;
    int n;

    for (i = 0; i < sizeof(struct in_addr); i++) {
        if (*s == '\0') {
            return 0;
        }

        n = 0;

        while (*s) {
            if ('0' <= *s && *s <= '9') {
                n = n * 10 + (*s - '0');
                s++;
            } else {
                s++;
                break;
            }
        }

        if (n > 255) {
            return 0;
        }
    }

    if (*s != '\0' || i != sizeof(struct in_addr)) {
        return 0;
    }

    return 1;
}

#ifdef _WIN32
static int dns_add_local_ns(dns_context *ctx) {
    FIXED_INFO *fInfo;
    ULONG fInfoLen;
    IP_ADDR_STRING *ipAddr;

    fInfo = calloc(1, sizeof(FIXED_INFO));
    if (!fInfo) {
        DBGERR("calloc error");
        return -1;
    }
    fInfoLen = sizeof(FIXED_INFO);

    if (GetNetworkParams(fInfo, &fInfoLen) == ERROR_BUFFER_OVERFLOW) {
        free(fInfo);
        fInfo = calloc(1, fInfoLen);
        if (!fInfo) {
            DBGERR("calloc error");
            return -1;
        }
    }

    if (GetNetworkParams(fInfo, &fInfoLen) != NO_ERROR) {
        DBGERR("GetNetworkParams error");
        free(fInfo);
        return -1;
    }

    ipAddr = &fInfo->DnsServerList;

    while (ipAddr) {
        if (!check_is_ipv4(ipAddr->IpAddress.String)) {
            DBG("Currently only supports the use of IPv4 DNS servers");
            ipAddr = ipAddr->Next;
            continue;
        }
        if (dns_add_ns(dns, ipAddr->IpAddress.String, 53) == -1) {
            DBG("dns_add_dns_server error");
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
#else  /* No define _WIN32 */
static int dns_add_local_ns(dns_context *ctx) {
    FILE *fp;
    char buf[256], *ptr;

    fp = fopen("/etc/resolv.conf", "r");
    if (!fp) {
        DBGERR("fopen error");
        return -1;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
            break;
        }

        /* skip comment */
        if (buf[0] == '#') {
            continue;
        }

        ptr = strchr(buf, '\n');
        if (!ptr) {
            continue;
        }

        *ptr = '\0';

        if (strncmp(buf, "nameserver ", 11) != 0) {
            continue;
        }

        ptr = &buf[11];

        if (!check_is_ipv4(ptr)) {
            DBG("currently only supports the use of IPv4 DNS servers");
            continue;
        }

        if (dns_add_ns(ctx, ptr, 53) == -1) {
            DBG("dns_add_dns_server error");
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
