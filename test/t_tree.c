#include "tree.h"

#include "log.h"
#include "mem.h"
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

TEST(tree_reset)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(tree_node_t), ALLOC_STD);

	tree_node_t root, n1;
	tree_node_t *data = tree_node(&tree, &root);

	*data = 10;

	tree_node(&tree, &n1);
	tree_add(&tree, root, n1);

	tree_reset(NULL, 0);
	tree_reset(&tree, 1);

	data = tree_get(&tree, root);

	tree_node_t *child = data - 1;

	EXPECT_EQ(*child, (tree_node_t)-1);
	EXPECT_EQ(*data, 10);
	EXPECT_EQ(tree.cnt, 1);

	tree_reset(&tree, 2);
	EXPECT_EQ(tree.cnt, 1);

	tree_free(&tree);

	END;
}

TEST(tree_node)
{
	START;

	tree_t tree = {0};
	log_set_quiet(0, 1);
	tree_init(&tree, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	tree_node_t node;
	EXPECT_EQ(tree_node(NULL, NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(tree_node(&tree, &node), NULL);
	mem_oom(0);
	EXPECT_NE(tree_node(&tree, &node), NULL);
	EXPECT_EQ(node, 0);

	EXPECT_EQ(tree.cnt, 1);
	EXPECT_EQ(tree.cap, 1);

	tree_free(&tree);

	END;
}

TEST(tree_add)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_add(NULL, tree.cnt, tree.cnt), 1);

	tree_node_t root, n1, n2, n12, got;
	tree_node(&tree, &root);

	log_set_quiet(0, 1);
	EXPECT_EQ(tree_add(&tree, tree.cnt, tree.cnt), 1);
	EXPECT_EQ(tree_add(&tree, root, tree.cnt), 1);
	log_set_quiet(0, 0);

	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_node(&tree, &n12);

	EXPECT_EQ(tree_add(&tree, root, n1), 0);
	EXPECT_EQ(tree_add(&tree, root, n2), 0);
	EXPECT_EQ(tree_add(&tree, n1, n12), 0);

	tree_get_child(&tree, root, &got);
	EXPECT_EQ(got, n1);
	tree_get_next(&tree, n1, &got);
	EXPECT_EQ(got, n2);

	tree_free(&tree);

	END;
}

TEST(tree_app)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, node;
	tree_node(&tree, &root);

	EXPECT_EQ(tree_app(NULL, tree.cnt, tree.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_app(&tree, tree.cnt, tree.cnt), 1);
	EXPECT_EQ(tree_app(&tree, tree.cnt, root), 1);
	log_set_quiet(0, 0);
	tree_node(&tree, &node);
	EXPECT_EQ(tree_app(&tree, root, node), 0);

	EXPECT_EQ(tree.cnt, 2);
	EXPECT_EQ(tree.cap, 2);

	tree_free(&tree);

	END;
}

TEST(tree_remove)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_remove(NULL, tree.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_remove(&tree, tree.cnt), 1);
	log_set_quiet(0, 0);

	tree_free(&tree);

	END;
}

TEST(tree_remove_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2, n3, node;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_node(&tree, &n3);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);
	tree_add(&tree, root, n3);

	EXPECT_EQ(tree_remove(&tree, n2), 0);

	tree_get_next(&tree, n1, &node);
	EXPECT_EQ(node, n3);

	tree_free(&tree);

	END;
}

TEST(tree_remove_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2, n3, node;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_node(&tree, &n3);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);
	tree_add(&tree, root, n3);

	EXPECT_EQ(tree_remove(&tree, n1), 0);

	tree_get_child(&tree, root, &node);
	EXPECT_EQ(node, n2);

	tree_free(&tree);

	END;
}

TEST(tree_get)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_get(NULL, tree.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_get(&tree, tree.cnt), NULL);
	log_set_quiet(0, 0);

	tree_node_t root;
	*(int *)tree_node(&tree, &root) = 1;
	EXPECT_EQ(*(int *)tree_get(&tree, root), 1);

	tree_free(&tree);

	END;
}

TEST(tree_get_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(tree_get_child(NULL, tree.cnt, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_get_child(&tree, tree.cnt, NULL), NULL);
	log_set_quiet(0, 0);

	tree_node_t root, n1, n2, n3, node;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_node(&tree, &n3);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);
	tree_add(&tree, n1, n3);

	EXPECT_NE(tree_get_child(&tree, root, &node), NULL);
	EXPECT_EQ(node, n1);

	tree_free(&tree);

	END;
}

TEST(tree_get_child_data)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1;
	*(int *)tree_node(&tree, &root) = 1;
	*(int *)tree_node(&tree, &n1)	= 2;
	tree_add(&tree, root, n1);

	EXPECT_EQ(*(int *)tree_get(&tree, 0), 1);
	EXPECT_EQ(*(int *)tree_get(&tree, 1), 2);

	tree_free(&tree);

	END;
}

TEST(tree_get_next)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2, next;
	tree_node(&tree, &root);
	*(int *)tree_node(&tree, &n1) = 1;
	*(int *)tree_node(&tree, &n2) = 2;
	tree_app(&tree, root, n1);
	tree_app(&tree, root, n2);

	EXPECT_EQ(*(int *)tree_get_next(&tree, root, &next), 1);
	EXPECT_EQ(next, n1);

	EXPECT_EQ(*(int *)tree_get_next(&tree, n1, &next), 2);
	EXPECT_EQ(next, n2);

	EXPECT_EQ(tree_get_next(&tree, n2, &next), NULL);
	EXPECT_EQ(next, (tree_node_t)-1);

	tree_free(&tree);

	END;
}

static int test_iterate_pre_root_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int depth, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)value;

	EXPECT_EQ(node, 0);
	EXPECT_EQ(depth, 0);
	EXPECT_EQ(last, 0);

	*(int *)priv += 1;

	CEND;
	return ret;
}

TEST(tree_iterate_pre_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root;
	tree_node(&tree, &root);

	int cnt = 0;

	EXPECT_EQ(tree_iterate_pre(NULL, tree.cnt, NULL, 0, &cnt), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_iterate_pre(&tree, tree.cnt, NULL, 0, &cnt), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(tree_iterate_pre(&tree, root, NULL, 0, &cnt), 0);
	EXPECT_EQ(tree_iterate_pre(&tree, root, test_iterate_pre_root_cb, 0, &cnt), 0);

	EXPECT_EQ(cnt, 1);

	tree_free(&tree);

	END;
}

static int test_iterate_pre_child_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int depth, int last, void *priv)
{
	CSTART;

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

	CEND;
	return ret;
}

TEST(tree_iterate_pre_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, child;
	tree_node(&tree, &root);
	tree_node(&tree, &child);
	tree_add(&tree, root, child);

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, root, test_iterate_pre_child_cb, 0, &cnt);

	EXPECT_EQ(child, 1);
	EXPECT_EQ(cnt, 2);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_pre_childs_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int depth, int last, void *priv)
{
	CSTART;

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

	CEND;
	return ret;
}

TEST(tree_iterate_pre_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, root, test_iterate_pre_childs_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_pre_grand_child_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int depth, int last, void *priv)
{
	CSTART;

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

	CEND;
	return ret;
}

TEST(tree_iterate_pre_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_add(&tree, root, n1);
	tree_add(&tree, n1, n2);

	int cnt = 0;

	int ret = tree_iterate_pre(&tree, root, test_iterate_pre_grand_child_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	return ret + RES;
}

static int test_iterate_childs_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)node;
	(void)value;
	(void)last;
	(void)priv;

	EXPECT_FAIL("%s", "Not expected child");

	CEND;
	return ret;
}

TEST(tree_iterate_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root;
	tree_node(&tree, &root);

	int ret = tree_iterate_childs(&tree, root, test_iterate_childs_cb, 0, NULL);

	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_childs_root_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)value;
	(void)last;

	EXPECT_EQ(node, 1);

	*(int *)priv += 1;

	CEND;
	return ret;
}

TEST(tree_iterate_childs_root)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_add(&tree, root, n1);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, root, test_iterate_childs_root_cb, 0, &cnt);

	EXPECT_EQ(cnt, 1);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_childs_child_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)value;

	EXPECT_EQ(node, 1);
	EXPECT_EQ(last, 1);

	*(int *)priv += 1;

	CEND;
	return ret;
}

TEST(tree_iterate_childs_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, child;
	tree_node(&tree, &root);
	tree_node(&tree, &child);
	tree_add(&tree, root, child);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, root, test_iterate_childs_child_cb, 0, &cnt);

	EXPECT_EQ(child, 1);
	EXPECT_EQ(cnt, 1);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_childs_childs_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)value;

	switch (node) {
	case 1: EXPECT_EQ(last, 0); break;
	case 2: EXPECT_EQ(last, 1); break;
	}

	*(int *)priv += 1;

	CEND;
	return ret;
}

TEST(tree_iterate_childs_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, root, test_iterate_childs_childs_cb, 0, &cnt);

	EXPECT_EQ(n1, 1);
	EXPECT_EQ(n2, 2);
	EXPECT_EQ(cnt, 2);
	EXPECT_EQ(ret, 0);

	tree_free(&tree);

	END;
}

static int test_iterate_childs_grand_child_cb(const tree_t *tree, tree_node_t node, void *value, int ret, int last, void *priv)
{
	CSTART;

	(void)tree;
	(void)value;

	switch (node) {
	case 1: EXPECT_EQ(last, 0); break;
	case 2: EXPECT_EQ(last, 1); break;
	}

	*(int *)priv += 1;

	CEND;
	return ret;
}

TEST(tree_iterate_childs_grand_child)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2, n3;
	tree_node(&tree, &root);
	tree_node(&tree, &n1);
	tree_node(&tree, &n2);
	tree_node(&tree, &n3);
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);
	tree_add(&tree, n1, n3);

	int cnt = 0;

	int ret = tree_iterate_childs(&tree, root, test_iterate_childs_grand_child_cb, 0, &cnt);

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
	RUN(tree_iterate_childs);
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

	tree_it_begin(NULL, tree.cnt);
	tree_it_begin(&tree, tree.cnt);

	tree_node_t root;
	tree_node(&tree, &root);
	tree_it_begin(&tree, root);

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

	tree_node_t root;
	*(int *)tree_node(&tree, &root) = 0;

	tree_node_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, tree.cnt, node, depth)
	{
	}

	tree_foreach(&tree, root, node, depth)
	{
		const int *value = tree_get(&tree, node);
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

	tree_node_t root, n1;
	*(int *)tree_node(&tree, &root) = 0;
	*(int *)tree_node(&tree, &n1)	= 1;
	tree_add(&tree, root, n1);

	tree_node_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, root, node, depth)
	{
		const int *value = tree_get(&tree, node);
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

	tree_node_t root, n1, n2;
	*(int *)tree_node(&tree, &root) = 0;
	*(int *)tree_node(&tree, &n1)	= 1;
	*(int *)tree_node(&tree, &n2)	= 2;
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);

	tree_node_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, root, node, depth)
	{
		const int *value = tree_get(&tree, node);
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

	tree_node_t root, n1, n2;
	*(int *)tree_node(&tree, &root) = 0;
	*(int *)tree_node(&tree, &n1)	= 1;
	*(int *)tree_node(&tree, &n2)	= 2;
	tree_add(&tree, root, n1);
	tree_add(&tree, n1, n2);

	tree_node_t node;
	int depth;

	int i = 0;
	tree_foreach(&tree, root, node, depth)
	{
		const int *value = tree_get(&tree, node);
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

	tree_node_t root;
	tree_node(&tree, &root);

	tree_node_t node;

	int i = 0;
	int *value;
	tree_foreach_child(&tree, root, node, value)
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

	tree_node_t root, n1;
	tree_node(&tree, &root);
	*(int *)tree_node(&tree, &n1) = 0;
	tree_add(&tree, root, n1);

	tree_node_t node;

	int i = 0;
	int *value;
	tree_foreach_child(&tree, root, node, value)
	{
		EXPECT_EQ(*value, i);
		i++;
	}

	EXPECT_EQ(i, 1);

	log_set_quiet(0, 1);
	i = 0;
	tree_foreach_child(&tree, tree.cnt, node, value)
	{
		i++;
	}
	log_set_quiet(0, 0);

	EXPECT_EQ(i, 0);

	tree_free(&tree);

	END;
}

TEST(tree_foreach_child_childs)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root, n1, n2;
	tree_node(&tree, &root);
	*(int *)tree_node(&tree, &n1) = 0;
	*(int *)tree_node(&tree, &n2) = 1;
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);

	tree_node_t node;

	int i = 0;
	int *value;
	tree_foreach_child(&tree, root, node, value)
	{
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

	tree_node_t root, n1, n2, n3;
	tree_node(&tree, &root);
	*(int *)tree_node(&tree, &n1) = 0;
	*(int *)tree_node(&tree, &n2) = 1;
	*(int *)tree_node(&tree, &n3) = 2;
	tree_add(&tree, root, n1);
	tree_add(&tree, root, n2);
	tree_add(&tree, n1, n3);

	tree_node_t node;

	int i = 0;
	int *value;
	tree_foreach_child(&tree, root, node, value)
	{
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

static size_t print_tree(void *data, dst_t dst, const void *priv)
{
	(void)priv;
	return dputf(dst, "%d\n", *(int *)data);
}

TEST(tree_print)
{
	START;

	tree_t tree = {0};
	tree_init(&tree, 1, sizeof(int), ALLOC_STD);

	tree_node_t root;
	*(int *)tree_node(&tree, &root) = 0;

	tree_node_t n1, n2, n11, n111;

	*(int *)tree_node(&tree, &n1)	= 1;
	*(int *)tree_node(&tree, &n2)	= 2;
	*(int *)tree_node(&tree, &n11)	= 11;
	*(int *)tree_node(&tree, &n111) = 111;
	tree_add(&tree, 0, n1);
	tree_add(&tree, 0, n2);
	tree_add(&tree, n1, n11);
	tree_add(&tree, n11, n111);

	char buf[64] = {0};
	EXPECT_EQ(tree_print(NULL, tree.cnt, NULL, DST_BUF(buf), NULL), 0);
	EXPECT_EQ(tree_print(&tree, tree.cnt, NULL, DST_BUF(buf), NULL), 0);
	EXPECT_EQ(tree_print(&tree, root, NULL, DST_BUF(buf), NULL), 0);

	EXPECT_EQ(tree_print(&tree, root, NULL, DST_BUF(buf), NULL), 0);
	EXPECT_EQ(tree_print(&tree, root, print_tree, DST_BUF(buf), NULL), 47);
	EXPECT_STR(buf,
		   "0\n"
		   "├─1\n"
		   "│ └─11\n"
		   "│   └─111\n"
		   "└─2\n");

	tree_free(&tree);

	END;
}

static size_t print_tree_depth(void *data, dst_t dst, const void *priv)
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

	tree_node_t root, n1;
	tree_node(&tree, &root);
	tree_node_t child = root;

	for (int i = 0; i < TREE_MAX_DEPTH; i++) {
		tree_node(&tree, &n1);
		tree_add(&tree, child, n1);
		child = n1;
	}

	char buf[32000] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(tree_print(&tree, root, print_tree_depth, DST_BUF(buf), NULL), 16764);
	log_set_quiet(0, 0);

	tree_free(&tree);

	END;
}

STEST(tree)
{
	SSTART;

	RUN(tree_init_free);
	RUN(tree_reset);
	RUN(tree_node);
	RUN(tree_add);
	RUN(tree_app);
	RUN(tree_remove);
	RUN(tree_remove_next);
	RUN(tree_remove_child);
	RUN(tree_get);
	RUN(tree_get_child);
	RUN(tree_get_child_data);
	RUN(tree_get_next);
	RUN(tree_iterate);
	RUN(tree_foreach);
	RUN(tree_print);
	RUN(tree_print_depth);

	SEND;
}
