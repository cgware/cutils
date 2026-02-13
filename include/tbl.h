#ifndef TBL_H
#define TBL_H

#include "schema.h"
#include "strvbuf.h"

typedef struct tbl_s {
	schema_t schema;
	arr_t rows;
	strvbuf_t strs;
} tbl_t;

tbl_t *tbl_init(tbl_t *tbl, uint cols_cap, uint layouts_cap, uint strs_cap, alloc_t alloc);
void tbl_free(tbl_t *tbl);

int tbl_init_rows(tbl_t *tbl, size_t cap, alloc_t alloc);
void *tbl_add_row(tbl_t *tbl, uint *id);

int tbl_set_cell(tbl_t *tbl, uint row, uint col, uint layout, void *val);
int tbl_set_cell_str(tbl_t *tbl, uint row, uint col, uint layout, strv_t val);
const void *tbl_get_cell(tbl_t *tbl, uint row, uint col);

typedef int (*tbl_map_fn)(tbl_t *tbl, uint row, uint col, const void *data, void *priv);
int tbl_map(tbl_t *tbl, uint from, uint to, tbl_map_fn fn, void *priv);

size_t tbl_print(const tbl_t *tbl, dst_t dst);

#define row_foreach(_tbl, _i, _row) arr_foreach(&(_tbl)->rows, _i, _row)

#endif
