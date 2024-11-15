#include "ctime.h"

#include "test.h"

TEST(time_int)
{
	START;

	u64 start, end;

	start = c_time();

	c_sleep(1000);

	end = c_time();

	u64 eps = end - start - 1000;

	EXPECT_LT(eps, 162);

	END;
}

TEST(time_str)
{
	START;

	char buf[CTIME_BUF_SIZE] = {0};

	c_time_str(NULL);
	c_time_str(buf);

	uint y, m, d, H, M, S, U;

	EXPECT_FMT(buf, 7, "%4u-%2u-%2u %2u:%2u:%2u.%3u", &y, &m, &d, &H, &M, &S, &U);

	EXPECT_GT(y, 2022);
	EXPECT_LE(m, 12);
	EXPECT_LE(d, 31);
	EXPECT_LT(H, 24);
	EXPECT_LT(M, 60);
	EXPECT_LT(S, 60);
	EXPECT_LE(U, 999);

	END;
}

STEST(ctime)
{
	SSTART;
	RUN(time_int);
	RUN(time_str);
	SEND;
}
