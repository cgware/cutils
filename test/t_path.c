#include "path.h"

#include "log.h"
#include "platform.h"
#include "test.h"

TEST(path_init)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_init(NULL, STRV_NULL), NULL);
	EXPECT_EQ(path_init(&path, STRVN("", 256)), NULL);
	EXPECT_EQ(path_init(&path, STRV(".")), &path);
	EXPECT_EQ(path.len, 1);
	EXPECT_EQ(path_init(&path, STRV_NULL), &path);
	EXPECT_EQ(path.len, 0);
	EXPECT_EQ(path_init(&path, STRV("")), &path);
	EXPECT_EQ(path.len, 0);

	END;
}

TEST(path_child_s)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a"));

	EXPECT_EQ(path_child_s(NULL, STRV_NULL, 0), NULL);
	EXPECT_EQ(path_child_s(&path, STRV_NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_s(&path, STRV("b"), '/'), &path);
	log_set_quiet(0, 0);

	EXPECT_EQ(path.len, 3);
	EXPECT_STR(path.data, "a/b");

	END;
}

TEST(path_child)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a/"));

	EXPECT_EQ(path_child(NULL, STRV_NULL), NULL);
	EXPECT_EQ(path_child(&path, STRV_NULL), NULL);
	EXPECT_EQ(path_child(&path, STRV("b")), &path);

	EXPECT_EQ(path.len, 3);

	END;
}

TEST(path_parent)
{
	START;

	path_t path = {0};
	path_init(&path, STRV("a/b\\c"));

	EXPECT_EQ(path_parent(NULL), NULL);
	EXPECT_EQ(path_parent(&path), &path);
	EXPECT_STR(path.data, "a/b");
	EXPECT_EQ(path_parent(&path), &path);
	EXPECT_STR(path.data, "a");
	EXPECT_EQ(path_parent(&path), &path);
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

TEST(pathv_get_dir)
{
	START;

	strv_t pathv = {0};

	strv_t dir = {0};
	strv_t child;

	dir = pathv_get_dir(STRV_NULL, NULL);
	EXPECT_EQ(dir.data, NULL);
	EXPECT_EQ(dir.len, 0);

	pathv = STRV("a/b/c");
	dir   = pathv_get_dir(pathv, NULL);
	EXPECT_STRN(dir.data, "a/b/", dir.len);

	pathv = STRV("a/b/c");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "a/b/", dir.len);
	EXPECT_STRN(child.data, "c", child.len);

	pathv = STRV("a/b/c/");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "a/b/", dir.len);
	EXPECT_STRN(child.data, "c", child.len);

	pathv = STRV("/a");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "/", dir.len);
	EXPECT_STRN(child.data, "a", child.len);

	pathv = STRV("/");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "", dir.len);
	EXPECT_STRN(child.data, "", child.len);

	pathv = STRV("a");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "", dir.len);
	EXPECT_STRN(child.data, "a", child.len);

	pathv = STRV("");
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.data, "", dir.len);
	EXPECT_STRN(child.data, "", child.len);

	END;
}

STEST(path_merge)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_merge(NULL, STRV_NULL), NULL);

	path_init(&path, STRV(""));
	EXPECT_EQ(path_merge(&path, STRV_NULL), &path);
	EXPECT_STR(path.data, "");

	path_init(&path, STRV("/home"));
	EXPECT_EQ(path_merge(&path, STRV(".")), &path);
	EXPECT_STR(path.data, "/home");

	path_init(&path, STRV("/home/"));
	EXPECT_EQ(path_merge(&path, STRV("./")), &path);
	EXPECT_STR(path.data, "/home");

	path_init(&path, STRV("/home"));
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "");

	path_init(&path, STRV("/home/"));
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "");

	path_init(&path, STRV("/home/user"));
	EXPECT_EQ(path_merge(&path, STRV("..")), &path);
	EXPECT_STR(path.data, "/home");

	path_init(&path, STRV("/home/user/"));
	EXPECT_EQ(path_merge(&path, STRV("../")), &path);
	EXPECT_STR(path.data, "/home");

	path_init(&path, STRV("/home"));
	EXPECT_EQ(path_merge(&path, STRV("./user")), &path);
	EXPECT_STR(path.data, "/home" SEP "user");

	path_init(&path, STRV("/home/"));
	EXPECT_EQ(path_merge(&path, STRV("./user/")), &path);
	EXPECT_STR(path.data, "/home" SEP "user");

	path_init(&path, STRV("/home/user"));
	EXPECT_EQ(path_merge(&path, STRV("../temp")), &path);
	EXPECT_STR(path.data, "/home" SEP "temp");

	path_init(&path, STRV("/home/user/"));
	EXPECT_EQ(path_merge(&path, STRV("../temp/")), &path);
	EXPECT_STR(path.data, "/home" SEP "temp");

	path_init(&path, STRV("/a/b/c"));
	EXPECT_EQ(path_merge(&path, STRV("../../e/f")), &path);
	EXPECT_STR(path.data, "/a" SEP "e" SEP "f");

	END;
}

STEST(path)
{
	SSTART;
	RUN(path_init);
	RUN(path_child_s);
	RUN(path_child);
	RUN(path_parent);
	RUN(path_set_len);
	RUN(path_ends);
	RUN(path_calc_rel);
	RUN(pathv_is_rel);
	RUN(pathv_get_dir);
	RUN(path_merge);
	SEND;
}
