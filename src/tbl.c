#include "tbl.h"

#include "log.h"
#include "mem.h"

typedef struct tbl_enum_s {
	arr_t vals;
	size_t str;
} tbl_enum_t;

tbl_t *tbl_init(tbl_t *tbl, uint cols_cap, uint layouts_cap, uint strs_cap, alloc_t alloc)
{
	if (tbl == NULL) {
		return NULL;
	}

	if (schema_init(&tbl->schema, cols_cap, layouts_cap, strs_cap, alloc) == NULL) {
		return NULL;
	}

	return tbl;
}

void tbl_free(tbl_t *tbl)
{
	if (tbl == NULL) {
		return;
	}

	schema_free(&tbl->schema);
	arr_free(&tbl->rows);
	strvbuf_free(&tbl->strs);
}

int tbl_init_rows(tbl_t *tbl, size_t cap, alloc_t alloc)
{
	if (tbl == NULL) {
		return 1;
	}

	const schema_layout_t *layout = schema_get_layout(&tbl->schema, 0);
	if (layout == NULL) {
		return 1;
	}

	if (arr_init(&tbl->rows, cap, layout->size, alloc) == NULL || strvbuf_init(&tbl->strs, cap, 16, alloc) == NULL) {
		return 1;
	}

	return 0;
}

void *tbl_add_row(tbl_t *tbl, uint *id)
{
	if (tbl == NULL) {
		return NULL;
	}

	uint tmp;
	void *row = arr_add(&tbl->rows, &tmp);
	if (row == NULL) {
		log_error("cutils", "tbl", NULL, "failed to add row");
		return NULL;
	}

	if (id) {
		*id = tmp;
	}

	return row;
}

int tbl_set_cell(tbl_t *tbl, uint row, uint col, uint layout, void *val)
{
	if (tbl == NULL) {
		return 1;
	}

	void *data = arr_get(&tbl->rows, row);
	if (data == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get row");
		return 1;
	}

	if (schema_set_val(&tbl->schema, layout, col, data, val)) {
		log_error("cutils", "tbl", NULL, "failed to set cell");
		return 1;
	}

	return 0;
}

int tbl_set_cell_str(tbl_t *tbl, uint row, uint col, uint layout, strv_t val)
{
	if (tbl == NULL) {
		return 1;
	}

	void *data = arr_get(&tbl->rows, row);
	if (data == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get row");
		return 1;
	}

	size_t strs_cnt;
	if (strvbuf_add(&tbl->strs, val, &strs_cnt)) {
		log_error("cutils", "tbl", NULL, "failed to add string");
		return 1;
	}

	if (schema_set_val(&tbl->schema, layout, col, data, &strs_cnt)) {
		log_error("cutils", "tbl", NULL, "failed to set cell");
		return 1;
	}

	const schema_member_t *member = schema_get_member(&tbl->schema, 0, col);
	schema_field_t *field	      = (schema_field_t *)schema_get_field(&tbl->schema, member->field);
	if (val.len > field->len) {
		field->len = val.len;
	}

	return 0;
}

const void *tbl_get_cell(tbl_t *tbl, uint row, uint col)
{
	if (tbl == NULL) {
		return NULL;
	}

	void *ptr = arr_get(&tbl->rows, row);
	if (ptr == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get cell row");
		return NULL;
	}

	return schema_get_val(&tbl->schema, col, ptr);
}

int tbl_map(tbl_t *tbl, uint from, uint to, tbl_map_fn fn, void *priv)
{
	if (tbl == NULL) {
		return 1;
	}

	int ret = 0;

	void *row;
	uint i = 0;
	arr_foreach(&tbl->rows, i, row)
	{
		const void *data = tbl_get_cell(tbl, i, from);
		ret |= fn(tbl, i, to, data, priv);
	}

	return ret;
}

size_t tbl_print(const tbl_t *tbl, dst_t dst)
{
	if (tbl == NULL) {
		return 0;
	}

	size_t off = dst.off;

#define SUB(a, b) ((a) > (b) ? (a) - (b) : 0)

	const schema_layout_t *layout = schema_get_layout(&tbl->schema, 0);

	if (layout == NULL) {
		return 0;
	}

	for (uint i = layout->members; i < layout->members + layout->members_cnt; i++) {
		schema_member_t *member = arr_get(&tbl->schema.members, i);
		if (i > 0) {
			dst.off += dputf(dst, " ");
		}

		const schema_field_t *field = schema_get_field(&tbl->schema, member->field);
		strv_t name		    = schema_get_str(&tbl->schema, field->name);
		dst.off += dputf(dst, "%.*s", name.len, name.data);
		dst.off += dputf(dst, "%*s", SUB(field->len, name.len), "");
	}
	dst.off += dputf(dst, "\n");

	byte *row;
	uint r = 0;
	row_foreach(tbl, r, row)
	{
		for (uint c = layout->members; c < layout->members + layout->members_cnt; c++) {
			schema_member_t *member = arr_get(&tbl->schema.members, c);
			if (c > 0) {
				dst.off += dputf(dst, " ");
			}

			const byte *cell = &row[member->off];
			size_t col_start = dst.off;

			const schema_field_t *field = schema_get_field(&tbl->schema, member->field);

			switch (field->type) {
			case SCHEMA_TYPE_STR: {
				dst.off += strv_print(strvbuf_get(&tbl->strs, *(size_t *)cell), dst);
				break;
			}
			default: dst.off += schema_print_val(&tbl->schema, 0, c, cell, dst); break;
			}

			dst.off += dputf(dst, "%*s", SUB(field->len, dst.off - col_start), "");
		}
		dst.off += dputf(dst, "\n");
	}

	return dst.off - off;
}
