#include "list.h"

#include "log.h"
#include "mem.h"

typedef struct header_s {
	lnode_t next;
} header_t;

list_t *list_init(list_t *list, uint cap, size_t size, alloc_t alloc)
{
	return arr_init(list, cap, sizeof(header_t) + size, alloc);
}

void list_free(list_t *list)
{
	arr_free(list);
}

void *list_add(list_t *list, lnode_t *node)
{
	if (list == NULL) {
		return NULL;
	}

	header_t *header = arr_add(list, node);
	if (header == NULL) {
		log_error("cutils", "list", NULL, "failed to add node");
		return NULL;
	}

	header->next = (lnode_t)-1;

	return header + 1;
}

int list_remove(list_t *list, lnode_t node)
{
	if (list == NULL) {
		return 1;
	}

	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return 1;
	}

	for (uint i = 0; i < list->cnt; i++) {
		header_t *prev = arr_get(list, i);
		if (i != node && prev->next == node) {
			prev->next = header->next;
		}
	}

	return 0;
}

void *list_add_next(list_t *list, lnode_t node, lnode_t *next)
{
	if (arr_get(list, node) == NULL) {
		return NULL;
	}

	lnode_t l;
	void *data = list_add(list, &l);
	if (data == NULL) {
		return NULL;
	}

	list_set_next(list, node, l);

	if (next) {
		*next = l;
	}

	return data;
}

int list_set_next(list_t *list, lnode_t node, lnode_t next)
{
	header_t *header = arr_get(list, node);
	if (header == NULL || list_get(list, next) == NULL) {
		return 1;
	}

	lnode_t *target = &header->next;
	while (*target < list->cnt) {
		target = &((header_t *)arr_get(list, *target))->next;
	}

	*target = next;

	return 0;
}

void *list_get_next(const list_t *list, lnode_t node, lnode_t *next)
{
	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return NULL;
	}

	if (next) {
		*next = header->next;
	}

	if (header->next == (lnode_t)-1) {
		return NULL;
	}

	return list_get(list, header->next);
}

void *list_get_at(const list_t *list, lnode_t start, uint index, lnode_t *node)
{
	if (list == NULL) {
		return NULL;
	}

	lnode_t cur	 = start;
	header_t *header = arr_get(list, cur);
	for (lnode_t i = 0; i < index && cur < list->cnt; i++, cur = header->next) {
		header = arr_get(list, cur);
	}

	if (node) {
		*node = cur;
	}

	return header == NULL ? NULL : header + 1;
}

void list_set_cnt(list_t *list, uint cnt)
{
	if (list == NULL) {
		return;
	}

	list->cnt = cnt;

	header_t *val;
	uint i = 0;
	arr_foreach(list, i, val)
	{
		if (val->next >= cnt) {
			val->next = (lnode_t)-1;
		}
	}
}

void *list_get(const list_t *list, lnode_t node)
{
	header_t *header = arr_get(list, node);
	return header == NULL ? NULL : header + 1;
}

size_t list_print(const list_t *list, lnode_t node, list_print_cb cb, dst_t dst, const void *priv)
{
	if (list == NULL || cb == NULL) {
		return 0;
	}

	size_t off = dst.off;
	void *value;
	list_foreach(list, node, value)
	{
		dst.off += cb(value, dst, priv);
	}

	return dst.off - off;
}
