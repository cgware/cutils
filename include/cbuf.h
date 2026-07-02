#ifndef CBUF_H
#define CBUF_H

#include "type.h"

#include <stddef.h>

int cbuf_set_le(void *buf, size_t off, const void *val, size_t size);
int cbuf_set_u8le(void *buf, size_t off, u8 val);
int cbuf_set_u16le(void *buf, size_t off, u16 val);
int cbuf_set_u32le(void *buf, size_t off, u32 val);
int cbuf_set_u64le(void *buf, size_t off, u64 val);

int cbuf_set_be(void *buf, size_t off, const void *val, size_t size);
int cbuf_set_u8be(void *buf, size_t off, u8 val);
int cbuf_set_u16be(void *buf, size_t off, u16 val);
int cbuf_set_u32be(void *buf, size_t off, u32 val);
int cbuf_set_u64be(void *buf, size_t off, u64 val);

int cbuf_get_le(const void *buf, size_t off, void *val, size_t size);
int cbuf_get_u8le(const void *buf, size_t off, u8 *val);
int cbuf_get_u16le(const void *buf, size_t off, u16 *val);
int cbuf_get_u32le(const void *buf, size_t off, u32 *val);
int cbuf_get_u64le(const void *buf, size_t off, u64 *val);

int cbuf_get_be(const void *buf, size_t off, void *val, size_t size);
int cbuf_get_u8be(const void *buf, size_t off, u8 *val);
int cbuf_get_u16be(const void *buf, size_t off, u16 *val);
int cbuf_get_u32be(const void *buf, size_t off, u32 *val);
int cbuf_get_u64be(const void *buf, size_t off, u64 *val);

int cbuf_write_le(void *buf, size_t *off, const void *val, size_t size);
int cbuf_write_u8le(void *buf, size_t *off, u8 val);
int cbuf_write_u16le(void *buf, size_t *off, u16 val);
int cbuf_write_u32le(void *buf, size_t *off, u32 val);
int cbuf_write_u64le(void *buf, size_t *off, u64 val);

int cbuf_write_be(void *buf, size_t *off, const void *val, size_t size);
int cbuf_write_u8be(void *buf, size_t *off, u8 val);
int cbuf_write_u16be(void *buf, size_t *off, u16 val);
int cbuf_write_u32be(void *buf, size_t *off, u32 val);
int cbuf_write_u64be(void *buf, size_t *off, u64 val);

int cbuf_read_le(const void *buf, size_t *off, void *val, size_t size);
int cbuf_read_u8le(const void *buf, size_t *off, u8 *val);
int cbuf_read_u16le(const void *buf, size_t *off, u16 *val);
int cbuf_read_u32le(const void *buf, size_t *off, u32 *val);
int cbuf_read_u64le(const void *buf, size_t *off, u64 *val);

int cbuf_read_be(const void *buf, size_t *off, void *val, size_t size);
int cbuf_read_u8be(const void *buf, size_t *off, u8 *val);
int cbuf_read_u16be(const void *buf, size_t *off, u16 *val);
int cbuf_read_u32be(const void *buf, size_t *off, u32 *val);
int cbuf_read_u64be(const void *buf, size_t *off, u64 *val);

#endif
