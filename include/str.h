#ifndef STR_H
#define STR_H

#include "print.h"
#include "strv.h"

typedef struct str_s {
	size_t size;
	size_t len;
	char *data;
} str_t;

str_t strz(size_t size);
str_t strn(const char *cstr, size_t len, size_t size);
str_t strv(const char *fmt, va_list args);
str_t strf(const char *fmt, ...);

void str_free(str_t *str);

void str_zero(str_t *str);

int str_resize(str_t *str, size_t size);

str_t *str_cat(str_t *str, strv_t src);

int str_to_upper(strv_t str, str_t *dst);

int str_replace(str_t *str, strv_t from, strv_t to, int *found);
int str_replaces(str_t *str, const strv_t *from, const strv_t *to, size_t cnt, int *found);
int str_rreplaces(str_t *str, const strv_t *from, const strv_t *to, size_t cnt);

int str_subreplace(str_t *dst, size_t start, size_t end, strv_t str);

#define STRB(_buf, _len)  ((str_t){.size = sizeof(_buf), .len = _len, .data = _buf})
#define STRN(_str, _size) strn(_str, sizeof(_str) - 1, _size)
#define STRS(_str)	  strn(_str, (_str).len, (_str).len + 1)
#define STR(_str)	  STRN(_str, sizeof(_str))
#define STR_NULL	  ((str_t){0})

#endif
