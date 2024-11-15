#ifndef FILE_H
#define FILE_H

#include "path.h"

#include <stdio.h>

FILE *file_open(const char *path, const char *mode);
FILE *file_open_v(const char *format, const char *mode, va_list args);
FILE *file_open_f(const char *format, const char *mode, ...);

FILE *file_reopen(const char *path, const char *mode, FILE *file);

size_t file_read(FILE *file, size_t size, char *data, size_t data_size);
size_t file_read_t(const char *path, char *data, size_t data_size);
size_t file_read_ft(FILE *file, char *data, size_t data_size);

size_t file_size(FILE *file);

int file_close(FILE *file);

int file_delete(const char *path);

int file_exists(const char *path);
int file_exists_v(const char *format, va_list args);
int file_exists_f(const char *format, ...);

int folder_create(const char *path);
int folder_create_v(const char *path, va_list args);
int folder_create_f(const char *path, ...);
int folder_delete(const char *path);
int folder_delete_v(const char *path, va_list args);
int folder_delete_f(const char *path, ...);
int folder_exists(const char *path);
int folder_exists_v(const char *format, va_list args);
int folder_exists_f(const char *format, ...);

typedef int (*files_foreach_cb)(path_t *path, const char *folder, void *priv);
int files_foreach(const path_t *path, files_foreach_cb on_folder, files_foreach_cb on_file, void *priv);

int c_fflush(FILE *file);

int c_fprintv(FILE *file, const char *fmt, va_list args);
int c_fprintf(FILE *file, const char *fmt, ...);
int c_fprintv_cb(print_dst_t dst, const char *fmt, va_list args);

#define PRINT_DST_FILE(_file) ((print_dst_t){.cb = c_fprintv_cb, .dst = _file})

#endif
