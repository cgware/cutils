#ifndef PATH_H
#define PATH_H

#include "platform.h"
#include "str.h"
#include "strv.h"

#if defined(C_WIN)
	#define SEP "\\"
#else
	#define SEP "/"
#endif

typedef struct path_s {
	size_t len;
	char data[C_MAX_PATH];
} path_t;

typedef struct pathv_s {
	size_t len;
	const char *path;
} pathv_t;

path_t *path_init(path_t *path, strv_t str);
path_t *path_child_s(path_t *path, strv_t child, char sep);
path_t *path_child(path_t *path, strv_t child);

int path_is_dir(const path_t *path);

path_t *path_child_dir(path_t *path, const char *dir, size_t len);
path_t *path_child_folder(path_t *path, const char *folder, size_t len);
path_t *path_parent(path_t *path);
path_t *path_set_len(path_t *path, size_t len);
int path_ends(const path_t *path, const char *str, size_t len);
int path_calc_rel(const char *path, size_t path_len, const char *dest, size_t dest_len, path_t *out);

pathv_t pathv_path(const path_t *path);
pathv_t pathv_get_dir(pathv_t pathv, str_t *child);

#endif
