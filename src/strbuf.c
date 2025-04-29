#include "strbuf.h"

strbuf_t *strbuf_init(strbuf_t *buf, uint cap, size_t len, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	if (buf_init(&buf->buf, cap * (sizeof(size_t) + len), alloc) == NULL || arr_init(&buf->off, cap, sizeof(size_t), alloc) == NULL) {
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

	if (buf_add(&buf->buf, &strv.len, sizeof(size_t), NULL)) {
		return 1;
	}

	size_t used = buf->buf.used;

	if (buf_add(&buf->buf, strv.data, strv.len, NULL)) {
		return 1;
	}

	size_t *off = arr_add(&buf->off, index);
	if (off == NULL) {
		return 1;
	}

	*off = used;

	return 0;
}

strv_t strbuf_get(const strbuf_t *buf, uint index)
{
	if (buf == NULL) {
		return STRV_NULL;
	}

	size_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return STRV_NULL;
	}

	void *ptr = buf_get(&buf->buf, *off);

	return (strv_t){
		.len  = *((size_t *)ptr - 1),
		.data = (const char *)ptr,
	};
}

int strbuf_find(const strbuf_t *buf, strv_t strv, uint *index)
{
	if (buf == NULL) {
		return 1;
	}

	uint i = 0;
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

	size_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return 1;
	}

	size_t *len = (size_t *)buf_get(&buf->buf, *off) - 1;

	if (buf_replace(&buf->buf, *off, strv.data, *len, strv.len) == NULL) {
		return 1;
	}

	off = arr_get(&buf->off, index);
	len = (size_t *)buf_get(&buf->buf, *off) - 1;

	index++;
	arr_foreach_i(&buf->off, off, index)
	{
		*off += (strv.len - *len);
	}

	*len = strv.len;

	return 0;
}

int strbuf_app(strbuf_t *buf, strv_t strv, uint index)
{
	if (buf == NULL) {
		return 1;
	}

	size_t *off = arr_get(&buf->off, index);
	if (off == NULL) {
		return 1;
	}

	size_t *len = (size_t *)buf_get(&buf->buf, *off) - 1;

	if (buf_replace(&buf->buf, *off + *len, strv.data, 0, strv.len) == NULL) {
		return 1;
	}

	off = arr_get(&buf->off, index);
	len = (size_t *)buf_get(&buf->buf, *off) - 1;

	index++;
	arr_foreach_i(&buf->off, off, index)
	{
		*off += strv.len;
	}

	*len += strv.len;

	return 0;
}

static int cmp(const void *v1, const void *v2, const void *priv)
{
	const buf_t *buf = priv;

	const void *p1 = buf_get(buf, *(size_t *)v1);
	const void *p2 = buf_get(buf, *(size_t *)v2);

	return strv_cmp(STRVN((const char *)p1, *((const size_t *)p1 - 1)), STRVN((const char *)p2, *((const size_t *)p2 - 1)));
}

strbuf_t *strbuf_sort(strbuf_t *buf)
{
	if (buf == NULL) {
		return NULL;
	}

	arr_sort(&buf->off, cmp, &buf->buf);

	return buf;
}
