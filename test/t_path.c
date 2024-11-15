#include "path.h"

#include "cstr.h"
#include "log.h"
#include "platform.h"
#include "test.h"

TEST(path_init)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_init(NULL, NULL, 0), NULL);
	EXPECT_EQ(path_init(&path, NULL, 0), NULL);
	EXPECT_EQ(path_init(&path, "", 1), &path);

	END;
}

TEST(path_child_s)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a"));

	EXPECT_EQ(path_child_s(NULL, NULL, 0, 0), NULL);
	EXPECT_EQ(path_child_s(&path, NULL, 0, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_s(&path, CSTR("b"), '/'), &path);
	log_set_quiet(0, 0);

	EXPECT_EQ(path.len, 3);
	EXPECT_STR(path.path, "a/b");

	END;
}

TEST(path_child)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a/"));

	EXPECT_EQ(path_child(NULL, NULL, 0), NULL);
	EXPECT_EQ(path_child(&path, NULL, 0), NULL);
	EXPECT_EQ(path_child(&path, CSTR("b")), &path);

	EXPECT_EQ(path.len, 3);

	END;
}

TEST(path_child_dir)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a"));

	EXPECT_EQ(path_child_dir(NULL, NULL, 0), NULL);
	EXPECT_EQ(path_child_dir(&path, NULL, 0), NULL);

	path_init(&path, CSTR("a"));
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_dir(&path, CSTR("b")), &path);
	log_set_quiet(0, 0);
	EXPECT_STRN(path.path, "a" SEP "b" SEP, path.len);

	path_init(&path, CSTR("a/"));
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_dir(&path, CSTR("b")), &path);
	log_set_quiet(0, 0);
	EXPECT_STRN(path.path, "a/b" SEP, path.len);

	path_init(&path, CSTR("a/"));
	EXPECT_EQ(path_child_dir(&path, CSTR("b/")), &path);
	EXPECT_STRN(path.path, "a/b/", path.len);

	END;
}

TEST(path_child_folder)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a"));

	EXPECT_EQ(path_child_folder(NULL, NULL, 0), NULL);
	EXPECT_EQ(path_child_folder(&path, NULL, 0), NULL);

	path_init(&path, CSTR("a"));
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_folder(&path, CSTR("b")), &path);
	log_set_quiet(0, 0);
	EXPECT_STRN(path.path, "a" SEP "b" SEP, path.len);

	path_init(&path, CSTR("a/"));
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_folder(&path, CSTR("b")), &path);
	log_set_quiet(0, 0);
	EXPECT_STRN(path.path, "a/b" SEP, path.len);

	path_init(&path, CSTR("a/"));
	log_set_quiet(0, 1);
	EXPECT_EQ(path_child_folder(&path, CSTR("b/")), &path);
	log_set_quiet(0, 0);
	EXPECT_STRN(path.path, "a/b/", path.len);

	END;
}

TEST(path_parent)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a/b\\c"));

	EXPECT_EQ(path_parent(NULL), NULL);
	EXPECT_EQ(path_parent(&path), &path);
	EXPECT_STR(path.path, "a/b");
	EXPECT_EQ(path_parent(&path), &path);
	EXPECT_STR(path.path, "a");
	EXPECT_EQ(path_parent(&path), NULL);

	END;
}

TEST(path_set_len)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a"));

	EXPECT_EQ(path_set_len(NULL, 0), NULL);
	EXPECT_EQ(path_set_len(&path, 0), &path);

	EXPECT_EQ(path.len, 0);
	EXPECT_STR(path.path, "");

	END;
}

TEST(path_ends)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a/b/c"));

	EXPECT_EQ(path_ends(NULL, NULL, 0), 0);
	EXPECT_EQ(path_ends(&path, NULL, 0), 1);
	EXPECT_EQ(path_ends(&path, CSTR("c")), 1);

	END;
}

TEST(path_calc_rel)
{
	START;

	path_t path = {0};

	EXPECT_EQ(path_calc_rel(NULL, 0, NULL, 0, NULL), 1);
	EXPECT_EQ(path_calc_rel(CSTR("a/b/c/"), NULL, 0, NULL), 1);
	EXPECT_EQ(path_calc_rel(CSTR("a/b/c/"), CSTR("a/b/c/d/"), NULL), 1);

	EXPECT_EQ(path_calc_rel(CSTR("a"), CSTR("a"), &path), 0);
	EXPECT_STR(path.path, "");

	EXPECT_EQ(path_calc_rel(CSTR("/a"), CSTR("/a"), &path), 0);
	EXPECT_STR(path.path, "");

	EXPECT_EQ(path_calc_rel(CSTR("a"), CSTR("b"), &path), 0);
	EXPECT_STR(path.path, "b");

	EXPECT_EQ(path_calc_rel(CSTR("/a"), CSTR("/b"), &path), 0);
	EXPECT_STR(path.path, "b");

	EXPECT_EQ(path_calc_rel(CSTR("a/"), CSTR("a/"), &path), 0);
	EXPECT_STR(path.path, "");

	EXPECT_EQ(path_calc_rel(CSTR("/a/"), CSTR("/a/"), &path), 0);
	EXPECT_STR(path.path, "");

	EXPECT_EQ(path_calc_rel(CSTR("a/"), CSTR("b/"), &path), 0);
	EXPECT_STR(path.path, ".." SEP "b/");

	EXPECT_EQ(path_calc_rel(CSTR("a/"), CSTR("a/b/"), &path), 0);
	EXPECT_STR(path.path, "b/");

	EXPECT_EQ(path_calc_rel(CSTR("a/b"), CSTR("a/"), &path), 0);
	EXPECT_STR(path.path, "");

	EXPECT_EQ(path_calc_rel(CSTR("a/b"), CSTR("a"), &path), 0);
	EXPECT_STR(path.path, ".." SEP "a");

	EXPECT_EQ(path_calc_rel(CSTR("a/b/"), CSTR("a/"), &path), 0);
	EXPECT_STR(path.path, ".." SEP);

	EXPECT_EQ(path_calc_rel(CSTR("a/b/"), CSTR("a"), &path), 0);
	EXPECT_STR(path.path, ".." SEP ".." SEP "a");

	EXPECT_EQ(path_calc_rel(CSTR("a/b"), CSTR("a/bc"), &path), 0);
	EXPECT_STR(path.path, "bc");

	EXPECT_EQ(path_calc_rel(CSTR("a/bc"), CSTR("a/b"), &path), 0);
	EXPECT_STR(path.path, "b");

	EXPECT_EQ(path_calc_rel(CSTR("/"), CSTR("/a"), &path), 0);
	EXPECT_STR(path.path, "a");

	EXPECT_EQ(path_calc_rel(CSTR("/a"), CSTR("/"), &path), 0);
	EXPECT_STR(path.path, "");

	END;
}

TEST(pathv_path)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("a/b/c"));

	pathv_t pathv = {0};

	pathv = pathv_path(NULL);
	EXPECT_EQ(pathv.path, NULL);
	EXPECT_EQ(pathv.len, 0);

	pathv = pathv_path(&path);
	EXPECT_STRN(pathv.path, "a/b/c", pathv.len);

	END;
}

TEST(pathv_get_dir)
{
	START;

	path_t path   = {0};
	pathv_t pathv = {0};

	pathv_t dir = {0};
	str_t child;

	path_init(&path, CSTR("a/b/c"));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir((pathv_t){0}, NULL);
	EXPECT_EQ(dir.path, NULL);
	EXPECT_EQ(dir.len, 0);

	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, NULL);
	EXPECT_STRN(dir.path, "a/b/", dir.len);

	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "a/b/", dir.len);
	EXPECT_STRN(child.data, "c", child.len);

	path_init(&path, CSTR("a/b/c/"));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "a/b/", dir.len);
	EXPECT_STRN(child.data, "c", child.len);

	path_init(&path, CSTR("/a"));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "/", dir.len);
	EXPECT_STRN(child.data, "a", child.len);

	path_init(&path, CSTR("/"));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "", dir.len);
	EXPECT_STRN(child.data, "", child.len);

	path_init(&path, CSTR("a"));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "", dir.len);
	EXPECT_STRN(child.data, "a", child.len);

	path_init(&path, CSTR(""));
	pathv = pathv_path(&path);
	dir   = pathv_get_dir(pathv, &child);
	EXPECT_STRN(dir.path, "", dir.len);
	EXPECT_STRN(child.data, "", child.len);

	END;
}

STEST(path)
{
	SSTART;
	RUN(path_init);
	RUN(path_child_s);
	RUN(path_child);
	RUN(path_child_dir);
	RUN(path_child_folder);
	RUN(path_parent);
	RUN(path_set_len);
	RUN(path_ends);
	RUN(path_calc_rel);
	RUN(pathv_path);
	RUN(pathv_get_dir);
	SEND;
}
