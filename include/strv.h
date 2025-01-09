#ifndef STRV_H
#define STRV_H

#include <stddef.h>

typedef struct strv_s {
	size_t len;
	const char *data;
} strv_t;

#define STRVS(_str, _len) {.data = _str, .len = _len}
#define STRVC(_str)	  (strv_t) STRVS(_str, sizeof(_str) - 1)
#define STRV(_str, _len)  (strv_t) STRVS(_str, _len)
#define STRV_NULL	  ((strv_t){0})

#endif
