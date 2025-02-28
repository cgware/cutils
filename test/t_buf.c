#include "buf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(buf_init_free)
{
	START;

	buf_t buf = {0};

	EXPECT_EQ(buf_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(buf_init(&buf, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(buf_init(&buf, 1, ALLOC_STD), &buf);

	EXPECT_NE(buf.data, NULL);
	EXPECT_EQ(buf.size, 1);
	EXPECT_EQ(buf.used, 0);

	buf_free(NULL);
	buf_free(&buf);

	EXPECT_EQ(buf.data, NULL);
	EXPECT_EQ(buf.size, 0);
	EXPECT_EQ(buf.used, 0);

	END;
}

TEST(buf_add)
{
	START;

	buf_t buf = {0};
	log_set_quiet(0, 1);
	buf_init(&buf, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(buf_add(&buf, NULL, 1, NULL), 1);
	mem_oom(0);
	uint val = 1;
	size_t index;
	EXPECT_EQ(buf_add(&buf, &val, sizeof(val), &index), 0);
	EXPECT_EQ(index, 0);

	buf_free(&buf);

	END;
}

TEST(buf_replace)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, "a<>c", 4, NULL);

	EXPECT_EQ(buf_replace(NULL, 0, NULL, 0, 0), NULL);

	mem_oom(1);
	EXPECT_EQ(buf_replace(&buf, 1, "bbbb", 2, 4), NULL);
	mem_oom(0);
	EXPECT_EQ(buf_replace(&buf, 1, "b", 2, 1), &buf);

	EXPECT_STRN((char *)buf.data, "abc", 3);
	EXPECT_EQ(buf.used, 3);

	buf_free(&buf);

	END;
}

TEST(buf_get)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	uint val = 1;
	size_t index;
	buf_add(&buf, &val, sizeof(uint), &index);
	*(uint *)buf_get(&buf, index) = 1;

	EXPECT_EQ(*(uint *)buf_get(&buf, 0), 1);

	buf_free(&buf);

	END;
}

TEST(buf_print)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 16, ALLOC_STD);

	for (uint i = 0; i < 8; i++) {
		buf_add(&buf, &i, sizeof(i), NULL);
	}

	char tmp[128] = {0};
	EXPECT_EQ(buf_print(NULL, PRINT_DST_BUF(tmp, sizeof(tmp), 0)), 0);
	EXPECT_EQ(buf_print(&buf, PRINT_DST_BUF(tmp, sizeof(tmp), 0)), 120);

	EXPECT_STR(tmp,
		   "00000000  00 00 00 00 01 00 00 00  02 00 00 00 03 00 00 00 \n"
		   "00000010  04 00 00 00 05 00 00 00  06 00 00 00 07 00 00 00 \n");

	buf_free(&buf);

	END;
}

STEST(buf)
{
	SSTART;

	RUN(buf_init_free);
	RUN(buf_add);
	RUN(buf_replace);
	RUN(buf_get);
	RUN(buf_print);

	SEND;
}
