#include "fs.h"

#include "cerr.h"
#include "log.h"
#include "mem.h"
#include "test.h"

#define TEST_FILE "t_fs_file.txt"
#define TEST_DIR  "t_fs_tmp"

TEST(fs_init_free)
{
	START;

	fs_t fs = {0};

	EXPECT_EQ(fs_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(fs_init(&fs, 1, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(fs_init(&fs, 1, 1, ALLOC_STD), &fs);

	fs_free(NULL);
	fs_free(&fs);

	END;
}

TEST(fs_open)
{
	START;

	EXPECT_EQ(fs_open(NULL, STRV_NULL, NULL, NULL), CERR_VAL);

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	log_set_quiet(0, 1);
	fs_init(&vfs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	void *file;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), NULL, &file), CERR_VAL);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), NULL, &file), CERR_VAL);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "w", NULL), CERR_VAL);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "w", NULL), CERR_VAL);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "w", &file), CERR_MEM);
	mem_oom(0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	size_t paths_used = vfs.paths.used;

	vfs.paths.used = 0;

	void *file;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "r", &file), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.paths.used = paths_used;

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_open_dir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	void *file;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_DIR), "r", &file), CERR_TYPE);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_DIR), "r", &file), CERR_TYPE);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_empty_path)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *file;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&fs, STRV(""), "r", &file), CERR_NOT_FOUND);
	EXPECT_EQ(fs_open(&vfs, STRV(""), "r", &file), CERR_NOT_FOUND);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "f", &file), CERR_VAL);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "f", &file), CERR_VAL);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_r_not_found)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *file;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "r", &file), CERR_NOT_FOUND);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "r", &file), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_w_does_not_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "w", &f), CERR_OK);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "w", &vf), CERR_OK);

	EXPECT_NE(f, NULL);
	EXPECT_NE(vf, NULL);

	EXPECT_EQ(fs_close(&fs, f), 0);
	EXPECT_EQ(fs_close(&vfs, vf), 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_wb_does_not_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "wb", &f), CERR_OK);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "wb", &vf), CERR_OK);

	EXPECT_NE(f, NULL);
	EXPECT_NE(vf, NULL);

	EXPECT_EQ(fs_close(&fs, f), 0);
	EXPECT_EQ(fs_close(&vfs, vf), 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_w_does_not_exist_folder)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *file;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_open(&fs, STRV(TEST_DIR "/" TEST_FILE), "w", &file), CERR_NOT_FOUND);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_DIR "/" TEST_FILE), "w", &file), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_r_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "r", &f), CERR_OK);
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "r", &vf), CERR_OK);

	EXPECT_NE(f, NULL);
	EXPECT_NE(vf, NULL);

	EXPECT_EQ(fs_close(&fs, f), 0);
	EXPECT_EQ(fs_close(&vfs, vf), 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_close)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	EXPECT_EQ(fs_close(NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_close(&fs, NULL), CERR_VAL);
	EXPECT_EQ(fs_close(&vfs, NULL), CERR_VAL);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_close_valid)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	EXPECT_EQ(fs_close(&fs, f), 0);
	EXPECT_EQ(fs_close(&vfs, vf), 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_close_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *vf;
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_close(&vfs, vf), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_free(&vfs);

	END;
}

TEST(fs_write)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "r", &f);
	fs_open(&vfs, STRV(TEST_FILE), "r", &vf);

	EXPECT_EQ(fs_write(NULL, NULL, STRV_NULL), CERR_VAL);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_write(&fs, NULL, STRV_NULL), CERR_VAL);
	EXPECT_EQ(fs_write(&vfs, NULL, STRV_NULL), CERR_VAL);
	EXPECT_EQ(fs_write(&vfs, (void *)-1, STRV("a")), CERR_NOT_FOUND);
	EXPECT_EQ(fs_write(&fs, f, STRV("a")), CERR_DESC);
	EXPECT_EQ(fs_write(&vfs, vf, STRV("a")), CERR_DESC);
	log_set_quiet(0, 0);

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_write_oom)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *vf;
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	mem_oom(1);
	EXPECT_EQ(fs_write(&vfs, vf, STRVN("a", 256)), CERR_MEM);
	mem_oom(0);

	fs_close(&vfs, vf);

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_write_valid)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	EXPECT_EQ(fs_write(&fs, f, STRV("a")), 0);
	EXPECT_EQ(fs_write(&vfs, vf, STRV("a")), 0);

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_read)
{
	START;

	EXPECT_EQ(fs_read(NULL, STRV_NULL, 0, NULL), CERR_VAL);

	END;
}

TEST(fs_read_not_found)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	char buf[1] = {0};
	str_t str   = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_read(&fs, STRV(TEST_FILE), 0, &str), CERR_NOT_FOUND);
	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 0, &str), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_read_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	char buf[1] = {0};
	str_t str   = STRB(buf, 0);

	size_t paths_used = vfs.paths.used;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 0, &str), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.paths.used = paths_used;

	fs_free(&vfs);

	END;
}

TEST(fs_read_oom)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	fs_write(&fs, f, STRV("a"));
	fs_write(&vfs, vf, STRV("a"));

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	char buf[1] = {0};
	str_t str   = STRB(buf, 0);

	mem_oom(1);
	EXPECT_EQ(fs_read(&fs, STRV(TEST_FILE), 0, &str), CERR_MEM);
	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 0, &str), CERR_MEM);
	mem_oom(0);

	fs_rmfile(&fs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_read_empty)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	char buf[1] = {0};
	str_t str   = STRB(buf, 0);

	EXPECT_EQ(fs_read(&fs, STRV(TEST_FILE), 0, &str), 0);
	EXPECT_STRN(str.data, "", str.len);

	str.len = 0;

	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 0, &str), 0);
	EXPECT_STRN(str.data, "", str.len);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_read_str)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	fs_write(&fs, f, STRV("ab"));
	fs_write(&vfs, vf, STRV("ab"));

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	char buf[4] = {0};
	str_t str   = STRB(buf, 0);

	EXPECT_EQ(fs_read(&fs, STRV(TEST_FILE), 0, &str), 0);
	EXPECT_STRN(str.data, "ab", str.len);

	str.len = 0;

	EXPECT_EQ(fs_read(&fs, STRV(TEST_FILE), 1, &str), 0);
	EXPECT_STRN(str.data, "ab", str.len);

	str.len = 0;

	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 0, &str), 0);
	EXPECT_STRN(str.data, "ab", str.len);

	str.len = 0;

	EXPECT_EQ(fs_read(&vfs, STRV(TEST_FILE), 1, &str), 0);
	EXPECT_STRN(str.data, "ab", str.len);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_du)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *f, *vf;
	fs_open(&fs, STRV(TEST_FILE), "w", &f);
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	fs_write(&fs, f, STRV("a"));
	fs_write(&vfs, vf, STRV("a"));

	EXPECT_EQ(fs_du(NULL, NULL, NULL), CERR_VAL);

	size_t size;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_du(&fs, NULL, NULL), CERR_VAL);
	EXPECT_EQ(fs_du(&vfs, NULL, NULL), CERR_VAL);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_du(&fs, f, &size), CERR_OK);
	EXPECT_EQ(size, 1);
	EXPECT_EQ(fs_du(&vfs, vf, &size), CERR_OK);
	EXPECT_EQ(size, 1);

	fs_close(&fs, f);
	fs_close(&vfs, vf);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_du_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *vf;
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	size_t size;
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_du(&vfs, vf, &size), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_close(&vfs, vf);

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_isdir)
{
	START;

	EXPECT_EQ(fs_isdir(NULL, STRV_NULL), 0);

	END;
}

TEST(fs_isdir_dir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_DIR)), 1);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 1);

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_isdir_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_FILE)), 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_isdir_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&vfs, STRV(TEST_DIR));

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 0);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&vfs);

	END;
}

TEST(fs_isfile)
{
	START;

	EXPECT_EQ(fs_isfile(NULL, STRV_NULL), 0);

	END;
}

TEST(fs_isfile_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 1);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 1);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_isfile_dir)
{
	START;

	EXPECT_EQ(fs_isfile(NULL, STRV_NULL), 0);

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_DIR)), 0);

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_isfile_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 0);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_mkdir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	log_set_quiet(0, 1);
	fs_init(&vfs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	EXPECT_EQ(fs_mkdir(NULL, STRV_NULL), CERR_VAL);

	mem_oom(1);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), CERR_MEM);
	mem_oom(0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkdir(&fs, STRV_NULL), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&vfs, STRV_NULL), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&fs, STRV("")), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&vfs, STRV("")), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR "/" TEST_DIR)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR "/" TEST_DIR)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_FILE "/" TEST_DIR)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_FILE "/" TEST_DIR)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), CERR_OK);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), CERR_OK);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_mkdir_oom)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	size_t paths_size = vfs.paths.size;
	vfs.paths.size	  = 0;

	mem_oom(1);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), CERR_MEM);
	mem_oom(0);

	vfs.paths.size = paths_size;

	fs_mkdir(&vfs, STRV(TEST_DIR));

	EXPECT_EQ(vfs.nodes.cnt, 1);

	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&vfs);

	END;
}

TEST(fs_mkdir_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), CERR_EXIST);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), CERR_EXIST);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_mkdir_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_DIR));
	fs_mkfile(&vfs, STRV(TEST_DIR));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), CERR_EXIST);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), CERR_EXIST);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_DIR));
	fs_rmfile(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_mkfile)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	log_set_quiet(0, 1);
	fs_init(&vfs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_mkfile(NULL, STRV_NULL), CERR_VAL);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV_NULL), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&vfs, STRV_NULL), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_MEM);
	mem_oom(0);

	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), CERR_OK);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_OK);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV("")), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&vfs, STRV("")), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_DIR "/" TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_DIR "/" TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE "/" TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE "/" TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_mkfile_oom)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	size_t paths_size = vfs.paths.size;
	vfs.paths.size	  = 0;

	mem_oom(1);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_MEM);
	mem_oom(0);

	vfs.paths.size = paths_size;

	fs_mkfile(&vfs, STRV(TEST_FILE));

	EXPECT_EQ(vfs.nodes.cnt, 1);

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_mkfile_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), CERR_EXIST);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_EXIST);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_mkfile_dir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_FILE));
	fs_mkdir(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), CERR_TYPE);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_TYPE);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_FILE));
	fs_rmdir(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmdir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	EXPECT_EQ(fs_rmdir(NULL, STRV_NULL), CERR_VAL);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), 0);

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmdir_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&vfs, STRV(TEST_FILE));

	size_t paths_used = vfs.paths.used;

	vfs.paths.used = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.paths.used = paths_used;

	fs_rmdir(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_rmdir_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_DIR));
	fs_mkfile(&vfs, STRV(TEST_DIR));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_DIR));
	fs_rmfile(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmdir_not_found)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmdir_not_empty)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	fs_mkdir(&fs, STRV(TEST_DIR "/a"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/a"));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), CERR_NOT_EMPTY);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), CERR_NOT_EMPTY);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_DIR "/a"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/a"));

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmfile)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	EXPECT_EQ(fs_rmfile(NULL, STRV_NULL), CERR_VAL);

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), 0);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_DIR)), 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmfile_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

TEST(fs_rmfile_dir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_FILE));
	fs_mkdir(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_FILE));
	fs_rmdir(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmfile_not_found)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_dir_mk_is_rm)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 0);

	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), 0);

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_DIR)), 1);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 1);

	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), 0);

	EXPECT_EQ(fs_isdir(&fs, STRV(TEST_DIR)), 0);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_file_mk_is_rm)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 0);

	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), CERR_EXIST);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), CERR_EXIST);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 1);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 1);

	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_getcwd)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	char buf[256];
	str_t path = STRB(buf, sizeof(buf));

	str_t path_null = STR_NULL;
	str_t path_val	= {.data = buf, .size = 0};
	str_t path_oom	= {.data = buf, .size = 1};

	EXPECT_EQ(fs_getcwd(NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_getcwd(&fs, NULL), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&vfs, NULL), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&fs, &path_null), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&vfs, &path_null), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&fs, &path_val), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&vfs, &path_val), CERR_VAL);
	EXPECT_EQ(fs_getcwd(&fs, &path_oom), CERR_MEM);
	EXPECT_EQ(fs_getcwd(&vfs, &path_oom), CERR_MEM);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_getcwd(&fs, &path), CERR_OK);
	EXPECT_EQ(fs_getcwd(&vfs, &path), CERR_OK);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_lsdir)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	strbuf_t dirs  = {0};
	strbuf_t vdirs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 4, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	strbuf_init(&dirs, 0, 0, ALLOC_STD);
	strbuf_init(&vdirs, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	strv_t path = STRV(TEST_DIR);
	strv_t not  = STRV("not/");

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));
	fs_mkdir(&fs, STRV(TEST_DIR "_"));
	fs_mkdir(&vfs, STRV(TEST_DIR "_"));

	fs_mkfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/a.txt"));
	fs_mkdir(&fs, STRV(TEST_DIR "/a"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/a"));
	fs_mkdir(&fs, STRV(TEST_DIR "/b"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/b"));
	fs_mkdir(&fs, STRV(TEST_DIR "/c"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/c"));
	fs_mkdir(&fs, STRV(TEST_DIR "_/a"));
	fs_mkdir(&vfs, STRV(TEST_DIR "_/a"));

	EXPECT_EQ(fs_lsdir(&fs, STRV_NULL, NULL), CERR_VAL);
	EXPECT_EQ(fs_lsdir(&vfs, STRV_NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsdir(&fs, not, &dirs), CERR_NOT_FOUND);
	EXPECT_EQ(fs_lsdir(&vfs, not, &vdirs), CERR_NOT_FOUND);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(fs_lsdir(&fs, path, &dirs), CERR_MEM);
	EXPECT_EQ(fs_lsdir(&vfs, path, &vdirs), CERR_MEM);
	mem_oom(0);
	EXPECT_EQ(fs_lsdir(&fs, path, &dirs), 0);
	EXPECT_EQ(fs_lsdir(&vfs, path, &vdirs), 0);

	EXPECT_EQ(dirs.off.cnt, 3);
	EXPECT_EQ(vdirs.off.cnt, 3);

	strv_t dir;
	strv_t vdir;

	dir  = strbuf_get(&dirs, 0);
	vdir = strbuf_get(&vdirs, 0);
	EXPECT_STRN(dir.data, "a", dir.len);
	EXPECT_STRN(vdir.data, "a", vdir.len);
	dir  = strbuf_get(&dirs, 1);
	vdir = strbuf_get(&vdirs, 1);
	EXPECT_STRN(dir.data, "b", dir.len);
	EXPECT_STRN(vdir.data, "b", vdir.len);
	dir  = strbuf_get(&dirs, 2);
	vdir = strbuf_get(&vdirs, 2);
	EXPECT_STRN(dir.data, "c", dir.len);
	EXPECT_STRN(vdir.data, "c", vdir.len);

	strbuf_free(&dirs);
	strbuf_free(&vdirs);

	strbuf_init(&dirs, 1, 1, ALLOC_STD);
	strbuf_init(&vdirs, 1, 1, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(fs_lsdir(&fs, path, &dirs), CERR_MEM);
	EXPECT_EQ(fs_lsdir(&vfs, path, &vdirs), CERR_MEM);
	mem_oom(0);

	fs_rmdir(&fs, STRV(TEST_DIR "_/a"));
	fs_rmdir(&vfs, STRV(TEST_DIR "_/a"));
	fs_rmdir(&fs, STRV(TEST_DIR "/c"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/c"));
	fs_rmdir(&fs, STRV(TEST_DIR "/b"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/b"));
	fs_rmdir(&fs, STRV(TEST_DIR "/a"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/a"));
	fs_rmfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/a.txt"));

	fs_rmdir(&fs, STRV(TEST_DIR "_"));
	fs_rmdir(&vfs, STRV(TEST_DIR "_"));
	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	strbuf_free(&dirs);
	strbuf_free(&vdirs);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_lsdir_arr)
{
	START;

	fs_t vfs = {0};

	strbuf_t dirs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&vfs, STRV(TEST_DIR));

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsdir(&vfs, STRV(TEST_DIR), &dirs), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&vfs);

	END;
}

TEST(fs_lsdir_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	strbuf_t dirs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsdir(&fs, STRV(TEST_FILE), &dirs), CERR_TYPE);
	EXPECT_EQ(fs_lsdir(&vfs, STRV(TEST_FILE), &dirs), CERR_TYPE);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_lsfile)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	strbuf_t files	= {0};
	strbuf_t vfiles = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 4, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	strbuf_init(&files, 0, 0, ALLOC_STD);
	strbuf_init(&vfiles, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	strv_t path = STRV(TEST_DIR);
	strv_t not  = STRV("not/");

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	fs_mkdir(&fs, STRV(TEST_DIR "/a"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/a"));
	fs_mkdir(&fs, STRV(TEST_DIR "/b"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/b"));
	fs_mkfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&fs, STRV(TEST_DIR "/b.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/b.txt"));
	fs_mkfile(&fs, STRV(TEST_DIR "/c.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/c.txt"));
	fs_mkfile(&fs, STRV(TEST_DIR "/b/a.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/b/a.txt"));

	EXPECT_EQ(fs_lsfile(&fs, STRV_NULL, NULL), CERR_VAL);
	EXPECT_EQ(fs_lsfile(&vfs, STRV_NULL, NULL), CERR_VAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsfile(&fs, not, &files), CERR_NOT_FOUND);
	EXPECT_EQ(fs_lsfile(&vfs, not, &vfiles), CERR_NOT_FOUND);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(fs_lsfile(&fs, path, &files), CERR_MEM);
	EXPECT_EQ(fs_lsfile(&vfs, path, &vfiles), CERR_MEM);
	mem_oom(0);
	EXPECT_EQ(fs_lsfile(&fs, path, &files), 0);
	EXPECT_EQ(fs_lsfile(&vfs, path, &vfiles), 0);

	strv_t dir;
	strv_t vdir;

	dir  = strbuf_get(&files, 0);
	vdir = strbuf_get(&vfiles, 0);
	EXPECT_STRN(dir.data, "a.txt", dir.len);
	EXPECT_STRN(vdir.data, "a.txt", vdir.len);
	dir  = strbuf_get(&files, 1);
	vdir = strbuf_get(&vfiles, 1);
	EXPECT_STRN(dir.data, "b.txt", dir.len);
	EXPECT_STRN(vdir.data, "b.txt", vdir.len);
	dir  = strbuf_get(&files, 2);
	vdir = strbuf_get(&vfiles, 2);
	EXPECT_STRN(dir.data, "c.txt", dir.len);
	EXPECT_STRN(vdir.data, "c.txt", vdir.len);

	strbuf_free(&files);
	strbuf_free(&vfiles);

	strbuf_init(&files, 1, 5, ALLOC_STD);
	strbuf_init(&vfiles, 1, 5, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(fs_lsfile(&fs, path, &files), CERR_MEM);
	EXPECT_EQ(fs_lsfile(&vfs, path, &vfiles), CERR_MEM);
	mem_oom(0);

	fs_rmfile(&fs, STRV(TEST_DIR "/b/a.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/b/a.txt"));
	fs_rmfile(&fs, STRV(TEST_DIR "/c.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/c.txt"));
	fs_rmfile(&fs, STRV(TEST_DIR "/b.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/b.txt"));
	fs_rmfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/a.txt"));
	fs_rmdir(&fs, STRV(TEST_DIR "/b"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/b"));
	fs_rmdir(&fs, STRV(TEST_DIR "/a"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/a"));

	fs_rmdir(&fs, STRV(TEST_DIR));
	fs_rmdir(&vfs, STRV(TEST_DIR));

	strbuf_free(&files);
	strbuf_free(&vfiles);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_lsfile_arr)
{
	START;

	fs_t vfs = {0};

	strbuf_t files = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&vfs, STRV(TEST_DIR));

	uint nodes_cnt = vfs.nodes.cnt;

	vfs.nodes.cnt = 0;

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsfile(&vfs, STRV(TEST_DIR), &files), CERR_NOT_FOUND);
	log_set_quiet(0, 0);

	vfs.nodes.cnt = nodes_cnt;

	fs_rmdir(&vfs, STRV(TEST_DIR));

	fs_free(&vfs);

	END;
}

TEST(fs_lsfile_file)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	strbuf_t files = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsfile(&fs, STRV(TEST_FILE), &files), CERR_TYPE);
	EXPECT_EQ(fs_lsfile(&vfs, STRV(TEST_FILE), &files), CERR_TYPE);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(dput_fs)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	void *vf;
	void *n = (void *)2;
	fs_open(&vfs, STRV(TEST_FILE), "w", &vf);
	log_set_quiet(0, 1);
	EXPECT_EQ(dputf(DST_FS(&vfs, n), "%s", "a"), 0);
	EXPECT_EQ(dputs(DST_FS(&vfs, n), STRV("b")), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(dputf(DST_FS(&vfs, vf), "%s", "a"), 1);
	EXPECT_EQ(dputs(DST_FS(&vfs, vf), STRV("b")), 1);
	fs_close(&vfs, vf);

	char buf[4] = {0};
	str_t str   = STRB(buf, 0);

	fs_read(&vfs, STRV(TEST_FILE), 0, &str);

	EXPECT_STRN(str.data, "ab", str.len);

	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&vfs);

	END;
}

STEST(fs)
{
	SSTART;

	RUN(fs_init_free);
	RUN(fs_open);
	RUN(fs_open_arr);
	RUN(fs_open_dir);
	RUN(fs_open_empty_path);
	RUN(fs_open_r_not_found);
	RUN(fs_open_w_does_not_exist);
	RUN(fs_open_wb_does_not_exist);
	RUN(fs_open_w_does_not_exist_folder);
	RUN(fs_open_r_exist);
	RUN(fs_close);
	RUN(fs_close_valid);
	RUN(fs_close_arr);
	RUN(fs_write);
	RUN(fs_write_oom);
	RUN(fs_write_valid);
	RUN(fs_read);
	RUN(fs_read_not_found);
	RUN(fs_read_arr);
	RUN(fs_read_oom);
	RUN(fs_read_empty);
	RUN(fs_read_str);
	RUN(fs_du);
	RUN(fs_du_arr);
	RUN(fs_isdir);
	RUN(fs_isdir_dir);
	RUN(fs_isdir_file);
	RUN(fs_isdir_arr);
	RUN(fs_isfile);
	RUN(fs_isfile_file);
	RUN(fs_isfile_dir);
	RUN(fs_isfile_arr);
	RUN(fs_mkdir);
	RUN(fs_mkdir_oom);
	RUN(fs_mkdir_exist);
	RUN(fs_mkdir_file);
	RUN(fs_mkfile);
	RUN(fs_mkfile_oom);
	RUN(fs_mkfile_exist);
	RUN(fs_mkfile_dir);
	RUN(fs_rmdir);
	RUN(fs_rmdir_arr);
	RUN(fs_rmdir_file);
	RUN(fs_rmdir_not_found);
	RUN(fs_rmdir_not_empty);
	RUN(fs_rmfile);
	RUN(fs_rmfile_arr);
	RUN(fs_rmfile_dir);
	RUN(fs_rmfile_not_found);
	RUN(fs_dir_mk_is_rm);
	RUN(fs_file_mk_is_rm);
	RUN(fs_getcwd);
	RUN(fs_lsdir);
	RUN(fs_lsdir_arr);
	RUN(fs_lsdir_file);
	RUN(fs_lsfile);
	RUN(fs_lsfile_arr);
	RUN(fs_lsfile_file);
	RUN(dput_fs);

	SEND;
}
