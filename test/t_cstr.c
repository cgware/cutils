#include "cstr.h"

#include "test.h"

#include <wchar.h>

TEST(cstrv)
{
	START;

	char buf[9] = {0};

	EXPECT_EQ(cstrf(NULL, 0, NULL), 0);
	EXPECT_EQ(cstrf(buf, 0, NULL), 0);
	EXPECT_EQ(cstrf(buf, sizeof(buf), NULL), 0);
	EXPECT_EQ(cstrf(buf, sizeof(buf), "12345678"), 8);
	EXPECT_STR(buf, "12345678");

	END;
}

TEST(cstr_zero)
{
	START;

	char buf[8] = {0};
	cstrf(buf, sizeof(buf), "1234567");

	EXPECT_EQ(cstr_zero(NULL, 0), NULL);
	EXPECT_EQ(cstr_zero(NULL, sizeof(buf)), NULL);
	EXPECT_EQ(cstr_zero(buf, 0), buf);
	EXPECT_EQ(cstr_zero(buf, sizeof(buf)), buf);

	EXPECT_STR(buf, "");

	END;
}

TEST(cstr_len)
{
	START;

	const char *buf = "a";

	EXPECT_EQ(cstr_len(NULL), 0);
	EXPECT_EQ(cstr_len(buf), 1);

	END;
}

TEST(cstr_catn)
{
	START;

	char buf[8] = "abc";

	EXPECT_EQ(cstr_catn(NULL, 0, 0, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_catn(NULL, 0, 0, "def", 3, 0), 0);
	EXPECT_EQ(cstr_catn(buf, 0, 10, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_catn(buf, 0, 0, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_catn(buf, sizeof(buf), 3, "def", 2, 3), 5);
	EXPECT_STR(buf, "abcde");
	EXPECT_EQ(cstr_catn(buf, sizeof(buf), 3, "defghi", 6, 3), 6);
	EXPECT_STR(buf, "abcdef");
	EXPECT_EQ(cstr_catn(buf, sizeof(buf), 6, "gh", 2, 2), 6);
	EXPECT_STR(buf, "abcdef");

	END;
}

TEST(cstr_cat)
{
	START;

	char buf[8] = "abc";

	EXPECT_EQ(cstr_cat(NULL, 0, 0, NULL, 0), 0);
	EXPECT_EQ(cstr_cat(NULL, 0, 0, "def", 3), 0);
	EXPECT_EQ(cstr_cat(buf, 0, 10, NULL, 0), 0);
	EXPECT_EQ(cstr_cat(buf, 0, 0, NULL, 0), 0);
	EXPECT_EQ(cstr_cat(buf, sizeof(buf), 3, "def", 3), 6);
	EXPECT_STR(buf, "abcdef");
	EXPECT_EQ(cstr_cat(buf, sizeof(buf), 6, "gh", 2), 6);
	EXPECT_STR(buf, "abcdef");

	END;
}

TEST(cstr_cmpn)
{
	START;

	EXPECT_EQ(cstr_cmpn(NULL, 0, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_cmpn("abc", 0, NULL, 0, 0), 1);
	EXPECT_EQ(cstr_cmpn("abc", 3, NULL, 0, 0), 1);
	EXPECT_EQ(cstr_cmpn(NULL, 0, "abc", 0, 0), -1);
	EXPECT_EQ(cstr_cmpn(NULL, 0, "abc", 3, 0), -1);

	EXPECT_EQ(cstr_cmpn("a", 1, "a", 1, 1), 0);
	EXPECT_GT(cstr_cmpn("b", 1, "a", 1, 1), 0);
	EXPECT_LT(cstr_cmpn("a", 1, "b", 1, 1), 0);
	EXPECT_GT(cstr_cmpn("a", 1, "b", 0, 1), 0);
	EXPECT_LT(cstr_cmpn("ac", 2, "b", 1, 2), 0);
	EXPECT_LT(cstr_cmpn("ac", 2, "bcd", 3, 2), 0);
	EXPECT_EQ(cstr_cmpn("a", 1, "b", 0, 0), 0);
	EXPECT_EQ(cstr_cmpn("a", 1, "b", 1, 0), 0);
	EXPECT_EQ(cstr_cmpn("b", 1, "a", 1, 0), 0);

	END;
}

TEST(cstr_cmp)
{
	START;

	EXPECT_EQ(cstr_cmp(NULL, 0, NULL, 0), 0);
	EXPECT_EQ(cstr_cmp("abc", 0, NULL, 0), 1);
	EXPECT_EQ(cstr_cmp("abc", 3, NULL, 0), 1);
	EXPECT_EQ(cstr_cmp(NULL, 0, "abc", 0), -1);
	EXPECT_EQ(cstr_cmp(NULL, 0, "abc", 3), -1);

	EXPECT_EQ(cstr_cmp("a", 1, "a", 1), 0);
	EXPECT_GT(cstr_cmp("b", 1, "a", 1), 0);
	EXPECT_LT(cstr_cmp("a", 1, "b", 1), 0);
	EXPECT_GT(cstr_cmp("a", 1, "b", 0), 0);
	EXPECT_LT(cstr_cmp("ac", 2, "b", 1), 0);
	EXPECT_LT(cstr_cmp("ac", 2, "bcd", 3), 0);

	END;
}

TEST(cstr_eqn)
{
	START;

	EXPECT_EQ(cstr_eqn(NULL, 0, NULL, 0, 0), 1);
	EXPECT_EQ(cstr_eqn("abc", 0, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_eqn("abc", 3, NULL, 0, 0), 0);
	EXPECT_EQ(cstr_eqn(NULL, 0, "abc", 0, 0), 0);
	EXPECT_EQ(cstr_eqn(NULL, 0, "abc", 3, 0), 0);

	EXPECT_EQ(cstr_eqn("a", 1, "a", 1, 1), 1);
	EXPECT_EQ(cstr_eqn("b", 1, "a", 1, 1), 0);
	EXPECT_EQ(cstr_eqn("a", 1, "b", 1, 1), 0);
	EXPECT_EQ(cstr_eqn("a", 1, "b", 0, 1), 0);
	EXPECT_EQ(cstr_eqn("ac", 2, "b", 1, 2), 0);
	EXPECT_EQ(cstr_eqn("ac", 2, "bcd", 3, 2), 0);
	EXPECT_EQ(cstr_eqn("a", 1, "b", 0, 0), 1);
	EXPECT_EQ(cstr_eqn("a", 1, "b", 1, 0), 1);
	EXPECT_EQ(cstr_eqn("b", 1, "a", 1, 0), 1);

	END;
}

TEST(cstr_eq)
{
	START;

	EXPECT_EQ(cstr_eq(NULL, 0, NULL, 0), 1);
	EXPECT_EQ(cstr_eq("abc", 0, NULL, 0), 0);
	EXPECT_EQ(cstr_eq("abc", 3, NULL, 0), 0);
	EXPECT_EQ(cstr_eq(NULL, 0, "abc", 0), 0);
	EXPECT_EQ(cstr_eq(NULL, 0, "abc", 3), 0);

	EXPECT_EQ(cstr_eq("a", 1, "a", 1), 1);

	END;
}

TEST(cstr_cpy)
{
	START;

	char buf[16] = {0};

	EXPECT_EQ(cstr_cpy(NULL, 0, NULL, 0), NULL);
	EXPECT_EQ(cstr_cpy("", 0, NULL, 0), NULL);
	EXPECT_EQ(cstr_cpy(NULL, 0, "", 0), NULL);
	EXPECT_EQ(cstr_cpy(buf, 0, "", 10), NULL);

	EXPECT_EQ(cstr_cpy(buf, sizeof(buf), "abcd", 4), buf);
	EXPECT_STR(buf, "abcd");

	END;
}

TEST(cstr_chr)
{
	START;

	const char *cstr = "abcbd";

	EXPECT_EQ(cstr_chr(NULL, 0), NULL);
	EXPECT_EQ(cstr_chr(cstr, 'b'), cstr + 1);

	END;
}

TEST(cstr_rchr)
{
	START;

	const char *cstr = "abcbd";

	EXPECT_EQ(cstr_rchr(NULL, 0), NULL);
	EXPECT_EQ(cstr_rchr(cstr, 'b'), cstr + 3);

	END;
}

TEST(cstr_cstr)
{
	START;

	const char *cstr = "abcbd";

	EXPECT_EQ(cstr_cstr(NULL, NULL), NULL);
	EXPECT_EQ(cstr_cstr("", NULL), NULL);
	EXPECT_EQ(cstr_cstr(NULL, ""), NULL);
	EXPECT_EQ(cstr_cstr(cstr, "b"), cstr + 1);

	END;
}

TEST(cstr_replace_short)
{
	START;

	char cstr[32] = "ab<char>de";

	EXPECT_EQ(cstr_replace(NULL, 0, 0, NULL, 0, NULL, 0, NULL), 0);
	EXPECT_EQ(cstr_replace(cstr, 0, 10, NULL, 0, NULL, 0, NULL), 0);
	EXPECT_EQ(cstr_replace(cstr, 32, 4, "<char>", 6, "c", 1, NULL), 4);
	EXPECT_EQ(cstr_replace(cstr, 32, 10, "<char>", 6, "c", 1, NULL), 5);
	EXPECT_STR(cstr, "abcde");

	END;
}

TEST(cstr_replace_long)
{
	START;

	char cstr[32] = "ab<c>op";

	EXPECT_EQ(cstr_replace(cstr, 32, 7, "<c>", 3, "cdefghijklm", 11, NULL), 15);
	EXPECT_STR(cstr, "abcdefghijklmop");

	END;
}

TEST(cstr_replace_same)
{
	START;

	char cstr[32] = "ab<c>op";

	EXPECT_EQ(cstr_replace(cstr, 32, 7, "<c>", 3, "cde", 3, NULL), 7);
	EXPECT_STR(cstr, "abcdeop");

	END;
}

TEST(cstr_replace_short_multi)
{
	START;

	char cstr[32] = "<char>ab<char>de<char>";

	EXPECT_EQ(cstr_replace(cstr, 32, 22, "<char>", 6, "c", 1, NULL), 7);
	EXPECT_STR(cstr, "cabcdec");

	END;
}

TEST(cstr_replace_long_multi)
{
	START;

	char cstr[64] = "<c>ab<c>op<c>";

	EXPECT_EQ(cstr_replace(cstr, 64, 13, "<c>", 3, "cdefghijklm", 11, NULL), 37);
	EXPECT_STR(cstr, "cdefghijklmabcdefghijklmopcdefghijklm");

	END;
}

TEST(cstr_replace_overflow)
{
	START;

	char cstr[12] = "ab<c>de";

	size_t len = cstr_replace(cstr, 12, 7, "<c>", 3, "cdefghijklm", 11, NULL);

	EXPECT_EQ(len, 0);
	EXPECT_STR(cstr, "ab<c>de");

	END;
}

TEST(cstr_replaces)
{
	START;

	char cstr[32] = "ab<char>d<none><ignore>e<str>";

	const char *from[] = {
		"<char>",
		"<ignore>",
		"<none>",
		"<str>",
	};

	const char *to[] = {
		"c",
		NULL,
		"",
		"string",
	};

	EXPECT_EQ(cstr_replaces(NULL, 0, 0, from, to, 4, NULL), 0);
	EXPECT_EQ(cstr_replaces(cstr, 32, 29, NULL, NULL, 4, NULL), 29);

	EXPECT_EQ(cstr_replaces(cstr, 32, 29, from, to, 4, NULL), 19);
	EXPECT_STR(cstr, "abcd<ignore>estring");

	END;
}

TEST(cstr_rreplaces)
{
	START;

	char cstr[32] = "<string> world";

	const char *from[] = {
		"<word>",
		"<string>",
	};

	const char *to[] = {
		"hello",
		"string:<word>",
	};

	size_t len = cstr_rreplaces(cstr, 32, 14, from, to, 2);

	EXPECT_EQ(len, 18);
	EXPECT_STR(cstr, "string:hello world");

	END;
}

STEST(cstr_replace)
{
	SSTART;
	RUN(cstr_replace_short);
	RUN(cstr_replace_long);
	RUN(cstr_replace_same);
	RUN(cstr_replace_short_multi);
	RUN(cstr_replace_long_multi);
	RUN(cstr_replace_overflow);
	RUN(cstr_replaces);
	RUN(cstr_rreplaces);
	SEND;
}

TEST(wcstr_catn)
{
	START;

	wchar_t buf[16] = L"abc";

	EXPECT_EQ(wcstr_catn(NULL, 0, NULL, 0), NULL);
	EXPECT_EQ(wcstr_catn(NULL, 0, L"def", 3), NULL);
	EXPECT_EQ(wcstr_catn(buf, 0, NULL, 0), buf);
	EXPECT_EQ(wcstr_catn(buf, sizeof(buf), L"def", 3), buf);

	END;
}

STEST(cstr)
{
	SSTART;
	RUN(cstrv);
	RUN(cstr_zero);
	RUN(cstr_len);
	RUN(cstr_catn);
	RUN(cstr_cat);
	RUN(cstr_cmpn);
	RUN(cstr_cmp);
	RUN(cstr_eqn);
	RUN(cstr_eq);
	RUN(cstr_cpy);
	RUN(cstr_chr);
	RUN(cstr_rchr);
	RUN(cstr_cstr);
	RUN(cstr_replace);
	RUN(wcstr_catn);

	SEND;
}
