#include "arr.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(arr_init_free)
{
	START;

	arr_t arr = {0};

	EXPECT_EQ(arr_init(NULL, 0, sizeof(int), ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(arr_init(&arr, 1, sizeof(int), ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(arr_init(&arr, 1, sizeof(int), ALLOC_STD), &arr);

	EXPECT_NE(arr.data, NULL);
	EXPECT_EQ(arr.cap, 1);
	EXPECT_EQ(arr.cnt, 0);
	EXPECT_EQ(arr.size, sizeof(int));

	arr_free(&arr);
	arr_free(NULL);

	EXPECT_EQ(arr.data, NULL);
	EXPECT_EQ(arr.cap, 0);
	EXPECT_EQ(arr.cnt, 0);
	EXPECT_EQ(arr.size, 0);

	END;
}

TEST(arr_reset)
{
	START;

	arr_t arr = {0};

	arr_reset(NULL, 0, 0);
	arr_reset(&arr, 0, 1);

	EXPECT_EQ(arr.cnt, 0);

	END;
}

TEST(arr_add)
{
	START;

	arr_t arr = {0};
	log_set_quiet(0, 1);
	arr_init(&arr, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(arr_add(NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(arr_add(&arr), NULL);
	mem_oom(0);
	EXPECT_NE(arr_add(&arr), NULL);
	EXPECT_NE(arr_add(&arr), NULL);
	EXPECT_EQ(arr.cnt, 2);
	EXPECT_EQ(arr.cap, 2);

	arr_free(&arr);

	END;
}

TEST(arr_get)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 1, sizeof(int), ALLOC_STD);

	EXPECT_EQ(arr_get(NULL, ARR_END), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(arr_get(&arr, ARR_END), NULL);
	log_set_quiet(0, 0);
	arr_add(&arr);
	*(int *)arr_get(&arr, 0) = 1;

	EXPECT_EQ(*(int *)arr_get(&arr, 0), 1);

	arr_free(&arr);

	END;
}

TEST(arr_set)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 1, sizeof(int), ALLOC_STD);

	int value = 1;

	arr_add(&arr);

	EXPECT_EQ(arr_set(NULL, ARR_END, NULL), NULL);
	EXPECT_EQ(arr_set(&arr, ARR_END, NULL), NULL);
	EXPECT_EQ(arr_set(&arr, 0, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(arr_set(&arr, ARR_END, &value), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(arr_set(&arr, 0, &value), NULL);

	EXPECT_EQ(*(int *)arr_get(&arr, 0), value);

	arr_free(&arr);

	END;
}

TEST(arr_addv)
{
	START;

	arr_t arr = {0};
	log_set_quiet(0, 1);
	arr_init(&arr, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	const int v0 = 1;
	const int v1 = 2;

	EXPECT_EQ(arr_addv(NULL, NULL), ARR_END);
	mem_oom(1);
	EXPECT_EQ(arr_addv(&arr, &v0), ARR_END);
	mem_oom(0);

	EXPECT_EQ(arr_addv(&arr, &v0), 0);
	EXPECT_EQ(arr_addv(&arr, &v1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 0), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 2);
	EXPECT_EQ(arr.cnt, 2);
	EXPECT_EQ(arr.cap, 2);

	arr_free(&arr);

	END;
}

TEST(arr_addu)
{
	START;

	arr_t arr = {0};
	log_set_quiet(0, 1);
	arr_init(&arr, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);

	const int v0 = 1;
	const int v1 = 2;

	EXPECT_EQ(arr_addu(NULL, NULL), ARR_END);
	mem_oom(1);
	EXPECT_EQ(arr_addu(&arr, &v0), ARR_END);
	mem_oom(0);

	EXPECT_EQ(arr_addu(&arr, &v0), 0);
	EXPECT_EQ(arr_addu(&arr, &v0), 0);
	EXPECT_EQ(arr_addu(&arr, &v1), 1);
	EXPECT_EQ(arr_addu(&arr, &v1), 1);

	arr_free(&arr);

	END;
}

TEST(arr_find)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 2, sizeof(int), ALLOC_STD);

	const int value0 = 0;
	const int value1 = 1;

	*(int *)arr_add(&arr) = value0;
	*(int *)arr_add(&arr) = value1;

	EXPECT_EQ(arr_find(NULL, NULL), ARR_END);
	EXPECT_EQ(arr_find(&arr, NULL), ARR_END);
	EXPECT_EQ(arr_find(&arr, &value0), 0);
	EXPECT_EQ(arr_find(&arr, &value1), 1);

	arr_free(&arr);

	END;
}

static int index_cmp_cb(const void *value1, const void *value2, const void *priv)
{
	(void)priv;
	return *(int *)value1 == *(int *)value2;
}

TEST(arr_find_cmp)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 2, sizeof(int), ALLOC_STD);

	const int value0 = 0;
	const int value1 = 1;
	const int value2 = 2;

	*(int *)arr_add(&arr) = value0;
	*(int *)arr_add(&arr) = value1;

	EXPECT_EQ(arr_find_cmp(NULL, NULL, NULL, NULL), ARR_END);
	EXPECT_EQ(arr_find_cmp(&arr, NULL, NULL, NULL), ARR_END);
	EXPECT_EQ(arr_find_cmp(&arr, &value0, NULL, NULL), ARR_END);
	EXPECT_EQ(arr_find_cmp(&arr, &value2, index_cmp_cb, NULL), ARR_END);
	EXPECT_EQ(arr_find_cmp(&arr, &value0, index_cmp_cb, NULL), 0);
	EXPECT_EQ(arr_find_cmp(&arr, &value1, index_cmp_cb, NULL), 1);

	arr_free(&arr);

	END;
}

TEST(arr_add_all)
{
	START;

	arr_t arr  = {0};
	arr_t arr0 = {0};
	arr_t arr1 = {0};

	arr_init(&arr, 4, sizeof(int), ALLOC_STD);
	arr_init(&arr0, 2, sizeof(int), ALLOC_STD);
	arr_init(&arr1, 2, sizeof(int), ALLOC_STD);

	*(int *)arr_add(&arr0) = 0;
	*(int *)arr_add(&arr0) = 1;
	*(int *)arr_add(&arr1) = 1;
	*(int *)arr_add(&arr1) = 2;

	EXPECT_EQ(arr_add_all(NULL, &arr1), NULL);
	EXPECT_EQ(arr_add_all(&arr, NULL), NULL);
	EXPECT_EQ(arr_add_all(&arr, &arr0), &arr);
	EXPECT_EQ(arr_add_all(&arr, &arr1), &arr);

	EXPECT_EQ(arr.cnt, 4);
	EXPECT_EQ(arr.cap, 4);
	EXPECT_EQ(*(int *)arr_get(&arr, 0), 0);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 2), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 3), 2);

	arr_free(&arr);
	arr_free(&arr0);
	arr_free(&arr1);

	END;
}

TEST(arr_add_unique)
{
	START;

	arr_t arr  = {0};
	arr_t arr0 = {0};
	arr_t arr1 = {0};

	log_set_quiet(0, 1);
	arr_init(&arr, 0, sizeof(int), ALLOC_STD);
	log_set_quiet(0, 0);
	arr_init(&arr0, 2, sizeof(int), ALLOC_STD);
	arr_init(&arr1, 2, sizeof(int), ALLOC_STD);

	*(int *)arr_add(&arr0) = 0;
	*(int *)arr_add(&arr0) = 1;
	*(int *)arr_add(&arr1) = 1;
	*(int *)arr_add(&arr1) = 2;

	EXPECT_EQ(arr_add_unique(NULL, &arr1), NULL);
	EXPECT_EQ(arr_add_unique(&arr, NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(arr_add_unique(&arr, &arr0), NULL);
	mem_oom(0);
	EXPECT_EQ(arr_add_unique(&arr, &arr0), &arr);
	EXPECT_EQ(arr_add_unique(&arr, &arr1), &arr);

	EXPECT_EQ(arr.cnt, 3);
	EXPECT_EQ(arr.cap, 4);
	EXPECT_EQ(*(int *)arr_get(&arr, 0), 0);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 2), 2);

	arr_free(&arr);
	arr_free(&arr0);
	arr_free(&arr1);

	END;
}

TEST(arr_merge_all)
{
	START;

	arr_t arr  = {0};
	arr_t arr0 = {0};
	arr_t arr1 = {0};
	arr_t arrs = {0};

	arr_init(&arr0, 2, sizeof(int), ALLOC_STD);
	arr_init(&arr1, 2, sizeof(int), ALLOC_STD);
	arr_init(&arrs, 2, sizeof(long long), ALLOC_STD);

	*(int *)arr_add(&arr0) = 0;
	*(int *)arr_add(&arr0) = 1;
	*(int *)arr_add(&arr1) = 1;
	*(int *)arr_add(&arr1) = 2;

	EXPECT_EQ(arr_merge_all(NULL, &arr0, &arr1), NULL);
	EXPECT_EQ(arr_merge_all(&arr, NULL, &arr1), NULL);
	EXPECT_EQ(arr_merge_all(&arr, &arr0, NULL), NULL);
	EXPECT_EQ(arr_merge_all(&arr, &arrs, &arr1), NULL);
	EXPECT_EQ(arr_merge_all(&arr, &arr0, &arrs), NULL);
	EXPECT_EQ(arr_merge_all(&arr, &arr0, &arr1), &arr);

	EXPECT_EQ(arr.cnt, 4);
	EXPECT_EQ(arr.cap, 4);
	EXPECT_EQ(*(int *)arr_get(&arr, 0), 0);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 2), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 3), 2);

	arr_free(&arr);
	arr_free(&arr0);
	arr_free(&arr1);
	arr_free(&arrs);

	END;
}

TEST(arr_merge_unique)
{
	START;

	arr_t arr  = {0};
	arr_t arr0 = {0};
	arr_t arr1 = {0};
	arr_t arrs = {0};

	arr_init(&arr0, 2, sizeof(int), ALLOC_STD);
	arr_init(&arr1, 2, sizeof(int), ALLOC_STD);
	arr_init(&arrs, 2, sizeof(long long), ALLOC_STD);

	*(int *)arr_add(&arr0) = 0;
	*(int *)arr_add(&arr0) = 1;
	*(int *)arr_add(&arr1) = 1;
	*(int *)arr_add(&arr1) = 2;

	EXPECT_EQ(arr_merge_unique(NULL, &arr0, &arr1), NULL);
	EXPECT_EQ(arr_merge_unique(&arr, NULL, &arr1), NULL);
	EXPECT_EQ(arr_merge_unique(&arr, &arr0, NULL), NULL);
	EXPECT_EQ(arr_merge_unique(&arr, &arrs, &arr1), NULL);
	EXPECT_EQ(arr_merge_unique(&arr, &arr0, &arrs), NULL);
	EXPECT_NE(arr_merge_unique(&arr, &arr0, &arr1), NULL);

	EXPECT_EQ(arr.cnt, 3);
	EXPECT_EQ(arr.cap, 4);
	EXPECT_EQ(*(int *)arr_get(&arr, 0), 0);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 2), 2);

	arr_free(&arr);
	arr_free(&arr0);
	arr_free(&arr1);
	arr_free(&arrs);

	END;
}

static int t_arr_sort_cb(const void *a, const void *b, const void *priv)
{
	(void)priv;
	return *(int *)a < *(int *)b ? -1 : 1;
}

TEST(arr_sort)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 2, sizeof(int), ALLOC_STD);

	*(int *)arr_add(&arr) = 3;
	*(int *)arr_add(&arr) = 1;
	*(int *)arr_add(&arr) = 2;
	*(int *)arr_add(&arr) = 0;

	EXPECT_EQ(arr_sort(NULL, NULL, NULL), NULL);
	EXPECT_EQ(arr_sort(&arr, NULL, NULL), &arr);
	EXPECT_EQ(arr_sort(&arr, t_arr_sort_cb, NULL), &arr);

	EXPECT_EQ(*(int *)arr_get(&arr, 0), 0);
	EXPECT_EQ(*(int *)arr_get(&arr, 1), 1);
	EXPECT_EQ(*(int *)arr_get(&arr, 2), 2);
	EXPECT_EQ(*(int *)arr_get(&arr, 3), 3);

	arr_free(&arr);

	END;
}

TEST(arr_foreach)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 2, sizeof(int), ALLOC_STD);
	*(int *)arr_add(&arr) = 0;
	*(int *)arr_add(&arr) = 1;

	int *value = NULL;

	int i = 0;
	arr_foreach(&arr, value)
	{
		EXPECT_EQ(*value, i);
		i++;
	}

	EXPECT_EQ(i, 2);

	arr_free(&arr);

	END;
}

static int print_arr(void *data, print_dst_t dst, const void *priv)
{
	(void)priv;
	return c_dprintf(dst, "%d\n", *(int *)data);
}

TEST(arr_print)
{
	START;

	arr_t arr = {0};
	arr_init(&arr, 1, sizeof(int), ALLOC_STD);

	*(int *)arr_add(&arr) = 0;
	*(int *)arr_add(&arr) = 1;
	*(int *)arr_add(&arr) = 2;

	char buf[16] = {0};
	EXPECT_EQ(arr_print(NULL, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);
	EXPECT_EQ(arr_print(&arr, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 0);

	EXPECT_EQ(arr_print(&arr, print_arr, PRINT_DST_BUF(buf, sizeof(buf), 0), NULL), 6);
	EXPECT_STR(buf,
		   "0\n"
		   "1\n"
		   "2\n");

	arr_free(&arr);

	END;
}

STEST(arr)
{
	SSTART;

	RUN(arr_init_free);
	RUN(arr_reset);
	RUN(arr_add);
	RUN(arr_get);
	RUN(arr_set);
	RUN(arr_addv);
	RUN(arr_addu);
	RUN(arr_find);
	RUN(arr_find_cmp);
	RUN(arr_add_all);
	RUN(arr_add_unique);
	RUN(arr_merge_all);
	RUN(arr_merge_unique);
	RUN(arr_sort);
	RUN(arr_foreach);
	RUN(arr_print);

	SEND;
}
