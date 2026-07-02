#ifndef BUF_H
#define BUF_H

#include "alloc.h"
#include "dst.h"
#include "loc.h"

typedef enum endian_e {
	ENDIAN_HOST,
	ENDIAN_LITTLE,
	ENDIAN_BIG,
} endian_t;

typedef struct buf_s {
	void *data;
	size_t size;
	size_t used;
	alloc_t alloc;
} buf_t;

void *buf_init(buf_t *buf, size_t size, alloc_t alloc);
void buf_free(buf_t *buf);

void buf_reset(buf_t *buf, size_t used);

int buf_resize(buf_t *buf, size_t size);

int buf_set(buf_t *buf, size_t off, size_t size, const void *data);
int buf_add(buf_t *buf, size_t size, const void *data, size_t *off);

int buf_set_int(buf_t *buf, size_t off, size_t size, endian_t endian, const void *data);
int buf_add_int(buf_t *buf, size_t size, endian_t endian, const void *data);

int buf_set_str(buf_t *buf, size_t off, strv_t str, loc_t *loc);
int buf_add_str(buf_t *buf, strv_t str, loc_t *loc);

void *buf_get(const buf_t *buf, size_t off);
void *buf_read(const buf_t *buf, size_t size, size_t *off);

int buf_get_int(const buf_t *buf, size_t off, size_t size, endian_t endian, void *val);
int buf_read_int(const buf_t *buf, size_t *off, size_t size, endian_t endian, void *val);

strv_t buf_get_str(const buf_t *buf, loc_t loc);
strv_t buf_read_str(const buf_t *buf, loc_t loc, size_t *off);

int buf_cmp(const buf_t *buf, size_t off, size_t size, const void *data);

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len);

size_t buf_print(const buf_t *buf, dst_t dst);

#endif
