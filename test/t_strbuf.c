#include "strbuf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strbuf_init_free)
{
	START;

	strbuf_t strbuf = {0};

	EXPECT_EQ(strbuf_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_init(&strbuf, 1, 1, ALLOC_STD), NULL);
	EXPECT_EQ(strbuf_init(&strbuf, 1, 0, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(strbuf_init(&strbuf, 1, 1, ALLOC_STD), &strbuf);

	EXPECT_NE(strbuf.buf.data, NULL);
	EXPECT_EQ(strbuf.buf.size, sizeof(char) * 1);
	EXPECT_EQ(strbuf.buf.used, 0);

	strbuf_free(NULL);
	strbuf_free(&strbuf);

	EXPECT_EQ(strbuf.buf.data, NULL);
	EXPECT_EQ(strbuf.buf.size, 0);
	EXPECT_EQ(strbuf.buf.used, 0);

	END;
}

TEST(strbuf_reset)
{
	START;

	strbuf_t strbuf = {0};

	strbuf_reset(NULL, 0);
	strbuf_reset(&strbuf, 1);

	EXPECT_EQ(strbuf.off.cnt, 0);

	END;
}

TEST(strbuf_add)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(strbuf_add(NULL, STRV("abc"), NULL), 1);
	mem_oom(1);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("abc"), NULL), 1);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("de"), NULL), 1);
	mem_oom(0);
	uint index;
	EXPECT_EQ(strbuf_add(&strbuf, STRV("abc"), NULL), 0);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("de"), &index), 0);

	EXPECT_EQ(index, 1);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_add_off)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 1, 16, ALLOC_STD);

	EXPECT_EQ(strbuf_add(&strbuf, STRV("abc"), NULL), 0);
	mem_oom(1);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("de"), NULL), 1);
	mem_oom(0);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_get)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 1, 1, ALLOC_STD);

	EXPECT_EQ(strbuf_get(NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_get(&strbuf, 0).data, NULL);
	log_set_quiet(0, 0);

	EXPECT_EQ(strbuf_add(&strbuf, STRV("abc"), NULL), 0);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("de"), NULL), 0);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("fg"), NULL), 0);

	{
		strv_t strv = strbuf_get(&strbuf, 0);
		EXPECT_STRN(strv.data, "abc", strv.len);
		EXPECT_EQ(strv.len, 3);
	}

	{
		strv_t strv = strbuf_get(&strbuf, 1);
		EXPECT_STRN(strv.data, "de", strv.len);
		EXPECT_EQ(strv.len, 2);
	}

	{
		strv_t strv = strbuf_get(&strbuf, 2);
		EXPECT_STRN(strv.data, "fg", strv.len);
		EXPECT_EQ(strv.len, 2);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_get_index)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	strbuf_add(&strbuf, STRV("abc"), NULL);
	strbuf_add(&strbuf, STRV("de"), NULL);

	uint index;

	EXPECT_EQ(strbuf_get_index(NULL, STRV("abc"), &index), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_get_index(&strbuf, STRVN("abc", 2), &index), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(strbuf_get_index(&strbuf, STRV("de"), &index), 0);
	EXPECT_EQ(index, 1);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_set)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 2, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(strbuf_set(NULL, STRV_NULL, 0), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_set(&strbuf, STRV("a"), 0), 1);
	log_set_quiet(0, 0);
	strbuf_add(&strbuf, STRV("a"), NULL);
	strbuf_add(&strbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_set(&strbuf, STRV("bc"), 0), 1);
	mem_oom(0);
	EXPECT_EQ(strbuf_set(&strbuf, STRV("bc"), 0), 0);

	{
		strv_t val = strbuf_get(&strbuf, 0);

		EXPECT_EQ(val.len, 2);
		EXPECT_STRN(val.data, "bc", val.len);
	}
	{
		strv_t val = strbuf_get(&strbuf, 1);

		EXPECT_EQ(val.len, 1);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_app)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 2, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(strbuf_app(NULL, STRV_NULL, 0), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_app(&strbuf, STRV("a"), 0), 1);
	log_set_quiet(0, 0);
	strbuf_add(&strbuf, STRV("a"), NULL);
	strbuf_add(&strbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_app(&strbuf, STRV("bc"), 0), 1);
	mem_oom(0);
	EXPECT_EQ(strbuf_app(&strbuf, STRV("bc"), 0), 0);

	{
		strv_t val = strbuf_get(&strbuf, 0);

		EXPECT_EQ(val.len, 3);
		EXPECT_STRN(val.data, "abc", val.len);
	}
	{
		strv_t val = strbuf_get(&strbuf, 1);

		EXPECT_EQ(val.len, 1);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_foreach)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 1, 1, ALLOC_STD);

	uint index = 0;
	strv_t strv;

	strbuf_foreach(&strbuf, index, strv);

	strbuf_add(&strbuf, STRV("abc"), NULL);
	strbuf_add(&strbuf, STRVN("de", 2), NULL);

	strbuf_foreach(&strbuf, index, strv)
	{
		if (index == 0) {
			EXPECT_STRN(strv.data, "abc", strv.len);
			EXPECT_EQ(strv.len, 3)
		} else {
			EXPECT_STRN(strv.data, "de", strv.len);
			EXPECT_EQ(strv.len, 2);
		}
	}

	strbuf_free(&strbuf);

	END;
}

STEST(strbuf)
{
	SSTART;

	RUN(strbuf_init_free);
	RUN(strbuf_reset);
	RUN(strbuf_add);
	RUN(strbuf_add_off);
	RUN(strbuf_get);
	RUN(strbuf_get_index);
	RUN(strbuf_set);
	RUN(strbuf_app);
	RUN(strbuf_foreach);

	SEND;
}
