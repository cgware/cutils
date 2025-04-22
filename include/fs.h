#ifndef FS_H
#define FS_H

#include "str.h"
#include "strbuf.h"

typedef struct fs_s fs_t;

typedef void *(*fs_open_fn)(fs_t *fs, strv_t path, const char *mode);
typedef int (*fs_close_fn)(fs_t *fs, void *file);

typedef int (*fs_write_fn)(fs_t *fs, void *file, strv_t str);
typedef int (*fs_read_fn)(fs_t *fs, void *file, str_t *str);

typedef size_t (*fs_du_fn)(fs_t *fs, void *file);

typedef int (*fs_isdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_isfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_mkdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_mkfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_rmdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_rmfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_lsdir_cb)(strv_t path, strv_t dir, strbuf_t *dirs);
typedef int (*fs_lsdir_fn)(fs_t *fs, strv_t path, strbuf_t *dirs);
typedef int (*fs_lsfile_cb)(strv_t path, strv_t file, strbuf_t files);
typedef int (*fs_lsfile_fn)(fs_t *fs, strv_t path, strbuf_t *files);

typedef struct fs_ops_s {
	fs_open_fn open;
	fs_close_fn close;
	fs_write_fn write;
	fs_read_fn read;
	fs_du_fn du;
	fs_isdir_fn isdir;
	fs_isfile_fn isfile;
	fs_mkdir_fn mkdir;
	fs_mkfile_fn mkfile;
	fs_rmdir_fn rmdir;
	fs_rmfile_fn rmfile;
	fs_lsdir_fn lsdir;
	fs_lsfile_fn lsfile;
} fs_ops_t;

struct fs_s {
	strbuf_t paths;
	arr_t nodes;
	int virt;
	fs_ops_t ops;
};

fs_t *fs_init(fs_t *fs, uint nodes_cap, int virt, alloc_t alloc);
void fs_free(fs_t *fs);

void *fs_open(fs_t *fs, strv_t path, const char *mode);
int fs_close(fs_t *fs, void *file);

int fs_write(fs_t *fs, void *file, strv_t str);
int fs_read(fs_t *fs, strv_t path, int b, str_t *str);

int fs_isdir(fs_t *fs, strv_t path);
int fs_isfile(fs_t *fs, strv_t path);

int fs_mkdir(fs_t *fs, strv_t path);
int fs_mkfile(fs_t *fs, strv_t path);

int fs_rmdir(fs_t *fs, strv_t path);
int fs_rmfile(fs_t *fs, strv_t path);

int fs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs);
int fs_lsfile(fs_t *fs, strv_t path, strbuf_t *files);

#endif
