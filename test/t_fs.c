#include "fs.h"

#include "log.h"
#include "mem.h"
#include "test.h"

#include <errno.h>

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

	errno = 0;
	EXPECT_EQ(fs_open(NULL, STRV_NULL, NULL), NULL);
	EXPECT_EQ(errno, EINVAL);

	fs_t fs = {0};

	log_set_quiet(0, 1);
	fs_init(&fs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	errno = 0;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "w"), NULL);
	EXPECT_EQ(errno, ENOMEM);
	mem_oom(0);

	fs_free(&fs);

	END;
}

TEST(fs_open_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	errno = 0;
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "r"), NULL);
	EXPECT_EQ(errno, EINVAL);
	log_set_quiet(0, 0);

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

	log_set_quiet(0, 1);
	errno = 0;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_DIR), "r"), NULL);
	EXPECT_EQ(errno, EISDIR);
	errno = 0;
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_DIR), "r"), NULL);
	EXPECT_EQ(errno, EISDIR);
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

	log_set_quiet(0, 1);
	errno = 0;
	EXPECT_EQ(fs_open(&fs, STRV(""), "r"), NULL);
	EXPECT_EQ(errno, EINVAL);
	errno = 0;
	EXPECT_EQ(fs_open(&vfs, STRV(""), "r"), NULL);
	EXPECT_EQ(errno, EINVAL);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_r_does_not_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	errno = 0;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_FILE), "r"), NULL);
	EXPECT_EQ(errno, ENOENT);
	errno = 0;
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_FILE), "r"), NULL);
	EXPECT_EQ(errno, ENOENT);
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

	void *f	 = fs_open(&fs, STRV(TEST_FILE), "w");
	void *vf = fs_open(&vfs, STRV(TEST_FILE), "w");

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

	void *f	 = fs_open(&fs, STRV(TEST_FILE), "wb");
	void *vf = fs_open(&vfs, STRV(TEST_FILE), "wb");

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

	log_set_quiet(0, 1);
	errno = 0;
	EXPECT_EQ(fs_open(&fs, STRV(TEST_DIR "/" TEST_FILE), "w"), NULL);
	EXPECT_EQ(errno, ENOENT);
	errno = 0;
	EXPECT_EQ(fs_open(&vfs, STRV(TEST_DIR "/" TEST_FILE), "w"), NULL);
	EXPECT_EQ(errno, ENOENT);
	log_set_quiet(0, 0);

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_open_r_exists)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_close(&fs, fs_open(&fs, STRV(TEST_FILE), "w"));
	fs_close(&vfs, fs_open(&vfs, STRV(TEST_FILE), "w"));

	void *f	 = fs_open(&fs, STRV(TEST_FILE), "r");
	void *vf = fs_open(&vfs, STRV(TEST_FILE), "r");

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

	fs_t fs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);

	EXPECT_EQ(fs_close(NULL, NULL), EINVAL);

	fs_free(&fs);

	END;
}

TEST(fs_close_null)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	EXPECT_EQ(fs_close(&fs, NULL), EINVAL);
	EXPECT_EQ(fs_close(&vfs, NULL), EINVAL);

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

	void *f	 = fs_open(&fs, STRV(TEST_FILE), "w");
	void *vf = fs_open(&vfs, STRV(TEST_FILE), "w");

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

	void *vf = fs_open(&vfs, STRV(TEST_FILE), "w");

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_close(&vfs, vf), 1);
	log_set_quiet(0, 0);

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

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_isdir(&vfs, STRV(TEST_DIR)), 0);
	log_set_quiet(0, 0);

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

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 0);
	log_set_quiet(0, 0);

	fs_free(&vfs);

	END;
}

TEST(fs_mkdir)
{
	START;

	fs_t fs = {0};

	log_set_quiet(0, 1);
	fs_init(&fs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_mkdir(NULL, STRV_NULL), EINVAL);
	mem_oom(1);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), ENOMEM);
	mem_oom(0);

	fs_free(&fs);

	END;
}

TEST(fs_mkdir_exists)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&fs, STRV(TEST_DIR));
	fs_mkdir(&vfs, STRV(TEST_DIR));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), EEXIST);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), EEXIST);
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
	EXPECT_EQ(fs_mkdir(&fs, STRV(TEST_DIR)), EEXIST);
	EXPECT_EQ(fs_mkdir(&vfs, STRV(TEST_DIR)), EEXIST);
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

	fs_t fs = {0};

	log_set_quiet(0, 1);
	fs_init(&fs, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_mkfile(NULL, STRV_NULL), EINVAL);
	mem_oom(1);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), ENOMEM);
	mem_oom(0);

	fs_free(&fs);

	END;
}

TEST(fs_mkfile_exists)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&fs, STRV(TEST_FILE));
	fs_mkfile(&vfs, STRV(TEST_FILE));

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), EEXIST);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), EEXIST);
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
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), EEXIST);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), EEXIST);
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

	fs_t fs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);

	EXPECT_EQ(fs_rmdir(NULL, STRV_NULL), EINVAL);

	fs_free(&fs);

	END;
}

TEST(fs_rmdir_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkdir(&vfs, STRV(TEST_FILE));

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_FILE)), EINVAL);
	log_set_quiet(0, 0);

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
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), EINVAL);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), EINVAL);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_DIR));
	fs_rmfile(&vfs, STRV(TEST_DIR));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmdir_does_not_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), ENOENT);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), ENOENT);
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
	EXPECT_EQ(fs_rmdir(&fs, STRV(TEST_DIR)), ENOTEMPTY);
	EXPECT_EQ(fs_rmdir(&vfs, STRV(TEST_DIR)), ENOTEMPTY);
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

	fs_t fs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);

	EXPECT_EQ(fs_rmfile(NULL, STRV_NULL), EINVAL);

	fs_free(&fs);

	END;
}

TEST(fs_rmfile_arr)
{
	START;

	fs_t vfs = {0};

	fs_init(&vfs, 1, 1, ALLOC_STD);

	fs_mkfile(&vfs, STRV(TEST_FILE));

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), EINVAL);
	log_set_quiet(0, 0);

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
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), EISDIR);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), EISDIR);
	log_set_quiet(0, 0);

	fs_rmdir(&fs, STRV(TEST_FILE));
	fs_rmdir(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
	fs_free(&vfs);

	END;
}

TEST(fs_rmfile_does_not_exist)
{
	START;

	fs_t fs	 = {0};
	fs_t vfs = {0};

	fs_init(&fs, 0, 0, ALLOC_STD);
	fs_init(&vfs, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), ENOENT);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), ENOENT);
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
	EXPECT_EQ(fs_mkfile(&fs, STRV(TEST_FILE)), EEXIST);
	EXPECT_EQ(fs_mkfile(&vfs, STRV(TEST_FILE)), EEXIST);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 1);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 1);

	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_rmfile(&fs, STRV(TEST_FILE)), ENOENT);
	EXPECT_EQ(fs_rmfile(&vfs, STRV(TEST_FILE)), ENOENT);
	log_set_quiet(0, 0);

	EXPECT_EQ(fs_isfile(&fs, STRV(TEST_FILE)), 0);
	EXPECT_EQ(fs_isfile(&vfs, STRV(TEST_FILE)), 0);

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

	fs_mkfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/a.txt"));
	fs_mkdir(&fs, STRV(TEST_DIR "/a"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/a"));
	fs_mkdir(&fs, STRV(TEST_DIR "/b"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/b"));
	fs_mkdir(&fs, STRV(TEST_DIR "/c"));
	fs_mkdir(&vfs, STRV(TEST_DIR "/c"));

	EXPECT_EQ(fs_lsdir(&fs, STRV_NULL, NULL), EINVAL);
	EXPECT_EQ(fs_lsdir(&vfs, STRV_NULL, NULL), EINVAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsdir(&fs, not, &dirs), ENOENT);
	EXPECT_EQ(fs_lsdir(&vfs, not, &vdirs), ENOENT);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(fs_lsdir(&fs, path, &dirs), ENOMEM);
	EXPECT_EQ(fs_lsdir(&vfs, path, &vdirs), ENOMEM);
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

	fs_rmdir(&fs, STRV(TEST_DIR "/c"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/c"));
	fs_rmdir(&fs, STRV(TEST_DIR "/b"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/b"));
	fs_rmdir(&fs, STRV(TEST_DIR "/a"));
	fs_rmdir(&vfs, STRV(TEST_DIR "/a"));
	fs_rmfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/a.txt"));

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

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsdir(&vfs, STRV(TEST_DIR), &dirs), EINVAL);
	log_set_quiet(0, 0);

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
	EXPECT_EQ(fs_lsdir(&fs, STRV(TEST_FILE), &dirs), ENOTDIR);
	EXPECT_EQ(fs_lsdir(&vfs, STRV(TEST_FILE), &dirs), ENOTDIR);
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
	fs_mkfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/a.txt"));
	fs_mkfile(&fs, STRV(TEST_DIR "/b.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/b.txt"));
	fs_mkfile(&fs, STRV(TEST_DIR "/c.txt"));
	fs_mkfile(&vfs, STRV(TEST_DIR "/c.txt"));

	EXPECT_EQ(fs_lsfile(&fs, STRV_NULL, NULL), EINVAL);
	EXPECT_EQ(fs_lsfile(&vfs, STRV_NULL, NULL), EINVAL);
	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsfile(&fs, not, &files), ENOENT);
	EXPECT_EQ(fs_lsfile(&vfs, not, &vfiles), ENOENT);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(fs_lsfile(&fs, path, &files), ENOMEM);
	EXPECT_EQ(fs_lsfile(&vfs, path, &vfiles), ENOMEM);
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

	fs_rmfile(&fs, STRV(TEST_DIR "/c.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/c.txt"));
	fs_rmfile(&fs, STRV(TEST_DIR "/b.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/b.txt"));
	fs_rmfile(&fs, STRV(TEST_DIR "/a.txt"));
	fs_rmfile(&vfs, STRV(TEST_DIR "/a.txt"));
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

	arr_reset(&vfs.nodes, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(fs_lsfile(&vfs, STRV(TEST_DIR), &files), EINVAL);
	log_set_quiet(0, 0);

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
	EXPECT_EQ(fs_lsfile(&fs, STRV(TEST_FILE), &files), ENOTDIR);
	EXPECT_EQ(fs_lsfile(&vfs, STRV(TEST_FILE), &files), ENOTDIR);
	log_set_quiet(0, 0);

	fs_rmfile(&fs, STRV(TEST_FILE));
	fs_rmfile(&vfs, STRV(TEST_FILE));

	fs_free(&fs);
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
	RUN(fs_open_r_does_not_exist);
	RUN(fs_open_w_does_not_exist);
	RUN(fs_open_wb_does_not_exist);
	RUN(fs_open_w_does_not_exist_folder);
	RUN(fs_open_r_exists);
	RUN(fs_close);
	RUN(fs_close_null);
	RUN(fs_close_valid);
	RUN(fs_close_arr);
	RUN(fs_isdir);
	RUN(fs_isdir_dir);
	RUN(fs_isdir_file);
	RUN(fs_isdir_arr);
	RUN(fs_isfile);
	RUN(fs_isfile_file);
	RUN(fs_isfile_dir);
	RUN(fs_isfile_arr);
	RUN(fs_mkdir);
	RUN(fs_mkdir_exists);
	RUN(fs_mkdir_file);
	RUN(fs_mkfile);
	RUN(fs_mkfile_exists);
	RUN(fs_mkfile_dir);
	RUN(fs_rmdir);
	RUN(fs_rmdir_arr);
	RUN(fs_rmdir_file);
	RUN(fs_rmdir_does_not_exist);
	RUN(fs_rmdir_not_empty);
	RUN(fs_rmfile);
	RUN(fs_rmfile_arr);
	RUN(fs_rmfile_dir);
	RUN(fs_rmfile_does_not_exist);
	RUN(fs_dir_mk_is_rm);
	RUN(fs_file_mk_is_rm);
	RUN(fs_lsdir);
	RUN(fs_lsdir_arr);
	RUN(fs_lsdir_file);
	RUN(fs_lsfile);
	RUN(fs_lsfile_arr);
	RUN(fs_lsfile_file);

	SEND;
}
