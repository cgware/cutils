#include "bin.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(bin_init_free)
{
	START;

	bin_t bin = {0};

	EXPECT_EQ(bin_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(bin_init(&bin, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(bin_init(&bin, 1, ALLOC_STD), &bin);

	bin_free(NULL);
	bin_free(&bin);

	END;
}

TEST(bin_resize)
{
	START;

	bin_t bin = {0};
	bin_init(&bin, 1, ALLOC_STD);

	EXPECT_EQ(bin_resize(NULL, 0), 1);
	EXPECT_EQ(bin_resize(&bin, 1), 0);
	mem_oom(1);
	EXPECT_EQ(bin_resize(&bin, 2), 1);
	mem_oom(0);
	EXPECT_EQ(bin_resize(&bin, 2), 0);

	bin_free(&bin);

	END;
}

TEST(bin_cmp)
{
	START;

	bin_t bin = {0};
	bin_init(&bin, 1, ALLOC_STD);

	uint val = 1;
	bin_add(&bin, &val, sizeof(uint));

	EXPECT_EQ(bin_cmp(NULL, 0, NULL, sizeof(uint)), 1);
	EXPECT_EQ(bin_cmp(&bin, 0, &val, sizeof(uint) * 2), 1);
	val = 0;
	EXPECT_EQ(bin_cmp(&bin, 0, &val, sizeof(uint)), 1);
	val = 1;
	EXPECT_EQ(bin_cmp(&bin, 0, &val, sizeof(uint)), 0);

	bin_free(&bin);

	END;
}

TEST(bin_add)
{
	START;

	bin_t bin = {0};
	log_set_quiet(0, 1);
	bin_init(&bin, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(bin_add(NULL, NULL, 1), 1);
	mem_oom(1);
	EXPECT_EQ(bin_add(&bin, NULL, 1), 1);
	mem_oom(0);
	uint val = 1;
	EXPECT_EQ(bin_add(&bin, &val, sizeof(val)), 0);

	bin_free(&bin);

	END;
}

TEST(bin_get)
{
	START;

	bin_t bin = {0};
	bin_init(&bin, 1, ALLOC_STD);

	uint val = 1;
	bin_add(&bin, &val, sizeof(uint));

	EXPECT_EQ(bin_get(NULL, sizeof(uint), 0), NULL);
	log_set_quiet(0, 1);
	size_t off = bin.buf.used + 1;
	EXPECT_EQ(bin_get(&bin, sizeof(uint), &off), NULL);
	off = bin.buf.used;
	EXPECT_EQ(bin_get(&bin, 0, &off), NULL);
	log_set_quiet(0, 0);
	off = 0;
	EXPECT_EQ(*(uint *)bin_get(&bin, sizeof(uint), &off), 1);
	EXPECT_EQ(off, sizeof(uint));

	bin_free(&bin);

	END;
}

TEST(bin_get_int)
{
	START;

	bin_t bin = {0};
	bin_init(&bin, 1, ALLOC_STD);

	uint val = 1;
	bin_add(&bin, &val, sizeof(uint));

	EXPECT_EQ(bin_get_int(NULL, NULL, sizeof(uint), 0), 1);
	log_set_quiet(0, 1);
	size_t off = bin.buf.used + 1;
	uint got;
	EXPECT_EQ(bin_get_int(&bin, &got, sizeof(uint), &off), 1);
	log_set_quiet(0, 0);
	off = 0;
	EXPECT_EQ(bin_get_int(&bin, &got, sizeof(uint), &off), 0);
	EXPECT_EQ(got, 1);

	bin_free(&bin);

	END;
}

STEST(bin)
{
	SSTART;

	RUN(bin_init_free);
	RUN(bin_resize);
	RUN(bin_cmp);
	RUN(bin_add);
	RUN(bin_get);
	RUN(bin_get_int);

	SEND;
}
