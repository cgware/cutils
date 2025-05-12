#include "tree.h"

#include "log.h"
#include "mem.h"

typedef struct header_s {
	tnode_t child;
} header_t;

static inline header_t *get_node(const tree_t *tree, tnode_t node)
{
	return list_get(tree, node);
}

tree_t *tree_init(tree_t *tree, uint cap, size_t size, alloc_t alloc)
{
	return list_init(tree, cap, sizeof(header_t) + size, alloc);
}

void tree_free(tree_t *tree)
{
	list_free(tree);
}

void *tree_add(tree_t *tree, tnode_t *node)
{
	header_t *header = list_node(tree, node);
	if (header == NULL) {
		return NULL;
	}

	header->child = (tnode_t)-1;

	return header + 1;
}

int tree_remove(tree_t *tree, tnode_t node)
{
	if (tree == NULL) {
		return 1;
	}

	for (uint i = 0; i < tree->cnt; i++) {
		header_t *data = get_node(tree, i);
		if (data->child == node) {
			list_get_next(tree, node, &data->child);
		}
	}

	return list_remove(tree, node);
}

void *tree_add_child(tree_t *tree, tnode_t node, tnode_t *child)
{
	if (get_node(tree, node) == NULL) {
		return NULL;
	}

	tnode_t t;
	void *data = tree_add(tree, &t);
	if (data == NULL) {
		return NULL;
	}

	tree_set_child(tree, node, t);

	if (child) {
		*child = t;
	}

	return data;
}

int tree_set_child(tree_t *tree, tnode_t node, tnode_t child)
{
	header_t *header = get_node(tree, node);
	if (header == NULL || tree_get(tree, child) == NULL) {
		return 1;
	}

	if (header->child == (tnode_t)-1) {
		header->child = child;
		return 0;
	}

	return list_app(tree, header->child, child);
}

void *tree_get_child(const tree_t *tree, tnode_t node, tnode_t *child)
{
	header_t *header = get_node(tree, node);
	if (header == NULL) {
		return NULL;
	}

	if (child) {
		*child = header->child;
	}

	if (header->child == (tnode_t)-1) {
		return NULL;
	}

	return tree_get(tree, header->child);
}

int tree_has_child(const tree_t *tree, tnode_t node)
{
	header_t *header = get_node(tree, node);
	return header != NULL && header->child < tree->cnt;
}

void *tree_add_next(tree_t *tree, tnode_t node, tnode_t *next)
{
	if (get_node(tree, node) == NULL) {
		return NULL;
	}

	tnode_t t;
	void *data = tree_add(tree, &t);
	if (data == NULL) {
		return NULL;
	}

	tree_set_next(tree, node, t);

	if (next) {
		*next = t;
	}

	return data;
}

int tree_set_next(tree_t *tree, tnode_t node, tnode_t next)
{
	return list_app(tree, node, next);
}

void *tree_get_next(const tree_t *tree, tnode_t node, tnode_t *next)
{
	header_t *header = list_get_next(tree, node, next);
	return header == NULL ? NULL : header + 1;
}

void tree_set_cnt(tree_t *tree, uint cnt)
{
	if (tree == NULL) {
		return;
	}

	list_reset(tree, cnt);

	tnode_t node = 0;
	tree_foreach_all(tree, node)
	{
		header_t *header = get_node(tree, node);
		if (header->child >= cnt) {
			header->child = (tnode_t)-1;
		}
	}
}

void *tree_get(const tree_t *tree, tnode_t node)
{
	header_t *header = get_node(tree, node);
	return header == NULL ? NULL : header + 1;
}

static int node_iterate_pre(const tree_t *tree, tnode_t node, tree_iterate_cb cb, int ret, void *priv, int depth, int last)
{
	void *data = tree_get(tree, node);
	if (data == NULL) {
		return ret;
	}

	if (cb) {
		ret = cb(tree, node, data, ret, depth, last, priv);
	}

	tnode_t child;
	if (tree_get_child(tree, node, &child) == NULL) {
		return ret;
	}

	tnode_t next;
	while (child < tree->cnt) {
		tree_get_next(tree, child, &next);
		ret   = node_iterate_pre(tree, child, cb, ret, priv, depth + 1, last | ((next >= tree->cnt) << depth));
		child = next;
	}

	return ret;
}

int tree_iterate_pre(const tree_t *tree, tnode_t node, tree_iterate_cb cb, int ret, void *priv)
{
	return node_iterate_pre(tree, node, cb, ret, priv, 0, 0);
}

int tree_iterate_childs(const tree_t *tree, tnode_t node, tree_iterate_childs_cb cb, int ret, void *priv)
{
	tnode_t child;
	if (tree_get_child(tree, node, &child) == NULL) {
		return ret;
	}

	tnode_t next;
	while (child < tree->cnt) {
		tree_get_next(tree, child, &next);
		ret   = cb(tree, child, tree_get(tree, child), ret, next >= tree->cnt, priv);
		child = next;
	}

	return ret;
}

size_t tree_print(const tree_t *tree, tnode_t node, tree_print_cb cb, dst_t dst, const void *priv)
{
	if (tree == NULL || cb == NULL) {
		return 0;
	}

	size_t off = dst.off;
	tnode_t cur;
	int depth;
	tree_foreach(tree, node, cur, depth)
	{
		tnode_t next;
		for (int i = 0; i < depth - 1; i++) {
			tree_get_next(tree, _it.stack[i + 1], &next);
			dst.off += dputs(dst, next < tree->cnt ? STRV("│ ") : STRV("  "));
		}

		if (depth > 0) {
			tree_get_next(tree, _it.stack[depth], &next);
			dst.off += dputs(dst, next < tree->cnt ? STRV("├─") : STRV("└─"));
		}

		dst.off += cb(tree_get(tree, cur), dst, priv);
	}

	return dst.off - off;
}

tree_it tree_it_begin(const tree_t *tree, tnode_t node)
{
	if (tree == NULL) {
		return (tree_it){0};
	}

	tree_it it  = {0};
	it.tree	    = tree;
	it.stack[0] = node;
	it.top	    = 1;

	return it;
}

void *tree_it_next(tree_it *it)
{
	if (it == NULL || it->tree == NULL) {
		return NULL;
	}

	const tnode_t node = it->stack[it->top - 1];
	tnode_t child;
	void *data;
	if ((data = tree_get_child(it->tree, node, &child))) {
		if (it->top >= TREE_MAX_DEPTH) {
			log_error("cutils", "tree", NULL, "exceeded max depth of %d", TREE_MAX_DEPTH);
			it->stack[it->top - 1] = (tnode_t)-1;
			return NULL;
		}
		it->stack[it->top++] = child;
	} else {
		while ((data = tree_get_next(it->tree, it->stack[it->top - 1], &it->stack[it->top - 1])) == NULL && --it->top > 0) {
		}
	}

	return data;
}
