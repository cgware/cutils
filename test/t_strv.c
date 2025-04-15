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

TEST(strv_print)
{
	START;

	strv_t str = STRVN("abc\r\n\t\0", 7);

	char buf[12] = {0};
	EXPECT_EQ(strv_print(STRV_NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(strv_print(str, PRINT_DST_BUF(buf, sizeof(buf), 0)), 11);

	EXPECT_STR(buf, "abc\\r\\n\\t\\0");

	END;
}

STEST(strv)
{
	SSTART;
	RUN(strv_cstr);
	RUN(strv_eq);
	RUN(strv_cmp);
	RUN(strv_print);
	SEND;
}
