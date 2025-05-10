#ifndef BUF_H
#define BUF_H

#include "alloc.h"
#include "dst.h"

typedef struct buf_s {
	void *data;
	size_t size;
	size_t used;
	alloc_t alloc;
} buf_t;

void *buf_init(buf_t *buf, size_t size, alloc_t alloc);
void buf_free(buf_t *buf);

void buf_reset(buf_t *buf, size_t used);

int buf_add(buf_t *buf, const void *data, size_t size, size_t *off);
void *buf_get(const buf_t *buf, size_t off);

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len);

size_t buf_print(const buf_t *buf, dst_t dst);

#endif
