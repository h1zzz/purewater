/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _LIST_H
#define _LIST_H

typedef struct llist llist_t;

typedef int lnode_find_t(const void *, const void *);
typedef void lnode_free_t(void *);

struct lnode {
    struct lnode *prev;
    struct lnode *next;
    llist_t *list;
};

struct llist {
    struct lnode *head;
    struct lnode *tail;
    lnode_find_t *lnode_find;
    lnode_free_t *lnode_free;
};

void llist_init(llist_t *list, lnode_find_t *lnode_find,
                lnode_free_t *lnode_free);
void llist_insert_next(llist_t *list, struct lnode *pos, struct lnode *node);
void llist_remove(llist_t *list, struct lnode *node);
struct lnode *llist_find(llist_t *list, const void *key);
void llist_destroy(llist_t *list);

#endif /* list.h */
