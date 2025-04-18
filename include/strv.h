#ifndef STRV_H
#define STRV_H

#include "print.h"

typedef struct strv_s {
	size_t len;
	const char *data;
} strv_t;

strv_t strv_cstr(const char *cstr);

int strv_eq(strv_t l, strv_t r);
int strv_cmp(strv_t l, strv_t r);
int strv_cmpn(strv_t l, strv_t r, size_t len);

int strv_to_int(strv_t str, int *res);

int strv_split(strv_t str, char c, strv_t *l, strv_t *r);
int strv_rsplit(strv_t str, char c, strv_t *l, strv_t *r);

int strv_print(strv_t str, print_dst_t dst);

#define STRVTN(_str, _len) {.len = _len, .data = _str}
#define STRVT(_str)	   STRVTN(_str, sizeof(_str) - 1)
#define STRVN(_str, _len)  (strv_t) STRVTN(_str, _len)
#define STRV(_str)	   (strv_t) STRVT(_str)
#define STRVS(_str)	   STRVN((_str).data, (_str).len)
#define STRV_NULL	   ((strv_t){0})

#endif
