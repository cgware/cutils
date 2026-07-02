#include "buf.h"

#include "log.h"
#include "mem.h"

static int add_overflows(size_t a, size_t b)
{
	return b > (size_t)-1 - a;
}

static int host_is_le(void)
{
	u16 e = 1;
	return *(u8 *)&e;
}

static int endian_needs_swap(endian_t endian)
{
	if (endian == ENDIAN_HOST) {
		return 0;
	}

	return (endian == ENDIAN_LITTLE) != host_is_le();
}

void *buf_init(buf_t *buf, size_t size, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	void *data = alloc_alloc(&alloc, size);
	if (data == NULL) {
		log_error("cutils", "buf", NULL, "failed to allocate data");
		return NULL;
	}

	buf->data  = data;
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

int buf_resize(buf_t *buf, size_t size)
{
	if (buf == NULL) {
		return 1;
	}

	if (size <= buf->size) {
		return 0;
	}

	if (alloc_realloc(&buf->alloc, &buf->data, &buf->size, size)) {
		log_error("cutils", "buf", NULL, "failed to resize buffer");
		return 1;
	}

	return 0;
}

int buf_set(buf_t *buf, size_t off, size_t size, const void *data)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(off, size) || off + size > buf->size) {
		return 1;
	}

	if (size == 0) {
		return 0;
	}

	if (data == NULL) {
		return 1;
	}

	mem_copy((uint8_t *)buf->data + off, buf->size - off, data, size);
	return 0;
}

int buf_add(buf_t *buf, size_t size, const void *data, size_t *off)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, size)) {
		return 1;
	}

	size_t used = buf->used + size;
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	if (buf_set(buf, buf->used, size, data)) {
		return 1;
	}

	if (off) {
		*off = buf->used;
	}

	buf->used += size;

	return 0;
}

int buf_set_int(buf_t *buf, size_t off, size_t size, endian_t endian, const void *data)
{
	if (buf == NULL || data == NULL) {
		return 1;
	}

	if (add_overflows(off, size) || off + size > buf->size) {
		return 1;
	}

	u8 *dst	      = (u8 *)buf->data + off;
	const u8 *src = data;

	if (endian_needs_swap(endian)) {
		for (size_t i = 0; i < size; i++) {
			dst[i] = src[size - i - 1];
		}
	} else {
		for (size_t i = 0; i < size; i++) {
			dst[i] = src[i];
		}
	}

	return 0;
}

int buf_add_int(buf_t *buf, size_t size, endian_t endian, const void *data)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, size)) {
		return 1;
	}

	size_t used = buf->used + size;
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	if (buf_set_int(buf, buf->used, size, endian, data)) {
		return 1;
	}

	buf->used = used;
	return 0;
}

int buf_set_str(buf_t *buf, size_t off, strv_t str, loc_t *loc)
{
	if (buf_set(buf, off, str.len, str.data)) {
		return 1;
	}

	if (loc) {
		loc->off = off;
		loc->len = str.len;
	}

	return 0;
}

int buf_add_str(buf_t *buf, strv_t str, loc_t *loc)
{
	size_t off;
	if (buf_add(buf, str.len, str.data, &off)) {
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

void *buf_read(const buf_t *buf, size_t size, size_t *off)
{
	if (buf == NULL || off == NULL) {
		return NULL;
	}

	if (add_overflows(*off, size) || *off + size > buf->used) {
		return NULL;
	}

	void *data = buf_get(buf, *off);
	if (data != NULL) {
		*off += size;
	}

	return data;
}

int buf_get_int(const buf_t *buf, size_t off, size_t size, endian_t endian, void *val)
{
	if (buf == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(off, size) || off + size > buf->used) {
		return 1;
	}

	u8 *data = buf_get(buf, off);
	if (data == NULL) {
		return 1;
	}

	u8 *ptr = val;
	if (endian_needs_swap(endian)) {
		for (size_t i = 0; i < size; i++) {
			ptr[i] = data[size - i - 1];
		}
	} else {
		for (size_t i = 0; i < size; i++) {
			ptr[i] = data[i];
		}
	}

	return 0;
}

int buf_read_int(const buf_t *buf, size_t *off, size_t size, endian_t endian, void *val)
{
	if (off == NULL) {
		return 1;
	}

	int ret = buf_get_int(buf, *off, size, endian, val);
	if (ret == 0) {
		*off += size;
	}

	return ret;
}

strv_t buf_get_str(const buf_t *buf, loc_t loc)
{
	if (loc.len == 0) {
		return STRV_NULL;
	}

	if (buf == NULL || add_overflows(loc.off, loc.len) || loc.off + loc.len > buf->used) {
		return STRV_NULL;
	}

	return STRVN(buf_get(buf, loc.off), loc.len);
}

strv_t buf_read_str(const buf_t *buf, loc_t loc, size_t *off)
{
	if (off == NULL) {
		return STRV_NULL;
	}

	strv_t ret = buf_get_str(buf, loc);
	if (ret.data != NULL) {
		*off += loc.len;
	}

	return ret;
}

int buf_cmp(const buf_t *buf, size_t off, size_t size, const void *data)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(off, size) || off + size > buf->used) {
		return 1;
	}

	if (size == 0) {
		return 0;
	}

	if (data == NULL) {
		return 1;
	}

	const u8 *ptr = buf->data;
	return mem_cmp(&ptr[off], data, size);
}

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len)
{
	if (buf == NULL) {
		return NULL;
	}

	if (add_overflows(off, old_len) || off + old_len > buf->used) {
		return NULL;
	}

	if (new_len > old_len && add_overflows(buf->used - old_len, new_len)) {
		return NULL;
	}

	size_t used = buf->used - old_len + new_len;
	if (new_len > old_len && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
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
