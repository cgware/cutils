#include "path.h"

#include "log.h"
#include "mem.h"
#include "platform.h"
#include "str.h"

#if defined(C_WIN)
	#define CSEP '\\'
#else
	#define CSEP '/'
#endif

path_t *path_init(path_t *path, strv_t str)
{
	return path_init_s(path, str, CSEP);
}

path_t *path_init_s(path_t *path, strv_t str, char sep)
{
	if (path == NULL || str.len + 1 > sizeof(path->data)) {
		return NULL;
	}

	if (str.data) {
		for (size_t i = 0; i < str.len; i++) {
			char c	      = str.data[i];
			path->data[i] = (c == '/' || c == '\\') ? sep : c;
		}
	}

	path->len = str.len;

	path->data[path->len] = '\0';

	return path;
}

path_t *path_push(path_t *path, strv_t child)
{
	return path_push_s(path, child, CSEP);
}

path_t *path_push_s(path_t *path, strv_t child, char sep)
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

path_t *path_pop(path_t *path)
{
	if (path == NULL) {
		return NULL;
	}

	size_t len = path->len;
	path->len  = 0;
	if (len > 0) {
		len--;
	}

	while (len > 0) {
		len--;
		if (path->data[len] == '/' || path->data[len] == '\\') {
			path->len = len + 1;
			break;
		}
	}

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

int path_ends(const path_t *path, strv_t str)
{
	if (path == NULL) {
		return 0;
	}

	return path->len > str.len && mem_cmp(path->data + path->len - str.len, str.data, str.len) == 0;
}

int path_calc_rel(strv_t path, strv_t dest, path_t *out)
{
	if (path.data == NULL || dest.data == NULL || out == NULL) {
		return 1;
	}

	int same = path.len == dest.len;

	size_t prefix_len = -1;

	for (size_t i = 0; i < path.len && i < dest.len; i++) {
		if (path.data[i] != dest.data[i]) {
			same = 0;
			break;
		}

		if (path.data[i] == '/' || path.data[i] == '\\') {
			prefix_len = i;
		}
	}

	out->len = 0;
	if (same) {
		out->data[0] = '\0';
		return 0;
	}

	for (size_t i = prefix_len + 1; i < path.len; i++) {
		if (path.data[i] == '/' || path.data[i] == '\\') {
			out->data[out->len++] = '.';
			out->data[out->len++] = '.';
			out->data[out->len++] = CSEP;
		}
	}

	path_push(out, STRVN(&dest.data[prefix_len + 1], dest.len - prefix_len - 1));
	return 0;
}

int pathv_is_rel(strv_t path)
{
	if (path.data == NULL || path.len == 0) {
		return 0;
	}

#if defined(C_WIN)
	return path.len < 2 || !(path.data[0] >= 'A' && path.data[0] <= 'Z' && path.data[1] == ':');
#else
	return path.data[0] != '/';
#endif
}

int pathv_lsplit(strv_t path, strv_t *l, strv_t *r)
{
	return strv_lsplit(path, CSEP, l, r);
}

int pathv_rsplit(strv_t path, strv_t *l, strv_t *r)
{
	return strv_rsplit(path, CSEP, l, r);
}

path_t *path_merge(path_t *path, strv_t rel)
{
	if (path == NULL || rel.data == NULL) {
		return path;
	}

	if (rel.len > 0 && (rel.data[0] == '/' || rel.data[0] == '\\')) {
		path->len = 0;
		path_push(path, rel);
		return path;
	}

	size_t i = 0;
	while (i < rel.len) {
		if (rel.data[i] != '.') {
			// *
			path_push(path, STRVN(&rel.data[i], rel.len - i));
			break;
		}

		if (i + 1 >= rel.len) {
			// .
			break;
		}

		if (rel.data[i + 1] == '/' || rel.data[i + 1] == '\\') {
			// ./
			i += 2;
			continue;
		}

		if (rel.data[i + 1] != '.') {
			// .*
			path_push(path, STRVN(&rel.data[i], rel.len - i));
			break;
		}

		if (i + 2 >= rel.len) {
			// ..
			path_pop(path);
			break;
		}

		if (rel.data[i + 2] == '/' || rel.data[i + 2] == '\\') {
			// ../
			path_pop(path);
			i += 3;
			continue;
		}

		// ..*
		path_push(path, STRVN(&rel.data[i], rel.len - i));
		break;
	}

	path->data[path->len] = '\0';
	return path;
}
