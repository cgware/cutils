#include "fs.h"

#include "log.h"
#include "path.h"
#include "platform.h"

#include <errno.h>
#include <stdio.h>

#if defined(C_WIN)
	#include <Windows.h>
#else
	#include <dirent.h>
	#include <sys/stat.h>
#endif

typedef enum fs_node_type_e {
	FS_NODE_TYPE_UNKNOWN,
	FS_NODE_TYPE_FILE,
	FS_NODE_TYPE_DIR,
} fs_node_type_t;

typedef enum fs_node_flag_e {
	FS_NODE_FLAG_OPEN,
} fs_node_flag_t;

typedef struct fs_node_s {
	fs_node_type_t type;
	int flags;
} fs_node_t;

static int ofs_isdir(fs_t *fs, strv_t path);

static void *ofs_open(fs_t *fs, strv_t path, const char *mode)
{
	(void)fs;
	FILE *file = NULL;

	errno = 0;
#if defined(C_WIN)
	fopen_s(&file, path.data, mode);
#else
	if (ofs_isdir(fs, path)) {
		int errnum = EISDIR;
		log_error("cutils", "file", NULL, "failed to open file \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
		return NULL;
	}
	file = fopen(path.data, mode);
#endif
	if (file == NULL) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to open file \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
	}
	return file;
}

static void *vfs_open(fs_t *fs, strv_t path, const char *mode)
{
	uint index = -1;
	if (strbuf_get_index(&fs->paths, path, &index)) {
		strv_t name = {0};
		strv_t dir  = pathv_get_dir(path, &name);

		if (mode[0] == 'w' && name.len > 0 && (dir.len == 0 || (dir.len == 1 && dir.data[0] == '.'))) {
			strbuf_add(&fs->paths, name, &index);
			fs_node_t *node = arr_add(&fs->nodes);
			if (node == NULL) {
				return NULL;
			}
			node->type = FS_NODE_TYPE_FILE;
		} else {
			int errnum = ENOENT;
			log_error("cutils",
				  "file",
				  NULL,
				  "failed to open file \"%.*s\": %s (%d)",
				  path.len,
				  path.data,
				  log_strerror(errnum),
				  errnum);
			return NULL;
		}
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return NULL;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		int errnum = node->type == FS_NODE_TYPE_DIR ? EISDIR : EINVAL;
		log_error(
			"cutils", "file", NULL, "failed to open file \"%.*s\": %s (%d)", path.len, path.data, log_strerror(errnum), errnum);
		return NULL;
	}

	node->flags |= 1 << FS_NODE_FLAG_OPEN;

	return (void *)((size_t)index + 1);
}

static int ofs_close(fs_t *fs, void *file)
{
	(void)fs;
	fclose(file);

	return 0;
}

static int vfs_close(fs_t *fs, void *file)
{
	uint index = (size_t)file - 1;

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return 1;
	}

	node->flags &= ~(1 << FS_NODE_FLAG_OPEN);

	return 0;
}

static int ofs_isdir(fs_t *fs, strv_t path)
{
	(void)fs;

#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path.data);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (stat(path.data, &buffer)) {
		return 0;
	} else {
		return S_ISDIR(buffer.st_mode);
	}
#endif
}

static int vfs_isdir(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_get_index(&fs->paths, path, &index)) {
		return 0;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return 0;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return 0;
	}

	return 1;
}

static int ofs_isfile(fs_t *fs, strv_t path)
{
	(void)fs;

#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path.data);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (stat(path.data, &buffer)) {
		return 0;
	} else {
		return S_ISREG(buffer.st_mode);
	}
#endif
}

static int vfs_isfile(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_get_index(&fs->paths, path, &index)) {
		return 0;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return 0;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		return 0;
	}

	return 1;
}

static int ofs_mkdir(fs_t *fs, strv_t path)
{
	(void)fs;

	int ret;
#if defined(C_WIN)
	ret = CreateDirectoryA(path.data, NULL) == 0 ? 1 : 0;
#else
	errno = 0;
	ret   = mkdir(path.data, 0700);
	if (ret != 0) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to create directory \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
		ret = 1;
	}
	return ret;
#endif
}

static int vfs_mkdir(fs_t *fs, strv_t path)
{
	if (strbuf_get_index(&fs->paths, path, NULL) == 0) {
		int errnum = EEXIST;
		log_error("cutils",
			  "file",
			  NULL,
			  "failed to create directory \"%.*s\": %s (%d)",
			  path.len,
			  path.data,
			  log_strerror(errnum),
			  errnum);
		return 1;
	}

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes);
	if (node == NULL) {
		return 1;
	}
	node->type = FS_NODE_TYPE_DIR;

	return 0;
}

static int ofs_mkfile(fs_t *fs, strv_t path)
{
	(void)fs;

#if defined(C_WIN)
	return CreateDirectoryA(path.data, NULL) == 0 ? 1 : 0;
#else
	if (ofs_isfile(fs, path)) {
		int errnum = EEXIST;
		log_error("cutils", "file", NULL, "failed to create file \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
		return 1;
	}
	ofs_close(fs, ofs_open(fs, path, "w"));
	return 0;
#endif
}

static int vfs_mkfile(fs_t *fs, strv_t path)
{
	if (strbuf_get_index(&fs->paths, path, NULL) == 0) {
		int errnum = EEXIST;
		log_error("cutils",
			  "file",
			  NULL,
			  "failed to create file \"%.*s\": %s (%d)",
			  path.len,
			  path.data,
			  log_strerror(errnum),
			  errnum);
		return 1;
	}

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes);
	if (node == NULL) {
		return 1;
	}
	node->type = FS_NODE_TYPE_FILE;

	return 0;
}

static int ofs_rmdir(fs_t *fs, strv_t path)
{
	(void)fs;

	int ret;
#if defined(C_WIN)
	ret = RemoveDirectoryA(path.data) == 0 ? 1 : 0;
#else
	errno = 0;
	ret   = remove(path.data);
	if (ret != 0) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to remove directory \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
		ret = 1;
	}
	return ret;
#endif
}

static int vfs_rmdir(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_get_index(&fs->paths, path, &index)) {
		int errnum = ENOENT;
		log_error("cutils",
			  "file",
			  NULL,
			  "failed to remove directory \"%.*s\": %s (%d)",
			  path.len,
			  path.data,
			  log_strerror(errnum),
			  errnum);
		return 1;
	}

	strbuf_set(&fs->paths, STRV(""), index);
	return 0;
}

static int ofs_rmfile(fs_t *fs, strv_t path)
{
	(void)fs;

	int ret;
#if defined(C_WIN)
	ret = DeleteFileA(path.data) == 0 ? 1 : 0;
#else
	errno = 0;
	ret   = remove(path.data);
	if (ret != 0) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to delete file \"%s\": %s (%d)", path.data, log_strerror(errnum), errnum);
		ret = 1;
	}
#endif
	return ret;
}

static int vfs_rmfile(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_get_index(&fs->paths, path, &index)) {
		int errnum = ENOENT;
		log_error("cutils",
			  "file",
			  NULL,
			  "failed to delete file \"%.*s\": %s (%d)",
			  path.len,
			  path.data,
			  log_strerror(errnum),
			  errnum);
		return 1;
	}

	strbuf_set(&fs->paths, STRV(""), index);
	return 0;
}

static const fs_ops_t s_fs_ops[] = {
	[0] =
		{
			.open	= ofs_open,
			.close	= ofs_close,
			.isdir	= ofs_isdir,
			.isfile = ofs_isfile,
			.mkdir	= ofs_mkdir,
			.mkfile = ofs_mkfile,
			.rmdir	= ofs_rmdir,
			.rmfile = ofs_rmfile,
		},
	[1] =
		{
			.open	= vfs_open,
			.close	= vfs_close,
			.isdir	= vfs_isdir,
			.isfile = vfs_isfile,
			.mkdir	= vfs_mkdir,
			.mkfile = vfs_mkfile,
			.rmdir	= vfs_rmdir,
			.rmfile = vfs_rmfile,
		},
};

fs_t *fs_init(fs_t *fs, uint nodes_cap, int virt, alloc_t alloc)
{
	if (fs == NULL) {
		return NULL;
	}

	if (virt &&
	    (strbuf_init(&fs->paths, nodes_cap, 16, alloc) == NULL || arr_init(&fs->nodes, nodes_cap, sizeof(fs_node_t), alloc) == NULL)) {
		return NULL;
	}

	fs->virt = virt;

	fs->ops = s_fs_ops[virt ? 1 : 0];

	return fs;
}

void fs_free(fs_t *fs)
{
	if (fs == NULL || !fs->virt) {
		return;
	}

	arr_free(&fs->nodes);
	strbuf_free(&fs->paths);
}

void *fs_open(fs_t *fs, strv_t path, const char *mode)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || mode == NULL || path_init(&buf, path) == NULL) {
		return NULL;
	}

	return fs->ops.open(fs, STRVN(buf.data, buf.len), mode);
}

int fs_close(fs_t *fs, void *file)
{
	if (fs == NULL || file == NULL) {
		return 1;
	}

	return fs->ops.close(fs, file);
}

int fs_isdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 0;
	}

	return fs->ops.isdir(fs, STRVN(buf.data, buf.len));
}

int fs_isfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 0;
	}

	return fs->ops.isfile(fs, STRVN(buf.data, buf.len));
}

int fs_mkdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 1;
	}

	return fs->ops.mkdir(fs, STRVN(buf.data, buf.len));
}

int fs_mkfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 1;
	}

	return fs->ops.mkfile(fs, STRVN(buf.data, buf.len));
}

int fs_rmdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 1;
	}

	return fs->ops.rmdir(fs, STRVN(buf.data, buf.len));
}

int fs_rmfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 1;
	}

	return fs->ops.rmfile(fs, STRVN(buf.data, buf.len));
}
