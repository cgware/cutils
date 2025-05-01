#ifndef LIST_H
#define LIST_H

#include "arr.h"

typedef uint lnode_t;
typedef arr_t list_t;

list_t *list_init(list_t *list, uint cap, size_t size, alloc_t alloc);
void list_free(list_t *list);

void *list_add(list_t *list, lnode_t *node);
int list_remove(list_t *list, lnode_t node);

void *list_add_next(list_t *list, lnode_t node, lnode_t *next);
int list_set_next(list_t *list, lnode_t node, lnode_t next);
void *list_get_next(const list_t *list, lnode_t node, lnode_t *next);
void *list_get_at(const list_t *list, lnode_t start, uint index, lnode_t *node);

void list_set_cnt(list_t *list, uint cnt);

void *list_get(const list_t *list, lnode_t node);

typedef size_t (*list_print_cb)(void *value, dst_t dst, const void *priv);
size_t list_print(const list_t *list, lnode_t node, list_print_cb cb, dst_t dst, const void *priv);

#define list_foreach(_list, _i, _val) for (; _i < (_list)->cnt && (_val = list_get(_list, _i)); list_get_next(_list, _i, &(_i)))

#define list_foreach_all(_list, _i, _val) for (; _i < (_list)->cnt && (_val = list_get(_list, _i)); _i++)

#endif
