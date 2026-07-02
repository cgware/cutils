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

TEST(buf_set_int)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 8, ALLOC_STD);

	u32 val = 0x12345678;
	EXPECT_EQ(buf_set_int(NULL, 0, sizeof(val), ENDIAN_HOST, &val), 1);
	EXPECT_EQ(buf_set_int(&buf, 0, sizeof(val), ENDIAN_HOST, NULL), 1);
	EXPECT_EQ(buf_set_int(&buf, 8, sizeof(val), ENDIAN_HOST, &val), 1);

	EXPECT_EQ(buf_set_int(&buf, 0, sizeof(val), ENDIAN_HOST, &val), 0);
	EXPECT_EQ(*(u32 *)buf.data, val);

	EXPECT_EQ(buf_set_int(&buf, 0, sizeof(val), ENDIAN_BIG, &val), 0);
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x78);

	EXPECT_EQ(buf_set_int(&buf, 4, sizeof(val), ENDIAN_LITTLE, &val), 0);
	EXPECT_EQ(((u8 *)buf.data)[4], 0x78);
	EXPECT_EQ(((u8 *)buf.data)[5], 0x56);
	EXPECT_EQ(((u8 *)buf.data)[6], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[7], 0x12);

	buf_free(&buf);

	END;
}

TEST(buf_add_int)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u16 val = 0x1234;
	EXPECT_EQ(buf_add_int(NULL, sizeof(val), ENDIAN_BIG, &val), 1);
	mem_oom(1);
	EXPECT_EQ(buf_add_int(&buf, sizeof(val), ENDIAN_BIG, &val), 1);
	mem_oom(0);
	EXPECT_EQ(buf_add_int(&buf, sizeof(val), ENDIAN_BIG, NULL), 1);
	EXPECT_EQ(buf.used, 0);

	EXPECT_EQ(buf_add_int(&buf, sizeof(val), ENDIAN_BIG, &val), 0);
	EXPECT_EQ(buf.used, sizeof(val));
	EXPECT_EQ(((u8 *)buf.data)[0], 0x12);
	EXPECT_EQ(((u8 *)buf.data)[1], 0x34);

	EXPECT_EQ(buf_add_int(&buf, sizeof(val), ENDIAN_LITTLE, &val), 0);
	EXPECT_EQ(buf.used, sizeof(val) * 2);
	EXPECT_EQ(((u8 *)buf.data)[2], 0x34);
	EXPECT_EQ(((u8 *)buf.data)[3], 0x12);
	buf.used = (size_t)-1;
	EXPECT_EQ(buf_add_int(&buf, 1, ENDIAN_HOST, &val), 1);

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

TEST(buf_get_int)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	uint val = 0x123456;
	buf_add(&buf, sizeof(uint), &val, NULL);

	EXPECT_EQ(buf_get_int(NULL, 0, sizeof(uint), ENDIAN_HOST, NULL), 1);
	log_set_quiet(0, 1);
	size_t off = buf.used + 1;
	uint got;
	EXPECT_EQ(buf_get_int(&buf, off, sizeof(uint), ENDIAN_HOST, &got), 1);
	log_set_quiet(0, 0);
	off = 0;
	EXPECT_EQ(buf_get_int(&buf, off, sizeof(uint), ENDIAN_HOST, &got), 0);
	EXPECT_EQ(got, 0x123456);
	EXPECT_EQ(buf_get_int(&buf, 1, sizeof(uint), ENDIAN_HOST, &got), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(buf_get_int(&buf, buf.used, 0, ENDIAN_HOST, &got), 1);
	log_set_quiet(0, 0);

	buf_free(&buf);

	END;
}

TEST(buf_read_int)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 4, ALLOC_STD);

	u16 val = 0x1234;
	buf_add_int(&buf, sizeof(val), ENDIAN_BIG, &val);

	EXPECT_EQ(buf_read_int(&buf, NULL, sizeof(val), ENDIAN_BIG, &val), 1);

	size_t off = 1;
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(val), ENDIAN_BIG, &val), 1);
	EXPECT_EQ(off, 1);

	off = 0;
	val = 0;
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(val), ENDIAN_BIG, &val), 0);
	EXPECT_EQ(val, 0x1234);
	EXPECT_EQ(off, sizeof(val));

	buf_free(&buf);

	END;
}

TEST(buf_read_int_be)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u8 data[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
	buf_add(&buf, sizeof(data), data, NULL);

	size_t off = 0;
	u8 got1;
	u16 got2;
	u32 got3;
	u64 got4;
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got1), ENDIAN_BIG, &got1), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got2), ENDIAN_BIG, &got2), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got3), ENDIAN_BIG, &got3), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got4), ENDIAN_BIG, &got4), 0);
	EXPECT_EQ(got1, 0x12);
	EXPECT_EQ(got2, 0x3456);
	EXPECT_EQ(got3, 0x789abcde);
	EXPECT_EQ(got4, 0xf0123456789abcde);
	EXPECT_EQ(off, sizeof(data));

	buf_free(&buf);

	END;
}

TEST(buf_read_int_le)
{
	START;

	buf_t buf = {0};
	buf_init(&buf, 1, ALLOC_STD);

	u8 data[] = {0x12, 0x56, 0x34, 0xde, 0xbc, 0x9a, 0x78, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0xf0};
	buf_add(&buf, sizeof(data), data, NULL);

	size_t off = 0;
	u8 got1;
	u16 got2;
	u32 got3;
	u64 got4;
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got1), ENDIAN_LITTLE, &got1), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got2), ENDIAN_LITTLE, &got2), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got3), ENDIAN_LITTLE, &got3), 0);
	EXPECT_EQ(buf_read_int(&buf, &off, sizeof(got4), ENDIAN_LITTLE, &got4), 0);
	EXPECT_EQ(got1, 0x12);
	EXPECT_EQ(got2, 0x3456);
	EXPECT_EQ(got3, 0x789abcde);
	EXPECT_EQ(got4, 0xf0123456789abcde);
	EXPECT_EQ(off, sizeof(data));

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
	RUN(buf_set_int);
	RUN(buf_add_int);
	RUN(buf_set_str);
	RUN(buf_add_str);
	RUN(buf_get);
	RUN(buf_read);
	RUN(buf_get_int);
	RUN(buf_read_int);
	RUN(buf_read_int_be);
	RUN(buf_read_int_le);
	RUN(buf_get_str);
	RUN(buf_read_str);
	RUN(buf_cmp);
	RUN(buf_replace);
	RUN(buf_print);

	SEND;
}
