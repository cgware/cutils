#include "alloc.h"

#include "log.h"
#include "mem.h"

void *alloc_alloc_std(alloc_t *alloc, size_t size)
{
	(void)alloc;
	return mem_alloc(size);
}

int alloc_realloc_std(alloc_t *alloc, void **ptr, size_t *old_size, size_t new_size)
{
	(void)alloc;
	void *data = mem_realloc(*ptr, new_size, *old_size);
	if (data == NULL) {
		log_error("cutils", "alloc", NULL, "failed to reallocate memory");
		return 1;
	}

	*ptr	  = data;
	*old_size = new_size;

	return 0;
}

void alloc_free_std(alloc_t *alloc, void *ptr, size_t size)
{
	(void)alloc;
	mem_free(ptr, size);
}
