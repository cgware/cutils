#include "list.h"

#include "log.h"
#include "mem.h"

typedef struct header_s {
	list_node_t next;
} header_t;

list_t *list_init(list_t *list, uint cap, size_t size, alloc_t alloc)
{
	return arr_init(list, cap, sizeof(header_t) + size, alloc);
}

void list_free(list_t *list)
{
	arr_free(list);
}

void list_reset(list_t *list, uint cnt)
{
	if (list == NULL) {
		return;
	}

	if (cnt > list->cnt) {
		cnt = list->cnt;
	}

	list->cnt = cnt;

	header_t *val;
	uint i = 0;
	arr_foreach(list, i, val)
	{
		if (val->next >= cnt) {
			val->next = (list_node_t)-1;
		}
	}
}

void *list_node(list_t *list, list_node_t *node)
{
	if (list == NULL) {
		return NULL;
	}

	header_t *header = arr_add(list, node);
	if (header == NULL) {
		log_error("cutils", "list", NULL, "failed to create node");
		return NULL;
	}

	header->next = (list_node_t)-1;

	return header + 1;
}

int list_app(list_t *list, list_node_t node, list_node_t next)
{
	if (list == NULL) {
		return 1;
	}

	header_t *header = arr_get(list, node);
	if (header == NULL) {
		log_error("cutils", "list", NULL, "invalid node: %d", node);
		return 1;
	}

	if (list_get(list, next) == NULL) {
		log_error("cutils", "list", NULL, "invalid node: %d", next);
		return 1;
	}

	list_node_t *target = &header->next;
	while (*target < list->cnt) {
		target = &((header_t *)arr_get(list, *target))->next;
	}

	*target = next;

	return 0;
}

int list_remove(list_t *list, list_node_t node)
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

void *list_get(const list_t *list, list_node_t node)
{
	if (list == NULL) {
		return NULL;
	}

	header_t *header = arr_get(list, node);
	if (header == NULL) {
		log_error("cutils", "list", NULL, "invalid node: %d", node);
		return NULL;
	}

	return header + 1;
}

void *list_get_next(const list_t *list, list_node_t node, list_node_t *next)
{
	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return NULL;
	}

	if (next) {
		*next = header->next;
	}

	if (header->next == (list_node_t)-1) {
		return NULL;
	}

	return list_get(list, header->next);
}

void *list_get_at(const list_t *list, list_node_t start, uint index, list_node_t *node)
{
	if (list == NULL) {
		return NULL;
	}

	list_node_t cur	 = start;
	header_t *header = arr_get(list, cur);
	for (uint i = 0; i < index && header && cur < list->cnt; i++) {
		cur    = header->next;
		header = arr_get(list, cur);
	}

	if (node) {
		*node = cur;
	}

	return header == NULL ? NULL : header + 1;
}

size_t list_print(const list_t *list, list_node_t node, list_print_cb cb, dst_t dst, const void *priv)
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
