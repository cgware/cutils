#include "file.h"

#include "cstr.h"
#include "log.h"
#include "print.h"
#include "test.h"

#include <string.h>

#define TEST_FILE "t_file.txt"

TEST(file_open_close)
{
	START;

	EXPECT_EQ(file_open(NULL, NULL), NULL);
	EXPECT_EQ(file_open(TEST_FILE, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_open("not.txt", "r"), NULL);
	log_set_quiet(0, 0);
	FILE *file = file_open(TEST_FILE, "w+");

	EXPECT_NE(file, 0);

	EXPECT_NE(file_close(NULL), 0);
	EXPECT_EQ(file_close(file), 0);

	file_delete(TEST_FILE);

	END;
}

TEST(file_open_f)
{
	START;

	EXPECT_EQ(file_open_f("%s",
			      "w+",
			      "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"
			      "901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"
			      "78901234567890123456789012345678901234abcde"),
		  NULL);
	FILE *file = file_open_f("%s", "w+", TEST_FILE);
	EXPECT_NE(file, NULL);

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_reopen)
{
	START;

	FILE *file = file_open(TEST_FILE, "w+");

	EXPECT_EQ(file_reopen(NULL, NULL, NULL), NULL);
	EXPECT_EQ(file_reopen(TEST_FILE, NULL, NULL), NULL);
	EXPECT_EQ(file_reopen(TEST_FILE, "w+", NULL), NULL);
	EXPECT_EQ(file_reopen(TEST_FILE, "w+", file), file);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_reopen("not.txt", "r", file), NULL);
	log_set_quiet(0, 0);

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_read)
{
	START;

	FILE *file = file_open(TEST_FILE, "wb+");

	char data[64] = {0};

	c_fprintf(file, "");
	EXPECT_EQ(file_read_ft(file, data, sizeof(data)), 0);

	c_fprintf(file, "Test");
	EXPECT_EQ(file_read(NULL, 0, NULL, 0), 0);
	EXPECT_EQ(file_read(file, 0, NULL, 0), 0);
	EXPECT_EQ(file_read(file, 10, NULL, 0), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_read(file, 10, data, 0), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(file_read(file, 10, data, sizeof(data)), 4);
	EXPECT_STR(data, "Test");

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_read_t)
{
	START;

	FILE *file = file_open(TEST_FILE, "w+");
	c_fprintf(file, "Test");
	file_close(file);

	char data[64] = {0};

	EXPECT_EQ(file_read_t(NULL, NULL, 0), (size_t)-1);
	EXPECT_EQ(file_read_t(TEST_FILE, NULL, 0), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_read_t(TEST_FILE, data, 0), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(file_read_t(TEST_FILE, data, sizeof(data)), 4);
	EXPECT_STR(data, "Test");

	file_delete(TEST_FILE);

	END;
}

TEST(file_read_ft)
{
	START;

	FILE *file = file_open(TEST_FILE, "w+");

	c_fprintf(file, "Test");

	char data[64] = {0};

	EXPECT_EQ(file_read_ft(NULL, NULL, 0), 0);
	EXPECT_EQ(file_read_ft(file, NULL, 0), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_read_ft(file, data, 0), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(file_read_ft(file, data, sizeof(data)), 4);
	EXPECT_STR(data, "Test");

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_read_ft_r)
{
	START;

	FILE *file = file_open(TEST_FILE, "wb+");

	c_fprintf(file, "Test\r\nTest");

	char data[64] = {0};

	size_t len = file_read_ft(file, data, sizeof(data));

	EXPECT_EQ(len, 9);
	EXPECT_STR(data, "Test\nTest");

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_size)
{
	START;

	FILE *file = file_open(TEST_FILE, "w+");

	c_fprintf(file, "Test");

	EXPECT_EQ(file_size(NULL), 0);
	EXPECT_EQ(file_size(file), 4);

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(file_delete)
{
	START;

	FILE *file = file_open(TEST_FILE, "w+");

	file_close(file);

	EXPECT_NE(file_delete(NULL), 0);
	EXPECT_EQ(file_delete(TEST_FILE), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(file_delete("not.txt"), 1);
	log_set_quiet(0, 0);

	END;
}

TEST(file_exists)
{
	START;

	EXPECT_EQ(file_exists(NULL), 0);
	EXPECT_EQ(file_exists(TEST_FILE), 0);
	EXPECT_EQ(file_exists_f("%s",
				"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"
				"7890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012"
				"345678901234567890123456789012345678901234abcde"),
		  0);

	FILE *file = file_open(TEST_FILE, "w+");

	EXPECT_EQ(file_exists(TEST_FILE), 1);
	EXPECT_EQ(file_exists_f("%s", TEST_FILE), 1);

	file_close(file);
	file_delete(TEST_FILE);

	END;
}

TEST(folder_create_delete)
{
	START;

	EXPECT_EQ(folder_create(NULL), 1);
	EXPECT_EQ(folder_create_f(NULL), 1);
	EXPECT_EQ(folder_create_f("temp"), 0);

	EXPECT_EQ(folder_delete(NULL), 1);
	EXPECT_EQ(folder_delete_f(NULL), 1);
	EXPECT_EQ(folder_delete_f("temp"), 0);

	END;
}

TEST(folder_exists)
{
	START;

	EXPECT_EQ(folder_exists(NULL), 0);
	EXPECT_EQ(folder_exists("temp"), 0);
	EXPECT_EQ(folder_exists_f(NULL), 0);
	EXPECT_EQ(folder_exists_f("%s", "temp"), 0);

	folder_create_f("%s", "temp");

	EXPECT_EQ(folder_exists("temp"), 1);
	EXPECT_EQ(folder_exists_f("%s", "temp"), 1);

	folder_delete_f("%s", "temp");

	EXPECT_EQ(folder_exists("temp"), 0);
	EXPECT_EQ(folder_exists_f("%s", "temp"), 0);

	END;
}

static int file_cb(path_t *path, const char *folder, void *priv)
{
	(void)path;
	(void)folder;
	(void)priv;
	return 0;
}

static int file_err_cb(path_t *path, const char *folder, void *priv)
{
	(void)path;
	(void)folder;
	(void)priv;
	return -1;
}

static int file_cnt_cb(path_t *path, const char *folder, void *priv)
{
	(void)path;

	int *index = (int *)priv;

	const char *file = NULL;
	switch (*index) {
	case 0: file = "d.txt"; break;
	case 1: file = "e.txt"; break;
	case 2: file = "f.txt"; break;
	}

	(*index)++;
	return file != NULL && strcmp(folder, file) == 0 ? 0 : -1;
}

static int folder_cnt_cb(path_t *path, const char *folder, void *priv)
{
	(void)path;

	int *index = (int *)priv;

	const char *file = NULL;
	switch (*index) {
	case 3: file = "a"; break;
	case 4: file = "b"; break;
	case 5: file = "c"; break;
	}

	(*index)++;
	return file != NULL && strcmp(folder, file) == 0 ? 0 : -1;
}

TEST(files_foreach)
{
	START;

	path_t path = {0};
	path_init(&path, CSTR("./tmp"));
	path_t not = {0};
	path_init(&not, CSTR("not"));

	folder_create("tmp");

	folder_create("tmp/a");
	folder_create("tmp/b");
	folder_create("tmp/c");

	file_close(file_open("tmp/d.txt", "w"));
	file_close(file_open("tmp/e.txt", "w"));
	file_close(file_open("tmp/f.txt", "w"));

	int index = 0;

	EXPECT_EQ(files_foreach(NULL, NULL, NULL, NULL), 1);
	EXPECT_EQ(files_foreach(&not, file_cb, file_cb, NULL), 1);
	EXPECT_EQ(files_foreach(&path, file_cb, file_cb, NULL), 0);
	EXPECT_EQ(files_foreach(&path, file_err_cb, file_cb, NULL), -1);
	EXPECT_EQ(files_foreach(&path, file_cb, file_err_cb, NULL), -1);
	EXPECT_EQ(files_foreach(&path, folder_cnt_cb, file_cnt_cb, &index), 0);

	file_delete("tmp/f.txt");
	file_delete("tmp/e.txt");
	file_delete("tmp/d.txt");

	folder_delete("tmp/c");
	folder_delete("tmp/b");
	folder_delete("tmp/a");

	folder_delete("tmp");

	EXPECT_EQ(index, 6);

	END;
}

TESTP(c_fflush, FILE *file)
{
	START;

	EXPECT_EQ(c_fflush(NULL), 1);
	EXPECT_EQ(c_fflush(file), 0);

	END;
}

TESTP(c_fprintf, FILE *file)
{
	START;

	EXPECT_EQ(c_fprintf(NULL, NULL), 0);

	EXPECT_EQ(c_fprintf(stdout, "\n"), 1);

	file_reopen(TEST_FILE, "r", file);
	log_set_quiet(0, 1);
	EXPECT_EQ(c_fprintf(file, ""), 0);
	log_set_quiet(0, 0);

	{
		file_reopen(TEST_FILE, "wb+", file);

		EXPECT_EQ(c_fprintf(file, NULL), 0);
		EXPECT_EQ(c_fprintf(file, "test"), 4);

		char buf[64] = {0};
		file_read_ft(file, buf, sizeof(buf));

		EXPECT_STR(buf, "test");
	}

	END;
}

TESTP(c_fprintv_cb, FILE *file)
{
	START;

	EXPECT_EQ(c_fprintv_cb(PRINT_DST_NONE(), NULL, NULL), 0);
	EXPECT_EQ(c_fprintv_cb(PRINT_DST_FILE(file), NULL, NULL), 0);

	{
		file_reopen(TEST_FILE, "wb+", file);

		va_list empty = {0};
		EXPECT_EQ(c_fprintv_cb(PRINT_DST_FILE(file), "Test", empty), 4);

		char buf[16] = {0};
		file_read_ft(file, buf, sizeof(buf));

		EXPECT_STR(buf, "Test");
	}

	END;
}

STEST(file)
{
	SSTART;
	RUN(file_open_close);
	RUN(file_open_f);
	RUN(file_reopen);
	RUN(file_read);
	RUN(file_read_t);
	RUN(file_read_ft);
	RUN(file_read_ft_r);
	RUN(file_size);
	RUN(file_delete);
	RUN(file_exists);
	RUN(folder_create_delete);
	RUN(folder_exists);
	RUN(files_foreach);

	FILE *file = file_open(TEST_FILE, "wb+");

	RUNP(c_fflush, file);
	RUNP(c_fprintf, file);
	RUNP(c_fprintv_cb, file);

	file_close(file);
	file_delete(TEST_FILE);

	SEND;
}
