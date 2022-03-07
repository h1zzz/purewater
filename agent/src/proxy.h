/* MIT License Copyright (c) 2022, h1zzz */

#ifndef _PROXY_H
#define _PROXY_H

#include <stdint.h>

#include <mbedtls/net_sockets.h>

struct http_proxy;

struct http_proxy *http_proxy_new(const char *host, uint16_t port,
                                  const char *user, const char *passwd);
int http_proxy_connect(struct http_proxy *proxy, mbedtls_net_context *ctx,
                       const char *host, uint16_t port);
void http_proxy_free(struct http_proxy *proxy);

#endif /* proxy.h */
