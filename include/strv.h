#ifndef STRV_H
#define STRV_H

#include "print.h"

typedef struct strv_s {
	size_t len;
	const char *data;
} strv_t;

int strv_eq(strv_t l, strv_t r);
int strv_print(strv_t str, print_dst_t dst);

#define STRVSN(_str, _len) {.data = _str, .len = _len}
#define STRVS(_str)	   STRVSN(_str, sizeof(_str) - 1)
#define STRVN(_str, _len)  (strv_t) STRVSN(_str, _len)
#define STRV(_str)	   (strv_t) STRVS(_str)
#define STRV_NULL	   ((strv_t){0})

#endif
