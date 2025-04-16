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

	if (ofs_isdir(fs, path)) {
		errno = EISDIR;
		return NULL;
	}

	errno = 0;
#if defined(C_WIN)
	fopen_s(&file, path.data, mode);
#else
	if (path.len == 0) {
		errno = EINVAL;
		return NULL;
	}
	file = fopen(path.data, mode);
#endif
	return file;
}

static void *vfs_open(fs_t *fs, strv_t path, const char *mode)
{
	if (path.len == 0) {
		errno = EINVAL;
		return NULL;
	}

	uint index;
	if (strbuf_find(&fs->paths, path, &index)) {
		strv_t name = {0};
		strv_t dir  = pathv_get_dir(path, &name);

		if (mode[0] == 'w' && name.len > 0 && (dir.len == 0 || (dir.len == 1 && dir.data[0] == '.'))) {
			strbuf_add(&fs->paths, name, &index);
			fs_node_t *node = arr_add(&fs->nodes);
			if (node == NULL) {
				errno = ENOMEM;
				return NULL;
			}
			node->type = FS_NODE_TYPE_FILE;
		} else {
			errno = ENOENT;
			return NULL;
		}
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		errno = node->type == FS_NODE_TYPE_DIR ? EISDIR : EINVAL;
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
	uint index = (uint)((size_t)file - 1);

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
	if (strbuf_find(&fs->paths, path, &index)) {
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
	if (strbuf_find(&fs->paths, path, &index)) {
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

#if defined(C_WIN)
	if (CreateDirectoryA(path.data, NULL) == 0) {
		DWORD err = GetLastError();
		if (err == ERROR_ALREADY_EXISTS) {
			return EEXIST;
		} else {
			return -1;
		}
	}
#else
	errno = 0;
	if (mkdir(path.data, 0700)) {
		return errno;
	}
#endif
	return 0;
}

static int vfs_mkdir(fs_t *fs, strv_t path)
{
	if (strbuf_find(&fs->paths, path, NULL) == 0) {
		return EEXIST;
	}

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes);
	if (node == NULL) {
		return ENOMEM;
	}
	node->type = FS_NODE_TYPE_DIR;

	return 0;
}

static int ofs_mkfile(fs_t *fs, strv_t path)
{
	(void)fs;

	if (fs_isdir(fs, path)) {
		return EEXIST;
	}

#if defined(C_WIN)
	HANDLE h = CreateFileA(path.data, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_FILE_EXISTS) {
			return EEXIST;
		} else {
			return -1;
		}
	}
	CloseHandle(h);
#else
	if (ofs_isfile(fs, path)) {
		return EEXIST;
	}
	ofs_close(fs, ofs_open(fs, path, "w"));
#endif
	return 0;
}

static int vfs_mkfile(fs_t *fs, strv_t path)
{
	if (strbuf_find(&fs->paths, path, NULL) == 0) {
		return EEXIST;
	}

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes);
	if (node == NULL) {
		return ENOMEM;
	}
	node->type = FS_NODE_TYPE_FILE;

	return 0;
}

static int ofs_rmdir(fs_t *fs, strv_t path)
{
	(void)fs;

	if (ofs_isfile(fs, path)) {
		return EINVAL;
	}

#if defined(C_WIN)
	if (RemoveDirectoryA(path.data) == 0) {
		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) {
			return ENOENT;
		} else if (err == ERROR_DIR_NOT_EMPTY) {
			return ENOTEMPTY;
		} else {
			return -1;
		}
	}
#else
	errno = 0;
	if (remove(path.data)) {
		return errno;
	}
#endif
	return 0;
}

static strv_t path_trim(strv_t path)
{
	if (path.len <= 0) {
		return path;
	}

	if (path.data[path.len - 1] == '/' || path.data[path.len - 1] == '\\') {
		path.len--;
	}

	return path;
}

static int vfs_rmdir(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_find(&fs->paths, path, &index)) {
		return ENOENT;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return EINVAL;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return EINVAL;
	}

	uint i = 0;
	strv_t s;
	strbuf_foreach(&fs->paths, i, s)
	{
		strv_t parent = path_trim(pathv_get_dir(s, NULL));

		if (strv_eq(parent, path)) {
			return ENOTEMPTY;
		}
	}

	strbuf_set(&fs->paths, STRV(""), index);
	return 0;
}

static int ofs_rmfile(fs_t *fs, strv_t path)
{
	(void)fs;

	if (ofs_isdir(fs, path)) {
		return EISDIR;
	}

#if defined(C_WIN)
	if (DeleteFileA(path.data) == 0) {
		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) {
			return ENOENT;
		} else {
			return -1;
		}
	}
#else
	errno = 0;
	if (remove(path.data)) {
		return errno;
	}
#endif
	return 0;
}

static int vfs_rmfile(fs_t *fs, strv_t path)
{
	uint index = -1;
	if (strbuf_find(&fs->paths, path, &index)) {
		return ENOENT;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return EINVAL;
	}

	if (node->type == FS_NODE_TYPE_DIR) {
		return EISDIR;
	}

	strbuf_set(&fs->paths, STRV(""), index);
	return 0;
}

static int ofs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	(void)fs;

#if defined(C_WIN)
	path_t child_path = {0};
	path_init(&child_path, path);

	WIN32_FIND_DATA data = {0};
	HANDLE find	     = NULL;

	path_child(&child_path, STRV("*.*"));

	if ((find = FindFirstFileA(child_path.data, (LPWIN32_FIND_DATAA)&data)) == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_PATH_NOT_FOUND) {
			return ENOENT;
		} else if (err == ERROR_DIRECTORY) {
			return ENOTDIR;
		} else {
			return -1;
		}
	}
	child_path.len = path.len;

	do {
		strv_t name = strv_cstr((char *)data.cFileName);
#else
	DIR *dir = opendir(path.data);
	if (!dir) {
		return errno;
	}

	struct dirent *dp;

	while ((dp = readdir(dir))) {
		strv_t name = strv_cstr(dp->d_name);
#endif
		if (strv_eq(name, STRV(".")) || strv_eq(name, STRV(".."))) {
			continue;
		}

#if defined(C_WIN)
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
#else
		if (dp->d_type != 4)
#endif
		{
			continue;
		}

		if (strbuf_add(dirs, name, NULL)) {
			return ENOMEM;
		}
	}
#if defined(C_WIN)
	while (FindNextFileA(find, (LPWIN32_FIND_DATAA)&data));
	FindClose(find);
#else
	closedir(dir);
#endif

	return 0;
}

static int vfs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	uint index = 0;
	strv_t dir;

	if (strbuf_find(&fs->paths, path, &index)) {
		return ENOENT;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return EINVAL;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return ENOTDIR;
	}

	index = 0;
	strbuf_foreach(&fs->paths, index, dir)
	{
		strv_t name;
		strv_t parent = path_trim(pathv_get_dir(dir, &name));

		if (!strv_eq(parent, path)) {
			continue;
		}

		node = arr_get(&fs->nodes, index);
		if (node == NULL) {
			return EINVAL;
		}

		if (node->type != FS_NODE_TYPE_DIR) {
			continue;
		}

		if (strbuf_add(dirs, name, NULL)) {
			return ENOMEM;
		}
	}

	return 0;
}

static int ofs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	(void)fs;

#if defined(C_WIN)
	path_t child_path = {0};
	path_init(&child_path, path);

	WIN32_FIND_DATA data = {0};
	HANDLE find	     = NULL;

	path_child(&child_path, STRV("*.*"));

	if ((find = FindFirstFileA(child_path.data, (LPWIN32_FIND_DATAA)&data)) == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_PATH_NOT_FOUND) {
			return ENOENT;
		} else if (err == ERROR_DIRECTORY) {
			return ENOTDIR;
		} else {
			return -1;
		}
	}
	child_path.len = path.len;

	do {
		strv_t name = strv_cstr((char *)data.cFileName);
#else
	DIR *dir = opendir(path.data);
	if (!dir) {
		return errno;
	}

	struct dirent *dp;

	while ((dp = readdir(dir))) {
		strv_t name = strv_cstr(dp->d_name);
#endif
		if (strv_eq(name, STRV(".")) || strv_eq(name, STRV(".."))) {
			continue;
		}

#if defined(C_WIN)
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
		if (dp->d_type != 8)
#endif
		{
			continue;
		}

		if (strbuf_add(files, name, NULL)) {
			return ENOMEM;
		}
	}
#if defined(C_WIN)
	while (FindNextFileA(find, (LPWIN32_FIND_DATAA)&data));
	FindClose(find);
#else
	closedir(dir);
#endif

	return 0;
}

static int vfs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	uint index = 0;
	strv_t dir;

	if (strbuf_find(&fs->paths, path, &index)) {
		return ENOENT;
	}

	fs_node_t *node = arr_get(&fs->nodes, index);
	if (node == NULL) {
		return EINVAL;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return ENOTDIR;
	}

	strbuf_foreach(&fs->paths, index, dir)
	{
		strv_t name;
		strv_t parent = path_trim(pathv_get_dir(dir, &name));

		if (!strv_eq(parent, path)) {
			continue;
		}

		node = arr_get(&fs->nodes, index);
		if (node == NULL) {
			return EINVAL;
		}

		if (node->type != FS_NODE_TYPE_FILE) {
			continue;
		}

		if (strbuf_add(files, name, NULL)) {
			return ENOMEM;
		}
	}

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
			.lsdir	= ofs_lsdir,
			.lsfile = ofs_lsfile,
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
			.lsdir	= vfs_lsdir,
			.lsfile = vfs_lsfile,
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
		errno = EINVAL;
		return NULL;
	}

	void *file = fs->ops.open(fs, STRVN(buf.data, buf.len), mode);

	if (file == NULL) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to open file : %s: \"%s\"", log_strerror(errnum), buf.data);
		errno = errnum;
		return NULL;
	}

	return file;
}

int fs_close(fs_t *fs, void *file)
{
	if (fs == NULL || file == NULL) {
		return EINVAL;
	}

	return fs->ops.close(fs, file);
}

int fs_isdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return 0;
	}

	return fs->ops.isdir(fs, path_trim(STRVN(buf.data, buf.len)));
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
		return EINVAL;
	}

	int ret = fs->ops.mkdir(fs, path_trim(STRVN(buf.data, buf.len)));
	if (ret) {
		log_error("cutils", "file", NULL, "failed to create directory: %s: \"%s\"", log_strerror(ret), buf.data);
	}

	return ret;
}

int fs_mkfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return EINVAL;
	}

	int ret = fs->ops.mkfile(fs, STRVN(buf.data, buf.len));
	if (ret) {
		log_error("cutils", "file", NULL, "failed to create file: %s: \"%s\"", log_strerror(ret), buf.data);
	}

	return ret;
}

int fs_rmdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return EINVAL;
	}

	int ret = fs->ops.rmdir(fs, path_trim(STRVN(buf.data, buf.len)));
	if (ret) {
		log_error("cutils", "file", NULL, "failed to remove directory: %s: \"%s\"", log_strerror(ret), buf.data);
	}

	return ret;
}

int fs_rmfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL) {
		return EINVAL;
	}

	int ret = fs->ops.rmfile(fs, STRVN(buf.data, buf.len));
	if (ret) {
		log_error("cutils", "file", NULL, "failed to remove file: %s: \"%s\"", log_strerror(ret), buf.data);
	}

	return ret;
}

int fs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	path = path_trim(path);

	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL || dirs == NULL) {
		return EINVAL;
	}

	int ret = fs->ops.lsdir(fs, STRVN(buf.data, buf.len), dirs);
	if (ret) {
		log_error("cutils", "file", NULL, "failed to list directories: %s: \"%s\"", log_strerror(ret), buf.data);
	} else {
		strbuf_sort(dirs);
	}

	return ret;
}

int fs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	path = path_trim(path);

	path_t buf = {0};
	if (fs == NULL || path.data == NULL || path_init(&buf, path) == NULL || files == NULL) {
		return EINVAL;
	}

	int ret = fs->ops.lsfile(fs, STRVN(buf.data, buf.len), files);
	if (ret) {
		log_error("cutils", "file", NULL, "failed to list files: %s: \"%s\"", log_strerror(ret), buf.data);
	} else {
		strbuf_sort(files);
	}

	return ret;
}
