/* MIT License Copyright (c) 2021, h1zzz */

#include "llist.h"

#include <stddef.h>

void llist_init(llist_t *list, lnode_find_t *lnode_find,
                lnode_free_t *lnode_free)
{
    list->lnode_find = lnode_find;
    list->lnode_free = lnode_free;
    list->head = NULL;
    list->tail = NULL;
}

void llist_insert_next(llist_t *list, struct lnode *pos, struct lnode *node)
{
    node->list = list;
    node->prev = pos;

    if (list->head) {
        node->next = pos ? pos->next : list->head;
        if (pos) {
            if (pos->next)
                pos->next->prev = node;
            else
                list->tail = node;
            pos->next = node;
        } else {
            /* Insert the node into the head of the linked list */
            list->head->prev = node;
            list->head = node;
        }
    } else {
        node->next = NULL;
        list->head = node;
        list->tail = node;
    }
}

void llist_remove(llist_t *list, struct lnode *node)
{
    if (node == list->head) {
        list->head = node->next;
        if (list->head)
            list->head->prev = NULL;
        else
            list->tail = NULL;
    } else {
        if (node->prev)
            node->prev->next = node->next;
        if (node->next)
            node->next->prev = node->prev;
        else
            list->tail = node->prev;
    }

    if (list->lnode_free)
        list->lnode_free(node);
}

struct lnode *llist_find(llist_t *list, const void *key)
{
    struct lnode *ptr = NULL;

    for (ptr = list->head; ptr; ptr = ptr->next) {
        if (list->lnode_find && list->lnode_find(ptr, key))
            break;
    }
    return ptr;
}

void llist_destroy(llist_t *list)
{
    while (list->head)
        llist_remove(list, list->head);
}
