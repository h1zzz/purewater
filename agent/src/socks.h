/* MIT License Copyright (c) 2022, h1zzz */

#ifndef _SOCKS_H
#define _SOCKS_H

#include <stdint.h>

#include "net.h"

struct socks5_client;
struct socks5_server;

struct socks5_client *socks5_client_new(const char *host, uint16_t port,
                                        const char *user, const char *passwd);

int socks5_client_connect(struct socks5_client *client, net_context *ctx,
                          const char *host, uint16_t port);

void socks5_client_free(struct socks5_client *client);

#endif /* socks.h */
