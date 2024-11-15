#include "tree.h"

#include "log.h"
#include "test.h"

TEST(tree_init_free)
{
	START;

	tree_t tree = {0};

	EXPECT_EQ(tree_init(NULL, 0, sizeof(int), ALLOC_STD), NULL);
	EXPECT_EQ(tree_init(&tree, 1, sizeof(int), ALLOC_STD), &tree);

	EXPECT_NE(tree.data, NULL);
	EXPECT_EQ(tree.cap, 1);
	EXPECT_EQ(tree.cnt, 0);
	EXPECT_NE(tree.size, 0);

	tree_free(&tree);
	tree_free(NULL);

	EXPECT_EQ(tree.data, NULL);
	EXPECT_EQ(tree.cap, 0);
	EXPECT_EQ(tree.cnt, 0);
	EXPECT_EQ(tree.size, 0);

	END;
}

TEST(tree_add)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_add(NULL), TREE_END);
	EXPECT_EQ(tree_add(&tree), 0);

	EXPECT_EQ(tree.cnt, 1);
	EXPECT_EQ(tree.cap, 1);

	tree_free(&tree);

	END;
}

TEST(tree_add_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_add_child(NULL, TREE_END), TREE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_add_child(&tree, TREE_END), TREE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(tree_add_child(&tree, tree_add(&tree)), 1);

	EXPECT_EQ(tree.cnt, 2);
	EXPECT_EQ(tree.cap, 2);

	tree_free(&tree);

	END;
}

TEST(tree_add_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(tree.cnt, 3);
	EXPECT_EQ(tree.cap, 4);

	tree_free(&tree);

	END;
}

TEST(tree_add_child_realloc)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	*(int *)tree_get_data(&tree, tree_add(&tree)) = 1;

	tnode_t child;

	*(int *)tree_get_data(&tree, (child = tree_add_child(&tree, 0))) = 2;

	EXPECT_EQ(child, 1);
	EXPECT_EQ(tree.cnt, 2);
	EXPECT_EQ(tree.cap, 2);
	EXPECT_EQ(*(int *)tree_get_data(&tree, 0), 1);
	EXPECT_EQ(*(int *)tree_get_data(&tree, child), 2);

	tree_free(&tree);

	END;
}

TEST(tree_add_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_add_next(NULL, TREE_END), TREE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_add_next(&tree, TREE_END), TREE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(tree_add_next(&tree, tree_add(&tree)), 1);

	EXPECT_EQ(tree.cnt, 2);
	EXPECT_EQ(tree.cap, 2);

	tree_free(&tree);

	END;
}

TEST(tree_add_nexts)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_next(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_next(&tree, 0);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(tree.cnt, 3);
	EXPECT_EQ(tree.cap, 4);

	tree_free(&tree);

	END;
}

TEST(tree_add_next_realloc)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	*(int *)tree_get_data(&tree, tree_add(&tree)) = 1;

	tnode_t next;

	*(int *)tree_get_data(&tree, (next = tree_add_next(&tree, 0))) = 2;

	EXPECT_EQ(next, 1);
	EXPECT_EQ(tree.cnt, 2);
	EXPECT_EQ(tree.cap, 2);
	EXPECT_EQ(*(int *)tree_get_data(&tree, 0), 1);
	EXPECT_EQ(*(int *)tree_get_data(&tree, next), 2);

	tree_free(&tree);

	END;
}

TEST(tree_add_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t child  = tree_add_child(&tree, tree_add(&tree));
	const tnode_t gchild = tree_add_child(&tree, child);

	EXPECT_EQ(child, 1);
	EXPECT_EQ(gchild, 2);
	EXPECT_EQ(tree.cnt, 3);
	EXPECT_EQ(tree.cap, 4);

	tree_free(&tree);

	END;
}

TEST(tree_add_multiple_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1  = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2  = tree_add_child(&tree, 0);
	const tnode_t n12 = tree_add_child(&tree, n1);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(n12, 3);
	EXPECT_EQ(tree.cnt, 4);
	EXPECT_EQ(tree.cap, 4);

	tree_free(&tree);

	END;
}

TEST(tree_adds)
{
	SSTART;
	RUN(tree_add);
	RUN(tree_add_child);
	RUN(tree_add_childs);
	RUN(tree_add_child_realloc);
	RUN(tree_add_next);
	RUN(tree_add_nexts);
	RUN(tree_add_next_realloc);
	RUN(tree_add_grand_child);
	RUN(tree_add_multiple_childs);
	SEND;
}

TEST(tree_set_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t node = tree_add(&tree);

	EXPECT_EQ(tree_set_child(NULL, TREE_END, TREE_END), TREE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_set_child(&tree, TREE_END, TREE_END), TREE_END);
	EXPECT_EQ(tree_set_child(&tree, TREE_END, node), TREE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(tree_set_child(&tree, node, node), 0);

	EXPECT_EQ(tree.cnt, 1);
	EXPECT_EQ(tree.cap, 1);

	tree_free(&tree);

	END;
}

TEST(tree_set_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t node = tree_add(&tree);

	EXPECT_EQ(tree_set_next(NULL, TREE_END, TREE_END), TREE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_set_next(&tree, TREE_END, TREE_END), TREE_END);
	EXPECT_EQ(tree_set_next(&tree, TREE_END, node), TREE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(tree_set_next(&tree, node, node), 0);

	EXPECT_EQ(tree.cnt, 1);
	EXPECT_EQ(tree.cap, 1);

	tree_free(&tree);

	END;
}

TEST(tree_set)
{
	SSTART;
	RUN(tree_set_child);
	RUN(tree_set_next);
	SEND;
}

TEST(tree_set_cnt)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(tnode_t), ALLOC_STD);

	tnode_t root  = tree_add(&tree);
	tnode_t *data = tree_get_data(&tree, root);

	*data = 10;

	tree_add_child(&tree, root);

	tree_set_cnt(NULL, 0);
	tree_set_cnt(&tree, 1);

	tnode_t *child = data - 1;

	EXPECT_EQ(*child, TREE_END);
	EXPECT_EQ(*data, 10);

	tree_free(&tree);

	END;
}

TEST(tree_get_data)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_get_data(NULL, TREE_END), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_get_data(&tree, TREE_END), NULL);
	log_set_quiet(0, 0);

	*(int *)tree_get_data(&tree, tree_add(&tree)) = 1;
	EXPECT_EQ(*(int *)tree_get_data(&tree, 0), 1);

	tree_free(&tree);

	END;
}

TEST(tree_has_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_has_child(NULL, TREE_END), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_has_child(&tree, TREE_END), 0);
	log_set_quiet(0, 0);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	tree_add_child(&tree, 0);
	tree_add_child(&tree, n1);

	EXPECT_EQ(tree_has_child(&tree, 0), 1);

	tree_free(&tree);

	END;
}

TEST(tree_get_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_get_child(NULL, TREE_END), TREE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_get_child(&tree, TREE_END), TREE_END);
	log_set_quiet(0, 0);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	tree_add_child(&tree, 0);
	tree_add_child(&tree, n1);

	EXPECT_EQ(tree_get_child(&tree, 0), 1);

	tree_free(&tree);

	END;
}

TEST(tree_get_child_data)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	*(int *)tree_get_data(&tree, tree_add(&tree))	       = 1;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, 0)) = 2;

	EXPECT_EQ(*(int *)tree_get_data(&tree, 0), 1);
	EXPECT_EQ(*(int *)tree_get_data(&tree, 1), 2);

	tree_free(&tree);

	END;
}

TEST(tree_get_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_next(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_next(&tree, 0);

	const tnode_t next1 = tree_get_next(&tree, 0);
	const tnode_t next2 = tree_get_next(&tree, next1);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(next1, 1);
	EXPECT_EQ(next2, 2);

	tree_free(&tree);

	END;
}

TEST(tree_get)
{
	SSTART;
	RUN(tree_get_data);
	RUN(tree_has_child);
	RUN(tree_get_child);
	RUN(tree_get_child_data);
	RUN(tree_get_next);
	SEND;
}

TEST(tree_remove)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_remove(NULL, TREE_END), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_remove(&tree, TREE_END), 1);
	log_set_quiet(0, 0);

	tree_free(&tree);

	END;
}

TEST(tree_remove_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);
	const tnode_t n3 = tree_add_child(&tree, 0);

	EXPECT_EQ(tree_remove(&tree, n2), 0);

	EXPECT_EQ(tree_get_next(&tree, n1), n3);

	tree_free(&tree);

	END;
}

TEST(tree_remove_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);
	tree_add_child(&tree, 0);

	EXPECT_EQ(tree_remove(&tree, n1), 0);

	EXPECT_EQ(tree_get_child(&tree, 0), n2);

	tree_free(&tree);

	END;
}

TEST(tree_removes)
{
	SSTART;
	RUN(tree_remove);
	RUN(tree_remove_next);
	RUN(tree_remove_child);
	SEND;
}

static int iterate_pre_root_cb(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	EXPECT_EQ(node, 0);
	EXPECT_EQ(depth, 0);
	EXPECT_EQ(last, 0);

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_pre_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_add(&tree);

	int cnt = 0;

	EXPECT_EQ(tree_iterate_pre(NULL, TREE_END, NULL, 0, &cnt), 0);
	EXPECT_EQ(tree_iterate_pre(&tree, TREE_END, NULL, 0, &cnt), 0);
	EXPECT_EQ(tree_iterate_pre(&tree, 0, NULL, 0, &cnt), 0);
	EXPECT_EQ(tree_iterate_pre(&tree, 0, iterate_pre_root_cb, 0, &cnt), 0);

	EXPECT_EQ(cnt, 1);

	tree_free(&tree);

	END;
}

static int iterate_pre_child_cb(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	switch (node) {
	case 0:
		EXPECT_EQ(depth, 0);
		EXPECT_EQ(last, 0);
		break;
	case 1:
		EXPECT_EQ(depth, 1);
		EXPECT_EQ(last, 1);
		break;
	}

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_pre_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t child = tree_add_child(&tree, tree_add(&tree));

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, 0, iterate_pre_child_cb, 0, &cnt);

	EXPECT_EQ(child, 1);
	EXPECT_EQ(cnt, 2);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int iterate_pre_childs_cb(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	switch (node) {
	case 0:
		EXPECT_EQ(depth, 0);
		EXPECT_EQ(last, 0);
		break;
	case 1:
		EXPECT_EQ(depth, 1);
		EXPECT_EQ(last, 0);
		break;
	case 2:
		EXPECT_EQ(depth, 1);
		EXPECT_EQ(last, 1);
		break;
	}

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_pre_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, 0, iterate_pre_childs_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int iterate_pre_grand_child_cb(const tree_t *tree, tnode_t node, void *value, int ret, int depth, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	switch (node) {
	case 0:
		EXPECT_EQ(depth, 0);
		EXPECT_EQ(last, 0);
		break;
	case 1:
		EXPECT_EQ(depth, 1);
		EXPECT_EQ(last, 1);
		break;
	case 2:
		EXPECT_EQ(depth, 2);
		EXPECT_EQ(last, 3);
		break;
	}

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_pre_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, n1);

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, 0, iterate_pre_grand_child_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	return ret + RES;
}

static int iterate_childs_root_cb(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;
	(void)last;

	EXPECT_EQ(node, 1);

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_childs_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_add_child(&tree, tree_add(&tree));

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, 0, iterate_childs_root_cb, 0, &cnt);

	EXPECT_EQ(cnt, 1);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int iterate_childs_child_cb(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	EXPECT_EQ(node, 1);
	EXPECT_EQ(last, 1);

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_childs_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t child = tree_add_child(&tree, tree_add(&tree));

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, 0, iterate_childs_child_cb, 0, &cnt);

	EXPECT_EQ(child, 1);
	EXPECT_EQ(cnt, 1);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int iterate_childs_childs_cb(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	switch (node) {
	case 1: EXPECT_EQ(last, 0); break;
	case 2: EXPECT_EQ(last, 1); break;
	}

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_childs_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, 0, iterate_childs_childs_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 2);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int iterate_childs_grand_child_cb(const tree_t *tree, tnode_t node, void *value, int ret, int last, void *priv)
{
	int _passed = 1;

	(void)tree;
	(void)value;

	switch (node) {
	case 1: EXPECT_EQ(last, 0); break;
	case 2: EXPECT_EQ(last, 1); break;
	}

	*(int *)priv += 1;

	return ret + !_passed;
}

TEST(tree_iterate_childs_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	const tnode_t n1 = tree_add_child(&tree, tree_add(&tree));
	const tnode_t n2 = tree_add_child(&tree, 0);
	const tnode_t n3 = tree_add_child(&tree, n1);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, 0, iterate_childs_grand_child_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(n3, 3);
	EXPECT_EQ(cnt, 2);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

TEST(tree_iterate)
{
	SSTART;
	RUN(tree_iterate_pre_root);
	RUN(tree_iterate_pre_child);
	RUN(tree_iterate_pre_childs);
	RUN(tree_iterate_pre_grand_child);
	RUN(tree_iterate_childs_root);
	RUN(tree_iterate_childs_child);
	RUN(tree_iterate_childs_childs);
	RUN(tree_iterate_childs_grand_child);
	SEND;
}

TEST(tree_it_begin)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_it_begin(NULL, TREE_END);
	tree_it_begin(&tree, TREE_END);
	tree_it_begin(&tree, tree_add(&tree));

	tree_free(&tree);

	END;
}

TEST(tree_it_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_it it = {0};

	tree_it_next(NULL);
	tree_it_next(&it);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root;
	*(int *)tree_get_data(&tree, root = tree_add(&tree)) = 0;

	tnode_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, TREE_END, node, depth)
	{
	}

	tree_foreach(&tree, 0, node, depth)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		EXPECT_EQ(depth, 0);
		i++;
	}

	EXPECT_EQ(i, 1);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root;
	*(int *)tree_get_data(&tree, root = tree_add(&tree))	  = 0;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, root)) = 1;

	tnode_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, 0, node, depth)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		EXPECT_EQ(depth, i);
		i++;
	}

	EXPECT_EQ(i, 2);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root;
	*(int *)tree_get_data(&tree, root = tree_add(&tree))	  = 0;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, root)) = 1;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, root)) = 2;

	tnode_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, 0, node, depth)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		EXPECT_EQ(depth, i != 0);
		i++;
	}

	EXPECT_EQ(i, 3);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root, n1;
	*(int *)tree_get_data(&tree, root = tree_add(&tree))	       = 0;
	*(int *)tree_get_data(&tree, n1 = tree_add_child(&tree, root)) = 1;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, n1))	       = 2;

	tnode_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, 0, node, depth)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		EXPECT_EQ(depth, i);
		i++;
	}

	EXPECT_EQ(i, 3);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_add(&tree);

	tnode_t node;

	int i = 0;
	tree_foreach_child(&tree, 0, node)
	{
	}

	EXPECT_EQ(i, 0);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	*(int *)tree_get_data(&tree, tree_add_child(&tree, tree_add(&tree))) = 0;

	tnode_t node;

	int i = 0;
	tree_foreach_child(&tree, 0, node)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		i++;
	}

	EXPECT_EQ(i, 1);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	*(int *)tree_get_data(&tree, tree_add_child(&tree, tree_add(&tree))) = 0;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, 0))		     = 1;

	tnode_t node;

	int i = 0;
	tree_foreach_child(&tree, 0, node)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		i++;
	}

	EXPECT_EQ(i, 2);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t n1;
	*(int *)tree_get_data(&tree, n1 = tree_add_child(&tree, tree_add(&tree))) = 0;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, 0))			  = 1;
	*(int *)tree_get_data(&tree, tree_add_child(&tree, n1))			  = 2;

	tnode_t node;

	int i = 0;
	tree_foreach_child(&tree, 0, node)
	{
		const int *value = tree_get_data(&tree, node);
		EXPECT_EQ(*value, i);
		i++;
	}

	EXPECT_EQ(i, 2);

	tree_free(&tree);

	END;
}

TEST(tree_foreach)
{
	SSTART;
	RUN(tree_it_begin);
	RUN(tree_it_next);
	RUN(tree_foreach_root);
	RUN(tree_foreach_child);
	RUN(tree_foreach_childs);
	RUN(tree_foreach_grand_child);
	RUN(tree_foreach_child_root);
	RUN(tree_foreach_child_child);
	RUN(tree_foreach_child_childs);
	RUN(tree_foreach_child_grand_child);
	SEND;
}

static int print_tree(void *data, print_dst_t dst, const void *priv)
{
	(void)priv;
	return c_dprintf(dst, "%d\n", *(int *)data);
}

TEST(tree_print)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root;
	*(int *)tree_get_data(&tree, root = tree_add(&tree)) = 0;

	tnode_t n1, n2, n11, n111;

	*(int *)tree_get_data(&tree, (n1 = tree_add_child(&tree, 0))) = 1;
	*(int *)tree_get_data(&tree, (n2 = tree_add_child(&tree, 0))) = 2;

	*(int *)tree_get_data(&tree, (n11 = tree_add_child(&tree, n1))) = 11;

	*(int *)tree_get_data(&tree, (n111 = tree_add_child(&tree, n11))) = 111;

	char buf[64] = {0};
	EXPECT_EQ(tree_print(NULL, TREE_END, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);
	EXPECT_EQ(tree_print(&tree, TREE_END, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);
	EXPECT_EQ(tree_print(&tree, root, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);

	EXPECT_EQ(tree_print(&tree, root, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);
	EXPECT_EQ(tree_print(&tree, root, print_tree, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 47);
	EXPECT_STR(buf,
		   "0\n"
		   "├─1\n"
		   "│ └─11\n"
		   "│   └─111\n"
		   "└─2\n");

	tree_free(&tree);

	END;
}

static int print_tree_depth(void *data, print_dst_t dst, const void *priv)
{
	(void)data;
	(void)dst;
	(void)priv;
	return 0;
}

TEST(tree_print_depth)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tnode_t root  = tree_add(&tree);
	tnode_t child = root;

	for (int i = 0; i < TREE_MAX_DEPTH; i++) {
		child = tree_add_child(&tree, child);
	}

	char buf[32000] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_print(&tree, root, print_tree_depth, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 16764);
	log_set_quiet(0, 0);

	tree_free(&tree);

	END;
}

STEST(tree)
{
	SSTART;

	RUN(tree_init_free);
	RUN(tree_adds);
	RUN(tree_set_cnt);
	RUN(tree_get);
	RUN(tree_set);
	RUN(tree_removes);
	RUN(tree_iterate);
	RUN(tree_foreach);
	RUN(tree_print);
	RUN(tree_print_depth);

	SEND;
}
