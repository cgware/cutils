#include "path.h"

#include "log.h"
#include "mem.h"
#include "platform.h"
#include "str.h"

#if defined(C_WIN)
	#include <Windows.h>
	#define CSEP '\\'
#else
	#include <dirent.h>
	#include <sys/stat.h>
	#define CSEP '/'
#endif

path_t *path_init(path_t *path, strv_t str)
{
	if (path == NULL || str.data == NULL || str.len + 1 > sizeof(path->data)) {
		return NULL;
	}

	mem_copy(path->data, sizeof(path->data), str.data, str.len);

	path->len = str.len;

	path->data[path->len] = '\0';

	return path;
}

path_t *path_child_s(path_t *path, strv_t child, char sep)
{
	if (path == NULL || child.data == NULL || path->len + child.len + 1 > sizeof(path->data)) {
		return NULL;
	}

	if (path->len > 0 && path->data[path->len - 1] != '/' && path->data[path->len - 1] != '\\') {
		path->data[path->len++] = sep;
	}

	mem_copy(path->data + path->len, sizeof(path->data) - path->len, child.data, child.len);

	path->len += child.len;

	path->data[path->len] = '\0';

	return path;
}

path_t *path_child(path_t *path, strv_t child)
{
	return path_child_s(path, child, CSEP);
}

int path_is_dir(const path_t *path)
{
	if (path == NULL || path->len == 0) {
		return 0;
	}

#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path->data);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buf;
	return stat(path->data, &buf) == 0 && S_ISDIR(buf.st_mode);
#endif
}

int path_is_rel(const path_t *path)
{
	if (path == NULL || path->len == 0) {
		return 0;
	}

#if defined(C_WIN)
	return path->len < 2 || !(path->data[0] >= 'A' && path->data[0] <= 'Z' && path->data[1] == ':');
#else
	return path->data[0] != '/';
#endif
}

path_t *path_parent(path_t *path)
{
	if (path == NULL) {
		return NULL;
	}

	size_t len = path->len;

	while (len > 0 && path->data[len] != '\\' && path->data[len] != '/') {
		len--;
	}

	if (len == 0) {
		return NULL;
	}

	path->len = len;

	path->data[path->len] = '\0';

	return path;
}

path_t *path_set_len(path_t *path, size_t len)
{
	if (path == NULL) {
		return NULL;
	}

	path->len	      = len;
	path->data[path->len] = '\0';

	return path;
}

int path_ends(const path_t *path, const char *str, size_t len)
{
	if (path == NULL) {
		return 0;
	}

	return path->len > len && mem_cmp(path->data + path->len - len, str, (size_t)len) == 0;
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
		out->data[0] = '\0';
		return 0;
	}

	for (size_t i = prefix_len + 1; i < path_len; i++) {
		if (path[i] == '/' || path[i] == '\\') {
			out->data[out->len++] = '.';
			out->data[out->len++] = '.';
			out->data[out->len++] = CSEP;
		}
	}

	path_child(out, STRVN(&dest[prefix_len + 1], dest_len - prefix_len - 1));
	return 0;
}

pathv_t pathv_path(const path_t *path)
{
	if (path == NULL) {
		return (pathv_t){0};
	}

	return (pathv_t){.path = path->data, .len = path->len};
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
