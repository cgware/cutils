#ifndef BUF_H
#define BUF_H

#include "alloc.h"
#include "dst.h"
#include "loc.h"

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
int buf_adds(buf_t *buf, strv_t str, loc_t *loc);
void *buf_get(const buf_t *buf, size_t off);
strv_t buf_gets(const buf_t *buf, loc_t loc);

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len);

size_t buf_print(const buf_t *buf, dst_t dst);

#endif
