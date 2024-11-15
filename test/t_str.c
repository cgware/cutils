#include "str.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(str_null)
{
	START;

	EXPECT_EQ(str_null().data, NULL);

	END;
}

TEST(strz)
{
	START;

	log_set_quiet(0, 1);
	strz(0);
	log_set_quiet(0, 0);
	str_t str = strz(1);

	str_free(&str);

	END;
}

TEST(strc)
{
	START;

	strc(NULL, 0);

	END;
}

TEST(strs)
{
	START;

	strs(STR(NULL));

	END;
}

TEST(strn)
{
	START;

	strn(NULL, 0, 0);

	strn("abc", 3, 1);
	str_t str = strn("abc", 2, 16);

	EXPECT_STR(str.data, "ab");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 2);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);

	END;
}

TEST(strf)
{
	START;

	strf(NULL);

	mem_oom(1);
	EXPECT_EQ(strf("%s", "a").data, NULL);
	mem_oom(0);

	str_t str = strf("%s", "a");

	EXPECT_STR(str.data, "a");
	EXPECT_EQ(str.size, 2);
	EXPECT_EQ(str.len, 1);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 0);

	END;
}

TEST(strb)
{
	START;

	char buf[16] = "abc";
	str_t str    = strb(buf, sizeof(buf), 3);

	EXPECT_STR(str.data, "abc");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 3);
	EXPECT_EQ(str.ref, 1);

	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 1);

	END;
}

TEST(strr)
{
	START;

	str_t str = strr();

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 1);

	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 1);

	END;
}

TEST(str_free)
{
	START;

	str_t str = strz(16);

	EXPECT_STR(str.data, "");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 0);

	str_free(NULL);
	str_free(&str);

	EXPECT_STR(str.data, NULL);
	EXPECT_EQ(str.size, 0);
	EXPECT_EQ(str.len, 0);
	EXPECT_EQ(str.ref, 0);

	END;
}

TEST(str_zero)
{
	START;

	str_t str = strz(16);

	str_zero(NULL);
	str_zero(&str);

	str_free(&str);

	END;
}

TEST(str_resize)
{
	START;

	str_t str = strz(16);

	EXPECT_EQ(str_resize(NULL, 0), 1);
	EXPECT_EQ(str_resize(&str, 8), 0);
	mem_oom(1);
	EXPECT_EQ(str_resize(&str, 32), 1);
	mem_oom(0);
	EXPECT_EQ(str_resize(&str, 32), 0);

	str_free(&str);

	END;
}

TEST(str_catc)
{
	START;

	str_t ref = strc("abc", 3);
	str_t str = strn("abc", 3, 4);

	EXPECT_EQ(str_catc(NULL, "", 0), NULL);
	EXPECT_EQ(str_catc(&str, NULL, 0), NULL);
	EXPECT_EQ(str_catc(&ref, "def", 2), NULL);
	mem_oom(1);
	EXPECT_EQ(str_catc(&str, "def", 2), NULL);
	mem_oom(0);
	EXPECT_EQ(str_catc(&str, "def", 2), &str);

	EXPECT_STR(str.data, "abcde");
	EXPECT_EQ(str.size, 6);
	EXPECT_EQ(str.len, 5);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);

	END;
}

TEST(str_catn)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("def", 3, 16);

	EXPECT_EQ(str_catn(NULL, str, 0), NULL);
	EXPECT_EQ(str_catn(&str, str, 0), &str);
	EXPECT_EQ(str_catn(&str, src, 2), &str);

	EXPECT_STR(str.data, "abcde");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 5);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_cat)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("def", 3, 16);

	EXPECT_EQ(str_cat(NULL, str), NULL);
	EXPECT_EQ(str_cat(&str, src), &str);

	EXPECT_STR(str.data, "abcdef");
	EXPECT_EQ(str.size, 16);
	EXPECT_EQ(str.len, 6);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_cmpnc)
{
	START;

	str_t str = strn("abc", 3, 16);

	EXPECT_EQ(str_cmpnc(str, NULL, 0, 0), 1);
	EXPECT_EQ(str_cmpnc(str, "abc", 3, 3), 0);

	str_free(&str);

	END;
}

TEST(str_cmpc)
{
	START;

	str_t str = strn("abc", 3, 16);

	EXPECT_EQ(str_cmpc(str, NULL, 0), 1);
	EXPECT_EQ(str_cmpc(str, "abc", 3), 0);

	str_free(&str);

	END;
}

TEST(str_cmpn)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("abc", 3, 16);

	EXPECT_EQ(str_cmpn(str, src, 0), 0);
	EXPECT_EQ(str_cmpn(str, src, 3), 0);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_cmp)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("abc", 3, 16);

	EXPECT_EQ(str_cmp(str, src), 0);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_eqnc)
{
	START;

	str_t str = strn("abc", 3, 16);

	EXPECT_EQ(str_eqnc(str, NULL, 0, 0), 0);
	EXPECT_EQ(str_eqnc(str, "abc", 3, 3), 1);

	str_free(&str);

	END;
}

TEST(str_eqc)
{
	START;

	str_t str = strn("abc", 3, 16);

	EXPECT_EQ(str_eqc(str, NULL, 0), 0);
	EXPECT_EQ(str_eqc(str, "abc", 3), 1);

	str_free(&str);

	END;
}

TEST(str_eqn)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("abc", 3, 16);

	EXPECT_EQ(str_eqn(str, src, 0), 1);
	EXPECT_EQ(str_eqn(str, src, 3), 1);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_eq)
{
	START;

	str_t str = strn("abc", 3, 16);
	str_t src = strn("abc", 3, 16);

	EXPECT_EQ(str_eq(str, src), 1);

	str_free(&str);
	str_free(&src);

	END;
}

TEST(str_chr)
{
	START;

	str_t str = strc("a:b:c", 5);
	str_t l	  = {0};
	str_t r	  = {0};

	EXPECT_EQ(str_chr(str, NULL, NULL, 0), 1);
	EXPECT_EQ(str_chr(str, NULL, NULL, ':'), 0);
	EXPECT_EQ(str_chr(str, &l, &r, ':'), 0);

	EXPECT_STRN(l.data, "a", 1);
	EXPECT_EQ(l.len, 1);
	EXPECT_STRN(r.data, "b:c", 3);
	EXPECT_EQ(r.len, 3);

	END;
}

TEST(str_cstr)
{
	START;

	str_t str = strc("a:=b:=c", 7);
	str_t l	  = {0};
	str_t r	  = {0};

	EXPECT_EQ(str_cstr(str, NULL, NULL, NULL, 0), 1);
	EXPECT_EQ(str_cstr(str, NULL, NULL, ":=", 2), 0);
	EXPECT_EQ(str_cstr(str, &l, &r, ":=", 2), 0);

	EXPECT_STRN(l.data, "a", 1);
	EXPECT_EQ(l.len, 1);
	EXPECT_STRN(r.data, "b:=c", 4);
	EXPECT_EQ(r.len, 4);

	END;
}

TEST(str_cpy)
{
	START;

	str_t src = strc("abc", 3);

	str_t str = str_cpy(src);

	EXPECT_STR(str.data, "abc");
	EXPECT_EQ(str.size, 4);
	EXPECT_EQ(str.len, 3);
	EXPECT_EQ(str.ref, 0);

	str_free(&str);

	END;
}

TEST(str_cpyd)
{
	START;

	str_t src = strc("abc", 3);
	str_t dst = strz(4);

	EXPECT_EQ(str_cpyd(src, NULL), 1);
	EXPECT_EQ(str_cpyd(dst, &src), 1);
	EXPECT_EQ(str_cpyd(str_null(), &dst), 1);
	EXPECT_EQ(str_cpyd(src, &dst), 0);

	EXPECT_STR(dst.data, "abc");
	EXPECT_EQ(dst.len, 3);

	str_free(&dst);

	END;
}

TEST(str_to_upper)
{
	START;

	str_t src = strc("abc;", 4);
	str_t dst = strz(5);

	EXPECT_EQ(str_to_upper(src, NULL), 1);
	EXPECT_EQ(str_to_upper(src, &dst), 0);

	EXPECT_STR(dst.data, "ABC;");
	EXPECT_EQ(dst.len, 4);

	str_free(&dst);

	END;
}

TEST(str_split)
{
	START;

	str_t str = strn("abc", 3, 4);
	str_t ref = strc("abc", 3);

	EXPECT_EQ(str_split(str, 0, NULL, NULL), 1);
	EXPECT_EQ(str_split(str, 0, &str, NULL), 1);
	EXPECT_EQ(str_split(str, 'b', &ref, NULL), 1);
	EXPECT_EQ(str_split(str, 0, NULL, &str), 1);
	EXPECT_EQ(str_split(str, 'b', NULL, &ref), 1);

	str_free(&str);

	END;
}

TEST(str_split_ref)
{
	START;

	str_t str = strc("abc defgh ijkl", 14);
	str_t l	  = strr();
	str_t r	  = strr();

	EXPECT_EQ(str_split(str, ' ', &l, &r), 0);

	EXPECT_STRN(l.data, "abc", 3);
	EXPECT_EQ(l.len, 3);
	EXPECT_STR(r.data, "defgh ijkl");
	EXPECT_EQ(r.len, 10);

	END;
}

TEST(str_split_buf)
{
	START;

	char lbuf[18] = {"buf1: "};
	char rbuf[18] = {"buf2: "};

	str_t str = strc("abc defgh ijkl", 14);
	str_t l	  = strb(lbuf, sizeof(lbuf), 6);
	str_t r	  = strb(rbuf, sizeof(rbuf), 6);

	EXPECT_EQ(str_split(str, ' ', &l, &r), 0);

	EXPECT_STRN(l.data, "buf1: abc", 9);
	EXPECT_EQ(l.len, 9);
	EXPECT_STR(r.data, "buf2: defgh ijkl");
	EXPECT_EQ(r.len, 16);

	END;
}

TEST(str_split_own)
{
	START;

	str_t str = strc("abc defgh ijkl", 14);
	log_set_quiet(0, 1);
	str_t l = strz(0);
	str_t r = strz(0);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(str_split(str, ' ', &l, &r), 1);
	mem_oom(0);
	EXPECT_EQ(str_split(str, ' ', &l, &r), 0);

	EXPECT_STRN(l.data, "abc", 3);
	EXPECT_EQ(l.len, 3);
	EXPECT_STR(r.data, "defgh ijkl");
	EXPECT_EQ(r.len, 10);

	str_free(&str);
	str_free(&l);
	str_free(&r);

	END;
}

TEST(str_rsplit)
{
	START;

	str_t str = strc("abc defgh ijkl", 14);
	str_t ref = strc("abc", 3);
	str_t l	  = strr();
	str_t r	  = strr();

	EXPECT_EQ(str_rsplit(str, 0, NULL, NULL), 1);
	EXPECT_EQ(str_rsplit(str, 0, &str, NULL), 1);
	EXPECT_EQ(str_rsplit(str, ' ', &ref, NULL), 1);
	EXPECT_EQ(str_rsplit(str, 0, NULL, &str), 1);
	EXPECT_EQ(str_rsplit(str, ' ', NULL, &ref), 1);
	EXPECT_EQ(str_rsplit(str, ' ', &l, &r), 0);

	EXPECT_STRN(l.data, "abc defgh", 9);
	EXPECT_EQ(l.len, 9);
	EXPECT_STR(r.data, "ijkl");
	EXPECT_EQ(r.len, 4);

	str_free(&str);
	str_free(&l);
	str_free(&r);

	END;
}

TEST(str_replace)
{
	START;

	char buf[32] = "ab<char>de";

	str_t str = strb(buf, 32, 10);

	EXPECT_EQ(str_replace(NULL, str, str), 0);
	EXPECT_EQ(str_replace(&str, STR("<char>"), STR("c")), 1);

	EXPECT_STR(str.data, "abcde");

	END;
}

TEST(str_replaces)
{
	START;

	char buf[32] = "ab<char>d<none><ignore>e<str>";

	str_t str = strb(buf, 32, 29);

	EXPECT_EQ(str_replaces(NULL, NULL, NULL, 0), 0);
	EXPECT_EQ(str_replaces(&str, NULL, NULL, 0), 0);

	const str_t from[] = {
		STR("<char>"),
		STR("<ignore>"),
		STR("<none>"),
		STR("<str>"),
	};

	const str_t to[] = {
		STR("c"),
		strb(NULL, 0, 0),
		STR(""),
		STR("string"),
	};

	EXPECT_EQ(str_replaces(&str, from, to, 4), 1);

	EXPECT_STR(str.data, "abcd<ignore>estring");

	END;
}

TEST(str_rreplaces)
{
	START;

	char buf[32] = "<string> world";

	str_t str = strb(buf, 32, 14);

	EXPECT_EQ(str_rreplaces(NULL, NULL, NULL, 0), 0);
	EXPECT_EQ(str_rreplaces(&str, NULL, NULL, 0), 0);

	const str_t from[] = {
		STR("<word>"),
		STR("<string>"),
	};

	const str_t to[] = {
		STR("hello"),
		STR("string:<word>"),
	};

	EXPECT_EQ(str_rreplaces(&str, from, to, 2), 1);
	EXPECT_STR(str.data, "string:hello world");

	END;
}

TEST(str_print)
{
	START;

	str_t str = strc("abc\r\n\t\0", 7);

	char buf[12] = {0};
	EXPECT_EQ(str_print(str_null(), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(str_print(str, PRINT_DST_BUF(buf, sizeof(buf), 0)), 11);

	EXPECT_STR(buf, "abc\\r\\n\\t\\0");

	str_free(&str);

	END;
}

STEST(str)
{
	SSTART;
	RUN(str_null);
	RUN(strz);
	RUN(strc);
	RUN(strs);
	RUN(strn);
	RUN(strf);
	RUN(strb);
	RUN(strr);
	RUN(str_free);
	RUN(str_zero);
	RUN(str_resize);
	RUN(str_catc);
	RUN(str_catn);
	RUN(str_cat);
	RUN(str_cmpnc);
	RUN(str_cmpc);
	RUN(str_cmpn);
	RUN(str_cmp);
	RUN(str_eqnc);
	RUN(str_eqc);
	RUN(str_eqn);
	RUN(str_eq);
	RUN(str_chr);
	RUN(str_cstr);
	RUN(str_cpy);
	RUN(str_cpyd);
	RUN(str_to_upper);
	RUN(str_split);
	RUN(str_split_ref);
	RUN(str_split_buf);
	RUN(str_split_own);
	RUN(str_rsplit);
	RUN(str_replace);
	RUN(str_replaces);
	RUN(str_rreplaces);
	RUN(str_print);
	SEND;
}
