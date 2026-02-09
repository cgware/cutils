#include "tbl.h"

#include "arr.h"
#include "log.h"
#include "mem.h"

typedef struct tbl_enum_s {
	arr_t vals;
	size_t str;
} tbl_enum_t;

tbl_t *tbl_init(tbl_t *tbl, size_t cols_cap, alloc_t alloc)
{
	if (tbl == NULL) {
		return NULL;
	}

	if (arr_init(&tbl->cols, cols_cap, sizeof(tbl_col_t), alloc) == NULL || strvbuf_init(&tbl->strs, cols_cap, 16, alloc) == NULL) {
		return NULL;
	}

	return tbl;
}

void tbl_free(tbl_t *tbl)
{
	if (tbl == NULL) {
		return;
	}

	tbl_col_t *col;
	uint i = 0;
	arr_foreach(&tbl->cols, i, col)
	{
		switch (col->type) {
		case TBL_COL_TYPE_ENUM:
		case TBL_COL_TYPE_FLAG: {
			arr_free(&col->vals);
			break;
		}
		default: break;
		}
	}

	arr_free(&tbl->cols);
	arr_free(&tbl->rows);
	strvbuf_free(&tbl->strs);
}

int tbl_add_col(tbl_t *tbl, strv_t name, tbl_col_type_t type, size_t size, alloc_t alloc, uint *col)
{
	if (tbl == NULL) {
		return 1;
	}

	size_t id;
	if (strvbuf_add(&tbl->strs, name, &id)) {
		return 1;
	}

	uint cols_cnt;
	tbl_col_t *tcol = arr_add(&tbl->cols, &cols_cnt);
	if (tcol == NULL) {
		return 1;
	}

	if (col) {
		*col = cols_cnt;
	}

	tcol->name = id;
	tcol->type = type;
	tcol->off  = tbl->rows.size;
	tcol->size = size;

	switch (type) {
	case TBL_COL_TYPE_STR:
		tbl->rows.size += sizeof(size_t);
		tcol->len = size;
		break;
	case TBL_COL_TYPE_ENUM:
		tbl->rows.size += size;
		tcol->len = 2 + size * 2;
		if (arr_init(&tcol->vals, 8, size + sizeof(size_t), alloc) == NULL) {
			arr_reset(&tbl->cols, cols_cnt);
			return 1;
		}
		break;
	case TBL_COL_TYPE_FLAG:
		tbl->rows.size += size;
		tcol->len = name.len;
		if (arr_init(&tcol->vals, 8, sizeof(u8) + sizeof(size_t), alloc) == NULL) {
			arr_reset(&tbl->cols, cols_cnt);
			return 1;
		}
		break;
	default:
		tbl->rows.size += size;
		tcol->len = 2 + size * 2;
		break;
	}

	if(name.len > tcol->len) {
		tcol->len = name.len;
	}

	return 0;
}

const tbl_col_t *tbl_get_col(const tbl_t *tbl, uint col)
{
	if (tbl == NULL) {
		return NULL;
	}

	tbl_col_t *c = arr_get(&tbl->cols, col);
	if (c == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get col");
		return NULL;
	}

	return c;
}

void *tbl_add_enum(tbl_t *tbl, uint col, strv_t str)
{
	if (tbl == NULL) {
		return NULL;
	}

	tbl_col_t *c = arr_get(&tbl->cols, col);
	if (c == NULL) {
		return NULL;
	}

	uint vals_cnt;
	void *val = arr_add(&c->vals, &vals_cnt);
	if (val == NULL) {
		return NULL;
	}

	if (strvbuf_add(&tbl->strs, str, (size_t *)((size_t)val + c->size))) {
		arr_reset(&c->vals, vals_cnt);
		return NULL;
	}

	if (str.len > c->len) {
		c->len = str.len;
	}

	return val;
}

void *tbl_add_flag(tbl_t *tbl, uint col, strv_t str)
{
	if (tbl == NULL) {
		return NULL;
	}

	tbl_col_t *c = arr_get(&tbl->cols, col);
	if (c == NULL) {
		return NULL;
	}

	uint vals_cnt;
	void *val = arr_add(&c->vals, &vals_cnt);
	if (val == NULL) {
		return NULL;
	}

	if (strvbuf_add(&tbl->strs, str, (size_t *)((size_t)val + sizeof(u8)))) {
		arr_reset(&c->vals, vals_cnt);
		return NULL;
	}

	if (c->vals.cnt > c->len) {
		c->len = c->vals.cnt;
	}

	return val;
}

int tbl_init_rows(tbl_t *tbl, size_t cap, alloc_t alloc)
{
	if (tbl == NULL) {
		return 1;
	}

	if (arr_init(&tbl->rows, cap, tbl->rows.size, alloc) == NULL) {
		return 1;
	}

	return 0;
}

int tbl_add_row(tbl_t *tbl, uint *row)
{
	if (tbl == NULL) {
		return 1;
	}

	uint tmp;
	if (arr_add(&tbl->rows, &tmp) == NULL) {
		log_error("cutils", "tbl", NULL, "failed to add row");
		return 1;
	}

	if (row) {
		*row = tmp;
	}

	return 0;
}

void *tbl_get_cell(tbl_t *tbl, uint row, uint col)
{
	if (tbl == NULL) {
		return NULL;
	}

	tbl_col_t *tcol = arr_get(&tbl->cols, col);
	if (tcol == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get cell column");
		return NULL;
	}

	void *ptr = arr_get(&tbl->rows, row);
	if (ptr == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get cell row");
		return NULL;
	}

	return (void *)((size_t)ptr + tcol->off);
}

int tbl_set_cell_str(tbl_t *tbl, uint row, uint col, strv_t val)
{
	if (tbl == NULL) {
		return 1;
	}

	size_t *cell = tbl_get_cell(tbl, row, col);
	if (cell == NULL) {
		log_error("cutils", "tbl", NULL, "failed to get string cell");
		return 1;
	}

	if (strvbuf_add(&tbl->strs, val, cell)) {
		log_error("cutils", "tbl", NULL, "failed to add string");
		return 1;
	}

	tbl_col_t *tcol = arr_get(&tbl->cols, col);
	if (val.len > tcol->len) {
		tcol->len = val.len;
	}

	return 0;
}

static int bit_is_set(const u8 *data, u8 bit)
{
	return data[bit >> 3] & (1u << (bit & 7));
}

size_t tbl_print(const tbl_t *tbl, dst_t dst)
{
	if (tbl == NULL) {
		return 0;
	}

	size_t off = dst.off;

#define SUB(a, b) ((a) > (b) ? (a) - (b) : 0)

	tbl_col_t *col;
	uint i = 0;
	arr_foreach(&tbl->cols, i, col)
	{
		if (i > 0) {
			dst.off += dputf(dst, " ");
		}

		strv_t name = strvbuf_get(&tbl->strs, col->name);
		dst.off += dputf(dst, "%.*s", name.len, name.data);
		dst.off += dputf(dst, "%*s", SUB(col->len, name.len), "");
	}
	dst.off += dputf(dst, "\n");

	u8 *row;
	uint r = 0;
	arr_foreach(&tbl->rows, r, row)
	{
		tbl_col_t *col;
		uint c = 0;
		arr_foreach(&tbl->cols, c, col)
		{
			if (c > 0) {
				dst.off += dputf(dst, " ");
			}

			const u8 *cell	 = &row[col->off];
			size_t col_start = dst.off;

			strv_t str = STRV_NULL;

			switch (col->type) {
			case TBL_COL_TYPE_STR: {
				str = strvbuf_get(&tbl->strs, *(size_t *)cell);
				break;
			}
			case TBL_COL_TYPE_ENUM: {
				uint i = 0;
				const u8 *val;
				arr_foreach(&col->vals, i, val)
				{
					if (mem_cmp(cell, val, col->size) != 0) {
						continue;
					}

					str = strvbuf_get(&tbl->strs, *(size_t *)(&val[col->size]));
					break;
				}

				break;
			}
			case TBL_COL_TYPE_FLAG: {
				uint i = 0;
				const u8 *val;
				arr_foreach(&col->vals, i, val)
				{
					if (!bit_is_set(cell, *val)) {
						continue;
					}

					str = strvbuf_get(&tbl->strs, *(size_t *)(&val[sizeof(u8)]));
					dst.off += dputf(dst, "%.*s", str.len, str.data);
				}

				str = STRV("");

				break;
			}
			default: break;
			}

			if (str.data) {
				dst.off += strv_print(str, dst);
			} else {
				dst.off += dputf(dst, "0x");
				for (uint i = 0; i < col->size; i++) {
					dst.off += dputf(dst, "%02X", cell[col->size - i - 1]);
				}
			}

			dst.off += dputf(dst, "%*s", SUB(col->len, dst.off - col_start), "");
		}
		dst.off += dputf(dst, "\n");
	}

	return dst.off - off;
}
