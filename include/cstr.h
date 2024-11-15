#ifndef CSTR_H
#define CSTR_H

#include <stdarg.h>
#include <stddef.h>

size_t cstrv(char *cstr, size_t size, const char *fmt, va_list args);
size_t cstrf(char *cstr, size_t size, const char *fmt, ...);

void *cstr_zero(char *cstr, size_t size);

size_t cstr_len(const char *cstr);

size_t cstr_catn(char *cstr, size_t cstr_size, size_t cstr_len, const char *src, size_t src_len, size_t len);
size_t cstr_cat(char *cstr, size_t cstr_size, size_t cstr_len, const char *src, size_t src_len);

int cstr_cmpn(const char *cstr, size_t cstr_len, const char *src, size_t src_len, size_t len);
int cstr_cmp(const char *cstr, size_t cstr_len, const char *src, size_t src_len);

int cstr_eqn(const char *cstr, size_t cstr_len, const char *src, size_t src_len, size_t len);
int cstr_eq(const char *cstr, size_t cstr_len, const char *src, size_t src_len);

void *cstr_cpy(char *cstr, size_t size, const char *src, size_t len);

char *cstr_chr(const char *cstr, char c);
char *cstr_rchr(const char *cstr, char c);
char *cstr_cstr(const char *cstr, const char *src);

size_t cstr_replace(char *str, size_t str_size, size_t str_len, const char *old, size_t old_len, const char *new, size_t new_len,
		    int *found);
size_t cstr_replaces(char *str, size_t str_size, size_t str_len, const char *const *old, const char *const *new, size_t cnt, int *found);
size_t cstr_rreplaces(char *str, size_t str_size, size_t str_len, const char *const *old, const char *const *new, size_t cnt);

wchar_t *wcstr_catn(wchar_t *wcstr, size_t wcstr_size, const wchar_t *src, size_t cnt);

#define CSTR(_str) _str, sizeof(_str) - 1

#endif
