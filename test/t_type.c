#include "test.h"

#include "type.h"

TEST(size)
{
	START;
	EXPECT_EQ(sizeof(s8), 1);
	EXPECT_EQ(sizeof(u8), 1);
	EXPECT_EQ(sizeof(s16), 2);
	EXPECT_EQ(sizeof(u16), 2);
	EXPECT_EQ(sizeof(s32), 4);
	EXPECT_EQ(sizeof(u32), 4);
	EXPECT_EQ(sizeof(s64), 8);
	EXPECT_EQ(sizeof(u64), 8);
	END;
}

STEST(type)
{
	SSTART;
	RUN(size);
	SEND;
}
