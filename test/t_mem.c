#include "mem.h"

#include "log.h"
#include "test.h"

TEST(mem_print)
{
	START;

	mem_stats_t *mem = (mem_stats_t *)mem_stats_get();

	size_t peak = mem->peak;

	char buf[256] = {0};

	mem->peak = 10;
	EXPECT_GT(mem_print(PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	mem->peak = 1024 + 10;
	EXPECT_GT(mem_print(PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	mem->peak = 1024 * 1024 + 10;
	EXPECT_GT(mem_print(PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	mem->peak = peak;

	mem_stats_set(NULL);
	EXPECT_EQ(mem_print(PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	mem_stats_set(mem);

	END;
}

TEST(mem_check)
{
	START;

	mem_stats_t *mem = (mem_stats_t *)mem_stats_get();

	size_t m = mem->mem;

	EXPECT_EQ(mem_check(), 0);
	mem->mem = 1;
	log_set_quiet(0, 1);
	EXPECT_EQ(mem_check(), 1);
	log_set_quiet(0, 0);
	mem->mem = m;

	END;
}

TEST(mem_alloc)
{
	START;

	log_set_quiet(0, 1);
	void *data = mem_alloc(0);
	log_set_quiet(0, 0);

	EXPECT_NE(data, NULL);

	mem_free(NULL, 0);
	mem_free(data, 0);

	END;
}

TEST(mem_calloc)
{
	START;

	log_set_quiet(0, 1);
	void *data = mem_calloc(0, 0);
	log_set_quiet(0, 0);

	EXPECT_NE(data, NULL);

	mem_free(data, 0);

	END;
}

TEST(mem_realloc)
{
	START;

	log_set_quiet(0, 1);
	EXPECT_EQ(mem_realloc(NULL, 0, 0), NULL);
	log_set_quiet(0, 0);

	log_set_quiet(0, 1);
	void *ptr  = mem_alloc(0);
	void *prev = ptr;
	EXPECT_EQ(ptr = mem_realloc(ptr, 0, 0), prev);
	log_set_quiet(0, 0);
	EXPECT_NE(ptr = mem_realloc(ptr, 1, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_NE(ptr = mem_realloc(ptr, 0, 1), NULL);
	EXPECT_NE(ptr = mem_realloc(ptr, 1, 1), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(ptr = mem_realloc(ptr, 2, 1), NULL);
	EXPECT_NE(ptr = mem_realloc(ptr, 1, 2), NULL);
	mem_free(ptr, 1);

	END;
}

TEST(mem_set)
{
	START;

	log_set_quiet(0, 1);
	void *data = mem_alloc(0);
	log_set_quiet(0, 0);

	EXPECT_EQ(mem_set(NULL, 0, 0), NULL);
	EXPECT_NE(mem_set(data, 0, 0), NULL);

	mem_free(data, 0);

	END;
}

TEST(mem_cpy)
{
	START;

	log_set_quiet(0, 1);
	void *src = mem_alloc(0);
	void *dst = mem_alloc(0);
	log_set_quiet(0, 0);

	EXPECT_EQ(mem_cpy(NULL, 0, NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(mem_cpy(dst, 0, src, 1), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(mem_cpy(dst, 0, src, 0), NULL);

	mem_free(src, 0);
	mem_free(dst, 0);

	END;
}

TEST(mem_cmp)
{
	START;

	log_set_quiet(0, 1);
	void *l = mem_alloc(0);
	void *r = mem_alloc(0);
	log_set_quiet(0, 0);

	EXPECT_EQ(mem_cmp(NULL, NULL, 0), 0);
	EXPECT_EQ(mem_cmp(l, r, 0), 0);

	mem_free(l, 0);
	mem_free(r, 0);

	END;
}

TEST(mem_swap)
{
	START;

	char a[] = "abc";
	char b[] = "def";

	EXPECT_EQ(mem_swap(NULL, NULL, 0), 1);
	EXPECT_EQ(mem_swap(a, NULL, 0), 1);
	EXPECT_EQ(mem_swap(a, b, sizeof(a)), 0);

	EXPECT_STR(a, "def");
	EXPECT_STR(b, "abc");

	END;
}

TEST(mem_oom)
{
	START;

	mem_oom(1);
	mem_oom(0);

	END;
}

STEST(mem)
{
	SSTART;

	mem_stats_t mm = {0};

	const mem_stats_t *mem = mem_stats_set(&mm);

	RUN(mem_print);
	RUN(mem_check);
	RUN(mem_alloc);
	RUN(mem_calloc);
	RUN(mem_realloc);
	RUN(mem_set);
	RUN(mem_cpy);
	RUN(mem_cmp);
	RUN(mem_swap);
	RUN(mem_oom);

	mem_stats_set((mem_stats_t *)mem);

	SEND;
}
