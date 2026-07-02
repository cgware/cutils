#include "buf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(buf_init_free)
{
	START;

	buf_t buf = {0};

	EXPECT_EQ(buf_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(buf_init(&buf, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(buf_init(&buf, 1, ALLOC_STD), &buf);

	EXPECT_NE(buf.data, NULL);
	EXPECT_EQ(buf.size, 1);
	EXPECT_EQ(buf.used, 0);

	buf_free(NULL);
	buf_free(&buf);

	EXPECT_EQ(buf.data, NULL);
	EXPECT_EQ(buf.size, 0);
	EXPECT_EQ(buf.used, 0);

	END;
}

TEST(buf_reset)
{
	START;

	buf_t buf = {0};

	buf_reset(NULL, 0);
	buf_reset(&buf, 1);

	EXPECT_EQ(buf.used, 0);

	END;
}

TEST(buf_resize)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_resize(NULL, 0), 1);
	EXPECT_EQ(buf_resize(&buf, 1), 0);
	mem_oom(1);
	EXPECT_EQ(buf_resize(&buf, 2), 1);
	mem_oom(0);
	EXPECT_EQ(buf_resize(&buf, 2), 0);

	EXPECT_EQ(buf.used, 0);

	buf_free(&buf);

	END;
}

TEST(buf_set)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, sizeof(uint), ALLOC_STD);

	EXPECT_EQ(buf_set(NULL, buf.size, 1, NULL), 1);
	EXPECT_EQ(buf_set(&buf, buf.size, 1, NULL), 1);
	uint val = 1;
	EXPECT_EQ(buf_set(&buf, 0, sizeof(val), &val), 0);
	EXPECT_EQ(buf_set(&buf, 1, sizeof(val), &val), 1);
	EXPECT_EQ(buf_set(&buf, 0, 1, NULL), 1);
	EXPECT_EQ(buf_set(&buf, 0, 0, NULL), 0);

	val = 2;
	buf.used = sizeof(val);
	EXPECT_EQ(buf_set(&buf, 0, sizeof(val), &val), 0);
	EXPECT_EQ(*(uint *)buf.data, 2);

	buf_free(&buf);

	END;
}

TEST(buf_add)
{
	START;

	buf_t buf = {0};
	log_set_quiet(0, 1);
	buf_init(&buf, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(buf_add(NULL, 1, NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(buf_add(&buf, 1, NULL, NULL), 1);
	mem_oom(0);
	uint val = 1;
	size_t index;
	EXPECT_EQ(buf_add(&buf, sizeof(val), &val, &index), 0);
	EXPECT_EQ(index, 0);
	EXPECT_EQ(buf_add(&buf, 1, NULL, NULL), 1);
	EXPECT_EQ(buf.used, sizeof(val));
	buf.used = (size_t)-1;
	EXPECT_EQ(buf_add(&buf, 1, &val, NULL), 1);

	buf_free(&buf);

	END;
}

TEST(buf_write_le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u32 val = 0x12345678;
	EXPECT_EQ(buf_write_le(NULL, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_write_le(&buf, NULL, sizeof(val)), 1);

	EXPECT_EQ(buf_write_le(&buf, &val, sizeof(val)), 0);
	EXPECT_EQ(buf.used, sizeof(val));
	EXPECT_EQ(((u8 *)buf.data)[0], 0x78);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u32 val = 0x12345678;
	EXPECT_EQ(buf_write_be(NULL, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_write_be(&buf, NULL, sizeof(val)), 1);

	EXPECT_EQ(buf_write_be(&buf, &val, sizeof(val)), 0);
	EXPECT_EQ(buf.used, sizeof(val));
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x78);

	buf_free(&buf);

	END;
}

TEST(buf_write_u8le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u8le(NULL, 0x12), 1);
	EXPECT_EQ(buf_write_u8le(&buf, 0x12), 0);
	EXPECT_EQ(buf.used, 1);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_u16le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u16le(&buf, 0x1234), 0);
	EXPECT_EQ(buf.used, 2);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_u32le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u32le(&buf, 0x12345678), 0);
	EXPECT_EQ(buf.used, 4);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x78);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_u64le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u64le(&buf, 0x123456789abcdef0), 0);
	EXPECT_EQ(buf.used, 8);
	EXPECT_EQ(((u8 *)buf.data)[0], 0xf0);
	EXPECT_EQ(((u8 *)buf.data)[1], 0xde);
	EXPECT_EQ(((u8 *)buf.data)[2], 0xbc);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x9a);
	EXPECT_EQ(((u8 *)buf.data)[4], 0x78);
	EXPECT_EQ(((u8 *)buf.data)[5], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[6], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[7], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_u8be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u8be(NULL, 0x12), 1);
	EXPECT_EQ(buf_write_u8be(&buf, 0x12), 0);
	EXPECT_EQ(buf.used, 1);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_write_u16be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u16be(&buf, 0x1234), 0);
	EXPECT_EQ(buf.used, 2);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);

	buf_free(&buf);

	END;
}

TEST(buf_write_u32be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u32be(&buf, 0x12345678), 0);
	EXPECT_EQ(buf.used, 4);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x78);

	buf_free(&buf);

	END;
}

TEST(buf_write_u64be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	EXPECT_EQ(buf_write_u64be(&buf, 0x123456789abcdef0), 0);
	EXPECT_EQ(buf.used, 8);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x78);
	EXPECT_EQ(((u8 *)buf.data)[4], 0x9a);
	EXPECT_EQ(((u8 *)buf.data)[5], 0xbc);
	EXPECT_EQ(((u8 *)buf.data)[6], 0xde);
	EXPECT_EQ(((u8 *)buf.data)[7], 0xf0);

	buf_free(&buf);

	END;
}

TEST(buf_write_oom)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u32 val = 0x12345678;
	mem_oom(1);
	EXPECT_EQ(buf_write_le(&buf, &val, sizeof(val)), 1);
	mem_oom(0);
	EXPECT_EQ(buf.used, 0);

	buf_free(&buf);

	END;
}

TEST(buf_write_overflow)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);
	buf.used = (size_t)-1;

	u8 val = 0;
	EXPECT_EQ(buf_write_le(&buf, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_write_u8le(&buf, val), 1);
	EXPECT_EQ(buf_write_be(&buf, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_write_u8be(&buf, val), 1);

	buf.used = 0;
	buf_free(&buf);

	END;
}

TEST(buf_write_typed_null)
{
	START;

	EXPECT_EQ(buf_write_u16le(NULL, 0), 1);
	EXPECT_EQ(buf_write_u32le(NULL, 0), 1);
	EXPECT_EQ(buf_write_u64le(NULL, 0), 1);
	EXPECT_EQ(buf_write_u16be(NULL, 0), 1);
	EXPECT_EQ(buf_write_u32be(NULL, 0), 1);
	EXPECT_EQ(buf_write_u64be(NULL, 0), 1);

	END;
}

TEST(buf_write_typed_overflow)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);
	buf.used = (size_t)-1;

	EXPECT_EQ(buf_write_u16le(&buf, 0), 1);
	EXPECT_EQ(buf_write_u32le(&buf, 0), 1);
	EXPECT_EQ(buf_write_u64le(&buf, 0), 1);
	EXPECT_EQ(buf_write_u16be(&buf, 0), 1);
	EXPECT_EQ(buf_write_u32be(&buf, 0), 1);
	EXPECT_EQ(buf_write_u64be(&buf, 0), 1);

	buf.used = 0;
	buf_free(&buf);

	END;
}

TEST(buf_write_resize_oom)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u32 val = 0;
	mem_oom(1);

	EXPECT_EQ(buf_write_be(&buf, &val, sizeof(val)), 1);

	buf.used = 1;
	EXPECT_EQ(buf_write_u8le(&buf, 0), 1);

	buf.used = 0;
	EXPECT_EQ(buf_write_u16le(&buf, 0), 1);
	EXPECT_EQ(buf_write_u32le(&buf, 0), 1);
	EXPECT_EQ(buf_write_u64le(&buf, 0), 1);

	buf.used = 1;
	EXPECT_EQ(buf_write_u8be(&buf, 0), 1);

	buf.used = 0;
	EXPECT_EQ(buf_write_u16be(&buf, 0), 1);
	EXPECT_EQ(buf_write_u32be(&buf, 0), 1);
	EXPECT_EQ(buf_write_u64be(&buf, 0), 1);

	mem_oom(0);
	buf_free(&buf);

	END;
}

TEST(buf_set_str)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);

	loc_t loc;
	EXPECT_EQ(buf_set_str(NULL, 0, STRV("a"), NULL), 1);
	EXPECT_EQ(buf_set_str(&buf, 0, STRV("abc"), &loc), 0);
	EXPECT_EQ(loc.off, 0);
	EXPECT_EQ(loc.len, 3);
	EXPECT_STRN(buf.data, "abc", 3);
	EXPECT_EQ(buf_set_str(&buf, 3, STRV("xy"), NULL), 1);

	buf_free(&buf);

	END;
}

TEST(buf_add_str)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);

	loc_t loc;
	EXPECT_EQ(buf_add_str(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(buf_add_str(&buf, STRV("a"), &loc), 0);
	EXPECT_EQ(loc.off, 0);
	EXPECT_EQ(loc.len, 1);
	EXPECT_EQ(buf_add_str(&buf, STRV("bc"), &loc), 0);
	EXPECT_EQ(loc.off, 1);
	EXPECT_EQ(loc.len, 2);

	buf_free(&buf);

	END;
}

TEST(buf_get)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	uint val = 1;
	size_t index;
	buf_add(&buf, sizeof(uint), &val, &index);
	*(uint *)buf_get(&buf, index) = 1;

	EXPECT_EQ(buf_get(NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(buf_get(&buf, buf.used + 1), NULL);
	log_set_quiet(0, 0);
	EXPECT_EQ(*(uint *)buf_get(&buf, 0), 1);

	buf_free(&buf);

	END;
}

TEST(buf_read)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, "abcd", NULL);

	size_t off = 0;
	EXPECT_EQ(buf_read(NULL, 1, &off), NULL);
	EXPECT_EQ(buf_read(&buf, 1, NULL), NULL);
	EXPECT_STRN(buf_read(&buf, 2, &off), "ab", 2);
	EXPECT_EQ(off, 2);
	EXPECT_EQ(buf_read(&buf, 3, &off), NULL);
	EXPECT_EQ(off, 2);

	buf_free(&buf);

	END;
}

TEST(buf_read_zero)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, "abcd", NULL);

	size_t off = 1;
	void *got  = buf_read(&buf, 0, &off);
	EXPECT_EQ(got, (void *)((u8 *)buf.data + 1));
	EXPECT_EQ(off, 1);

	buf_free(&buf);

	END;
}

TEST(buf_read_le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, (u8[]){0x78, 0x56, 0x34, 0x12}, NULL);

	u32 val	   = 0;
	size_t off = 0;
	EXPECT_EQ(buf_read_le(NULL, &off, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_read_le(&buf, NULL, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_read_le(&buf, &off, NULL, sizeof(val)), 1);

	EXPECT_EQ(buf_read_le(&buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);
	EXPECT_EQ(off, sizeof(val));

	buf_free(&buf);

	END;
}

TEST(buf_read_le_bounds)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 2, ALLOC_STD);
	buf_add(&buf, 2, (u8[]){0x12, 0x34}, NULL);

	u32 val	   = 0;
	size_t off = 0;
	EXPECT_EQ(buf_read_le(&buf, &off, &val, sizeof(val)), 1);
	EXPECT_EQ(off, 0);

	off = (size_t)-1;
	EXPECT_EQ(buf_read_le(&buf, &off, &val, 1), 1);
	EXPECT_EQ(off, (size_t)-1);

	buf_free(&buf);

	END;
}

TEST(buf_read_be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, (u8[]){0x12, 0x34, 0x56, 0x78}, NULL);

	u32 val	   = 0;
	size_t off = 0;
	EXPECT_EQ(buf_read_be(NULL, &off, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_read_be(&buf, NULL, &val, sizeof(val)), 1);
	EXPECT_EQ(buf_read_be(&buf, &off, NULL, sizeof(val)), 1);

	EXPECT_EQ(buf_read_be(&buf, &off, &val, sizeof(val)), 0);
	EXPECT_EQ(val, 0x12345678);
	EXPECT_EQ(off, sizeof(val));

	buf_free(&buf);

	END;
}

TEST(buf_read_typed_null)
{
	START;

	size_t off = 0;
	u8 u8v	   = 0;
	u16 u16v   = 0;
	u32 u32v   = 0;
	u64 u64v   = 0;

	EXPECT_EQ(buf_read_u8le(NULL, &off, &u8v), 1);
	EXPECT_EQ(buf_read_u16le(NULL, &off, &u16v), 1);
	EXPECT_EQ(buf_read_u32le(NULL, &off, &u32v), 1);
	EXPECT_EQ(buf_read_u64le(NULL, &off, &u64v), 1);
	EXPECT_EQ(buf_read_u8be(NULL, &off, &u8v), 1);
	EXPECT_EQ(buf_read_u16be(NULL, &off, &u16v), 1);
	EXPECT_EQ(buf_read_u32be(NULL, &off, &u32v), 1);
	EXPECT_EQ(buf_read_u64be(NULL, &off, &u64v), 1);

	END;
}

TEST(buf_read_typed_bounds)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);
	buf_add(&buf, 1, (u8[]){0x12}, NULL);

	u8 u8v	   = 0;
	u16 u16v   = 0;
	u32 u32v   = 0;
	u64 u64v   = 0;
	size_t off = 1;

	EXPECT_EQ(buf_read_u8le(&buf, &off, &u8v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u16le(&buf, &off, &u16v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u32le(&buf, &off, &u32v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u64le(&buf, &off, &u64v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u8be(&buf, &off, &u8v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u16be(&buf, &off, &u16v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u32be(&buf, &off, &u32v), 1);
	EXPECT_EQ(off, 1);

	off = 1;
	EXPECT_EQ(buf_read_u64be(&buf, &off, &u64v), 1);
	EXPECT_EQ(off, 1);

	buf_free(&buf);

	END;
}

TEST(buf_read_u8be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);
	buf_add(&buf, 1, (u8[]){0x12}, NULL);

	size_t off = 0;
	u8 got	   = 0;
	EXPECT_EQ(buf_read_u8be(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x12);
	EXPECT_EQ(off, 1);

	buf_free(&buf);

	END;
}

TEST(buf_read_u16be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 2, ALLOC_STD);
	buf_add(&buf, 2, (u8[]){0x12, 0x34}, NULL);

	size_t off = 0;
	u16 got	   = 0;
	EXPECT_EQ(buf_read_u16be(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x1234);
	EXPECT_EQ(off, 2);

	buf_free(&buf);

	END;
}

TEST(buf_read_u32be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, (u8[]){0x12, 0x34, 0x56, 0x78}, NULL);

	size_t off = 0;
	u32 got	   = 0;
	EXPECT_EQ(buf_read_u32be(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x12345678);
	EXPECT_EQ(off, 4);

	buf_free(&buf);

	END;
}

TEST(buf_read_u64be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 8, ALLOC_STD);
	buf_add(&buf, 8, (u8[]){0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0}, NULL);

	size_t off = 0;
	u64 got	   = 0;
	EXPECT_EQ(buf_read_u64be(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x123456789abcdef0);
	EXPECT_EQ(off, 8);

	buf_free(&buf);

	END;
}

TEST(buf_read_u8le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);
	buf_add(&buf, 1, (u8[]){0x12}, NULL);

	size_t off = 0;
	u8 got	   = 0;
	EXPECT_EQ(buf_read_u8le(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x12);
	EXPECT_EQ(off, 1);

	buf_free(&buf);

	END;
}

TEST(buf_read_u16le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 2, ALLOC_STD);
	buf_add(&buf, 2, (u8[]){0x34, 0x12}, NULL);

	size_t off = 0;
	u16 got	   = 0;
	EXPECT_EQ(buf_read_u16le(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x1234);
	EXPECT_EQ(off, 2);

	buf_free(&buf);

	END;
}

TEST(buf_read_u32le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, (u8[]){0x78, 0x56, 0x34, 0x12}, NULL);

	size_t off = 0;
	u32 got	   = 0;
	EXPECT_EQ(buf_read_u32le(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x12345678);
	EXPECT_EQ(off, 4);

	buf_free(&buf);

	END;
}

TEST(buf_read_u64le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 8, ALLOC_STD);
	buf_add(&buf, 8, (u8[]){0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12}, NULL);

	size_t off = 0;
	u64 got	   = 0;
	EXPECT_EQ(buf_read_u64le(&buf, &off, &got), 0);
	EXPECT_EQ(got, 0x123456789abcdef0);
	EXPECT_EQ(off, 8);

	buf_free(&buf);

	END;
}

TEST(buf_read_bounds)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 2, ALLOC_STD);
	buf_add(&buf, 2, (u8[]){0x12, 0x34}, NULL);

	size_t off = 1;
	u16 val	   = 0;
	EXPECT_EQ(buf_read_u16be(&buf, &off, &val), 1);
	EXPECT_EQ(off, 1);

	off = (size_t)-1;
	EXPECT_EQ(buf_read_be(&buf, &off, &val, 1), 1);
	EXPECT_EQ(off, (size_t)-1);

	buf_free(&buf);

	END;
}

TEST(buf_get_str)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	loc_t loc;
	buf_add_str(&buf, STRV("a"), &loc);

	EXPECT_EQ(buf_get_str(NULL, loc).data, NULL);
	EXPECT_EQ(buf_get_str(&buf, (loc_t){0}).data, NULL);
	EXPECT_EQ(buf_get_str(&buf, (loc_t){.off = 0, .len = 2}).data, NULL);
	strv_t str = buf_get_str(&buf, loc);
	EXPECT_STRN(buf.data, "a", str.len);

	buf_free(&buf);

	END;
}

TEST(buf_read_str)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);

	loc_t loc;
	buf_add_str(&buf, STRV("ab"), &loc);

	size_t off = 0;
	EXPECT_EQ(buf_read_str(&buf, loc, NULL).data, NULL);
	strv_t str = buf_read_str(&buf, loc, &off);
	EXPECT_STRN(str.data, "ab", str.len);
	EXPECT_EQ(off, 2);

	loc.len = 0;
	EXPECT_EQ(buf_read_str(&buf, loc, &off).data, NULL);
	EXPECT_EQ(off, 2);

	buf_free(&buf);

	END;
}

TEST(buf_cmp)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, "abcd", NULL);

	EXPECT_EQ(buf_cmp(NULL, 0, 0, NULL), 1);
	EXPECT_EQ(buf_cmp(&buf, 0, 0, NULL), 0);
	EXPECT_EQ(buf_cmp(&buf, 4, 0, NULL), 0);
	EXPECT_EQ(buf_cmp(&buf, 0, 1, NULL), 1);
	EXPECT_EQ(buf_cmp(&buf, 5, 0, NULL), 1);
	EXPECT_EQ(buf_cmp(&buf, 3, 2, "de"), 1);
	EXPECT_EQ(buf_cmp(&buf, (size_t)-1, 1, "a"), 1);

	EXPECT_EQ(buf_cmp(&buf, 0, 4, "abcd"), 0);
	EXPECT_EQ(buf_cmp(&buf, 1, 2, "bc"), 0);
	EXPECT_LT(buf_cmp(&buf, 0, 1, "b"), 0);
	EXPECT_GT(buf_cmp(&buf, 1, 1, "a"), 0);

	buf_free(&buf);

	END;
}

TEST(buf_replace)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);
	buf_add(&buf, 4, "a<>c", NULL);

	EXPECT_EQ(buf_replace(NULL, 0, NULL, 0, 0), NULL);
	EXPECT_EQ(buf_replace(&buf, 4, "b", 1, 1), NULL);
	buf.used = (size_t)-1;
	EXPECT_EQ(buf_replace(&buf, 0, "b", 0, 1), NULL);
	buf.used = 4;

	mem_oom(1);
	EXPECT_EQ(buf_replace(&buf, 1, "bbbb", 2, 4), NULL);
	mem_oom(0);
	EXPECT_EQ(buf_replace(&buf, 1, "b", 2, 1), &buf);
	EXPECT_EQ(buf_replace(&buf, 1, "bb", 1, 2), &buf);
	EXPECT_STRN((char *)buf.data, "abbc", 4);
	EXPECT_EQ(buf.used, 4);
	EXPECT_EQ(buf_replace(&buf, 1, NULL, 1, 0), &buf);

	EXPECT_STRN((char *)buf.data, "abc", 3);
	EXPECT_EQ(buf.used, 3);

	buf_free(&buf);

	END;
}

TEST(buf_print)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 16, ALLOC_STD);

	for (uint i = 0; i < 8; i++) {
		buf_add(&buf, sizeof(i), &i, NULL);
	}

	char tmp[128] = {0};
	EXPECT_EQ(buf_print(NULL, DST_BUF(tmp)), 0);
	EXPECT_EQ(buf_print(&buf, DST_BUF(tmp)), 120);

	EXPECT_STR(tmp,
		   "00000000  00 00 00 00 01 00 00 00  02 00 00 00 03 00 00 00 \n"
		   "00000010  04 00 00 00 05 00 00 00  06 00 00 00 07 00 00 00 \n");

	buf_free(&buf);

	END;
}

STEST(buf)
{
	SSTART;

	RUN(buf_init_free);
	RUN(buf_reset);
	RUN(buf_resize);
	RUN(buf_set);
	RUN(buf_add);
	RUN(buf_write_le);
	RUN(buf_write_be);
	RUN(buf_write_u8le);
	RUN(buf_write_u16le);
	RUN(buf_write_u32le);
	RUN(buf_write_u64le);
	RUN(buf_write_u8be);
	RUN(buf_write_u16be);
	RUN(buf_write_u32be);
	RUN(buf_write_u64be);
	RUN(buf_write_oom);
	RUN(buf_write_overflow);
	RUN(buf_write_typed_null);
	RUN(buf_write_typed_overflow);
	RUN(buf_write_resize_oom);
	RUN(buf_set_str);
	RUN(buf_add_str);
	RUN(buf_get);
	RUN(buf_read);
	RUN(buf_read_zero);
	RUN(buf_read_le);
	RUN(buf_read_le_bounds);
	RUN(buf_read_be);
	RUN(buf_read_typed_null);
	RUN(buf_read_typed_bounds);
	RUN(buf_read_u8be);
	RUN(buf_read_u16be);
	RUN(buf_read_u32be);
	RUN(buf_read_u64be);
	RUN(buf_read_u8le);
	RUN(buf_read_u16le);
	RUN(buf_read_u32le);
	RUN(buf_read_u64le);
	RUN(buf_read_bounds);
	RUN(buf_get_str);
	RUN(buf_read_str);
	RUN(buf_cmp);
	RUN(buf_replace);
	RUN(buf_print);

	SEND;
}
