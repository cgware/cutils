#ifndef STRBUF_H
#define STRBUF_H

#include "buf.h"

typedef buf_t strbuf_t;

#define strbuf_init buf_init
#define strbuf_free buf_free

static inline int strbuf_add(strbuf_t *buf, const char *str, uint8_t len, size_t *index)
{
	return buf_add(buf, &len, sizeof(len), index) || buf_add(buf, str, sizeof(char) * len, NULL);
}

#define strbuf_get(_buf, _i, _start, _len)                                                                                                 \
	_start = _i + sizeof(uint8_t);                                                                                                     \
	_len   = *(uint8_t *)((_buf)->data + _i)

#define strbuf_foreach(_buf, _i, _start, _len)                                                                                             \
	for (_start = _i + sizeof(uint8_t), _len = *(uint8_t *)((_buf)->data + _i); _i < (_buf)->used;                                     \
	     _i += sizeof(uint8_t) + sizeof(char) * _len,                                                                                  \
	    _start += sizeof(char) * _len + sizeof(uint8_t),                                                                               \
	    _len = *(uint8_t *)((_buf)->data + _i))

#endif
