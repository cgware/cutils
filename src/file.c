#include "file.h"

#include "arr.h"
#include "cstr.h"
#include "log.h"
#include "mem.h"
#include "platform.h"
#include "print.h"

#include <errno.h>
#include <string.h>

#if defined(C_WIN)
	#include <Windows.h>
#else
	#include <dirent.h>
	#include <sys/stat.h>
#endif

void *file_open(const char *path, const char *mode)
{
	if (path == NULL || mode == NULL) {
		return NULL;
	}

	FILE *file = NULL;

	errno = 0;
#if defined(C_WIN)
	fopen_s(&file, path, mode);
#else
	file = fopen(path, mode);
#endif
	if (file == NULL) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to open file \"%s\": %s (%d)", path, log_strerror(errnum), errnum);
	}
	return file;
}

FILE *file_open_v(const char *format, const char *mode, va_list args)
{
	char path[C_MAX_PATH] = {0};

	if (c_sprintv(path, sizeof(path) / sizeof(char), 0, format, args) < 0) {
		return NULL;
	}

	return file_open(path, mode);
}

FILE *file_open_f(const char *format, const char *mode, ...)
{
	va_list args;
	va_start(args, mode);
	FILE *file = file_open_v(format, mode, args);
	va_end(args);
	return file;
}

FILE *file_reopen(const char *path, const char *mode, FILE *file)
{
	if ((path == NULL && file != stdout && file != stderr) || mode == NULL || file == NULL) {
		return NULL;
	}

	errno = 0;
#if defined(C_WIN)
	if (path == NULL) {
		return NULL;
	}
	freopen_s(&file, path, mode, file);
#else
	file = freopen(path, mode, file);
#endif
	if (file == NULL) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to reopen file \"%s\": %s (%d)", path, log_strerror(errnum), errnum);
	}
	return file;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

size_t file_read(FILE *file, size_t size, char *data, size_t data_size)
{
	if (file == NULL || data == NULL) {
		return 0;
	}

	size_t cnt;
	size = MIN(file_size(file), size);

	if (size >= data_size) {
		log_error("cutils", "file", NULL, "buffer too small %d/%d", data_size, size);
		return 0;
	}

#if defined(C_WIN)
	cnt = fread_s(data, data_size, size, 1, file);
#else
	cnt = fread(data, size, 1, file);
#endif
	if (cnt != 1) {
		return 0;
	}

	return size;
}

size_t file_read_ft(FILE *file, char *data, size_t data_size)
{
	if (data == NULL) {
		return 0;
	}

	const size_t size = file_read(file, data_size, data, data_size);

	size_t len = 0;
	for (size_t i = 0; i < size; i++) {
		data[len] = data[i];
		if (data[i] != '\r') {
			len++;
		}
	}
	data[len] = '\0';

	return len;
}

size_t file_read_t(const char *path, char *data, size_t data_size)
{
	FILE *file = file_open(path, "rb");
	if (file == NULL) {
		return -1;
	}

	const size_t len = file_read_ft(file, data, data_size);

	file_close(file);

	return len;
}

size_t file_size(FILE *file)
{
	if (file == NULL) {
		return 0;
	}

	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	return size;
}

int file_close(void *file)
{
	if (file == NULL) {
		return EOF;
	}

	return fclose(file);
}

int file_delete(const char *path)
{
	if (path == NULL) {
		return 1;
	}

	int ret;
#if defined(C_WIN)
	ret = DeleteFileA(path) == 0 ? 1 : 0;
#else
	errno = 0;
	ret   = remove(path);
	if (ret != 0) {
		int errnum = errno;
		log_error("cutils", "file", NULL, "failed to delete file \"%s\": %s (%d)", path, log_strerror(errnum), errnum);
		ret = 1;
	}
#endif
	return ret;
}

int file_exists(const char *path)
{
	if (path == NULL) {
		return 0;
	}

#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (stat(path, &buffer)) {
		return 0;
	} else {
		return 1;
	}
#endif
}

int file_exists_v(const char *format, va_list args)
{
	char path[C_MAX_PATH] = {0};

	if (c_sprintv(path, sizeof(path) / sizeof(char), 0, format, args) < 0) {
		return 0;
	}

	return file_exists(path);
}

int file_exists_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int exists = file_exists_v(format, args);
	va_end(args);
	return exists;
}

int folder_create(const char *path)
{
	if (path == NULL) {
		return 1;
	}

#if defined(C_WIN)
	return CreateDirectoryA(path, NULL) == 0 ? 1 : 0 || ERROR_ALREADY_EXISTS == GetLastError();
#else
	struct stat st = {0};
	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
	}
	return 0;
#endif
}

int folder_create_v(const char *format, va_list args)
{
	char path[C_MAX_PATH] = {0};

	if (c_sprintv(path, sizeof(path) / sizeof(char), 0, format, args) < 0) {
		return 1;
	}

	return folder_create(path);
}

int folder_create_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = folder_create_v(format, args);
	va_end(args);
	return ret;
}

int folder_delete(const char *path)
{
#if defined(C_WIN)
	return RemoveDirectoryA(path) == 0 ? 1 : 0;
#else
	return remove(path) == 0 ? 0 : 1;
#endif
}

int folder_delete_v(const char *format, va_list args)
{
	char path[C_MAX_PATH] = {0};

	if (c_sprintv(path, sizeof(path) / sizeof(char), 0, format, args) < 0) {
		return 1;
	}

	return folder_delete(path);
}

int folder_delete_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = folder_delete_v(format, args);
	va_end(args);
	return ret;
}

int folder_exists(const char *path)
{
	if (path == NULL) {
		return 0;
	}

#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return 1;
	} else {
		return 0;
	}
#endif
}

int folder_exists_v(const char *format, va_list args)
{
	char path[C_MAX_PATH] = {0};

	if (c_sprintv(path, sizeof(path) / sizeof(char), 0, format, args) < 0) {
		return 0;
	}

	return folder_exists(path);
}

int folder_exists_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int exists = folder_exists_v(format, args);
	va_end(args);
	return exists;
}

static int name_cmp_cb(const void *a, const void *b)
{
	return strcmp(a, b);
}

int files_foreach(const path_t *path, files_foreach_cb on_folder, files_foreach_cb on_file, void *priv)
{
	if (path == NULL) {
		return 1;
	}

	int ret = 0;

	arr_t files = {0};
	arr_t dirs  = {0};

	arr_init(&files, 32, sizeof(char) * C_MAX_PATH, ALLOC_STD);
	arr_init(&dirs, 32, sizeof(char) * C_MAX_PATH, ALLOC_STD);

	path_t child_path = *path;

#if defined(C_WIN)
	WIN32_FIND_DATA file = {0};
	HANDLE find	     = NULL;

	path_child(&child_path, STRV("*.*"));

	if ((find = FindFirstFileA(child_path.data, (LPWIN32_FIND_DATAA)&file)) == INVALID_HANDLE_VALUE) {
		ret = 1;
		goto exit;
	}
	child_path.len = path->len;

	do {
		if (strcmp((char *)file.cFileName, ".") == 0 || strcmp((char *)file.cFileName, "..") == 0) {
			continue;
		}

		arr_t *arr = file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? &dirs : &files;

		if (arr == NULL) {
			continue;
		}

		char *dest = arr_add(arr);
		if (dest == NULL) {
			return 1;
		}

		mem_copy(dest, C_MAX_PATH, (char *)file.cFileName, cstr_len((char *)file.cFileName) + 1);

	} while (FindNextFileA(find, (LPWIN32_FIND_DATAA)&file));

	FindClose(find);

#else

	DIR *dir = opendir(path->data);
	if (!dir) {
		ret = 1;
		goto exit;
	}

	struct dirent *dp;

	while ((dp = readdir(dir))) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}

		arr_t *arr = NULL;
		switch (dp->d_type) {
		case 4: arr = &dirs; break;
		case 8: arr = &files; break;
		default: continue;
		}

		char *dest = arr_add(arr);
		if (dest == NULL) {
			return 1;
		}

		mem_copy(dest, C_MAX_PATH, dp->d_name, cstr_len(dp->d_name) + 1);
	}

	closedir(dir);
#endif

	const char *name;
	if (on_file) {
		arr_sort(&files, name_cmp_cb);
		arr_foreach(&files, name)
		{
			path_child(&child_path, STRVN(name, cstr_len(name)));

			ret = on_file(&child_path, name, priv);

			child_path.len = path->len;
			if (ret < 0) {
				goto exit;
			}
		}
	}

	if (on_folder) {
		arr_sort(&dirs, name_cmp_cb);
		arr_foreach(&dirs, name)
		{
			path_child(&child_path, STRVN(name, cstr_len(name)));

			ret = on_folder(&child_path, name, priv);

			child_path.len = path->len;
			if (ret < 0) {
				goto exit;
			}
		}
	}

exit:
	arr_free(&files);
	arr_free(&dirs);

	return ret;
}

int c_fflush(FILE *file)
{
	if (file == NULL) {
		return 1;
	}

	return fflush(file);
}

int c_fprintv(FILE *file, const char *fmt, va_list args)
{
	if (file == NULL || fmt == NULL) {
		return 0;
	}

	va_list copy;
	va_copy(copy, args);
	int ret;
	errno = 0;
#if defined(C_WIN)
	ret = vfprintf_s(file, fmt, copy);
#else
	ret = vfprintf(file, fmt, copy);
#endif
	if (ret < 0) {
		int errnum = errno;
		log_error("cutils", "print", NULL, "failed to write to file: %s (%d)", log_strerror(errnum), errnum);
		ret = 0;
	}
	va_end(copy);
	return ret;
}

int c_fprintf(FILE *file, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = c_fprintv(file, fmt, args);
	va_end(args);
	return ret;
}

int c_fprintv_cb(print_dst_t dst, const char *fmt, va_list args)
{
	int ret = c_fprintv((FILE *)dst.dst, fmt, args);
	c_fflush((FILE *)dst.dst);
	return ret;
}

void *c_fopen_cb(print_dst_ex_t dst, const char *path, const char *mode)
{
	(void)dst;
	if (path == NULL || mode == NULL) {
		return NULL;
	}
	return file_open(path, mode);
}

int c_fclose_cb(print_dst_ex_t dst)
{
	if (dst.dst.dst == NULL) {
		return 0;
	}

	return fclose(dst.dst.dst);
}

void *c_dopen(print_dst_ex_t dst, const char *path, const char *mode)
{
	if (dst.open == NULL) {
		return dst.dst.dst;
	}

	return dst.open(dst, path, mode);
}

int c_dclose(print_dst_ex_t dst)
{
	if (dst.close == NULL) {
		return 0;
	}

	return dst.close(dst);
}
