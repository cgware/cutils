#ifndef ARR_H
#define ARR_H

#include "alloc.h"
#include "print.h"
#include "type.h"

typedef struct arr_s {
	void *data;
	uint cap;
	uint cnt;
	size_t size;
	alloc_t alloc;
} arr_t;

arr_t *arr_init(arr_t *arr, uint cap, size_t size, alloc_t alloc);
void arr_free(arr_t *arr);

void arr_reset(arr_t *arr, int val, uint cnt);

void *arr_add(arr_t *arr, uint *id);

void *arr_get(const arr_t *arr, uint id);

void *arr_set(arr_t *arr, uint id, const void *value);

int arr_addv(arr_t *arr, const void *value, uint *id);
int arr_addu(arr_t *arr, const void *value, uint *id);

int arr_find(const arr_t *arr, const void *value, uint *id);

typedef int (*arr_cmp_cb)(const void *value1, const void *value2, const void *priv);
int arr_find_cmp(const arr_t *arr, const void *value, arr_cmp_cb cb, const void *priv, uint *id);

arr_t *arr_add_all(arr_t *arr, const arr_t *src);

arr_t *arr_add_unique(arr_t *arr, const arr_t *src);

arr_t *arr_merge_all(arr_t *arr, const arr_t *arr1, const arr_t *arr2);

arr_t *arr_merge_unique(arr_t *arr, const arr_t *arr1, const arr_t *arr2);

arr_t *arr_sort(arr_t *arr, arr_cmp_cb cb, const void *priv);

typedef int (*arr_print_cb)(void *value, print_dst_t dst, const void *priv);
int arr_print(const arr_t *arr, arr_print_cb cb, print_dst_t dst, const void *priv);

#define arr_foreach(_arr, _val)	      for (uint _i = 0; _i < (_arr)->cnt && (_val = (void *)((byte *)(_arr)->data + _i * (_arr)->size)); _i++)
#define arr_foreach_i(_arr, _val, _i) for (; _i < (_arr)->cnt && (_val = (void *)((byte *)(_arr)->data + _i * (_arr)->size)); _i++)

#endif
