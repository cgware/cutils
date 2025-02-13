#ifndef STRBUF_H
#define STRBUF_H

#include "buf.h"
#include "strv.h"
#include "type.h"

typedef struct strbuf_s {
	buf_t buf;
	uint cnt;
} strbuf_t;

strbuf_t *strbuf_init(strbuf_t *buf, size_t size, alloc_t alloc);
void strbuf_free(strbuf_t *buf);

int strbuf_add(strbuf_t *buf, strv_t strv, uint *index);

#define strbuf_get(_buf, _i, _start, _len)                                                                                                 \
	_start = _i + sizeof(uint8_t);                                                                                                     \
	_len   = *(uint8_t *)((_buf)->buf.data + _i)

int strbuf_get_index(const strbuf_t *buf, strv_t strv, uint *index);

#define strbuf_foreach(_buf, _i, _start, _len)                                                                                             \
	for (_start = _i + sizeof(uint8_t), _len = (_buf)->buf.used > 0 ? *(uint8_t *)((_buf)->buf.data + _i) : 0; _i < (_buf)->buf.used;  \
	     _i += sizeof(uint8_t) + sizeof(char) * _len,                                                                                  \
	    _start += sizeof(char) * _len + sizeof(uint8_t),                                                                               \
	    _len = *(uint8_t *)((_buf)->buf.data + _i))

#endif
