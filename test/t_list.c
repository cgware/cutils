#include "list.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(list_init_free)
{
	START;

	list_t list = {0};

	EXPECT_EQ(list_init(NULL, 0, sizeof(int), ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(list_init(&list, 1, sizeof(int), ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(list_init(&list, 1, sizeof(int), ALLOC_STD), &list);

	EXPECT_NE(list.data, NULL);
	EXPECT_EQ(list.cap, 1);
	EXPECT_EQ(list.cnt, 0);
	EXPECT_NE(list.size, 0);

	list_free(&list);
	list_free(NULL);

	EXPECT_EQ(list.data, NULL);
	EXPECT_EQ(list.cap, 0);
	EXPECT_EQ(list.cnt, 0);
	EXPECT_EQ(list.size, 0);

	END;
}

TEST(list_reset)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(list_node_t), ALLOC_STD);

	list_node_t root, n1;
	list_node_t *data = list_node(&list, &root);

	*data = 10;

	list_node(&list, &n1);
	list_app(&list, root, n1);

	list_reset(NULL, 0);
	list_reset(&list, 1);

	data = list_get(&list, root);

	list_node_t *next = data - 1;

	EXPECT_EQ(*next, (list_node_t)-1);
	EXPECT_EQ(*data, 10);

	list_reset(&list, 2);
	EXPECT_EQ(list.cnt, 1);

	list_free(&list);

	END;
}

TEST(list_node)
{
	START;

	list_t list = {0};
	log_set_quiet(0, 1);
	list_init(&list, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	list_node_t node;

	mem_oom(1);
	EXPECT_EQ(list_node(&list, NULL), NULL);
	mem_oom(0);
	EXPECT_EQ(list_node(NULL, NULL), NULL);
	EXPECT_NE(list_node(&list, &node), NULL);

	EXPECT_EQ(node, 0);
	EXPECT_EQ(list.cnt, 1);
	EXPECT_EQ(list.cap, 1);

	list_free(&list);

	END;
}

TEST(list_nodes)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node;

	EXPECT_NE(list_node(&list, &node), NULL);
	EXPECT_EQ(node, 0);
	EXPECT_NE(list_node(&list, &node), NULL);
	EXPECT_EQ(node, 1);

	EXPECT_EQ(list.cnt, 2);
	EXPECT_EQ(list.cap, 2);

	list_free(&list);

	END;
}

TEST(list_app)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t root, node;
	list_node(&list, &root);

	EXPECT_EQ(list_app(NULL, list.cnt, list.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_app(&list, list.cnt, list.cnt), 1);
	EXPECT_EQ(list_app(&list, root, list.cnt), 1);
	log_set_quiet(0, 0);
	list_node(&list, &node);
	EXPECT_EQ(list_app(&list, root, node), 0);

	EXPECT_EQ(list.cnt, 2);
	EXPECT_EQ(list.cap, 2);

	list_free(&list);

	END;
}

TEST(list_remove)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node;
	list_node(&list, &node);

	EXPECT_EQ(list_remove(NULL, list.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_remove(&list, list.cnt), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(list_remove(&list, node), 0);

	EXPECT_EQ(list.cnt, 1);

	list_free(&list);

	END;
}

TEST(list_remove_middle)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t root, n1, n2, node;

	list_node(&list, &root);
	list_node(&list, &n1);
	list_node(&list, &n2);
	list_app(&list, root, n1);
	list_app(&list, root, n2);

	list_remove(&list, n1);

	EXPECT_NE(list_get_next(&list, root, &node), NULL);
	EXPECT_EQ(node, n2)

	list_free(&list);

	END;
}

TEST(list_remove_last)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t root, n1, n2, node;

	list_node(&list, &root);
	list_node(&list, &n1);
	list_node(&list, &n2);
	list_app(&list, root, n1);
	list_app(&list, root, n2);

	list_remove(&list, n2);

	EXPECT_EQ(list_get_next(&list, n1, &node), 0);
	EXPECT_EQ(node, (list_node_t)-1);

	list_free(&list);

	END;
}

TEST(list_get)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node;
	list_node(&list, &node);

	EXPECT_EQ(list_get(NULL, list.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_get(&list, list.cnt), NULL);
	log_set_quiet(0, 0);
	*(int *)list_get(&list, node) = 8;

	EXPECT_EQ(list.cnt, 1);
	EXPECT_EQ(list.cap, 1);
	EXPECT_EQ(*(int *)list_get(&list, node), 8);

	list_free(&list);

	END;
}

TEST(list_get_next)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t root, next, node;
	list_node(&list, &root);
	list_node(&list, &next);
	list_app(&list, root, next);

	uint cnt = list.cnt;

	EXPECT_EQ(list_get_next(NULL, list.cnt, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_get_next(&list, list.cnt, NULL), NULL);
	list.cnt--;
	EXPECT_EQ(list_get_next(&list, root, &node), NULL);
	list.cnt = cnt;
	log_set_quiet(0, 0);
	EXPECT_NE(list_get_next(&list, root, &node), NULL);
	EXPECT_EQ(node, next);

	list_free(&list);

	END;
}

TEST(list_get_at)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t root, n1, n2, node;
	*(int *)list_node(&list, &root) = 1;
	*(int *)list_node(&list, &n1)	= 2;
	*(int *)list_node(&list, &n2)	= 3;

	list_app(&list, root, n1);
	list_app(&list, root, n2);

	EXPECT_EQ(list_get_at(NULL, list.cnt, list.cnt, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_get_at(&list, list.cnt, list.cnt, NULL), NULL);
	log_set_quiet(0, 0);
	EXPECT_EQ(*(int *)list_get_at(&list, root, 0, &node), 1);
	EXPECT_EQ(node, root);
	EXPECT_EQ(*(int *)list_get_at(&list, root, 1, &node), 2);
	EXPECT_EQ(node, n1);
	EXPECT_EQ(*(int *)list_get_at(&list, root, 2, &node), 3);
	EXPECT_EQ(node, n2);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_get_at(&list, root, 3, &node), NULL);
	log_set_quiet(0, 0);
	EXPECT_EQ(node, (uint)-1);

	list_free(&list);

	END;
}

TEST(list_foreach)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node, n1, n2;

	*(int *)list_node(&list, &node) = 0;
	*(int *)list_node(&list, &n1)	= 1;
	*(int *)list_node(&list, &n2)	= 2;
	list_app(&list, node, n1);
	list_app(&list, node, n2);

	int *value;

	int cnt = 0;
	list_foreach(&list, node, value)
	{
		EXPECT_EQ(*value, cnt);
		cnt++;
	}

	EXPECT_EQ(cnt, 3);

	list_free(&list);

	END;
}

TEST(list_foreach_all)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node, n1;

	*(int *)list_node(&list, &node) = 0;
	*(int *)list_node(&list, &n1)	= 1;
	list_app(&list, node, n1);
	*(int *)list_node(&list, NULL) = 2;

	int *value;

	list_node_t i = 0;
	int cnt	      = 0;
	list_foreach_all(&list, i, value)
	{
		EXPECT_EQ(*value, cnt);
		cnt++;
	}

	EXPECT_EQ(cnt, 3);

	list_free(&list);

	END;
}

static size_t print_list(void *data, dst_t dst, const void *priv)
{
	(void)priv;
	return dputf(dst, "%d\n", *(int *)data);
}

TEST(list_print)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	list_node_t node, n1, n2;

	*(int *)list_node(&list, &node) = 0;
	*(int *)list_node(&list, &n1)	= 1;
	*(int *)list_node(&list, &n2)	= 2;
	list_app(&list, node, n1);
	list_app(&list, node, n2);

	char buf[16] = {0};
	EXPECT_EQ(list_print(NULL, list.cnt, NULL, DST_BUF(buf), NULL), 0);
	EXPECT_EQ(list_print(&list, list.cnt, NULL, DST_BUF(buf), NULL), 0);
	EXPECT_EQ(list_print(&list, node, NULL, DST_BUF(buf), NULL), 0);

	EXPECT_EQ(list_print(&list, node, print_list, DST_BUF(buf), 0), 6);
	EXPECT_STR(buf,
		   "0\n"
		   "1\n"
		   "2\n");

	list_free(&list);

	END;
}

STEST(list)
{
	SSTART;

	RUN(list_init_free);
	RUN(list_reset);
	RUN(list_node);
	RUN(list_nodes);
	RUN(list_app)
	RUN(list_remove);
	RUN(list_remove_middle);
	RUN(list_remove_last);
	RUN(list_get);
	RUN(list_get_next);
	RUN(list_get_at);
	RUN(list_foreach);
	RUN(list_foreach_all);
	RUN(list_print);

	SEND;
}
