#ifndef FS_H
#define FS_H

#include "str.h"
#include "strbuf.h"

typedef struct fs_s {
	buf_t paths;
	arr_t nodes;
	int virt;
} fs_t;

fs_t *fs_init(fs_t *fs, uint nodes_cap, int virt, alloc_t alloc);
void fs_free(fs_t *fs);

int fs_open(fs_t *fs, strv_t path, const char *mode, void **file);
int fs_close(fs_t *fs, void *file);

int fs_write(fs_t *fs, void *file, strv_t str);
int fs_read(fs_t *fs, strv_t path, int b, str_t *str);

int fs_du(fs_t *fs, void *file, size_t *size);

int fs_isdir(fs_t *fs, strv_t path);
int fs_isfile(fs_t *fs, strv_t path);

int fs_mkdir(fs_t *fs, strv_t path);
int fs_mkfile(fs_t *fs, strv_t path);
int fs_mkpath(fs_t *fs, strv_t base, strv_t path);

int fs_rmdir(fs_t *fs, strv_t path);
int fs_rmfile(fs_t *fs, strv_t path);
int fs_rmpath(fs_t *fs, strv_t base, strv_t path);

int fs_getcwd(fs_t *fs, str_t *path);

int fs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs);
int fs_lsfile(fs_t *fs, strv_t path, strbuf_t *files);

size_t dputs_fs(dst_t dst, strv_t str);
size_t dputv_fs(dst_t dst, const char *fmt, va_list args);

#define DST_FS(_fs, _file) ((dst_t){.puts = dputs_fs, .putv = dputv_fs, .dst = _file, .priv = _fs})

#endif
