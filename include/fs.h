#ifndef FS_H
#define FS_H

#include "strbuf.h"

typedef struct fs_s fs_t;

typedef void *(*fs_open_fn)(fs_t *fs, strv_t path, const char *mode);
typedef void *(*fs_reopen_fn)(fs_t *fs, strv_t path, const char *mode, void *file);
typedef int (*fs_close_fn)(fs_t *fs, void *file);

typedef int (*fs_mkdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_mkfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_rmdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_rmfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_lsdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_lsfile_fn)(fs_t *fs, strv_t path);

typedef int (*fs_isdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_isfile_fn)(fs_t *fs, strv_t path);

typedef struct fs_ops_s {
	fs_open_fn open;
	fs_reopen_fn reopen;
	fs_close_fn close;
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

int fs_isdir(fs_t *fs, strv_t path);
int fs_isfile(fs_t *fs, strv_t path);

int fs_mkdir(fs_t *fs, strv_t path);
int fs_mkfile(fs_t *fs, strv_t path);

int fs_rmdir(fs_t *fs, strv_t path);
int fs_rmfile(fs_t *fs, strv_t path);

#endif
