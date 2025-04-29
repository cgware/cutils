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

lnode_t list_add(list_t *list)
{
	if (list == NULL) {
		return LIST_END;
	}

	lnode_t node;
	header_t *data = arr_add(list, &node);
	if (data == NULL) {
		log_error("cutils", "list", NULL, "failed to add element");
		return LIST_END;
	}

	data->next = LIST_END;

	return node;
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

lnode_t list_add_next(list_t *list, lnode_t node)
{
	if (arr_get(list, node) == NULL) {
		return LIST_END;
	}
	return list_set_next(list, node, list_add(list));
}

lnode_t list_set_next(list_t *list, lnode_t node, lnode_t next)
{
	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return LIST_END;
	}

	lnode_t *target = &header->next;
	while (*target < list->cnt) {
		target = &((header_t *)arr_get(list, *target))->next;
	}

	*target = next;

	return *target;
}

lnode_t list_get_next(const list_t *list, lnode_t node)
{
	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return LIST_END;
	}

	return header->next;
}

lnode_t list_get_at(const list_t *list, lnode_t start, lnode_t index)
{
	if (list == NULL) {
		return LIST_END;
	}

	lnode_t i   = 0;
	lnode_t cur = start;
	while (cur < list->cnt && i < index) {
		cur = ((header_t *)arr_get(list, cur))->next;
		i++;
	}

	return cur;
}

void list_set_cnt(list_t *list, uint cnt)
{
	if (list == NULL) {
		return;
	}

	list->cnt = cnt;

	header_t *val;
	arr_foreach(list, val)
	{
		if (val->next >= cnt) {
			val->next = LIST_END;
		}
	}
}

void *list_get_data(const list_t *list, lnode_t node)
{
	header_t *header = arr_get(list, node);
	if (header == NULL) {
		return NULL;
	}

	return (byte *)header + sizeof(header_t);
}

int list_print(const list_t *list, lnode_t node, list_print_cb cb, print_dst_t dst, const void *priv)
{
	if (list == NULL || cb == NULL) {
		return 0;
	}

	int off = dst.off;
	void *value;
	list_foreach(list, node, value)
	{
		dst.off += cb(value, dst, priv);
	}

	return dst.off - off;
}
