#ifndef TBL_H
#define TBL_H

#include "arr.h"
#include "strvbuf.h"

typedef enum tbl_col_type_e {
	TBL_COL_TYPE_UNKNOWN,
	TBL_COL_TYPE_INT,
	TBL_COL_TYPE_STR,
	TBL_COL_TYPE_ENUM,
	TBL_COL_TYPE_FLAG,
} tbl_col_type_t;

typedef struct tbl_col_s {
	size_t name;
	tbl_col_type_t type;
	size_t off;
	size_t size;
	size_t len;
	arr_t vals;
} tbl_col_t;

typedef struct tbl_s {
	arr_t cols;
	arr_t rows;
	strvbuf_t strs;
} tbl_t;

tbl_t *tbl_init(tbl_t *tbl, size_t cols_cap, alloc_t alloc);
void tbl_free(tbl_t *tbl);

int tbl_add_col(tbl_t *tbl, strv_t name, tbl_col_type_t type, size_t size, alloc_t alloc, uint *col);
const tbl_col_t *tbl_get_col(const tbl_t *tbl, uint col);

void *tbl_add_enum(tbl_t *tbl, uint col, strv_t str);
void *tbl_add_flag(tbl_t *tbl, uint col, strv_t str);

int tbl_init_rows(tbl_t *tbl, size_t cap, alloc_t alloc);
int tbl_add_row(tbl_t *tbl, uint *row);

void *tbl_get_cell(tbl_t *tbl, uint row, uint col);
int tbl_set_cell_str(tbl_t *tbl, uint row, uint col, strv_t val);

size_t tbl_print(const tbl_t *tbl, dst_t dst);

#endif
