#include "buf.h"

#include "alloc.h"
#include "log.h"
#include "mem.h"
#include "type.h"

void *buf_init(buf_t *buf, size_t size, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	buf->data = alloc_alloc(&alloc, size);
	if (buf->data == NULL) {
		log_error("cutils", "buf", NULL, "failed to allocate data");
		return NULL;
	}

	buf->size  = size;
	buf->used  = 0;
	buf->alloc = alloc;
	return buf;
}

void buf_free(buf_t *buf)
{
	if (buf == NULL) {
		return;
	}

	alloc_free(&buf->alloc, buf->data, buf->size);
	buf->data = NULL;
	buf->size = 0;
	buf->used = 0;
}

void buf_reset(buf_t *buf, size_t used)
{
	if (buf == NULL) {
		return;
	}

	if (used > buf->used) {
		used = buf->used;
	}

	buf->used = used;
}

static int buf_grow(buf_t *buf, size_t size)
{
	if (buf->used + size > buf->size) {
		if (alloc_realloc(&buf->alloc, &buf->data, &buf->size, (buf->used + size) * 2)) {
			log_error("cutils", "buf", NULL, "failed to grow buffer");
			return 1;
		}
	}

	return 0;
}

int buf_add(buf_t *buf, const void *data, size_t size, size_t *off)
{
	if (buf == NULL || buf_grow(buf, size)) {
		return 1;
	}

	mem_copy((uint8_t *)buf->data + buf->used, buf->size - buf->used, data, size);
	if (off) {
		*off = buf->used;
	}
	buf->used += size;
	return 0;
}

int buf_adds(buf_t *buf, strv_t str, loc_t *loc)
{
	size_t off;
	if (buf_add(buf, str.data, str.len, &off)) {
		return 1;
	}

	if (loc) {
		loc->off = off;
		loc->len = str.len;
	}

	return 0;
}

void *buf_get(const buf_t *buf, size_t off)
{
	if (buf == NULL) {
		return NULL;
	}

	if (off >= buf->used) {
		log_error("cutils", "buf", NULL, "invalid offset: %zu/%zu", off, buf->used);
		return NULL;
	}

	return (uint8_t *)buf->data + off;
}

strv_t buf_gets(const buf_t *buf, loc_t loc)
{
	if (loc.len == 0) {
		return STRV_NULL;
	}

	return STRVN(buf_get(buf, loc.off), loc.len);
}

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len)
{
	if (buf == NULL) {
		return NULL;
	}

	if (new_len > old_len && buf_grow(buf, new_len - old_len)) {
		return NULL;
	}

	mem_replace(&((uint8_t *)buf->data)[off], buf->size - off, buf->used - off, data, old_len, new_len);

	buf->used += new_len - old_len;
	return buf;
}

size_t buf_print(const buf_t *buf, dst_t dst)
{
	if (buf == 0) {
		return 0;
	}

	size_t off = dst.off;

	dst.off += dputf(dst, "%08X  ", 0);
	for (size_t i = 0; i < buf->used; i++) {
		dst.off += dputf(dst, "%02x ", ((uint8_t *)buf->data)[i]);
		if (i > 0 && (i + 1) % 16 == 0) {
			if (i + 1 < buf->used) {
				dst.off += dputf(dst, "\n%08X  ", i + 1);
			}
		} else if (i > 0 && (i + 1) % 8 == 0) {
			dst.off += dputs(dst, STRV(" "));
		}
	}

	dst.off += dputs(dst, STRV("\n"));
	return dst.off - off;
}
