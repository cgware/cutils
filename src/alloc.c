#include "alloc.h"

#include "mem.h"

void *alloc_alloc_std(alloc_t *alloc, size_t size)
{
	(void)alloc;
	return mem_alloc(size);
}

void *alloc_realloc_std(alloc_t *alloc, void *memory, size_t old_size, size_t new_size)
{
	(void)alloc;
	return mem_realloc(memory, new_size, old_size);
}

void alloc_free_std(alloc_t *alloc, void *memory, size_t size)
{
	(void)alloc;
	mem_free(memory, size);
}
