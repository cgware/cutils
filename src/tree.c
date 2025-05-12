#include "tree.h"

#include "log.h"
#include "mem.h"

typedef struct header_s {
	tree_node_t child;
} header_t;

tree_t *tree_init(tree_t *tree, uint cap, size_t size, alloc_t alloc)
{
	return list_init(tree, cap, sizeof(header_t) + size, alloc);
}

void tree_free(tree_t *tree)
{
	list_free(tree);
}

void tree_reset(tree_t *tree, uint cnt)
{
	if (tree == NULL) {
		return;
	}

	if (cnt > tree->cnt) {
		cnt = tree->cnt;
	}

	list_reset(tree, cnt);

	tree_node_t node = 0;
	tree_foreach_all(tree, node)
	{
		header_t *header = list_get(tree, node);
		if (header->child >= cnt) {
			header->child = (tree_node_t)-1;
		}
	}
}

void *tree_node(tree_t *tree, tree_node_t *node)
{
	if (tree == NULL) {
		return NULL;
	}

	header_t *header = list_node(tree, node);
	if (header == NULL) {
		log_error("cutils", "tree", NULL, "failed to create node");
		return NULL;
	}

	header->child = (tree_node_t)-1;

	return header + 1;
}

int tree_add(tree_t *tree, tree_node_t node, tree_node_t child)
{
	if (tree == NULL) {
		return 1;
	}

	header_t *header = list_get(tree, node);
	if (header == NULL) {
		log_error("cutils", "tree", NULL, "invalid node: %d", node);
		return 1;
	}

	if (list_get(tree, child) == NULL) {
		log_error("cutils", "tree", NULL, "invalid node: %d", child);
		return 1;
	}

	if (header->child == (tree_node_t)-1) {
		header->child = child;
		return 0;
	}

	return list_app(tree, header->child, child);
}

int tree_app(tree_t *tree, tree_node_t node, tree_node_t next)
{
	if (tree == NULL) {
		return 1;
	}

	if (list_app(tree, node, next)) {
		log_error("cutils", "tree", NULL, "failed to append");
		return 1;
	}

	return 0;
}

int tree_remove(tree_t *tree, tree_node_t node)
{
	if (tree == NULL) {
		return 1;
	}

	for (uint i = 0; i < tree->cnt; i++) {
		header_t *data = list_get(tree, i);
		if (data->child == node) {
			list_get_next(tree, node, &data->child);
		}
	}

	return list_remove(tree, node);
}

void *tree_get(const tree_t *tree, tree_node_t node)
{
	if (tree == NULL) {
		return NULL;
	}

	header_t *header = list_get(tree, node);
	if (header == NULL) {
		log_error("cutils", "tree", NULL, "invalid node: %d", node);
		return NULL;
	}

	return header == NULL ? NULL : header + 1;
}

void *tree_get_child(const tree_t *tree, tree_node_t node, tree_node_t *child)
{
	if (tree == NULL) {
		return NULL;
	}

	header_t *header = list_get(tree, node);
	if (header == NULL) {
		log_error("cutils", "tree", NULL, "invalid node: %d", node);
		return NULL;
	}

	if (child) {
		*child = header->child;
	}

	if (header->child == (tree_node_t)-1) {
		return NULL;
	}

	return tree_get(tree, header->child);
}

void *tree_get_next(const tree_t *tree, tree_node_t node, tree_node_t *next)
{
	header_t *header = list_get_next(tree, node, next);
	return header == NULL ? NULL : header + 1;
}

static int node_iterate_pre(const tree_t *tree, tree_node_t node, tree_iterate_cb cb, int ret, void *priv, int depth, int last)
{
	void *data = tree_get(tree, node);
	if (data == NULL) {
		return ret;
	}

	if (cb) {
		ret = cb(tree, node, data, ret, depth, last, priv);
	}

	tree_node_t child;
	if (tree_get_child(tree, node, &child) == NULL) {
		return ret;
	}

	tree_node_t next;
	while (child < tree->cnt) {
		tree_get_next(tree, child, &next);
		ret   = node_iterate_pre(tree, child, cb, ret, priv, depth + 1, last | ((next >= tree->cnt) << depth));
		child = next;
	}

	return ret;
}

int tree_iterate_pre(const tree_t *tree, tree_node_t node, tree_iterate_cb cb, int ret, void *priv)
{
	return node_iterate_pre(tree, node, cb, ret, priv, 0, 0);
}

int tree_iterate_childs(const tree_t *tree, tree_node_t node, tree_iterate_childs_cb cb, int ret, void *priv)
{
	tree_node_t child;
	if (tree_get_child(tree, node, &child) == NULL) {
		return ret;
	}

	tree_node_t next;
	while (child < tree->cnt) {
		tree_get_next(tree, child, &next);
		ret   = cb(tree, child, tree_get(tree, child), ret, next >= tree->cnt, priv);
		child = next;
	}

	return ret;
}

size_t tree_print(const tree_t *tree, tree_node_t node, tree_print_cb cb, dst_t dst, const void *priv)
{
	if (tree == NULL || cb == NULL) {
		return 0;
	}

	size_t off = dst.off;
	tree_node_t cur;
	int depth;
	tree_foreach(tree, node, cur, depth)
	{
		tree_node_t next;
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

tree_it tree_it_begin(const tree_t *tree, tree_node_t node)
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

	const tree_node_t node = it->stack[it->top - 1];
	tree_node_t child;
	void *data;
	if ((data = tree_get_child(it->tree, node, &child))) {
		if (it->top >= TREE_MAX_DEPTH) {
			log_error("cutils", "tree", NULL, "exceeded max depth of %d", TREE_MAX_DEPTH);
			it->stack[it->top - 1] = (tree_node_t)-1;
			return NULL;
		}
		it->stack[it->top++] = child;
	} else {
		while ((data = tree_get_next(it->tree, it->stack[it->top - 1], &it->stack[it->top - 1])) == NULL && --it->top > 0) {
		}
	}

	return data;
}
