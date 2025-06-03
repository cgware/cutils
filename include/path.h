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

path_t *path_init(path_t *path, strv_t str);
path_t *path_init_s(path_t *path, strv_t str, char sep);
path_t *path_child(path_t *path, strv_t child);
path_t *path_child_s(path_t *path, strv_t child, char sep);

path_t *path_parent(path_t *path);
path_t *path_set_len(path_t *path, size_t len);
int path_ends(const path_t *path, strv_t str);
int path_calc_rel(strv_t path, strv_t dest, path_t *out);

int pathv_is_rel(strv_t path);

strv_t pathv_get_dir(strv_t pathv, strv_t *child);

path_t *path_merge(path_t *path, strv_t child);

#endif
