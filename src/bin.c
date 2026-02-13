#include "bin.h"
#include "buf.h"
#include "mem.h"
#include "type.h"

bin_t *bin_init(bin_t *bin, size_t size, alloc_t alloc)
{
	if (bin == NULL) {
		return NULL;
	}

	if (buf_init(&bin->buf, size, alloc) == NULL) {
		return NULL;
	}

	return bin;
}

void bin_free(bin_t *bin)
{
	if (bin == NULL) {
		return;
	}

	buf_free(&bin->buf);
}

int bin_resize(bin_t *bin, size_t size)
{
	if (bin == NULL) {
		return 1;
	}

	return buf_resize(&bin->buf, size);
}

int bin_cmp(bin_t *bin, size_t off, void *data, size_t size)
{
	if (bin == NULL) {
		return 1;
	}

	if (off >= bin->buf.used) {
		return 1;
	}

	u8 *ptr = bin->buf.data;
	return mem_cmp(&ptr[off], data, size);
}

int bin_add(bin_t *bin, void *data, size_t size)
{
	if (bin == NULL) {
		return 1;
	}

	if (buf_add(&bin->buf, data, size, NULL)) {
		return 1;
	}

	return 0;
}

void *bin_get(bin_t *bin, size_t size, size_t *off)
{
	if (bin == NULL || off == NULL) {
		return NULL;
	}

	u8 *data = buf_get(&bin->buf, *off);
	if (data == NULL) {
		return NULL;
	}

	if (bin->buf.size - *off < size) {
		return NULL;
	}

	*off += size;

	return data;
}

int bin_get_int(bin_t *bin, void *val, size_t size, size_t *off)
{
	u8 *data = bin_get(bin, size, off);
	if (data == NULL) {
		return 1;
	}

	u8 *ptr = val;
	for (size_t i = 0; i < size; i++) {
		ptr[i] = data[i];
	}

	return 0;
}
