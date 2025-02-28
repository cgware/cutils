#include "strbuf.h"

strbuf_t *strbuf_init(strbuf_t *buf, uint cap, size_t size, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	if (buf_init(&buf->buf, cap * size, alloc) == NULL) {
		return NULL;
	}

	if (arr_init(&buf->off, cap, sizeof(uint8_t), alloc) == NULL) {
		return NULL;
	}

	return buf;
}

void strbuf_free(strbuf_t *buf)
{
	if (buf == NULL) {
		return;
	}

	buf_free(&buf->buf);
	arr_free(&buf->off);
}

void strbuf_reset(strbuf_t *buf, uint cnt)
{
	if (buf == NULL) {
		return;
	}

	if (cnt > buf->off.cap) {
		cnt = buf->off.cap;
	}

	arr_reset(&buf->off, 0, cnt);

	buf->buf.used = 0;
	buf->off.cnt  = cnt;
}

int strbuf_add(strbuf_t *buf, strv_t strv, uint *index)
{
	if (buf == NULL) {
		return 1;
	}

	if (index) {
		*index = buf->off.cnt;
	}

	size_t used = buf->buf.used;

	if (buf_add(&buf->buf, strv.data, strv.len, NULL)) {
		return 1;
	}

	uint8_t *off = arr_add(&buf->off);
	if (off == NULL) {
		return 1;
	}

	*off = (uint8_t)used;

	return 0;
}

strv_t strbuf_get(const strbuf_t *buf, uint index)
{
	if (buf == NULL) {
		return STRV_NULL;
	}

	uint8_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return STRV_NULL;
	}

	return (strv_t){
		.len  = ((index + 1 < buf->off.cnt) ? (size_t)*(uint8_t *)arr_get(&buf->off, index + 1) : buf->buf.used) - *off,
		.data = (const char *)buf_get(&buf->buf, *off),
	};
}

int strbuf_get_index(const strbuf_t *buf, strv_t strv, uint *index)
{
	if (buf == NULL) {
		return 1;
	}

	size_t i = 0;
	strv_t val;

	strbuf_foreach(buf, i, val)
	{
		if (strv_eq(val, strv)) {
			if (index) {
				*index = i;
			}
			return 0;
		}
	}

	return 1;
}

int strbuf_set(strbuf_t *buf, strv_t strv, uint index)
{
	if (buf == NULL) {
		return 1;
	}

	uint8_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return 1;
	}

	size_t len = ((index + 1 < buf->off.cnt) ? (size_t)*(uint8_t *)arr_get(&buf->off, index + 1) : buf->buf.used) - *off;

	if (buf_replace(&buf->buf, *off, strv.data, len, strv.len) == NULL) {
		return 1;
	}

	index++;
	arr_foreach_i(&buf->off, off, index)
	{
		*off += (uint8_t)(strv.len - len);
	}

	return 0;
}

int strbuf_app(strbuf_t *buf, strv_t strv, uint index)
{
	if (buf == NULL) {
		return 1;
	}

	uint8_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return 1;
	}

	size_t len = ((index + 1 < buf->off.cnt) ? (size_t)*(uint8_t *)arr_get(&buf->off, index + 1) : buf->buf.used) - *off;

	if (buf_replace(&buf->buf, *off + len, strv.data, 0, strv.len) == NULL) {
		return 1;
	}

	index++;
	arr_foreach_i(&buf->off, off, index)
	{
		*off += (uint8_t)strv.len;
	}

	return 0;
}
