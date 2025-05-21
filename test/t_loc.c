#include "loc.h"

#include "test.h"

TEST(loc_eq)
{
	START;

	EXPECT_EQ(loc_eq(LOC(0, 0), LOC(0, 0)), 1);
	EXPECT_EQ(loc_eq(LOC(0, 0), LOC(1, 0)), 0);
	EXPECT_EQ(loc_eq(LOC(0, 0), LOC(0, 1)), 0);
	EXPECT_EQ(loc_eq(LOC(1, 1), LOC(1, 1)), 1);

	END;
}

STEST(loc)
{
	SSTART;
	RUN(loc_eq);
	SEND;
}
