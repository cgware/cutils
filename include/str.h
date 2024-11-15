#ifndef STR_H
#define STR_H

#include "print.h"

typedef struct str_s {
	const char *data;
	size_t size;
	size_t len;
	int ref;
} str_t;

str_t str_null();
str_t strz(size_t size);
str_t strc(const char *cstr, size_t len);
str_t strs(str_t str);
str_t strn(const char *cstr, size_t len, size_t size);
str_t strv(const char *fmt, va_list args);
str_t strf(const char *fmt, ...);
str_t strb(const char *buf, size_t size, size_t len);
str_t strr();

void str_free(str_t *str);

void str_zero(str_t *str);

int str_resize(str_t *str, size_t size);

str_t *str_catc(str_t *str, const char *cstr, size_t len);
str_t *str_catn(str_t *str, str_t src, size_t len);
str_t *str_cat(str_t *str, str_t src);

int str_cmpnc(str_t str, const char *cstr, size_t cstr_len, size_t len);
int str_cmpc(str_t str, const char *cstr, size_t cstr_len);
int str_cmpn(str_t str, str_t src, size_t len);
int str_cmp(str_t str, str_t src);

int str_eqnc(str_t str, const char *cstr, size_t cstr_len, size_t len);
int str_eqc(str_t str, const char *cstr, size_t cstr_len);
int str_eqn(str_t str, str_t s, size_t len);
int str_eq(str_t str, str_t s);

int str_chr(str_t str, str_t *l, str_t *r, char c);
int str_cstr(str_t str, str_t *l, str_t *r, const char *s, size_t s_len);

str_t str_cpy(str_t src);
int str_cpyd(str_t src, str_t *dst);

int str_to_upper(str_t str, str_t *dst);

int str_split(str_t str, char c, str_t *l, str_t *r);
int str_rsplit(str_t str, char c, str_t *l, str_t *r);

int str_replace(str_t *str, str_t from, str_t to);
int str_replaces(str_t *str, const str_t *from, const str_t *to, size_t cnt);
int str_rreplaces(str_t *str, const str_t *from, const str_t *to, size_t cnt);

int str_print(str_t str, print_dst_t dst);

#define STR(_str)  strc(_str, sizeof(_str) - 1)
#define STRS(_str) {.data = _str, .size = 0, .len = sizeof(_str) - 1, .ref = 1}
#define STRH(_str) strn(_str, sizeof(_str) - 1, sizeof(_str))

#endif
