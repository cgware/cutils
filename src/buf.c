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
		log_error("cutils", "buf", NULL, "failed to allocate memory");
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

static int buf_grow(buf_t *buf, size_t size)
{
	if (buf->used + size > buf->size) {
		if (alloc_realloc(&buf->alloc, (void **)&buf->data, &buf->size, (buf->used + size) * 2)) {
			log_error("cutils", "buf", NULL, "failed to grow buffer");
			return 1;
		}
	}

	return 0;
}

int buf_add(buf_t *buf, const void *data, size_t size, size_t *off)
{
	if (buf_grow(buf, size)) {
		return 1;
	}

	mem_copy(buf->data + buf->used, buf->size - buf->used, data, size);
	if (off) {
		*off = buf->used;
	}
	buf->used += size;
	return 0;
}

buf_t *buf_replace(buf_t *buf, size_t off, const void *data, size_t old_len, size_t new_len)
{
	if (buf == NULL) {
		return NULL;
	}

	if (new_len > old_len && buf_grow(buf, new_len - old_len)) {
		return NULL;
	}

	mem_replace(&buf->data[off], buf->size - off, buf->used - off, data, old_len, new_len);

	buf->used += new_len - old_len;
	return buf;
}

int buf_print(const buf_t *buf, print_dst_t dst)
{
	if (buf == 0) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "%08X  ", 0);
	for (size_t i = 0; i < buf->used; i++) {
		dst.off += c_dprintf(dst, "%02x ", buf->data[i]);
		if (i > 0 && (i + 1) % 16 == 0) {
			if (i + 1 < buf->used) {
				dst.off += c_dprintf(dst, "\n%08X  ", i + 1);
			}
		} else if (i > 0 && (i + 1) % 8 == 0) {
			dst.off += c_dprintf(dst, " ");
		}
	}

	dst.off += c_dprintf(dst, "\n");
	return dst.off - off;
}
