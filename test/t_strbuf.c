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
	EXPECT_EQ(strbuf.buf.size, sizeof(size_t) + sizeof(char) * 1);
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
	uint id;
	EXPECT_EQ(strbuf_add(&strbuf, STRV("abc"), NULL), 0);
	EXPECT_EQ(strbuf_add(&strbuf, STRV("de"), &id), 0);

	EXPECT_EQ(id, 1);

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
	}

	{
		strv_t strv = strbuf_get(&strbuf, 1);
		EXPECT_STRN(strv.data, "de", strv.len);
	}

	{
		strv_t strv = strbuf_get(&strbuf, 2);
		EXPECT_STRN(strv.data, "fg", strv.len);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_find)
{
	START;

	strbuf_t strbuf = {0};
	log_set_quiet(0, 1);
	strbuf_init(&strbuf, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	strbuf_add(&strbuf, STRV("abc"), NULL);
	strbuf_add(&strbuf, STRV("de"), NULL);

	uint id;

	EXPECT_EQ(strbuf_find(NULL, STRV("abc"), &id), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_find(&strbuf, STRVN("abc", 2), &id), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(strbuf_find(&strbuf, STRV("de"), &id), 0);
	EXPECT_EQ(id, 1);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_set)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 2, 1, ALLOC_STD);

	EXPECT_EQ(strbuf_set(NULL, 0, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_set(&strbuf, 0, STRV("a")), 1);
	log_set_quiet(0, 0);
	strbuf_add(&strbuf, STRV("a"), NULL);
	strbuf_add(&strbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_set(&strbuf, 0, STRV("bc")), 1);
	mem_oom(0);
	EXPECT_EQ(strbuf_set(&strbuf, 0, STRV("bc")), 0);

	{
		strv_t val = strbuf_get(&strbuf, 0);
		EXPECT_STRN(val.data, "bc", val.len);
	}
	{
		strv_t val = strbuf_get(&strbuf, 1);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_app)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 2, 1, ALLOC_STD);

	EXPECT_EQ(strbuf_app(NULL, 0, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strbuf_app(&strbuf, 0, STRV("a")), 1);
	log_set_quiet(0, 0);
	strbuf_add(&strbuf, STRV("a"), NULL);
	strbuf_add(&strbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strbuf_app(&strbuf, 0, STRV("bc")), 1);
	mem_oom(0);
	EXPECT_EQ(strbuf_app(&strbuf, 0, STRV("bc")), 0);

	{
		strv_t val = strbuf_get(&strbuf, 0);
		EXPECT_STRN(val.data, "abc", val.len);
	}
	{
		strv_t val = strbuf_get(&strbuf, 1);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_sort)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 4, 1, ALLOC_STD);

	strbuf_add(&strbuf, STRV("d"), NULL);
	strbuf_add(&strbuf, STRV("c"), NULL);
	strbuf_add(&strbuf, STRV("bb"), NULL);
	strbuf_add(&strbuf, STRV("a"), NULL);

	EXPECT_EQ(strbuf_sort(NULL), NULL);
	EXPECT_EQ(strbuf_sort(&strbuf), &strbuf);

	strv_t val;
	val = strbuf_get(&strbuf, 0);
	EXPECT_STRN(val.data, "a", val.len);
	val = strbuf_get(&strbuf, 1);
	EXPECT_STRN(val.data, "bb", val.len);
	val = strbuf_get(&strbuf, 2);
	EXPECT_STRN(val.data, "c", val.len);
	val = strbuf_get(&strbuf, 3);
	EXPECT_STRN(val.data, "d", val.len);

	strbuf_free(&strbuf);

	END;
}

TEST(strbuf_foreach)
{
	START;

	strbuf_t strbuf = {0};
	strbuf_init(&strbuf, 1, 1, ALLOC_STD);

	uint id = 0;
	strv_t strv;

	strbuf_foreach(&strbuf, id, strv);

	strbuf_add(&strbuf, STRV("abc"), NULL);
	strbuf_add(&strbuf, STRVN("de", 2), NULL);

	strbuf_foreach(&strbuf, id, strv)
	{
		if (id == 0) {
			EXPECT_STRN(strv.data, "abc", strv.len);
		} else {
			EXPECT_STRN(strv.data, "de", strv.len);
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
	RUN(strbuf_find);
	RUN(strbuf_set);
	RUN(strbuf_app);
	RUN(strbuf_sort);
	RUN(strbuf_foreach);

	SEND;
}
