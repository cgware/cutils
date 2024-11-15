#include "path.h"

#include "log.h"
#include "mem.h"
#include "str.h"

#include "platform.h"

#if defined(C_WIN)
	#define CSEP '\\'
#else
	#define CSEP '/'
#endif

path_t *path_init(path_t *path, const char *dir, size_t len)
{
	if (path == NULL || dir == NULL || len + 1 > C_MAX_PATH) {
		return NULL;
	}

	mem_cpy(path->path, len, dir, len);
	path->len = len;

	path->path[path->len] = '\0';

	return path;
}

path_t *path_child_s(path_t *path, const char *dir, size_t len, char s)
{
	if (path == NULL || dir == NULL || path->len + len + 2 > C_MAX_PATH) {
		return NULL;
	}

	if (s != '\0' && path->len > 0 && path->path[path->len - 1] != '/' && path->path[path->len - 1] != '\\') {
		if (s == '/' || s == '\\') {
			log_warn("cutils", "path", NULL, "directory does not end with separator: %.*s", path->len, path->path);
		}
		path->path[path->len++] = s;
	}
	mem_cpy(path->path + path->len, len, dir, len);
	path->len += len;

	path->path[path->len] = '\0';

	return path;
}

path_t *path_child(path_t *path, const char *dir, size_t len)
{
	return path_child_s(path, dir, len, CSEP);
}

path_t *path_child_dir(path_t *path, const char *dir, size_t len)
{
	if (path == NULL || dir == NULL || path->len + len + 2 > C_MAX_PATH) {
		return NULL;
	}

	if (path->len > 0 && path->path[path->len - 1] != '/' && path->path[path->len - 1] != '\\') {
		log_warn("cutils", "path", NULL, "directory does not end with separator: %.*s", path->len, path->path);
		path->path[path->len++] = CSEP;
	}
	mem_cpy(path->path + path->len, len, dir, len);
	path->len += len;

	if (path->len > 0 && path->path[path->len - 1] != '/' && path->path[path->len - 1] != '\\') {
		log_warn("cutils", "path", NULL, "directory does not end with separator: %.*s", path->len, path->path);
		path->path[path->len++] = CSEP;
	}
	path->path[path->len] = '\0';

	return path;
}

path_t *path_child_folder(path_t *path, const char *folder, size_t len)
{
	if (path == NULL || folder == NULL || path->len + len + 2 > C_MAX_PATH) {
		return NULL;
	}

	if (path->len > 0 && path->path[path->len - 1] != '/' && path->path[path->len - 1] != '\\') {
		log_warn("cutils", "path", NULL, "directory does not end with separator: %.*s", path->len, path->path);
		path->path[path->len++] = CSEP;
	}
	mem_cpy(path->path + path->len, len, folder, len);
	path->len += len;

	if (path->len > 0 && (path->path[path->len - 1] == '/' || path->path[path->len - 1] == '\\')) {
		log_warn("cutils", "path", NULL, "folder ends with separator: %.*s", len, folder);
	} else {
		path->path[path->len++] = CSEP;
	}
	path->path[path->len] = '\0';

	return path;
}

path_t *path_parent(path_t *path)
{
	if (path == NULL) {
		return NULL;
	}

	size_t len = path->len;

	while (len > 0 && path->path[len] != '\\' && path->path[len] != '/') {
		len--;
	}

	if (len == 0) {
		return NULL;
	}

	path->len = len;

	path->path[path->len] = '\0';

	return path;
}

path_t *path_set_len(path_t *path, size_t len)
{
	if (path == NULL) {
		return NULL;
	}

	path->len	      = len;
	path->path[path->len] = '\0';

	return path;
}

int path_ends(const path_t *path, const char *str, size_t len)
{
	if (path == NULL) {
		return 0;
	}

	return path->len > len && mem_cmp(path->path + path->len - len, str, (size_t)len) == 0;
}

int path_calc_rel(const char *path, size_t path_len, const char *dest, size_t dest_len, path_t *out)
{
	if (path == NULL || dest == NULL || out == NULL) {
		return 1;
	}

	int same = path_len == dest_len;

	size_t prefix_len = -1;

	for (size_t i = 0; i < path_len && i < dest_len; i++) {
		if (path[i] != dest[i]) {
			same = 0;
			break;
		}

		if (path[i] == '/' || path[i] == '\\') {
			prefix_len = i;
		}
	}

	out->len = 0;
	if (same) {
		out->path[0] = '\0';
		return 0;
	}

	for (size_t i = prefix_len + 1; i < path_len; i++) {
		if (path[i] == '/' || path[i] == '\\') {
			out->path[out->len++] = '.';
			out->path[out->len++] = '.';
			out->path[out->len++] = CSEP;
		}
	}

	path_child(out, &dest[prefix_len + 1], dest_len - prefix_len - 1);
	return 0;
}

pathv_t pathv_path(const path_t *path)
{
	if (path == NULL) {
		return (pathv_t){0};
	}

	return (pathv_t){.path = path->path, .len = path->len};
}

pathv_t pathv_get_dir(pathv_t pathv, str_t *child)
{
	if (pathv.path == NULL) {
		return (pathv_t){0};
	}

	pathv_t dir = {
		.len  = pathv.len,
		.path = pathv.path,
	};

	if (dir.len > 0 && (dir.path[dir.len - 1] == '\\' || dir.path[dir.len - 1] == '/')) {
		dir.len--;
	}

	size_t child_end = dir.len;

	while (dir.len > 0 && dir.path[dir.len - 1] != '\\' && dir.path[dir.len - 1] != '/') {
		dir.len--;
	}

	if (child != NULL) {
		*child = strc(&dir.path[dir.len], child_end - dir.len);
	}

	return dir;
}
