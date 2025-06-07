#include "path.h"

#include "log.h"
#include "platform.h"
#include "test.h"

TEST(path_init)
{
	START;

	EXPECT_EQ(path_init(NULL, STRV_NULL), NULL);

	END;
}

TEST(path_init_s)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_init_s(NULL, STRV_NULL, 0), NULL);
	EXPECT_EQ(path_init_s(NULL, STRVN(NULL, 1), 0), NULL);
	EXPECT_EQ(path_init_s(&path, STRVN("", 256), 0), NULL);
	EXPECT_EQ(path_init_s(&path, STRV("."), 0), &path);
	EXPECT_EQ(path.len, 1);
	EXPECT_EQ(path_init_s(&path, STRV_NULL, 0), &path);
	EXPECT_EQ(path.len, 0);
	EXPECT_EQ(path_init_s(&path, STRV(""), 0), &path);
	EXPECT_EQ(path.len, 0);
	EXPECT_EQ(path_init_s(&path, STRV("a/b"), '/'), &path);
	EXPECT_STRN(path.data, "a/b", path.len);
	EXPECT_EQ(path_init_s(&path, STRV("a/b"), '\\'), &path);
	EXPECT_STRN(path.data, "a\\b", path.len);

	END;
}

TEST(path_push)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a/"));

	EXPECT_EQ(path_push(NULL, STRV_NULL), NULL);
	EXPECT_EQ(path_push(&path, STRV_NULL), NULL);
	EXPECT_EQ(path_push(&path, STRV("b")), &path);

	EXPECT_EQ(path.len, 3);

	END;
}

TEST(path_push_s)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a"));

	EXPECT_EQ(path_push_s(NULL, STRV_NULL, 0), NULL);
	EXPECT_EQ(path_push_s(&path, STRV_NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(path_push_s(&path, STRV("b"), '/'), &path);
	log_set_quiet(0, 0);

	EXPECT_EQ(path.len, 3);
	EXPECT_STR(path.data, "a/b");

	END;
}

TEST(path_pop)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a/b\\c"));
	path.data[1] = '/';
	path.data[3] = '\\';

	EXPECT_EQ(path_pop(NULL), NULL);
	EXPECT_EQ(path_pop(&path), &path);
	EXPECT_STR(path.data, "a/b\\");
	EXPECT_EQ(path_pop(&path), &path);
	EXPECT_STR(path.data, "a/");
	EXPECT_EQ(path_pop(&path), &path);
	EXPECT_STR(path.data, "");

	END;
}

TEST(path_set_len)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a"));

	EXPECT_EQ(path_set_len(NULL, 0), NULL);
	EXPECT_EQ(path_set_len(&path, 0), &path);

	EXPECT_EQ(path.len, 0);
	EXPECT_STR(path.data, "");

	END;
}

TEST(path_ends)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a/b/c"));

	EXPECT_EQ(path_ends(NULL, STRV_NULL), 0);
	EXPECT_EQ(path_ends(&path, STRV_NULL), 1);
	EXPECT_EQ(path_ends(&path, STRV("c")), 1);

	END;
}

TEST(path_calc_rel)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_calc_rel(STRV_NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(path_calc_rel(STRV("a/b/c/"), STRV_NULL, NULL), 1);
	EXPECT_EQ(path_calc_rel(STRV("a/b/c/"), STRV("a/b/c/d/"), NULL), 1);

	EXPECT_EQ(path_calc_rel(STRV("a"), STRV("a"), &path), 0);
	EXPECT_STR(path.data, "");

	EXPECT_EQ(path_calc_rel(STRV("/a"), STRV("/a"), &path), 0);
	EXPECT_STR(path.data, "");

	EXPECT_EQ(path_calc_rel(STRV("a"), STRV("b"), &path), 0);
	EXPECT_STR(path.data, "b");

	EXPECT_EQ(path_calc_rel(STRV("/a"), STRV("/b"), &path), 0);
	EXPECT_STR(path.data, "b");

	EXPECT_EQ(path_calc_rel(STRV("a/"), STRV("a/"), &path), 0);
	EXPECT_STR(path.data, "");

	EXPECT_EQ(path_calc_rel(STRV("/a/"), STRV("/a/"), &path), 0);
	EXPECT_STR(path.data, "");

	EXPECT_EQ(path_calc_rel(STRV("a/"), STRV("b/"), &path), 0);
	EXPECT_STR(path.data, ".." SEP "b/");

	EXPECT_EQ(path_calc_rel(STRV("a/"), STRV("a/b/"), &path), 0);
	EXPECT_STR(path.data, "b/");

	EXPECT_EQ(path_calc_rel(STRV("a/b"), STRV("a/"), &path), 0);
	EXPECT_STR(path.data, "");

	EXPECT_EQ(path_calc_rel(STRV("a/b"), STRV("a"), &path), 0);
	EXPECT_STR(path.data, ".." SEP "a");

	EXPECT_EQ(path_calc_rel(STRV("a/b/"), STRV("a/"), &path), 0);
	EXPECT_STR(path.data, ".." SEP);

	EXPECT_EQ(path_calc_rel(STRV("a/b/"), STRV("a"), &path), 0);
	EXPECT_STR(path.data, ".." SEP ".." SEP "a");

	EXPECT_EQ(path_calc_rel(STRV("a/b"), STRV("a/bc"), &path), 0);
	EXPECT_STR(path.data, "bc");

	EXPECT_EQ(path_calc_rel(STRV("a/bc"), STRV("a/b"), &path), 0);
	EXPECT_STR(path.data, "b");

	EXPECT_EQ(path_calc_rel(STRV("/"), STRV("/a"), &path), 0);
	EXPECT_STR(path.data, "a");

	EXPECT_EQ(path_calc_rel(STRV("/a"), STRV("/"), &path), 0);
	EXPECT_STR(path.data, "");

	END;
}

TEST(pathv_is_rel)
{
	START;

	EXPECT_EQ(pathv_is_rel(STRV_NULL), 0);
#if defined(C_WIN)
	EXPECT_EQ(pathv_is_rel(STRV("C:")), 0);
	EXPECT_EQ(pathv_is_rel(STRV("a")), 1);
	EXPECT_EQ(pathv_is_rel(STRV("abc")), 1);
#else
	EXPECT_EQ(pathv_is_rel(STRV("/")), 0);
	EXPECT_EQ(pathv_is_rel(STRV(".")), 1);
#endif

	END;
}

TEST(pathv_lsplit)
{
	START;

	strv_t l, r;

	EXPECT_EQ(pathv_lsplit(STRV_NULL, NULL, NULL), 1);

	EXPECT_EQ(pathv_lsplit(STRV("a" SEP "b" SEP "c"), &l, &r), 0);
	EXPECT_STRN(l.data, "a", l.len);
	EXPECT_STRN(r.data, "b" SEP "c", r.len);

	END;
}

TEST(pathv_rsplit)
{
	START;

	strv_t l, r;

	EXPECT_EQ(pathv_rsplit(STRV_NULL, NULL, NULL), 1);

	EXPECT_EQ(pathv_rsplit(STRV("a" SEP "b" SEP "c"), &l, &r), 0);
	EXPECT_STRN(l.data, "a" SEP "b", l.len);
	EXPECT_STRN(r.data, "c", r.len);

	END;
}

STEST(path_merge)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_merge(NULL, STRV_NULL), NULL);

	path_init_s(&path, STRV(""), '/');
	EXPECT_EQ(path_merge(&path, STRV_NULL), &path);
	EXPECT_STR(path.data, "");

	path_init_s(&path, STRV("/"), '/');
	EXPECT_EQ(path_merge(&path, STRV_NULL), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV_NULL), &path);
	EXPECT_STR(path.data, "/a");

	path_init_s(&path, STRV("/"), '/');
	EXPECT_EQ(path_merge(&path, STRV(".")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV(".")), &path);
	EXPECT_STR(path.data, "/a");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV("./")), &path);
	EXPECT_STR(path.data, "/a");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV(".")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("./")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a/b"), '/');
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV("b")), &path);
	EXPECT_STR(path.data, "/a" SEP "b");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("b")), &path);
	EXPECT_STR(path.data, "/a/b");

	path_init_s(&path, STRV("/a"), '/');
	EXPECT_EQ(path_merge(&path, STRV("b")), &path);
	EXPECT_STR(path.data, "/a" SEP "b");

	path_init_s(&path, STRV("/a/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("./b")), &path);
	EXPECT_STR(path.data, "/a/b");

	path_init_s(&path, STRV("/a/b"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../c")), &path);
	EXPECT_STR(path.data, "/a/c");

	path_init_s(&path, STRV("/a/b/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../c")), &path);
	EXPECT_STR(path.data, "/a/c");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../..")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../../")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/c/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../..")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/c/"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../../")), &path);
	EXPECT_STR(path.data, "/a/");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("../../e/f")), &path);
	EXPECT_STR(path.data, "/a/e/f");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV(".e/f")), &path);
	EXPECT_STR(path.data, "/a/b/c" SEP ".e/f");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("..e/f")), &path);
	EXPECT_STR(path.data, "/a/b/c" SEP "..e/f");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("/")), &path);
	EXPECT_STR(path.data, "/");

	path_init_s(&path, STRV("/a/b/c"), '/');
	EXPECT_EQ(path_merge(&path, STRV("/d")), &path);
	EXPECT_STR(path.data, "/d");
	END;
}

STEST(path)
{
	SSTART;
	RUN(path_init);
	RUN(path_init_s);
	RUN(path_push);
	RUN(path_push_s);
	RUN(path_pop);
	RUN(path_set_len);
	RUN(path_ends);
	RUN(path_calc_rel);
	RUN(pathv_is_rel);
	RUN(pathv_lsplit);
	RUN(pathv_rsplit);
	RUN(path_merge);
	SEND;
}
