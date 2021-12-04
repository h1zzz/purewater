/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _LIST_H
#define _LIST_H

typedef struct llist llist_t;
typedef struct lnode lnode_t;

typedef int lnode_find_t(const void *, const void *);
typedef void lnode_free_t(void *);

struct lnode {
    lnode_t *prev;
    lnode_t *next;
    llist_t *list;
};

struct llist {
    lnode_t *head;
    lnode_t *tail;
    lnode_find_t *_find;
    lnode_free_t *_free;
};

void llist_init(llist_t *list, lnode_find_t *_find, lnode_free_t *_free);
void llist_insert_next(llist_t *list, lnode_t *pos, lnode_t *node);
void llist_remove(llist_t *list, lnode_t *node);
lnode_t *llist_find(llist_t *list, const void *key);
void llist_destroy(llist_t *list);

#endif /* list.h */
