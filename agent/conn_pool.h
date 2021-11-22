/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _CONN_POOL_H
#define _CONN_POOL_H

#include <stddef.h>

#include "llist.h"

struct conn_pool_base_conn {
    struct lnode _node;
    void *conn; /* Basic connection handle */
    /*
     * Callback of the read function, it should return the number of bytes
     * successfully read if it succeeds, and -1 if it fails
     */
    int (*read)(void *, void *, size_t);
    /*
     * Callback of the write function, if it succeeds, it should return the
     * number of bytes written successfully, if it fails, it returns -1
     */
    int (*write)(void *, const void *, size_t);
};

struct conn_pool {
    struct llist base_conn;
};

int conn_pool_read(struct conn_node *conn);
int conn_pool_write(struct conn_node *conn);

#endif /* conn_pool.h */
