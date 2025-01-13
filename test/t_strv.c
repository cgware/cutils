#include "strv.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(strv_eq)
{
	START;

	strv_t str = STRV("abc");
	strv_t src = STRV("abc");

	EXPECT_EQ(strv_eq(STRV_NULL, src), 0);
	EXPECT_EQ(strv_eq(STRV_NULL, STRV_NULL), 0);
	EXPECT_EQ(strv_eq(str, src), 1);

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
	RUN(strv_eq);
	RUN(strv_print);
	SEND;
}
