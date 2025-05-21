#include "strvbuf.h"

#include "log.h"

strvbuf_t *strvbuf_init(strvbuf_t *buf, uint cap, size_t len, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	if (buf_init(buf, cap * (sizeof(size_t) + len), alloc) == NULL) {
		log_error("cutils", "strvbuf", NULL, "failed to intialize buffer");
		return NULL;
	}

	return buf;
}

void strvbuf_free(strvbuf_t *buf)
{
	buf_free(buf);
}

void strvbuf_reset(strvbuf_t *buf, size_t used)
{
	buf_reset(buf, used);
}

int strvbuf_add(strvbuf_t *buf, strv_t strv, size_t *off)
{
	if (buf == NULL) {
		return 1;
	}

	size_t used = buf->used;

	if (buf_add(buf, &strv.len, sizeof(size_t), off)) {
		log_error("cutils", "strvbuf", NULL, "failed to add size");
		return 1;
	}

	if (buf_add(buf, strv.data, strv.len, NULL)) {
		buf_reset(buf, used);
		log_error("cutils", "strvbuf", NULL, "failed to add string: %zu", strv.len);
		return 1;
	}

	return 0;
}

strv_t strvbuf_get(const strvbuf_t *buf, size_t off)
{
	if (buf == NULL) {
		return STRV_NULL;
	}

	size_t *data = buf_get(buf, off);
	if (data == NULL) {
		log_error("cutils", "strvbuf", NULL, "failed to get string");
		return STRV_NULL;
	}

	return (strv_t){
		.len  = *data,
		.data = (const char *)(data + 1),
	};
}

int strvbuf_set(strvbuf_t *buf, size_t off, strv_t strv)
{
	if (buf == NULL) {
		return 1;
	}

	size_t *len = buf_get(buf, off);
	if (len == NULL) {
		log_error("cutils", "strvbuf", NULL, "invalid offset: %zu", off);
		return 1;
	}

	if (buf_replace(buf, off + sizeof(size_t), strv.data, *len, strv.len) == NULL) {
		log_error("cutils", "strvbuf", NULL, "failed to set string: '%.*s'", strv.len, strv.data);
		return 1;
	}

	len  = buf_get(buf, off);
	*len = strv.len;

	return 0;
}

int strvbuf_app(strvbuf_t *buf, size_t off, strv_t strv)
{
	if (buf == NULL) {
		return 1;
	}

	size_t *len = buf_get(buf, off);
	if (len == NULL) {
		log_error("cutils", "strvbuf", NULL, "invalid offset: %zu", off);
		return 1;
	}

	if (buf_replace(buf, off + sizeof(size_t) + *len, strv.data, 0, strv.len) == NULL) {
		log_error("cutils", "strvbuf", NULL, "failed to append string: '%.*s'", strv.len, strv.data);
		return 1;
	}

	len = buf_get(buf, off);
	*len += strv.len;

	return 0;
}
