#include "mem.h"

#include "log.h"
#include "platform.h"

#include <memory.h>
#include <stdlib.h>

static int s_oom;

static void get_max_unit(size_t *size, char *u)
{
	if (*size >= 1024) {
		*size /= 1024;
		*u = 'K';
	}

	if (*size >= 1024) {
		*size /= 1024;
		*u = 'M';
	}
}

static int print_mem(size_t mem, print_dst_t dst)
{
	size_t umem = mem;

	char u = '\0';

	get_max_unit(&umem, &u);

	int off = dst.off;

	if (u == '\0') {
		dst.off += c_dprintf(dst, "%zu B\n", mem);
	} else {
		dst.off += c_dprintf(dst, "%zu %cB (%zu B)\n", umem, u, mem);
	}

	return dst.off - off;
}

int mem_print(print_dst_t dst)
{
	const mem_stats_t *stats = mem_stats_get();
	if (stats == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "memory stats:\n");
	dst.off += c_dprintf(dst, "    unfreed:  ");
	dst.off += print_mem(stats->mem, dst);
	dst.off += c_dprintf(dst, "    peak:     ");
	dst.off += print_mem(stats->peak, dst);
	dst.off += c_dprintf(dst, "    total:    ");
	dst.off += print_mem(stats->total, dst);
	dst.off += c_dprintf(dst, "    allocs:   %d\n", stats->allocs);
	dst.off += c_dprintf(dst, "    reallocs: %d\n", stats->reallocs);

	return dst.off - off;
}

int mem_check()
{
	const mem_stats_t *stats = mem_stats_get();
	if (stats->mem == 0) {
		return 0;
	}

	log_warn("cutils", "mem", NULL, "%d bytes not freed", stats->mem);
	return 1;
}

#define MAX(a, b) (a) > (b) ? (a) : (b)

void *mem_alloc(size_t size)
{
	if (size == 0) {
		log_warn("cutils", "mem", NULL, "malloc 0 bytes");
	}

	void *ptr = size > 0 && s_oom ? NULL : malloc(size);

	if (ptr == NULL) {
		log_error("cutils", "mem", NULL, "out of memory");
		return NULL;
	}

	mem_stats_alloc(size);

	return ptr;
}

void *mem_calloc(size_t count, size_t size)
{
	if (size == 0) {
		log_warn("cutils", "mem", NULL, "calloc 0 bytes");
	}

	void *ptr = count * size > 0 && s_oom ? NULL : calloc(count, size);

	if (ptr == NULL) {
		log_error("cutils", "mem", NULL, "out of memory");
		return NULL;
	}

	mem_stats_alloc(count * size);

	return ptr;
}

void *mem_realloc(void *memory, size_t new_size, size_t old_size)
{
	if (memory == NULL) {
		log_error("cutils", "mem", NULL, "realloc NULL");
		return NULL;
	}

	if (new_size == 0 || new_size == old_size) {
		log_warn("cutils", "mem", NULL, "realloc %zu -> %zu bytes", old_size, new_size);
	} else {
		log_trace("cutils", "mem", NULL, "realloc %zu -> %zu bytes", old_size, new_size);
	}

	if (new_size == 0) {
		return memory;
	}

	if (old_size == 0) {
		return mem_alloc(new_size);
	}

	void *ptr = new_size > old_size && s_oom ? NULL : realloc(memory, new_size);

	if (ptr == NULL) {
		log_error("cutils", "mem", NULL, "out of memory");
		return NULL;
	}

	mem_stats_realloc(old_size, new_size);

	return ptr;
}

void *mem_set(void *dst, int val, size_t size)
{
	if (dst == NULL) {
		return NULL;
	}

	return memset(dst, val, size);
}

void *mem_copy(void *dst, size_t size, const void *src, size_t len)
{
	if (len > size) {
		log_error("cutils", "mem", NULL, "destination too small: %d/%d", size, len);
		return NULL;
	}
#if defined(C_WIN)
	return memcpy_s(dst, size, src, len) ? NULL : dst;
#else
	return memcpy(dst, src, len);
#endif
}

void *mem_move(void *dst, size_t size, const void *src, size_t len)
{
	if (len > size) {
		log_error("cutils", "mem", NULL, "destination too small: %d/%d", size, len);
		return NULL;
	}
#if defined(C_WIN)
	return memmove_s(dst, size, src, len) ? NULL : dst;
#else
	return memmove(dst, src, len);
#endif
}

void *mem_replace(void *dst, size_t size, size_t len, const void *src, size_t old_len, size_t new_len)
{
	if (dst == NULL || src == NULL) {
		return NULL;
	}

	if (new_len > old_len && len + (new_len - old_len) > size) {
		log_error("cutils", "mem", NULL, "destination too small: %d/%d", size, len + (new_len - old_len));
		return NULL;
	}

	mem_move((byte *)dst + new_len, size - new_len, (byte *)dst + old_len, len - old_len);
	mem_copy(dst, new_len, src, new_len);

	return dst;
}

int mem_cmp(const void *l, const void *r, size_t size)
{
	return memcmp(l, r, size);
}

int mem_swap(void *ptr1, void *ptr2, size_t size)
{
	if (ptr1 == NULL || ptr2 == NULL) {
		return 1;
	}

	unsigned char *byte_ptr1 = (unsigned char *)ptr1;
	unsigned char *byte_ptr2 = (unsigned char *)ptr2;

	for (size_t i = 0; i < size; i++) {
		unsigned char temp = byte_ptr1[i];
		byte_ptr1[i]	   = byte_ptr2[i];
		byte_ptr2[i]	   = temp;
	}

	return 0;
}

void mem_free(void *memory, size_t size)
{
	if (memory == NULL) {
		return;
	}

	mem_stats_free(size);

	free(memory);
}

void mem_oom(int oom)
{
	s_oom = oom;
	log_set_quiet(0, oom ? 1 : 0);
}
