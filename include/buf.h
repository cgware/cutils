#ifndef BUF_H
#define BUF_H

#include "alloc.h"
#include "print.h"

#include <stdint.h>

typedef struct buf_s {
	uint8_t *data;
	size_t size;
	size_t used;
	alloc_t alloc;
} buf_t;

void *buf_init(buf_t *buf, size_t size, alloc_t alloc);
void buf_free(buf_t *buf);

int buf_add(buf_t *buf, const void *data, size_t size, size_t *index);
#define buf_get(_buf, _index) ((_buf)->data + _index)

int buf_print(const buf_t *buf, print_dst_t dst);

#endif
