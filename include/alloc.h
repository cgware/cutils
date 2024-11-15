#ifndef alloc_H
#define alloc_H

#include <stddef.h>

typedef struct alloc_s alloc_t;
struct alloc_s {
	void *(*alloc)(alloc_t *alloc, size_t size);
	void *(*realloc)(alloc_t *alloc, void *memory, size_t new_size, size_t old_size);
	void (*free)(alloc_t *alloc, void *memory, size_t size);
	void *mem;
};

void *alloc_alloc(alloc_t *alloc, size_t size);
void *alloc_realloc(alloc_t *alloc, void *memory, size_t new_size, size_t old_size);
void alloc_free(alloc_t *alloc, void *memory, size_t size);

void *alloc_alloc_std(alloc_t *alloc, size_t size);
void *alloc_realloc_std(alloc_t *alloc, void *memory, size_t new_size, size_t old_size);
void alloc_free_std(alloc_t *alloc, void *memory, size_t size);

#define ALLOC_STD ((alloc_t){.alloc = alloc_alloc_std, .realloc = alloc_realloc_std, .free = alloc_free_std})

#endif
