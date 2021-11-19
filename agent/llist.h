/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _LIST_H
#define _LIST_H

struct llist;

typedef int lnode_find_t(const void *, const void *);
typedef void lnode_free_t(void *);

struct lnode {
    struct lnode *prev;
    struct lnode *next;
    struct llist *list;
};

struct llist {
    struct lnode *head;
    struct lnode *tail;
    lnode_find_t *_find;
    lnode_free_t *_free;
};

void llist_init(struct llist *list, lnode_find_t *_find, lnode_free_t *_free);
void llist_insert_next(struct llist *list, struct lnode *pos,
                       struct lnode *node);
void llist_remove(struct llist *list, struct lnode *node);
struct lnode *llist_find(struct llist *list, const void *key);
void llist_destroy(struct llist *list);

#endif /* list.h */

