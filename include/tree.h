#ifndef TREE_H
#define TREE_H

#include "list.h"

#define TREE_END       LIST_END
#define TREE_MAX_DEPTH 128

typedef lnode_t tnode_t;
typedef list_t tree_t;

tree_t *tree_init(tree_t *tree, uint cap, size_t size, alloc_t alloc);
void tree_free(tree_t *tree);

tnode_t tree_add(tree_t *tree);
int tree_remove(tree_t *tree, tnode_t node);

tnode_t tree_add_child(tree_t *tree, tnode_t node);
tnode_t tree_set_child(tree_t *tree, tnode_t node, tnode_t child);
tnode_t tree_get_child(const tree_t *tree, tnode_t node);
bool tree_has_child(const tree_t *tree, tnode_t node);

tnode_t tree_add_next(tree_t *tree, tnode_t node);
tnode_t tree_set_next(tree_t *tree, tnode_t node, tnode_t next);
tnode_t tree_get_next(const tree_t *tree, tnode_t node);

void tree_set_cnt(tree_t *tree, uint cnt);

void *tree_get_data(const tree_t *tree, tnode_t node);

typedef int (*tree_iterate_cb)(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv);
int tree_iterate_pre(const tree_t *tree, tnode_t node, tree_iterate_cb cb, int ret, void *priv);

typedef int (*tree_iterate_childs_cb)(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv);
int tree_iterate_childs(const tree_t *tree, tnode_t node, tree_iterate_childs_cb cb, int ret, void *priv);

typedef int (*tree_print_cb)(void *data, print_dst_t dst, const void *priv);
int tree_print(const tree_t *tree, tnode_t node, tree_print_cb cb, print_dst_t dst, const void *priv);

typedef struct tree_it {
	const tree_t *tree;
	tnode_t stack[TREE_MAX_DEPTH];
	int top;
} tree_it;

tree_it tree_it_begin(const tree_t *tree, tnode_t node);
void tree_it_next(tree_it *it);

#define tree_foreach(_tree, _start, _node, _depth)                                                                                         \
	for (tree_it _it = tree_it_begin(_tree, _start);                                                                                   \
	     ((_depth = _it.top - 1) >= 0) && ((_node = _it.stack[_it.top - 1]) < (_tree)->cnt);                                           \
	     tree_it_next(&_it))

#define tree_foreach_all(_tree, _node) for (_node = 0; _node < (_tree)->cnt; _node++)

#define tree_foreach_child(_tree, _parent, _node)                                                                                          \
	for (_node = tree_get_child(_tree, _parent); _node < (_tree)->cnt; _node = tree_get_next(_tree, _node))

#define tree_add_child_node(_tree, _start, _node)                                                                                          \
	if (_start == TREE_END) {                                                                                                          \
		_node  = tree_add(_tree);                                                                                                  \
		_start = _node;                                                                                                            \
	} else {                                                                                                                           \
		_node = tree_add_child(_tree, _start);                                                                                     \
	}

#define tree_set_child_node(_tree, _node, _child) _node >= (_tree)->cnt ? _node = _child : tree_set_child(_tree, _node, _child);

#endif
