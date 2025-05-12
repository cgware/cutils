#ifndef LIST_H
#define LIST_H

#include "arr.h"

typedef uint list_node_t;
typedef arr_t list_t;

list_t *list_init(list_t *list, uint cap, size_t size, alloc_t alloc);
void list_free(list_t *list);

void list_reset(list_t *list, uint cnt);

void *list_node(list_t *list, list_node_t *node);
int list_app(list_t *list, list_node_t node, list_node_t next);
int list_remove(list_t *list, list_node_t node);

void *list_get(const list_t *list, list_node_t node);
void *list_get_next(const list_t *list, list_node_t node, list_node_t *next);
void *list_get_at(const list_t *list, list_node_t start, uint index, list_node_t *node);

typedef size_t (*list_print_cb)(void *value, dst_t dst, const void *priv);
size_t list_print(const list_t *list, list_node_t node, list_print_cb cb, dst_t dst, const void *priv);

#define list_foreach(_list, _i, _val)	  for (; _i < (_list)->cnt && (_val = list_get(_list, _i)); list_get_next(_list, _i, &(_i)))
#define list_foreach_all(_list, _i, _val) for (; _i < (_list)->cnt && (_val = list_get(_list, _i)); _i++)

#endif
