/* MIT License Copyright (c) 2021, h1zzz */

#include "linklist.h"

#include <stddef.h>

void linklist_init(struct linklist *list, linknode_find_t *linknode_find,
                   linknode_free_t *linknode_free)
{
    list->linknode_find = linknode_find;
    list->linknode_free = linknode_free;
    list->head = NULL;
    list->tail = NULL;
}

void linklist_insert_next(struct linklist *list, struct linknode *pos,
                          struct linknode *node)
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

void linklist_remove(struct linklist *list, struct linknode *node)
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

    if (list->linknode_free)
        list->linknode_free(node);
}

struct linknode *linklist_find(struct linklist *list, const void *key)
{
    struct linknode *ptr = NULL;

    for (ptr = list->head; ptr; ptr = ptr->next) {
        if (list->linknode_find && list->linknode_find(ptr, key))
            break;
    }
    return ptr;
}

void linklist_destroy(struct linklist *list)
{
    while (list->head)
        linklist_remove(list, list->head);
}
