#include "arr.h"

#include "log.h"
#include "mem.h"

#include <stdlib.h>

arr_t *arr_init(arr_t *arr, uint cap, size_t size, alloc_t alloc)
{
	if (arr == NULL) {
		return NULL;
	}

	arr->data = alloc_alloc(&alloc, cap * size);
	if (arr->data == NULL) {
		log_error("cutils", "arr", NULL, "failed to allocate memory");
		return NULL;
	}

	arr->cap   = cap;
	arr->cnt   = 0;
	arr->size  = size;
	arr->alloc = alloc;

	return arr;
}

void arr_free(arr_t *arr)
{
	if (arr == NULL) {
		return;
	}

	alloc_free(&arr->alloc, arr->data, arr->cap * arr->size);
	arr->data = NULL;
	arr->cap  = 0;
	arr->cnt  = 0;
	arr->size = 0;
}

void arr_reset(arr_t *arr, int val, uint cnt)
{
	if (arr == NULL) {
		return;
	}

	if (cnt > arr->cap) {
		cnt = arr->cap;
	}

	mem_set(arr->data, val, arr->size * cnt);
	arr->cnt = cnt;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static inline int arr_resize(arr_t *arr)
{
	if (arr->cnt < arr->cap) {
		return 0;
	}

	size_t old_size = arr->cap * arr->size;
	uint new_cap	= MAX(1, arr->cap * 2);

	if (alloc_realloc(&arr->alloc, &arr->data, &old_size, new_cap * arr->size)) {
		return 1;
	}

	arr->cap = new_cap;

	return 0;
}

void *arr_add(arr_t *arr)
{
	if (arr == NULL) {
		return NULL;
	}

	if (arr_resize(arr)) {
		log_error("cutils", "arr", NULL, "failed to add element");
		return NULL;
	}

	return (byte *)arr->data + (arr->cnt++) * arr->size;
}

void *arr_get(const arr_t *arr, uint index)
{
	if (arr == NULL) {
		return NULL;
	}

	if (index >= arr->cnt) {
		log_warn("cutils", "arr", NULL, "invalid id: %d", index);
		return NULL;
	}

	return (byte *)arr->data + index * arr->size;
}

void *arr_set(arr_t *arr, uint index, const void *value)
{
	if (arr == NULL || value == NULL) {
		return NULL;
	}

	void *dst = arr_get(arr, index);
	if (dst == NULL) {
		return NULL;
	}

	if (value) {
		mem_copy(dst, arr->size, value, arr->size);
	}

	return dst;
}

uint arr_addv(arr_t *arr, const void *value)
{
	if (arr == NULL || value == NULL) {
		return ARR_END;
	}

	uint index = arr->cnt;
	void *data = arr_add(arr);
	if (data == NULL) {
		return ARR_END;
	}

	mem_copy(data, arr->size, value, arr->size);

	return index;
}

uint arr_addu(arr_t *arr, const void *value)
{
	if (arr == NULL || value == NULL) {
		return ARR_END;
	}

	uint index = arr_index(arr, value);
	if (index < arr->cnt) {
		return index;
	}

	return arr_addv(arr, value);
}

uint arr_index(const arr_t *arr, const void *value)
{
	if (arr == NULL || value == NULL) {
		return ARR_END;
	}

	for (uint i = 0; i < arr->cnt; i++) {
		if (mem_cmp(arr_get(arr, i), value, arr->size) == 0) {
			return i;
		}
	}

	return ARR_END;
}

uint arr_index_cmp(const arr_t *arr, const void *value, arr_cmp_cb cb, const void *priv)
{
	if (arr == NULL || value == NULL || cb == NULL) {
		return ARR_END;
	}

	for (uint i = 0; i < arr->cnt; i++) {
		if (cb(arr_get(arr, i), value, priv)) {
			return i;
		}
	}

	return ARR_END;
}

arr_t *arr_add_all(arr_t *arr, const arr_t *src)
{
	if (arr == NULL || src == NULL || arr->cap - arr->cnt < src->cnt || arr->size != src->size) {
		return NULL;
	}

	mem_copy((byte *)arr->data + arr->cnt * arr->size, arr->cap * arr->size - arr->cnt * arr->size, src->data, src->cnt * src->size);

	arr->cnt += src->cnt;

	return arr;
}

arr_t *arr_add_unique(arr_t *arr, const arr_t *src)
{
	if (arr == NULL || src == NULL || arr->size != src->size) {
		return NULL;
	}

	for (uint i = 0; i < src->cnt; i++) {
		uint index = arr_addu(arr, arr_get(src, i));
		if (index >= arr->cnt) {
			return NULL;
		}
	}

	return arr;
}

arr_t *arr_merge_all(arr_t *arr, const arr_t *arr1, const arr_t *arr2)
{
	if (arr1 == NULL || arr2 == NULL || arr1->size != arr2->size) {
		return NULL;
	}

	if (arr_init(arr, arr1->cnt + arr2->cnt, arr1->size, arr1->alloc) == NULL) {
		return NULL;
	}

	arr_add_all(arr, arr1);
	arr_add_all(arr, arr2);

	return arr;
}

arr_t *arr_merge_unique(arr_t *arr, const arr_t *arr1, const arr_t *arr2)
{
	if (arr1 == NULL || arr2 == NULL || arr1->size != arr2->size) {
		return NULL;
	}

	if (arr_init(arr, arr1->cnt + arr2->cnt, arr1->size, arr1->alloc) == NULL) {
		return NULL;
	}

	arr_add_unique(arr, arr1);
	arr_add_unique(arr, arr2);

	return arr;
}

arr_t *arr_sort(arr_t *arr, arr_cmp_cb cb, const void *priv)
{
	if (arr == NULL) {
		return NULL;
	}

	if (cb == NULL) {
		return arr;
	}

	void *v1;
	arr_foreach(arr, v1)
	{
		void *v2;
		uint j = _i + 1;
		arr_foreach_i(arr, v2, j)
		{
			if (cb(v1, v2, priv) > 0) {
				mem_swap(v1, v2, arr->size);
			}
		}
	}

	return arr;
}

int arr_print(const arr_t *arr, arr_print_cb cb, print_dst_t dst, const void *priv)
{
	if (arr == NULL || cb == NULL) {
		return 0;
	}

	int off = dst.off;
	void *value;
	arr_foreach(arr, value)
	{
		dst.off += cb(value, dst, priv);
	}

	return dst.off - off;
}
