#include "cbuf.h"

#include "test.h"

TEST(cbuf_set_le)
{
	START;

	u8 buf[4] = {0};
	u32 val	  = 0x12345678;

	EXPECT_EQ(cbuf_set_le(buf, 0, &val, sizeof(val)), 0);
	EXPECT_EQ(buf[0], 0x78);
	EXPECT_EQ(buf[1], 0x56);
	EXPECT_EQ(buf[2], 0x34);
	EXPECT_EQ(buf[3], 0x12);

	END;
}

TEST(cbuf_set_be)
{
	START;

	u8 buf[4] = {0};
	u32 val	  = 0x12345678;

	EXPECT_EQ(cbuf_set_be(buf, 0, &val, sizeof(val)), 0);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);
	EXPECT_EQ(buf[2], 0x56);
	EXPECT_EQ(buf[3], 0x78);

	END;
}

TEST(cbuf_set_u8le)
{
	START;

	u8 buf[1] = {0};

	EXPECT_EQ(cbuf_set_u8le(buf, 0, 0x12), 0);
	EXPECT_EQ(buf[0], 0x12);

	END;
}

TEST(cbuf_set_u16le)
{
	START;

	u8 buf[2] = {0};

	EXPECT_EQ(cbuf_set_u16le(buf, 0, 0x1234), 0);
	EXPECT_EQ(buf[0], 0x34);
	EXPECT_EQ(buf[1], 0x12);

	END;
}

TEST(cbuf_set_u32le)
{
	START;

	u8 buf[4] = {0};

	EXPECT_EQ(cbuf_set_u32le(buf, 0, 0x12345678), 0);
	EXPECT_EQ(buf[0], 0x78);
	EXPECT_EQ(buf[1], 0x56);
	EXPECT_EQ(buf[2], 0x34);
	EXPECT_EQ(buf[3], 0x12);

	END;
}

TEST(cbuf_set_u64le)
{
	START;

	u8 buf[8] = {0};

	EXPECT_EQ(cbuf_set_u64le(buf, 0, 0x123456789abcdef0), 0);
	EXPECT_EQ(buf[0], 0xf0);
	EXPECT_EQ(buf[1], 0xde);
	EXPECT_EQ(buf[2], 0xbc);
	EXPECT_EQ(buf[3], 0x9a);
	EXPECT_EQ(buf[4], 0x78);
	EXPECT_EQ(buf[5], 0x56);
	EXPECT_EQ(buf[6], 0x34);
	EXPECT_EQ(buf[7], 0x12);

	END;
}

TEST(cbuf_set_u8be)
{
	START;

	u8 buf[1] = {0};

	EXPECT_EQ(cbuf_set_u8be(buf, 0, 0x12), 0);
	EXPECT_EQ(buf[0], 0x12);

	END;
}

TEST(cbuf_set_u16be)
{
	START;

	u8 buf[2] = {0};

	EXPECT_EQ(cbuf_set_u16be(buf, 0, 0x1234), 0);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);

	END;
}

TEST(cbuf_set_u32be)
{
	START;

	u8 buf[4] = {0};

	EXPECT_EQ(cbuf_set_u32be(buf, 0, 0x12345678), 0);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);
	EXPECT_EQ(buf[2], 0x56);
	EXPECT_EQ(buf[3], 0x78);

	END;
}

TEST(cbuf_set_u64be)
{
	START;

	u8 buf[8] = {0};

	EXPECT_EQ(cbuf_set_u64be(buf, 0, 0x123456789abcdef0), 0);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);
	EXPECT_EQ(buf[2], 0x56);
	EXPECT_EQ(buf[3], 0x78);
	EXPECT_EQ(buf[4], 0x9a);
	EXPECT_EQ(buf[5], 0xbc);
	EXPECT_EQ(buf[6], 0xde);
	EXPECT_EQ(buf[7], 0xf0);

	END;
}

TEST(cbuf_get_le)
{
	START;

	u8 buf[] = {0x78, 0x56, 0x34, 0x12};
	u32 val = 0;

	EXPECT_EQ(cbuf_get_le(buf, 0, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);

	END;
}

TEST(cbuf_get_be)
{
	START;

	u8 buf[] = {0x12, 0x34, 0x56, 0x78};
	u32 val = 0;

	EXPECT_EQ(cbuf_get_be(buf, 0, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);

	END;
}

TEST(cbuf_get_u8le)
{
	START;

	u8 buf[] = {0x12};
	u8 val	 = 0;

	EXPECT_EQ(cbuf_get_u8le(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x12);

	END;
}

TEST(cbuf_get_u16le)
{
	START;

	u8 buf[] = {0x34, 0x12};
	u16 val = 0;

	EXPECT_EQ(cbuf_get_u16le(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x1234);

	END;
}

TEST(cbuf_get_u32le)
{
	START;

	u8 buf[] = {0x78, 0x56, 0x34, 0x12};
	u32 val = 0;

	EXPECT_EQ(cbuf_get_u32le(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x12345678);

	END;
}

TEST(cbuf_get_u64le)
{
	START;

	u8 buf[] = {0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12};
	u64 val = 0;

	EXPECT_EQ(cbuf_get_u64le(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x123456789abcdef0);

	END;
}

TEST(cbuf_get_u8be)
{
	START;

	u8 buf[] = {0x12};
	u8 val	 = 0;

	EXPECT_EQ(cbuf_get_u8be(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x12);

	END;
}

TEST(cbuf_get_u16be)
{
	START;

	u8 buf[] = {0x12, 0x34};
	u16 val = 0;

	EXPECT_EQ(cbuf_get_u16be(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x1234);

	END;
}

TEST(cbuf_get_u32be)
{
	START;

	u8 buf[] = {0x12, 0x34, 0x56, 0x78};
	u32 val = 0;

	EXPECT_EQ(cbuf_get_u32be(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x12345678);

	END;
}

TEST(cbuf_get_u64be)
{
	START;

	u8 buf[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	u64 val = 0;

	EXPECT_EQ(cbuf_get_u64be(buf, 0, &val), 0);
	EXPECT_EQ(val, 0x123456789abcdef0);

	END;
}

TEST(cbuf_write_le)
{
	START;

	u8 buf[4] = {0};
	u32 val	  = 0x12345678;
	size_t off = 0;

	EXPECT_EQ(cbuf_write_le(buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(off, sizeof(val));
	EXPECT_EQ(buf[0], 0x78);
	EXPECT_EQ(buf[1], 0x56);
	EXPECT_EQ(buf[2], 0x34);
	EXPECT_EQ(buf[3], 0x12);

	END;
}

TEST(cbuf_write_be)
{
	START;

	u8 buf[4] = {0};
	u32 val	  = 0x12345678;
	size_t off = 0;

	EXPECT_EQ(cbuf_write_be(buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(off, sizeof(val));
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);
	EXPECT_EQ(buf[2], 0x56);
	EXPECT_EQ(buf[3], 0x78);

	END;
}

TEST(cbuf_write_u16le)
{
	START;

	u8 buf[2]  = {0};
	size_t off = 0;

	EXPECT_EQ(cbuf_write_u16le(buf, &off, 0x1234), 0);
	EXPECT_EQ(off, 2);
	EXPECT_EQ(buf[0], 0x34);
	EXPECT_EQ(buf[1], 0x12);

	END;
}

TEST(cbuf_write_u16be)
{
	START;

	u8 buf[2]  = {0};
	size_t off = 0;

	EXPECT_EQ(cbuf_write_u16be(buf, &off, 0x1234), 0);
	EXPECT_EQ(off, 2);
	EXPECT_EQ(buf[0], 0x12);
	EXPECT_EQ(buf[1], 0x34);

	END;
}

TEST(cbuf_read_le)
{
	START;

	u8 buf[]   = {0x78, 0x56, 0x34, 0x12};
	u32 val	   = 0;
	size_t off = 0;

	EXPECT_EQ(cbuf_read_le(buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);
	EXPECT_EQ(off, sizeof(val));

	END;
}

TEST(cbuf_read_be)
{
	START;

	u8 buf[]   = {0x12, 0x34, 0x56, 0x78};
	u32 val	   = 0;
	size_t off = 0;

	EXPECT_EQ(cbuf_read_be(buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);
	EXPECT_EQ(off, sizeof(val));

	END;
}

TEST(cbuf_read_u16le)
{
	START;

	u8 buf[]   = {0x34, 0x12};
	u16 val	   = 0;
	size_t off = 0;

	EXPECT_EQ(cbuf_read_u16le(buf, &off, &val), 0);
	EXPECT_EQ(val, 0x1234);
	EXPECT_EQ(off, 2);

	END;
}

TEST(cbuf_read_u16be)
{
	START;

	u8 buf[]   = {0x12, 0x34};
	u16 val	   = 0;
	size_t off = 0;

	EXPECT_EQ(cbuf_read_u16be(buf, &off, &val), 0);
	EXPECT_EQ(val, 0x1234);
	EXPECT_EQ(off, 2);

	END;
}

STEST(cbuf)
{
	SSTART;

	RUN(cbuf_set_le);
	RUN(cbuf_set_be);
	RUN(cbuf_set_u8le);
	RUN(cbuf_set_u16le);
	RUN(cbuf_set_u32le);
	RUN(cbuf_set_u64le);
	RUN(cbuf_set_u8be);
	RUN(cbuf_set_u16be);
	RUN(cbuf_set_u32be);
	RUN(cbuf_set_u64be);
	RUN(cbuf_get_le);
	RUN(cbuf_get_be);
	RUN(cbuf_get_u8le);
	RUN(cbuf_get_u16le);
	RUN(cbuf_get_u32le);
	RUN(cbuf_get_u64le);
	RUN(cbuf_get_u8be);
	RUN(cbuf_get_u16be);
	RUN(cbuf_get_u32be);
	RUN(cbuf_get_u64be);
	RUN(cbuf_write_le);
	RUN(cbuf_write_be);
	RUN(cbuf_write_u16le);
	RUN(cbuf_write_u16be);
	RUN(cbuf_read_le);
	RUN(cbuf_read_be);
	RUN(cbuf_read_u16le);
	RUN(cbuf_read_u16be);

	SEND;
}
