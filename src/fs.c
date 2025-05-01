#include "fs.h"

#include "cfs.h"
#include "log.h"
#include "path.h"

typedef cerr_t (*fs_open_fn)(fs_t *fs, strv_t path, const char *mode, void **file);
typedef cerr_t (*fs_close_fn)(fs_t *fs, void *file);

typedef cerr_t (*fs_write_fn)(fs_t *fs, void *file, strv_t str);
typedef cerr_t (*fs_read_fn)(fs_t *fs, void *file, str_t str, size_t size);

typedef cerr_t (*fs_du_fn)(fs_t *fs, void *file, size_t *size);

typedef int (*fs_isdir_fn)(fs_t *fs, strv_t path);
typedef int (*fs_isfile_fn)(fs_t *fs, strv_t path);

typedef cerr_t (*fs_mkdir_fn)(fs_t *fs, strv_t path);
typedef cerr_t (*fs_mkfile_fn)(fs_t *fs, strv_t path);

typedef cerr_t (*fs_rmdir_fn)(fs_t *fs, strv_t path);
typedef cerr_t (*fs_rmfile_fn)(fs_t *fs, strv_t path);

typedef cerr_t (*fs_getcwd_fn)(fs_t *fs, str_t *path);

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
	fs_getcwd_fn getcwd;
	fs_lsdir_fn lsdir;
	fs_lsfile_fn lsfile;
} fs_ops_t;

typedef enum fs_node_type_e {
	FS_NODE_TYPE_UNKNOWN,
	FS_NODE_TYPE_FILE,
	FS_NODE_TYPE_DIR,
} fs_node_type_t;

typedef enum fs_node_flag_e {
	FS_NODE_FLAG_OPEN,
	FS_NODE_FLAG_WRITE,
} fs_node_flag_t;

typedef struct fs_node_s {
	fs_node_type_t type;
	str_t data;
	int flags;
} fs_node_t;

static cerr_t ofs_open(fs_t *fs, strv_t path, const char *mode, void **file)
{
	(void)fs;
	return cfs_open(path.data, mode, file);
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

static cerr_t vfs_open(fs_t *fs, strv_t path, const char *mode, void **file)
{
	if (path.data == NULL || mode == NULL || file == NULL) {
		return CERR_VAL;
	}

	switch (mode[0]) {
	case 'r':
	case 'w':
	case 'a': break;
	default: return CERR_VAL;
	}

	uint id;
	if (strbuf_find(&fs->paths, path, &id)) {
		strv_t name = {0};
		strv_t dir  = path_trim(pathv_get_dir(path, &name));

		if ((mode[0] == 'w' || mode[0] == 'a') && name.len > 0 && (dir.len == 0 || fs_isdir(fs, dir))) {
			strbuf_add(&fs->paths, path, &id);
			fs_node_t *node = arr_add(&fs->nodes, NULL);
			if (node == NULL) {
				return CERR_MEM;
			}
			node->type = FS_NODE_TYPE_FILE;
			node->data = STR_NULL;
		} else {
			return CERR_NOT_FOUND;
		}
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_VAL;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		return node->type == FS_NODE_TYPE_DIR ? CERR_TYPE : CERR_VAL;
	}

	if (node->data.data == NULL) {
		node->data = strz(256);
	}

	switch (mode[0]) {
	case 'r': node->flags &= ~(1 << FS_NODE_FLAG_WRITE); break;
	case 'w': node->data.len = 0; // fall-through
	case 'a': node->flags |= 1 << FS_NODE_FLAG_WRITE; break;
	}

	node->flags |= 1 << FS_NODE_FLAG_OPEN;

	*file = (void *)((size_t)id + 1);
	return CERR_OK;
}

static cerr_t ofs_close(fs_t *fs, void *file)
{
	(void)fs;
	return cfs_close(file);
}

static cerr_t vfs_close(fs_t *fs, void *file)
{
	if (file == NULL) {
		return CERR_VAL;
	}

	uint id = (uint)((size_t)file - 1);

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	node->flags &= ~(1 << FS_NODE_FLAG_OPEN);
	node->flags &= ~(1 << FS_NODE_FLAG_WRITE);

	return CERR_OK;
}

static cerr_t ofs_write(fs_t *fs, void *file, strv_t str)
{
	(void)fs;
	return cfs_write(file, str.data, str.len);
}

static cerr_t vfs_write(fs_t *fs, void *file, strv_t str)
{
	if (file == NULL || str.data == NULL) {
		return CERR_VAL;
	}

	uint id = (uint)((size_t)file - 1);

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	if (!(node->flags & (1 << FS_NODE_FLAG_OPEN)) || !(node->flags & (1 << FS_NODE_FLAG_WRITE))) {
		return CERR_DESC;
	}

	if (str_cat(&node->data, str) == NULL) {
		return CERR_MEM;
	}

	return CERR_OK;
}

static cerr_t ofs_read(fs_t *fs, void *file, str_t str, size_t size)
{
	(void)fs;
	return cfs_read(file, str.data, size);
}

static cerr_t vfs_read(fs_t *fs, void *file, str_t str, size_t size)
{
	uint id = (uint)((size_t)file - 1);

	fs_node_t *node = arr_get(&fs->nodes, id);

	str_cat(&str, STRVN(node->data.data, size));

	return CERR_OK;
}

static cerr_t ofs_du(fs_t *fs, void *file, size_t *size)
{
	(void)fs;
	return cfs_du(file, size);
}

static cerr_t vfs_du(fs_t *fs, void *file, size_t *size)
{
	if (size == NULL) {
		return CERR_VAL;
	}

	uint id = (uint)((size_t)file - 1);

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	*size = node->data.len;
	return CERR_OK;
}

static int ofs_isdir(fs_t *fs, strv_t path)
{
	(void)fs;
	return cfs_isdir(path.data);
}

static int vfs_isdir(fs_t *fs, strv_t path)
{
	uint id = -1;
	if (strbuf_find(&fs->paths, path, &id)) {
		return 0;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
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
	return cfs_isfile(path.data);
}

static int vfs_isfile(fs_t *fs, strv_t path)
{
	uint id = -1;
	if (strbuf_find(&fs->paths, path, &id)) {
		return 0;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return 0;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		return 0;
	}

	return 1;
}

static cerr_t ofs_mkdir(fs_t *fs, strv_t path)
{
	(void)fs;
	return cfs_mkdir(path.data);
}

static cerr_t vfs_mkdir(fs_t *fs, strv_t path)
{
	if (path.len == 0) {
		return CERR_NOT_FOUND;
	}

	if (strbuf_find(&fs->paths, path, NULL) == 0) {
		return CERR_EXIST;
	}

	strv_t parent = path_trim(pathv_get_dir(path, NULL));
	if (parent.len > 0 && !vfs_isdir(fs, parent)) {
		return CERR_NOT_FOUND;
	}

	size_t paths_used = fs->paths.buf.used;
	uint paths_cnt	  = fs->paths.off.cnt;

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes, NULL);
	if (node == NULL) {
		fs->paths.buf.used = paths_used;
		fs->paths.off.cnt  = paths_cnt;
		return CERR_MEM;
	}
	node->type = FS_NODE_TYPE_DIR;

	return CERR_OK;
}

static cerr_t ofs_mkfile(fs_t *fs, strv_t path)
{
	(void)fs;
	return cfs_mkfile(path.data);
}

static cerr_t vfs_mkfile(fs_t *fs, strv_t path)
{
	if (path.len == 0) {
		return CERR_NOT_FOUND;
	}

	uint id;
	if (strbuf_find(&fs->paths, path, &id) == 0) {
		fs_node_t *node = arr_get(&fs->nodes, id);
		return node->type == FS_NODE_TYPE_FILE ? CERR_EXIST : CERR_TYPE;
	}

	strv_t parent = path_trim(pathv_get_dir(path, NULL));
	if (parent.len > 0 && !vfs_isdir(fs, parent)) {
		return CERR_NOT_FOUND;
	}

	strbuf_add(&fs->paths, path, NULL);
	fs_node_t *node = arr_add(&fs->nodes, NULL);
	if (node == NULL) {
		return CERR_MEM;
	}
	node->type = FS_NODE_TYPE_FILE;
	node->data = STR_NULL;

	return CERR_OK;
}

static cerr_t ofs_rmdir(fs_t *fs, strv_t path)
{
	(void)fs;
	return cfs_rmdir(path.data);
}

static cerr_t vfs_rmdir(fs_t *fs, strv_t path)
{
	uint id = -1;
	if (strbuf_find(&fs->paths, path, &id)) {
		return CERR_NOT_FOUND;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return CERR_NOT_FOUND;
	}

	uint i = 0;
	strv_t s;
	strbuf_foreach(&fs->paths, i, s)
	{
		strv_t parent = path_trim(pathv_get_dir(s, NULL));

		if (strv_eq(parent, path)) {
			return CERR_NOT_EMPTY;
		}
	}

	strbuf_set(&fs->paths, id, STRV(""));
	node->type = FS_NODE_TYPE_UNKNOWN;

	return CERR_OK;
}

static cerr_t ofs_rmfile(fs_t *fs, strv_t path)
{
	(void)fs;
	return cfs_rmfile(path.data);
}

static cerr_t vfs_rmfile(fs_t *fs, strv_t path)
{
	uint id;
	if (strbuf_find(&fs->paths, path, &id)) {
		return CERR_NOT_FOUND;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	if (node->type != FS_NODE_TYPE_FILE) {
		return CERR_NOT_FOUND;
	}

	if (node->data.data) {
		str_free(&node->data);
		node->data = STR_NULL;
	}

	strbuf_set(&fs->paths, id, STRV(""));
	node->type = FS_NODE_TYPE_UNKNOWN;

	return CERR_OK;
}

static cerr_t ofs_getcwd(fs_t *fs, str_t *path)
{
	(void)fs;
	cerr_t err = cfs_getcwd(path->data, path->size);
	if (err == CERR_OK) {
		path->len = strv_cstr(path->data).len;
	}

	return err;
}

static cerr_t vfs_getcwd(fs_t *fs, str_t *path)
{
	(void)fs;
	if (path->size == 0) {
		return CERR_VAL;
	}

	strv_t cwd = STRV("/");

	if (path->size < cwd.len + 1) {
		return CERR_MEM;
	}

	path->len = 0;
	str_cat(path, cwd);

	return CERR_OK;
}

static int ofs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	(void)fs;

	void *it;
	char name[256] = {0};

	cerr_t i;
	cfs_lsdir_foreach(i, path.data, &it, name, sizeof(name))
	{
		if (strbuf_add(dirs, strv_cstr(name), NULL)) {
			return CERR_MEM;
		}
	}

	return i == CERR_END ? 0 : i;
}

static int vfs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	uint id;
	strv_t dir;

	if (strbuf_find(&fs->paths, path, &id)) {
		return CERR_NOT_FOUND;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return CERR_TYPE;
	}

	id = 0;
	strbuf_foreach(&fs->paths, id, dir)
	{
		strv_t name;
		strv_t parent = path_trim(pathv_get_dir(dir, &name));

		if (!strv_eq(parent, path)) {
			continue;
		}

		node = arr_get(&fs->nodes, id);
		if (node == NULL) {
			return CERR_NOT_FOUND;
		}

		if (node->type != FS_NODE_TYPE_DIR) {
			continue;
		}

		if (strbuf_add(dirs, name, NULL)) {
			return CERR_MEM;
		}
	}

	return CERR_OK;
}

static int ofs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	(void)fs;

	void *it;
	char name[256] = {0};

	cerr_t i;
	cfs_lsfile_foreach(i, path.data, &it, name, sizeof(name))
	{
		if (strbuf_add(files, strv_cstr(name), NULL)) {
			return CERR_MEM;
		}
	}

	return i == CERR_END ? 0 : i;
}

static int vfs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	uint id = 0;
	strv_t dir;

	if (strbuf_find(&fs->paths, path, &id)) {
		return CERR_NOT_FOUND;
	}

	fs_node_t *node = arr_get(&fs->nodes, id);
	if (node == NULL) {
		return CERR_NOT_FOUND;
	}

	if (node->type != FS_NODE_TYPE_DIR) {
		return CERR_TYPE;
	}

	strbuf_foreach(&fs->paths, id, dir)
	{
		strv_t name;
		strv_t parent = path_trim(pathv_get_dir(dir, &name));

		if (!strv_eq(parent, path)) {
			continue;
		}

		node = arr_get(&fs->nodes, id);
		if (node == NULL) {
			return CERR_NOT_FOUND;
		}

		if (node->type != FS_NODE_TYPE_FILE) {
			continue;
		}

		if (strbuf_add(files, name, NULL)) {
			return CERR_MEM;
		}
	}

	return 0;
}

static const fs_ops_t s_fs_ops[] = {
	[0] =
		{
			.open	= ofs_open,
			.close	= ofs_close,
			.write	= ofs_write,
			.read	= ofs_read,
			.du	= ofs_du,
			.isdir	= ofs_isdir,
			.isfile = ofs_isfile,
			.mkdir	= ofs_mkdir,
			.mkfile = ofs_mkfile,
			.rmdir	= ofs_rmdir,
			.rmfile = ofs_rmfile,
			.getcwd = ofs_getcwd,
			.lsdir	= ofs_lsdir,
			.lsfile = ofs_lsfile,
		},
	[1] =
		{
			.open	= vfs_open,
			.close	= vfs_close,
			.write	= vfs_write,
			.read	= vfs_read,
			.du	= vfs_du,
			.isdir	= vfs_isdir,
			.isfile = vfs_isfile,
			.mkdir	= vfs_mkdir,
			.mkfile = vfs_mkfile,
			.rmdir	= vfs_rmdir,
			.rmfile = vfs_rmfile,
			.getcwd = vfs_getcwd,
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

	return fs;
}

void fs_free(fs_t *fs)
{
	if (fs == NULL || !fs->virt) {
		return;
	}

	fs_node_t *node;
	uint i = 0;
	arr_foreach(&fs->nodes, i, node)
	{
		if (node->type == FS_NODE_TYPE_FILE && node->data.data) {
			str_free(&node->data);
		}
	}

	arr_free(&fs->nodes);
	strbuf_free(&fs->paths);
}

int fs_open(fs_t *fs, strv_t path, const char *mode, void **file)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].open(fs, STRVS(buf), mode, file);
	if (err) {
		log_error("cutils", "file", NULL, "failed to open file for %s: %s: \"%s\"", mode, cerr_str(err), buf.data);
	}

	return err;
}

int fs_close(fs_t *fs, void *file)
{
	if (fs == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].close(fs, file);
	if (err) {
		log_error("cutils", "file", NULL, "failed to close file: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int fs_write(fs_t *fs, void *file, strv_t str)
{
	if (fs == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].write(fs, file, str);
	if (err) {
		log_error("cutils", "file", NULL, "failed to write file: %s", cerr_str(err));
		return err;
	}

	return CERR_OK;
}

int fs_read(fs_t *fs, strv_t path, int b, str_t *str)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err;

	void *file;
	err = fs_open(fs, STRVS(buf), "rb", &file);
	if (err) {
		return err;
	}

	size_t size;
	err = fs_du(fs, file, &size);
	if (err) {
		return err; // LCOV_EXCL_LINE
	}

	if (str_resize(str, size + !b)) {
		fs_close(fs, file);
		return CERR_MEM;
	}

	str->len = 0;
	s_fs_ops[fs->virt].read(fs, file, *str, size);

	if (b) {
		str->len = size;
	} else {
		str->len = 0;
		for (size_t i = 0; i < size; i++) {
			str->data[str->len] = str->data[i];
			if (str->data[i] != '\r') {
				str->len++;
			}
		}
		str->data[str->len] = '\0';
	}

	fs_close(fs, file);
	return CERR_OK;
}

int fs_du(fs_t *fs, void *file, size_t *size)
{
	if (fs == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].du(fs, file, size);
	if (err) {
		log_error("cutils", "file", NULL, "failed to get file size: %s", cerr_str(err));
	}

	return err;
}

int fs_isdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return 0;
	}

	return s_fs_ops[fs->virt].isdir(fs, path_trim(STRVS(buf)));
}

int fs_isfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return 0;
	}

	return s_fs_ops[fs->virt].isfile(fs, STRVS(buf));
}

int fs_mkdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].mkdir(fs, path_trim(STRVS(buf)));
	if (err) {
		log_error("cutils", "file", NULL, "failed to create directory: %s: \"%s\"", cerr_str(err), buf.data);
	}

	return err;
}

int fs_mkfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].mkfile(fs, STRVS(buf));
	if (err) {
		log_error("cutils", "file", NULL, "failed to create file: %s: \"%s\"", cerr_str(err), buf.data);
	}

	return err;
}

int fs_rmdir(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].rmdir(fs, path_trim(STRVS(buf)));
	if (err) {
		log_error("cutils", "file", NULL, "failed to remove directory: %s: \"%s\"", cerr_str(err), buf.data);
	}

	return err;
}

int fs_rmfile(fs_t *fs, strv_t path)
{
	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].rmfile(fs, STRVS(buf));
	if (err) {
		log_error("cutils", "file", NULL, "failed to remove file: %s: \"%s\"", cerr_str(err), buf.data);
	}

	return err;
}

int fs_getcwd(fs_t *fs, str_t *path)
{
	if (fs == NULL || path == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].getcwd(fs, path);
	if (err) {
		log_error("cutils", "file", NULL, "failed to get cwd: %s", cerr_str(err));
	}

	return err;
}

int fs_lsdir(fs_t *fs, strv_t path, strbuf_t *dirs)
{
	path = path_trim(path);

	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL || dirs == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].lsdir(fs, STRVS(buf), dirs);
	if (err) {
		log_error("cutils", "file", NULL, "failed to list directories: %s: \"%s\"", cerr_str(err), buf.data);
		return err;
	}

	strbuf_sort(dirs);

	return CERR_OK;
}

int fs_lsfile(fs_t *fs, strv_t path, strbuf_t *files)
{
	path = path_trim(path);

	path_t buf = {0};
	if (fs == NULL || path_init(&buf, path) == NULL || files == NULL) {
		return CERR_VAL;
	}

	cerr_t err = s_fs_ops[fs->virt].lsfile(fs, STRVS(buf), files);
	if (err) {
		log_error("cutils", "file", NULL, "failed to list files: %s: \"%s\"", cerr_str(err), buf.data);
		return err;
	}

	strbuf_sort(files);

	return CERR_OK;
}

size_t dputs_fs(dst_t dst, strv_t str)
{
	size_t len = str.len;
	if (fs_write((fs_t *)dst.priv, dst.dst, STRVS(str))) {
		len = 0;
	}
	return len;
}

size_t dputv_fs(dst_t dst, const char *fmt, va_list args)
{
	str_t str  = strv(fmt, args);
	size_t len = str.len;
	if (fs_write((fs_t *)dst.priv, dst.dst, STRVS(str))) {
		len = 0;
	}
	str_free(&str);
	return len;
}
