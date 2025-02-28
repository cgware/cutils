#ifndef MEM_H
#define MEM_H

#include "mem_stats.h"
#include "print.h"

int mem_print(print_dst_t dst);
int mem_check();

void *mem_alloc(size_t size);
void *mem_calloc(size_t count, size_t size);
void *mem_realloc(void *memory, size_t new_size, size_t old_size);

void *mem_set(void *dst, int val, size_t size);
void *mem_copy(void *dst, size_t size, const void *src, size_t len);
void *mem_move(void *dst, size_t size, const void *src, size_t len);
void *mem_replace(void *dst, size_t size, size_t len, const void *src, size_t old_len, size_t new_len);
int mem_cmp(const void *l, const void *r, size_t size);

int mem_swap(void *ptr1, void *ptr2, size_t size);

void mem_free(void *memory, size_t size);

void mem_oom(int oom);

#endif
