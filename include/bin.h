#ifndef BIN_H
#define BIN_H

#include "buf.h"

typedef struct bin_s {
	buf_t buf;
} bin_t;

bin_t *bin_init(bin_t *bin, size_t size, alloc_t alloc);
void bin_free(bin_t *bin);

int bin_resize(bin_t *bin, size_t size);

int bin_cmp(bin_t *bin, size_t off, void *data, size_t size);

int bin_add(bin_t *bin, void *data, size_t size);
void *bin_get(bin_t *bin, size_t size, size_t *off);

int bin_get_int(bin_t *bin, void *val, size_t size, size_t *off);

#endif
