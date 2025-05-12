#ifndef TREE_H
#define TREE_H

#include "list.h"

#define TREE_MAX_DEPTH 128

typedef list_node_t tnode_t;
typedef list_t tree_t;

tree_t *tree_init(tree_t *tree, uint cap, size_t size, alloc_t alloc);
void tree_free(tree_t *tree);

void *tree_add(tree_t *tree, tnode_t *node);
int tree_remove(tree_t *tree, tnode_t node);

void *tree_add_child(tree_t *tree, tnode_t node, tnode_t *child);
int tree_set_child(tree_t *tree, tnode_t node, tnode_t child);
void *tree_get_child(const tree_t *tree, tnode_t node, tnode_t *child);
int tree_has_child(const tree_t *tree, tnode_t node);

void *tree_add_next(tree_t *tree, tnode_t node, tnode_t *next);
int tree_set_next(tree_t *tree, tnode_t node, tnode_t next);
void *tree_get_next(const tree_t *tree, tnode_t node, tnode_t *next);

void tree_set_cnt(tree_t *tree, uint cnt);

void *tree_get(const tree_t *tree, tnode_t node);

typedef int (*tree_iterate_cb)(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv);
int tree_iterate_pre(const tree_t *tree, tnode_t node, tree_iterate_cb cb, int ret, void *priv);

typedef int (*tree_iterate_childs_cb)(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv);
int tree_iterate_childs(const tree_t *tree, tnode_t node, tree_iterate_childs_cb cb, int ret, void *priv);

typedef size_t (*tree_print_cb)(void *data, dst_t dst, const void *priv);
size_t tree_print(const tree_t *tree, tnode_t node, tree_print_cb cb, dst_t dst, const void *priv);

typedef struct tree_it {
	const tree_t *tree;
	tnode_t stack[TREE_MAX_DEPTH];
	int top;
} tree_it;

tree_it tree_it_begin(const tree_t *tree, tnode_t node);
void *tree_it_next(tree_it *it);

#define tree_foreach(_tree, _start, _node, _depth)                                                                                         \
	for (tree_it _it = tree_it_begin(_tree, _start);                                                                                   \
	     ((_depth = _it.top - 1) >= 0) && ((_node = _it.stack[_it.top - 1]) < (_tree)->cnt);                                           \
	     tree_it_next(&_it))

#define tree_foreach_all(_tree, _node) for (; _node < (_tree)->cnt; _node++)

#define tree_foreach_child(_tree, _parent, _node, _data)                                                                                   \
	for ((_data) = tree_get_child(_tree, _parent, &(_node)); (_data); (_data) = tree_get_next(_tree, _node, &(_node)))

#endif
