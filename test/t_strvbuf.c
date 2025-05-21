#include "strvbuf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strvbuf_init_free)
{
	START;

	strvbuf_t buf = {0};

	EXPECT_EQ(strvbuf_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(strvbuf_init(&buf, 1, 0, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(strvbuf_init(&buf, 1, 1, ALLOC_STD), &buf);

	EXPECT_NE(buf.data, NULL);
	EXPECT_EQ(buf.size, sizeof(size_t) + sizeof(char) * 1);
	EXPECT_EQ(buf.used, 0);

	strvbuf_free(NULL);
	strvbuf_free(&buf);

	EXPECT_EQ(buf.data, NULL);
	EXPECT_EQ(buf.size, 0);
	EXPECT_EQ(buf.used, 0);

	END;
}

TEST(strvbuf_reset)
{
	START;

	strvbuf_t buf = {0};

	strvbuf_reset(NULL, 0);
	strvbuf_reset(&buf, 1);

	EXPECT_EQ(buf.used, 0);

	END;
}

TEST(strvbuf_add)
{
	START;

	strvbuf_t buf = {0};
	log_set_quiet(0, 1);
	strvbuf_init(&buf, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(strvbuf_add(NULL, STRV("abc"), NULL), 1);
	mem_oom(1);
	EXPECT_EQ(strvbuf_add(&buf, STRV("abc"), NULL), 1);
	mem_oom(0);
	size_t off;
	EXPECT_EQ(strvbuf_add(&buf, STRV("abc"), NULL), 0);
	EXPECT_EQ(strvbuf_add(&buf, STRV("de"), &off), 0);

	EXPECT_EQ(off, sizeof(size_t) + 3);

	strvbuf_free(&buf);

	END;
}

TEST(strvbuf_add_oom)
{
	START;

	strvbuf_t buf = {0};
	strvbuf_init(&buf, 1, 0, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(strvbuf_add(&buf, STRV("abc"), NULL), 1);
	mem_oom(0);

	strvbuf_free(&buf);

	END;
}

TEST(strvbuf_get)
{
	START;

	strvbuf_t buf = {0};
	strvbuf_init(&buf, 1, 1, ALLOC_STD);

	EXPECT_EQ(strvbuf_get(NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(strvbuf_get(&buf, 0).data, NULL);
	log_set_quiet(0, 0);

	size_t off0, off1, off2;
	EXPECT_EQ(strvbuf_add(&buf, STRV("abc"), &off0), 0);
	EXPECT_EQ(strvbuf_add(&buf, STRV("de"), &off1), 0);
	EXPECT_EQ(strvbuf_add(&buf, STRV("fg"), &off2), 0);

	{
		strv_t strv = strvbuf_get(&buf, off0);
		EXPECT_STRN(strv.data, "abc", strv.len);
	}

	{
		strv_t strv = strvbuf_get(&buf, off1);
		EXPECT_STRN(strv.data, "de", strv.len);
	}

	{
		strv_t strv = strvbuf_get(&buf, off2);
		EXPECT_STRN(strv.data, "fg", strv.len);
	}

	strvbuf_free(&buf);

	END;
}

TEST(strvbuf_set)
{
	START;

	strvbuf_t strvbuf = {0};
	strvbuf_init(&strvbuf, 2, 1, ALLOC_STD);

	EXPECT_EQ(strvbuf_set(NULL, 0, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strvbuf_set(&strvbuf, 0, STRV("a")), 1);
	log_set_quiet(0, 0);
	strvbuf_add(&strvbuf, STRV("a"), NULL);
	strvbuf_add(&strvbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strvbuf_set(&strvbuf, 0, STRV("bc")), 1);
	mem_oom(0);
	EXPECT_EQ(strvbuf_set(&strvbuf, 0, STRV("bc")), 0);

	{
		strv_t val = strvbuf_get(&strvbuf, 0);
		EXPECT_STRN(val.data, "bc", val.len);
	}
	{
		strv_t val = strvbuf_get(&strvbuf, sizeof(size_t) + 2);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strvbuf_free(&strvbuf);

	END;
}

TEST(strvbuf_app)
{
	START;

	strvbuf_t strvbuf = {0};
	strvbuf_init(&strvbuf, 2, 1, ALLOC_STD);

	EXPECT_EQ(strvbuf_app(NULL, 0, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(strvbuf_app(&strvbuf, 0, STRV("a")), 1);
	log_set_quiet(0, 0);
	strvbuf_add(&strvbuf, STRV("a"), NULL);
	strvbuf_add(&strvbuf, STRV("d"), NULL);
	mem_oom(1);
	EXPECT_EQ(strvbuf_app(&strvbuf, 0, STRV("bc")), 1);
	mem_oom(0);
	EXPECT_EQ(strvbuf_app(&strvbuf, 0, STRV("bc")), 0);

	{
		strv_t val = strvbuf_get(&strvbuf, 0);
		EXPECT_STRN(val.data, "abc", val.len);
	}
	{
		strv_t val = strvbuf_get(&strvbuf, sizeof(size_t) + 3);
		EXPECT_STRN(val.data, "d", val.len);
	}

	strvbuf_free(&strvbuf);

	END;
}

STEST(strvbuf)
{
	SSTART;

	RUN(strvbuf_init_free);
	RUN(strvbuf_reset);
	RUN(strvbuf_add);
	RUN(strvbuf_add_oom);
	RUN(strvbuf_get);
	RUN(strvbuf_set);
	RUN(strvbuf_app);

	SEND;
}
