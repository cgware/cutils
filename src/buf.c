#include "buf.h"

#include "cbuf.h"
#include "log.h"
#include "mem.h"

static int add_overflows(size_t a, size_t b)
{
	return b > (size_t)-1 - a;
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

int buf_write_le(buf_t *buf, const void *val, size_t size)
{
	if (buf == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, size)) {
		return 1;
	}

	size_t used = buf->used + size;
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_le(buf->data, &buf->used, val, size);
}

int buf_write_u8le(buf_t *buf, u8 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u8le(buf->data, &buf->used, val);
}

int buf_write_u16le(buf_t *buf, u16 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u16le(buf->data, &buf->used, val);
}

int buf_write_u32le(buf_t *buf, u32 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u32le(buf->data, &buf->used, val);
}

int buf_write_u64le(buf_t *buf, u64 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u64le(buf->data, &buf->used, val);
}

int buf_write_be(buf_t *buf, const void *val, size_t size)
{
	if (buf == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, size)) {
		return 1;
	}

	size_t used = buf->used + size;
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_be(buf->data, &buf->used, val, size);
}

int buf_write_u8be(buf_t *buf, u8 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u8be(buf->data, &buf->used, val);
}

int buf_write_u16be(buf_t *buf, u16 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u16be(buf->data, &buf->used, val);
}

int buf_write_u32be(buf_t *buf, u32 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u32be(buf->data, &buf->used, val);
}

int buf_write_u64be(buf_t *buf, u64 val)
{
	if (buf == NULL) {
		return 1;
	}

	if (add_overflows(buf->used, sizeof(val))) {
		return 1;
	}

	size_t used = buf->used + sizeof(val);
	if (used > buf->size && (used > (size_t)-1 / 2 || buf_resize(buf, used * 2))) {
		return 1;
	}

	return cbuf_write_u64be(buf->data, &buf->used, val);
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

	if (size == 0) {
		return (uint8_t *)buf->data + *off;
	}

	void *data = buf_get(buf, *off);
	if (data != NULL) {
		*off += size;
	}

	return data;
}

int buf_read_le(const buf_t *buf, size_t *off, void *val, size_t size)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, size) || *off + size > buf->used) {
		return 1;
	}

	return cbuf_read_le(buf->data, off, val, size);
}

int buf_read_u8le(const buf_t *buf, size_t *off, u8 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u8le(buf->data, off, val);
}

int buf_read_u16le(const buf_t *buf, size_t *off, u16 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u16le(buf->data, off, val);
}

int buf_read_u32le(const buf_t *buf, size_t *off, u32 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u32le(buf->data, off, val);
}

int buf_read_u64le(const buf_t *buf, size_t *off, u64 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u64le(buf->data, off, val);
}

int buf_read_be(const buf_t *buf, size_t *off, void *val, size_t size)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, size) || *off + size > buf->used) {
		return 1;
	}

	return cbuf_read_be(buf->data, off, val, size);
}

int buf_read_u8be(const buf_t *buf, size_t *off, u8 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u8be(buf->data, off, val);
}

int buf_read_u16be(const buf_t *buf, size_t *off, u16 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u16be(buf->data, off, val);
}

int buf_read_u32be(const buf_t *buf, size_t *off, u32 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u32be(buf->data, off, val);
}

int buf_read_u64be(const buf_t *buf, size_t *off, u64 *val)
{
	if (buf == NULL || off == NULL || val == NULL) {
		return 1;
	}

	if (add_overflows(*off, sizeof(*val)) || *off + sizeof(*val) > buf->used) {
		return 1;
	}

	return cbuf_read_u64be(buf->data, off, val);
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
