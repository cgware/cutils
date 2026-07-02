#ifndef BUF_H
#define BUF_H

#include "alloc.h"
#include "dst.h"
#include "loc.h"
#include "type.h"

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

int buf_set_str(buf_t *buf, size_t off, strv_t str, loc_t *loc);

int buf_write_le(buf_t *buf, const void *val, size_t size);
int buf_write_u8le(buf_t *buf, u8 val);
int buf_write_u16le(buf_t *buf, u16 val);
int buf_write_u32le(buf_t *buf, u32 val);
int buf_write_u64le(buf_t *buf, u64 val);

int buf_write_be(buf_t *buf, const void *val, size_t size);
int buf_write_u8be(buf_t *buf, u8 val);
int buf_write_u16be(buf_t *buf, u16 val);
int buf_write_u32be(buf_t *buf, u32 val);
int buf_write_u64be(buf_t *buf, u64 val);

int buf_add_str(buf_t *buf, strv_t str, loc_t *loc);

void *buf_get(const buf_t *buf, size_t off);
void *buf_read(const buf_t *buf, size_t size, size_t *off);

int buf_read_le(const buf_t *buf, size_t *off, void *val, size_t size);
int buf_read_u8le(const buf_t *buf, size_t *off, u8 *val);
int buf_read_u16le(const buf_t *buf, size_t *off, u16 *val);
int buf_read_u32le(const buf_t *buf, size_t *off, u32 *val);
int buf_read_u64le(const buf_t *buf, size_t *off, u64 *val);

int buf_read_be(const buf_t *buf, size_t *off, void *val, size_t size);
int buf_read_u8be(const buf_t *buf, size_t *off, u8 *val);
int buf_read_u16be(const buf_t *buf, size_t *off, u16 *val);
int buf_read_u32be(const buf_t *buf, size_t *off, u32 *val);
int buf_read_u64be(const buf_t *buf, size_t *off, u64 *val);

strv_t buf_get_str(const buf_t *buf, loc_t loc);
strv_t buf_read_str(const buf_t *buf, loc_t loc, size_t *off);

int buf_cmp(const buf_t *buf, size_t off, size_t size, const void *data);

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len);

size_t buf_print(const buf_t *buf, dst_t dst);

#endif
