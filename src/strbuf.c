#include "strbuf.h"

#include "log.h"

strbuf_t *strbuf_init(strbuf_t *buf, uint cap, size_t len, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	if (strvbuf_init(&buf->buf, cap, len, alloc) == NULL || arr_init(&buf->off, cap, sizeof(size_t), alloc) == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to intialize buffer or array");
		return NULL;
	}

	return buf;
}

void strbuf_free(strbuf_t *buf)
{
	if (buf == NULL) {
		return;
	}

	strvbuf_free(&buf->buf);
	arr_free(&buf->off);
}

void strbuf_reset(strbuf_t *buf, uint cnt)
{
	if (buf == NULL) {
		return;
	}

	if (cnt > buf->off.cnt) {
		cnt = buf->off.cnt;
	}

	size_t used;

	if (cnt > 0) {
		size_t *off = arr_get(&buf->off, cnt - 1);
		strv_t last = strvbuf_get(&buf->buf, *off);
		used	    = *off + sizeof(size_t) + last.len;
	} else {
		used = 0;
	}

	strvbuf_reset(&buf->buf, used);
	arr_reset(&buf->off, cnt);
}

int strbuf_add(strbuf_t *buf, strv_t strv, uint *id)
{
	if (buf == NULL) {
		return 1;
	}

	uint cnt = buf->off.cnt;

	size_t *off = arr_add(&buf->off, id);
	if (off == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to add offset");
		return 1;
	}

	if (strvbuf_add(&buf->buf, strv, off)) {
		arr_reset(&buf->off, cnt);
		log_error("cutils", "strbuf", NULL, "failed to add string");
		return 1;
	}

	return 0;
}

strv_t strbuf_get(const strbuf_t *buf, uint id)
{
	if (buf == NULL) {
		return STRV_NULL;
	}

	size_t *off = arr_get(&buf->off, id);
	if (off == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to get offset");
		return STRV_NULL;
	}

	strv_t str = strvbuf_get(&buf->buf, *off);
	if (str.data == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to get string");
		return STRV_NULL;
	}

	return str;
}

int strbuf_find(const strbuf_t *buf, strv_t strv, uint *id)
{
	if (buf == NULL) {
		return 1;
	}

	uint i = 0;
	strv_t val;

	strbuf_foreach(buf, i, val)
	{
		if (strv_eq(val, strv)) {
			if (id) {
				*id = i;
			}
			return 0;
		}
	}

	return 1;
}

int strbuf_set(strbuf_t *buf, uint id, strv_t strv)
{
	if (buf == NULL) {
		return 1;
	}

	size_t *off = arr_get(&buf->off, id);
	if (off == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to get offset");
		return 1;
	}

	size_t *len = buf_get(&buf->buf, *off);
	size_t diff = strv.len - *len;

	if (strvbuf_set(&buf->buf, *off, strv)) {
		log_error("cutils", "strbuf", NULL, "failed to set string: '%.*s'", strv.len, strv.data);
		return 1;
	}

	id++;
	arr_foreach(&buf->off, id, off)
	{
		*off += diff;
	}

	return 0;
}

int strbuf_app(strbuf_t *buf, uint id, strv_t strv)
{
	if (buf == NULL) {
		return 1;
	}

	size_t *off = arr_get(&buf->off, id);
	if (off == NULL) {
		log_error("cutils", "strbuf", NULL, "failed to get offset");
		return 1;
	}

	if (strvbuf_app(&buf->buf, *off, strv)) {
		log_error("cutils", "strbuf", NULL, "failed to append string: '%.*s'", strv.len, strv.data);
		return 1;
	}

	id++;
	arr_foreach(&buf->off, id, off)
	{
		*off += strv.len;
	}

	return 0;
}

static int cmp(const void *v1, const void *v2, const void *priv)
{
	return strv_cmp(strvbuf_get(priv, *(size_t *)v1), strvbuf_get(priv, *(size_t *)v2));
}

strbuf_t *strbuf_sort(strbuf_t *buf)
{
	if (buf == NULL) {
		return NULL;
	}

	arr_sort(&buf->off, cmp, &buf->buf);

	return buf;
}
