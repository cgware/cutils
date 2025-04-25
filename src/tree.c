#include "tree.h"

#include "log.h"
#include "mem.h"

typedef struct header_s {
	tnode_t child;
} header_t;

static inline header_t *get_node(const tree_t *tree, tnode_t node)
{
	return list_get_data(tree, node);
}

static inline tnode_t init_node(tree_t *tree, tnode_t node)
{
	header_t *data = get_node(tree, node);
	if (data == NULL) {
		return TREE_END;
	}

	data->child = TREE_END;
	return node;
}

tree_t *tree_init(tree_t *tree, uint cap, size_t size, alloc_t alloc)
{
	return list_init(tree, cap, sizeof(header_t) + size, alloc);
}

void tree_free(tree_t *tree)
{
	list_free(tree);
}

tnode_t tree_add(tree_t *tree)
{
	return init_node(tree, list_add(tree));
}

int tree_remove(tree_t *tree, tnode_t node)
{
	if (tree == NULL) {
		return 1;
	}

	for (uint i = 0; i < tree->cnt; i++) {
		header_t *data = get_node(tree, i);
		if (data->child == node) {
			data->child = list_get_next(tree, node);
		}
	}

	return list_remove(tree, node);
}

tnode_t tree_add_child(tree_t *tree, tnode_t node)
{
	if (get_node(tree, node) == NULL) {
		return TREE_END;
	}

	return tree_set_child(tree, node, tree_add(tree));
}

tnode_t tree_set_child(tree_t *tree, tnode_t node, tnode_t child)
{
	header_t *header = get_node(tree, node);
	if (header == NULL) {
		return TREE_END;
	}

	return list_set_next_node(tree, header->child, child);
}

tnode_t tree_get_child(const tree_t *tree, tnode_t node)
{
	header_t *header = get_node(tree, node);
	return header == NULL ? TREE_END : header->child;
}

int tree_has_child(const tree_t *tree, tnode_t node)
{
	header_t *header = get_node(tree, node);
	return header != NULL && header->child < tree->cnt;
}

tnode_t tree_add_next(tree_t *tree, tnode_t node)
{
	if (get_node(tree, node) == NULL) {
		return TREE_END;
	}

	return list_set_next(tree, node, tree_add(tree));
}

tnode_t tree_set_next(tree_t *tree, tnode_t node, tnode_t next)
{
	return list_set_next(tree, node, next);
}

tnode_t tree_get_next(const tree_t *tree, tnode_t node)
{
	return list_get_next(tree, node);
}

void tree_set_cnt(tree_t *tree, uint cnt)
{
	if (tree == NULL) {
		return;
	}

	list_set_cnt(tree, cnt);

	tnode_t node;
	tree_foreach_all(tree, node)
	{
		header_t *header = get_node(tree, node);
		if (header->child >= cnt) {
			header->child = TREE_END;
		}
	}
}

void *tree_get_data(const tree_t *tree, tnode_t node)
{
	header_t *header = get_node(tree, node);
	return header == NULL ? NULL : header + 1;
}

static int node_iterate_pre(const tree_t *tree, tnode_t node, tree_iterate_cb cb, int ret, void *priv, int depth, int last)
{
	if (tree == NULL || node >= tree->cnt) {
		return ret;
	}

	if (cb) {
		ret = cb(tree, node, tree_get_data(tree, node), ret, depth, last, priv);
	}

	tnode_t child = tree_get_child(tree, node);
	tnode_t next;

	while (child < tree->cnt) {
		next  = tree_get_next(tree, child);
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
	tnode_t child = tree_get_child(tree, node);
	tnode_t next;

	while (child < tree->cnt) {
		next  = tree_get_next(tree, child);
		ret   = cb(tree, child, tree_get_data(tree, child), ret, next >= tree->cnt, priv);
		child = next;
	}

	return ret;
}

int tree_print(const tree_t *tree, tnode_t node, tree_print_cb cb, print_dst_t dst, const void *priv)
{
	if (tree == NULL || cb == NULL) {
		return 0;
	}

	int off = dst.off;
	tnode_t cur;
	int depth;
	tree_foreach(tree, node, cur, depth)
	{
		for (int i = 0; i < depth - 1; i++) {
			dst.off += c_dprintf(dst, tree_get_next(tree, _it.stack[i + 1]) < tree->cnt ? "│ " : "  ");
		}

		if (depth > 0) {
			dst.off += c_dprintf(dst, tree_get_next(tree, _it.stack[depth]) < tree->cnt ? "├─" : "└─");
		}

		dst.off += cb(tree_get_data(tree, cur), dst, priv);
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

void tree_it_next(tree_it *it)
{
	if (it == NULL || it->tree == NULL) {
		return;
	}

	const tnode_t node  = it->stack[it->top - 1];
	const tnode_t child = tree_get_child(it->tree, node);

	if (child < it->tree->cnt) {
		if (it->top >= TREE_MAX_DEPTH) {
			log_error("cutils", "tree", NULL, "exceeded max depth of %d", TREE_MAX_DEPTH);
			it->stack[it->top - 1] = TREE_END;
			return;
		}
		it->stack[it->top++] = child;
	} else {
		do {
			it->stack[it->top - 1] = tree_get_next(it->tree, it->stack[it->top - 1]);
		} while (it->stack[it->top - 1] >= it->tree->cnt && --it->top > 0);
	}
}
