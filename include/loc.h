#ifndef LOC_H
#define LOC_H

#include <stddef.h>

typedef struct loc_s {
	size_t off;
	size_t len;
} loc_t;

int loc_eq(loc_t l, loc_t r);

#define LOCT(_off, _len) {.off = _off, .len = _len}
#define LOC(_off, _len)	 ((loc_t)LOCT(_off, _len))

#endif
