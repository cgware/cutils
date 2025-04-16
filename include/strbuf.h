#ifndef STRBUF_H
#define STRBUF_H

#include "arr.h"
#include "buf.h"
#include "strv.h"
#include "type.h"

typedef struct strbuf_s {
	buf_t buf;
	arr_t off;
} strbuf_t;

strbuf_t *strbuf_init(strbuf_t *buf, uint cap, size_t len, alloc_t alloc);
void strbuf_free(strbuf_t *buf);

void strbuf_reset(strbuf_t *buf, uint cnt);

int strbuf_add(strbuf_t *buf, strv_t strv, uint *index);

strv_t strbuf_get(const strbuf_t *buf, uint index);
int strbuf_find(const strbuf_t *buf, strv_t strv, uint *index);

int strbuf_set(strbuf_t *buf, strv_t strv, uint index);
int strbuf_app(strbuf_t *buf, strv_t strv, uint index);

strbuf_t *strbuf_sort(strbuf_t *buf);

#define strbuf_foreach(_buf, _i, _strv) for (; _i < (_buf)->off.cnt && (_strv = strbuf_get(_buf, _i), 1); _i++)

#endif
