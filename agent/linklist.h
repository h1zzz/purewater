/* MIT License Copyright (c) 2021, h1zzz */

#ifndef _LINKLIST_H
#define _LINKLIST_H

typedef int linknode_find_t(const void *, const void *);
typedef void linknode_free_t(void *);

struct linklist;

struct linknode {
    struct linknode *prev;
    struct linknode *next;
    struct linklist *list;
};

struct linklist {
    struct linknode *head;
    struct linknode *tail;
    linknode_find_t *linknode_find;
    linknode_free_t *linknode_free;
};

void linklist_init(struct linklist *list, linknode_find_t *linknode_find,
                   linknode_free_t *linknode_free);
void linklist_insert_next(struct linklist *list, struct linknode *pos,
                          struct linknode *node);
void linklist_remove(struct linklist *list, struct linknode *node);
struct linknode *linklist_find(struct linklist *list, const void *key);
void linklist_destroy(struct linklist *list);

#endif /* linklist.h */
