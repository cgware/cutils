#ifndef alloc_H
#define alloc_H

#include <stddef.h>

typedef struct alloc_s alloc_t;
struct alloc_s {
	void *(*alloc)(alloc_t *alloc, size_t size);
	void *(*realloc)(alloc_t *alloc, void *ptr, size_t old_size, size_t new_size);
	void (*free)(alloc_t *alloc, void *ptr, size_t size);
	void *priv;
};

void *alloc_alloc_std(alloc_t *alloc, size_t size);
void *alloc_realloc_std(alloc_t *alloc, void *ptr, size_t new_size, size_t old_size);
void alloc_free_std(alloc_t *alloc, void *ptr, size_t size);

#define alloc_alloc(_alloc, _size)			  (_alloc)->alloc(_alloc, _size)
#define alloc_realloc(_alloc, _ptr, _old_size, _new_size) (_alloc)->realloc(_alloc, _ptr, _old_size, _new_size)
#define alloc_free(_alloc, _ptr, _size)			  (_alloc)->free(_alloc, _ptr, _size)

#define ALLOC_STD ((alloc_t){.alloc = alloc_alloc_std, .realloc = alloc_realloc_std, .free = alloc_free_std})

#endif
