#include "strbuf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strbuf_init_free)
{
	START;

	strbuf_t strbuf = {0};

	EXPECT_EQ(strbuf_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_init(&strbuf, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(strbuf_init(&strbuf, 1, ALLOC_STD), &strbuf);

	EXPECT_NE(strbuf.data, NULL);
	EXPECT_EQ(strbuf.size, sizeof(char) * 1);
	EXPECT_EQ(strbuf.used, 0);

	strbuf_free(&strbuf);

	EXPECT_EQ(strbuf.data, NULL);
	EXPECT_EQ(strbuf.size, 0);
	EXPECT_EQ(strbuf.used, 0);

	END;
}

TEST(strbuf_add_get)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(strbuf_add(&strbuf, "abc", 3, NULL), 1);
	EXPECT_EQ(strbuf_add(&strbuf, "de", 2, NULL), 1);
	mem_oom(0);
	size_t index;
	EXPECT_EQ(strbuf_add(&strbuf, "abc", 3, NULL), 0);
	EXPECT_EQ(strbuf_add(&strbuf, "de", 2, &index), 0);

	EXPECT_EQ(index, sizeof(uint8_t) + 3);

	uint start;
	uint len;

	strbuf_get(&strbuf, 0, start, len);
	EXPECT_EQ(start, 1);
	EXPECT_EQ(len, 3);

	strbuf_get(&strbuf, 4, start, len);
	EXPECT_EQ(start, 5);
	EXPECT_EQ(len, 2);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_foreach)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 1, ALLOC_STD);

	strbuf_add(&strbuf, "abc", 3, NULL);
	strbuf_add(&strbuf, "ab", 2, NULL);

	size_t index = 0;
	uint start;
	uint len;

	strbuf_foreach(&strbuf, index, start, len)
	{
		if (index == 0) {
			EXPECT_EQ(start, 1);
			EXPECT_EQ(len, 3);
		} else {
			EXPECT_EQ(start, 5);
			EXPECT_EQ(len, 2);
		}
	}

	strbuf_free(&strbuf);

	END;
}

STEST(strbuf)
{
	SSTART;

	RUN(strbuf_init_free);
	RUN(strbuf_add_get);
	RUN(strbuf_foreach);

	SEND;
}