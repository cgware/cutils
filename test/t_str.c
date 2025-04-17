#include "str.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strz)
{
	START;

	log_set_quiet(0, 1);
	strz(0);
	log_set_quiet(0, 0);
	str_t str = strz(1);

	str_free(&str);

	END;
}

TEST(strn)
{
	START;

	strn(NULL, 0, 0);

	strn("abc", 3, 1);
	str_t str = strn("abc", 2, 16);

	EXPECT_STR(str.data, "ab");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 2);

	str_free(&str);

	END;
}

TEST(strf)
{
	START;

	strf(NULL);

	mem_oom(1);
	EXPECT_EQ(strf("%s", "a").data, NULL);
	mem_oom(0);

	str_t str = strf("%s", "a");

	EXPECT_STR(str.data, "a");
	EXPECT_EQ(str.size, 2);
	EXPECT_EQ(str.len, 1);

	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);

	END;
}

TEST(str_free)
{
	START;

	str_t str = strz(16);

	EXPECT_STR(str.data, "");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 0);

	str_free(NULL);
	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);

	END;
}

TEST(str_zero)
{
	START;

	str_t str = strz(16);

	str_zero(NULL);
	str_zero(&str);

	str_free(&str);

	END;
}

TEST(str_resize)
{
	START;

	str_t str = strz(16);

	EXPECT_EQ(str_resize(NULL, 0), 1);
	EXPECT_EQ(str_resize(&str, 8), 0);
	mem_oom(1);
	EXPECT_EQ(str_resize(&str, 32), 1);
	mem_oom(0);
	EXPECT_EQ(str_resize(&str, 32), 0);

	str_free(&str);

	END;
}

TEST(str_cat)
{
	START;

	str_t str = STR("abc");

	EXPECT_EQ(str_cat(NULL, STRV("")), NULL);
	EXPECT_EQ(str_cat(&str, STRV_NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(str_cat(&str, STRVN("def", 2)), NULL);
	mem_oom(0);
	EXPECT_EQ(str_cat(&str, STRVN("def", 2)), &str);

	EXPECT_STR(str.data, "abcde");
	EXPECT_EQ(str.size, 6);
	EXPECT_EQ(str.len, 5);

	str_free(&str);

	END;
}

TEST(str_to_upper)
{
	START;

	str_t dst = strz(5);

	EXPECT_EQ(str_to_upper(STRV_NULL, NULL), 1);
	EXPECT_EQ(str_to_upper(STRV("abc;"), &dst), 0);

	EXPECT_STR(dst.data, "ABC;");
	EXPECT_EQ(dst.len, 4);

	str_free(&dst);

	END;
}

TEST(str_replace)
{
	START;

	char buf[32] = "ab<char>de";

	str_t str = STRB(buf, 10);
	int found = 0;

	EXPECT_EQ(str_replace(NULL, STRV_NULL, STRV_NULL, NULL), 1);

	EXPECT_EQ(str_replace(&str, STRV("<_>"), STRV("c"), &found), 0);
	EXPECT_EQ(found, 0);
	EXPECT_STR(str.data, "ab<char>de");

	EXPECT_EQ(str_replace(&str, STRV("<char>"), STRV("c"), &found), 0);
	EXPECT_EQ(found, 1);
	EXPECT_STR(str.data, "abcde");

	EXPECT_EQ(str_replace(&str, STRV("c"), STRV("<char>"), &found), 0);
	EXPECT_EQ(found, 1);
	EXPECT_STR(str.data, "ab<char>de");

	END;
}

TEST(str_replace_oom)
{
	START;

	char buf[2] = "a";

	str_t str = STRB(buf, 1);
	int found = 0;

	EXPECT_EQ(str_replace(&str, STRV("a"), STRV("bb"), &found), 1);
	EXPECT_EQ(found, 1);
	EXPECT_STR(str.data, "a");

	END;
}

TEST(str_replaces)
{
	START;

	char buf[32] = "ab<char>d<none><ignore>e<str>";

	str_t str = STRB(buf, 29);
	int found = 0;

	EXPECT_EQ(str_replaces(NULL, NULL, NULL, 0, NULL), 1);
	EXPECT_EQ(str_replaces(&str, NULL, NULL, 0, NULL), 1);

	const strv_t none[] = {
		STRVT("<_>"),
		STRVT("<_>"),
		STRVT("<_>"),
		STRVT("<_>"),
	};

	const strv_t from[] = {
		STRVT("<char>"),
		STRVT("<ignore>"),
		STRVT("<none>"),
		STRVT("<str>"),
	};

	const strv_t to[] = {
		STRVT("c"),
		STRV_NULL,
		STRVT(""),
		STRVT("string"),
	};

	EXPECT_EQ(str_replaces(&str, none, to, 4, &found), 0);
	EXPECT_EQ(found, 0);
	EXPECT_STR(str.data, "ab<char>d<none><ignore>e<str>");

	EXPECT_EQ(str_replaces(&str, from, to, 4, &found), 0);
	EXPECT_EQ(found, 1);
	EXPECT_STR(str.data, "abcd<ignore>estring");

	END;
}

TEST(str_replaces_oom)
{
	START;

	char buf[2] = "a";

	str_t str = STRB(buf, 1);
	int found = 0;

	const strv_t from[] = {
		STRVT("a"),
	};

	const strv_t to[] = {
		STRVT("bb"),
	};

	EXPECT_EQ(str_replaces(&str, from, to, 4, &found), 1);
	EXPECT_EQ(found, 1);
	EXPECT_STR(str.data, "a");

	END;
}

TEST(str_rreplaces)
{
	START;

	char buf[32] = "<string> world";

	str_t str = STRB(buf, 14);

	EXPECT_EQ(str_rreplaces(NULL, NULL, NULL, 0), 1);
	EXPECT_EQ(str_rreplaces(&str, NULL, NULL, 0), 1);

	const strv_t from[] = {
		STRVT("<word>"),
		STRVT("<string>"),
	};

	const strv_t to[] = {
		STRVT("hello"),
		STRVT("string:<word>"),
	};

	EXPECT_EQ(str_rreplaces(&str, from, to, 2), 0);
	EXPECT_STR(str.data, "string:hello world");

	END;
}

TEST(str_subreplace)
{
	START;

	char buf[12] = "ab<char>de";

	str_t str = STRB(buf, 10);

	EXPECT_EQ(str_subreplace(NULL, 0, 0, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(str_subreplace(&str, 2, 8, STRV("ccccccccccc")), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(str_subreplace(&str, 2, 8, STRV("c")), 0);
	EXPECT_STRN(str.data, "abcde", str.len);

	END;
}

STEST(str)
{
	SSTART;
	RUN(strz);
	RUN(strn);
	RUN(strf);
	RUN(str_free);
	RUN(str_zero);
	RUN(str_resize);
	RUN(str_cat);
	RUN(str_to_upper);
	RUN(str_replace);
	RUN(str_replace_oom);
	RUN(str_replaces);
	RUN(str_replaces_oom);
	RUN(str_rreplaces);
	RUN(str_subreplace);
	SEND;
}
