#include "strv.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strv_cstr)
{
	START;

	EXPECT_EQ(strv_cstr(NULL).len, 0);
	EXPECT_EQ(strv_cstr("A").len, 1);

	END;
}

TEST(strv_eq)
{
	START;

	strv_t str = STRV("abc");
	strv_t src = STRV("abc");

	EXPECT_EQ(strv_eq(STRV_NULL, src), 0);
	EXPECT_EQ(strv_eq(STRV_NULL, STRV_NULL), 1);
	EXPECT_EQ(strv_eq(str, src), 1);

	END;
}

TEST(strv_cmp)
{
	START;

	EXPECT_EQ(strv_cmp(STRV_NULL, STRV_NULL), 0);
	EXPECT_EQ(strv_cmp(STRVN("abc", 0), STRV_NULL), 1);
	EXPECT_EQ(strv_cmp(STRVN("abc", 3), STRV_NULL), 1);
	EXPECT_EQ(strv_cmp(STRV_NULL, STRVN("abc", 0)), -1);
	EXPECT_EQ(strv_cmp(STRV_NULL, STRVN("abc", 3)), -1);

	EXPECT_EQ(strv_cmp(STRVN("a", 1), STRVN("a", 1)), 0);
	EXPECT_GT(strv_cmp(STRVN("b", 1), STRVN("a", 1)), 0);
	EXPECT_LT(strv_cmp(STRVN("a", 1), STRVN("b", 1)), 0);
	EXPECT_GT(strv_cmp(STRVN("a", 1), STRVN("b", 0)), 0);
	EXPECT_LT(strv_cmp(STRVN("ac", 2), STRVN("b", 1)), 0);
	EXPECT_LT(strv_cmp(STRVN("ac", 2), STRVN("bcd", 3)), 0);

	END;
}

TEST(strv_cmpn)
{
	START;

	EXPECT_EQ(strv_cmpn(STRV_NULL, STRV_NULL, 0), 0);
	EXPECT_EQ(strv_cmpn(STRVN("abc", 0), STRV_NULL, 0), 1);
	EXPECT_EQ(strv_cmpn(STRV("abc"), STRV_NULL, 0), 1);
	EXPECT_EQ(strv_cmpn(STRV_NULL, STRVN("abc", 0), 0), -1);
	EXPECT_EQ(strv_cmpn(STRV_NULL, STRV("abc"), 0), -1);
	EXPECT_EQ(strv_cmpn(STRV("a"), STRV("a"), 1), 0);
	EXPECT_GT(strv_cmpn(STRV("b"), STRV("a"), 1), 0);
	EXPECT_LT(strv_cmpn(STRV("a"), STRV("b"), 1), 0);
	EXPECT_GT(strv_cmpn(STRV("a"), STRVN("b", 0), 1), 0);
	EXPECT_LT(strv_cmpn(STRV("ac"), STRV("b"), 2), 0);
	EXPECT_LT(strv_cmpn(STRV("ac"), STRV("bcd"), 2), 0);
	EXPECT_EQ(strv_cmpn(STRV("a"), STRVN("b", 0), 0), 0);
	EXPECT_EQ(strv_cmpn(STRV("a"), STRV("b"), 0), 0);
	EXPECT_EQ(strv_cmpn(STRV("b"), STRV("a"), 0), 0);

	END;
}

TEST(strv_to_int)
{
	START;

	int res;

	EXPECT_EQ(strv_to_int(STRV_NULL, NULL), 1);
	EXPECT_EQ(strv_to_int(STRV(""), NULL), 1);
	EXPECT_EQ(strv_to_int(STRV(" "), NULL), 1);
	EXPECT_EQ(strv_to_int(STRV("0 "), NULL), 1);
	EXPECT_EQ(strv_to_int(STRV("0"), &res), 0);
	EXPECT_EQ(res, 0);
	EXPECT_EQ(strv_to_int(STRV("-1"), &res), 0);
	EXPECT_EQ(res, -1);
	EXPECT_EQ(strv_to_int(STRV("01"), &res), 0);
	EXPECT_EQ(res, 1);
	EXPECT_EQ(strv_to_int(STRV("19"), &res), 0);
	EXPECT_EQ(res, 19);
	END;
}

TEST(strv_lsplit)
{
	START;

	EXPECT_EQ(strv_lsplit(STRV_NULL, 0, NULL, NULL), 1);

	strv_t l, r;

	EXPECT_EQ(strv_lsplit(STRV(""), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "", l.len);
	EXPECT_EQ(r.data, NULL);

	EXPECT_EQ(strv_lsplit(STRV(" "), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "", l.len);
	EXPECT_STRN(r.data, "", r.len);

	EXPECT_EQ(strv_lsplit(STRV("a"), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "a", l.len);
	EXPECT_EQ(r.data, NULL);

	EXPECT_EQ(strv_lsplit(STRV("a b"), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "a", l.len);
	EXPECT_STRN(r.data, "b", r.len);

	EXPECT_EQ(strv_lsplit(STRV("a b c"), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "a", l.len);
	EXPECT_STRN(r.data, "b c", r.len);

	END;
}

TEST(strv_rsplit)
{
	START;

	EXPECT_EQ(strv_rsplit(STRV_NULL, 0, NULL, NULL), 1);

	strv_t l, r;

	EXPECT_EQ(strv_rsplit(STRV(""), ' ', &l, &r), 0);
	EXPECT_EQ(l.data, NULL);
	EXPECT_STRN(r.data, "", r.len);

	EXPECT_EQ(strv_rsplit(STRV(" "), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "", l.len);
	EXPECT_STRN(r.data, "", r.len);

	EXPECT_EQ(strv_rsplit(STRV("a"), ' ', &l, &r), 0);
	EXPECT_EQ(l.data, NULL);
	EXPECT_STRN(r.data, "a", r.len);

	EXPECT_EQ(strv_rsplit(STRV("a b"), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "a", l.len);
	EXPECT_STRN(r.data, "b", r.len);

	EXPECT_EQ(strv_rsplit(STRV("a b c"), ' ', &l, &r), 0);
	EXPECT_STRN(l.data, "a b", l.len);
	EXPECT_STRN(r.data, "c", r.len);

	END;
}

TEST(strv_print)
{
	START;

	strv_t str = STRVN("abc\r\n\t\0", 7);

	char buf[12] = {0};
	EXPECT_EQ(strv_print(STRV_NULL, DST_BUF(buf)), 0);
	EXPECT_EQ(strv_print(str, DST_BUF(buf)), 11);

	EXPECT_STR(buf, "abc\\r\\n\\t\\0");

	END;
}

STEST(strv)
{
	SSTART;
	RUN(strv_cstr);
	RUN(strv_eq);
	RUN(strv_cmp);
	RUN(strv_cmpn);
	RUN(strv_to_int);
	RUN(strv_lsplit);
	RUN(strv_rsplit);
	RUN(strv_print);
	SEND;
}
