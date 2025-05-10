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

	list_reset(NULL, 0);
	list_reset(&list, 1);

	EXPECT_EQ(list.cnt, 0);

	END;
}

TEST(list_add)
{
	START;

	list_t list = {0};
	log_set_quiet(0, 1);
	list_init(&list, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	lnode_t node;

	mem_oom(1);
	EXPECT_EQ(list_add(&list, NULL), NULL);
	mem_oom(0);
	EXPECT_EQ(list_add(NULL, NULL), NULL);
	EXPECT_NE(list_add(&list, &node), NULL);

	EXPECT_EQ(node, 0);
	EXPECT_EQ(list.cnt, 1);
	EXPECT_EQ(list.cap, 1);

	list_free(&list);

	END;
}

TEST(list_adds)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t node;

	EXPECT_NE(list_add(&list, &node), NULL);
	EXPECT_EQ(node, 0);
	EXPECT_NE(list_add(&list, &node), NULL);
	EXPECT_EQ(node, 1);

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

	lnode_t node;
	list_add(&list, &node);

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

	lnode_t root, n1, n2, node;

	list_add(&list, &root);
	list_add_next(&list, root, &n1);
	list_add_next(&list, root, &n2);

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

	lnode_t root, n1, n2, node;

	list_add(&list, &root);
	list_add_next(&list, root, &n1);
	list_add_next(&list, root, &n2);

	list_remove(&list, n2);

	EXPECT_EQ(list_get_next(&list, n1, &node), 0);
	EXPECT_EQ(node, (lnode_t)-1);

	list_free(&list);

	END;
}

TEST(list_add_remove)
{
	SSTART;
	RUN(list_add);
	RUN(list_adds);
	RUN(list_remove);
	RUN(list_remove_middle);
	RUN(list_remove_last);
	SEND;
}

TEST(list_add_next)
{
	START;

	list_t list = {0};

	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t root, next;

	EXPECT_EQ(list_add_next(NULL, list.cnt, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_add_next(&list, list.cnt, NULL), NULL);
	log_set_quiet(0, 0);
	list_add(&list, &root);
	mem_oom(1);
	EXPECT_EQ(list_add_next(&list, root, NULL), NULL);
	mem_oom(0);
	EXPECT_NE(list_add_next(&list, root, &next), NULL);

	EXPECT_EQ(list.cnt, 2);
	EXPECT_EQ(list.cap, 2);

	list_free(&list);

	END;
}

TEST(list_add_nexts)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t root, next1, next2;

	list_add(&list, &root);
	list_add_next(&list, root, &next1);
	list_add_next(&list, root, &next2);

	EXPECT_EQ(next1, 1);
	EXPECT_EQ(next2, 2);
	EXPECT_EQ(list.cnt, 3);
	EXPECT_EQ(list.cap, 4);

	list_free(&list);

	END;
}

TEST(list_add_and_next)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t n1, n2, next1, next2;

	list_add(&list, &n1);
	list_add(&list, &n2);

	list_add_next(&list, n1, &next1);
	list_add_next(&list, n2, &next2);

	EXPECT_EQ(next1, 2);
	EXPECT_EQ(next2, 3);
	EXPECT_EQ(list.cnt, 4);
	EXPECT_EQ(list.cap, 4);

	list_free(&list);

	END;
}

TEST(list_set_next)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t root, node;
	list_add(&list, &root);

	EXPECT_EQ(list_set_next(NULL, list.cnt, list.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(list_set_next(&list, list.cnt, list.cnt), 1);
	EXPECT_EQ(list_set_next(&list, root, list.cnt), 1);
	log_set_quiet(0, 0);
	list_add(&list, &node);
	EXPECT_EQ(list_set_next(&list, root, node), 0);

	EXPECT_EQ(list.cnt, 2);
	EXPECT_EQ(list.cap, 2);

	list_free(&list);

	END;
}

TEST(list_get_next)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t root, next, node;
	list_add(&list, &root);
	list_add_next(&list, root, &next);

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

TEST(list_next)
{
	SSTART;
	RUN(list_add_next);
	RUN(list_add_nexts);
	RUN(list_add_and_next);
	RUN(list_set_next);
	RUN(list_get_next);
	SEND;
}

TEST(list_get_at)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t root, n1, n2, node;
	*(int *)list_add(&list, &root)		= 1;
	*(int *)list_add_next(&list, root, &n1) = 2;
	*(int *)list_add_next(&list, root, &n2) = 3;

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

TEST(list_set_cnt)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(lnode_t), ALLOC_STD);

	lnode_t root;
	lnode_t *data = list_add(&list, &root);

	*data = 10;

	list_add_next(&list, root, NULL);

	list_set_cnt(NULL, 0);
	list_set_cnt(&list, 1);

	data = list_get(&list, root);

	lnode_t *next = data - 1;

	EXPECT_EQ(*next, (lnode_t)-1);
	EXPECT_EQ(*data, 10);

	list_free(&list);

	END;
}

TEST(list_get)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t node;
	list_add(&list, &node);

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

TEST(list_foreach)
{
	START;

	list_t list = {0};
	list_init(&list, 1, sizeof(int), ALLOC_STD);

	lnode_t node;

	*(int *)list_add(&list, &node)		 = 0;
	*(int *)list_add_next(&list, node, NULL) = 1;
	*(int *)list_add_next(&list, node, NULL) = 2;

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

	lnode_t node;

	*(int *)list_add(&list, &node)		 = 0;
	*(int *)list_add_next(&list, node, NULL) = 1;
	*(int *)list_add(&list, NULL)		 = 2;

	int *value;

	lnode_t i = 0;
	int cnt	  = 0;
	list_foreach_all(&list, i, value)
	{
		EXPECT_EQ(*value, cnt);
		cnt++;
	}

	EXPECT_EQ(cnt, 3);

	list_free(&list);

	END;
}

TEST(list_foreachs)
{
	SSTART;
	RUN(list_foreach);
	RUN(list_foreach_all);
	SEND;
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

	lnode_t node;

	*(int *)list_add(&list, &node)		 = 0;
	*(int *)list_add_next(&list, node, NULL) = 1;
	*(int *)list_add_next(&list, node, NULL) = 2;

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
	RUN(list_add_remove);
	RUN(list_next);
	RUN(list_get_at);
	RUN(list_set_cnt);
	RUN(list_get);
	RUN(list_foreachs);
	RUN(list_print);

	SEND;
}
