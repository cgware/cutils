#include "tbl.h"

#include "alloc.h"
#include "dst.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(tbl_init_free)
{
	START;

	tbl_t tbl = {0};

	EXPECT_EQ(tbl_init(NULL, 0, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(tbl_init(&tbl, 1, 1, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(tbl_init(&tbl, 1, 1, 1, ALLOC_STD), &tbl);

	tbl_free(&tbl);
	tbl_free(NULL);

	END;
}

TEST(tbl_init_rows)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	uint layout;
	schema_add_layout(&tbl.schema, 1, &layout);

	EXPECT_EQ(tbl_init_rows(NULL, 0, ALLOC_STD), 1);
	mem_oom(1);
	EXPECT_EQ(tbl_init_rows(&tbl, 1, ALLOC_STD), 1);
	mem_oom(0);
	log_set_quiet(0, 1);
	tbl.schema.layouts.cnt = 0;
	EXPECT_EQ(tbl_init_rows(&tbl, 1, ALLOC_STD), 1);
	tbl.schema.layouts.cnt = 1;
	EXPECT_EQ(tbl_init_rows(&tbl, 1, ALLOC_STD), 0);
	log_set_quiet(0, 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_row)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	uint layout;
	schema_add_layout(&tbl.schema, 1, &layout);
	uint def;
	schema_add_def(&tbl.schema, FIELD_TYPE_INT, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, NULL);
	tbl_init_rows(&tbl, 1, ALLOC_STD);

	uint row;
	EXPECT_EQ(tbl_add_row(NULL, &row), NULL);
	mem_oom(1);
	tbl.rows.cnt = tbl.rows.cap;
	EXPECT_EQ(tbl_add_row(&tbl, &row), NULL);
	tbl.rows.cnt = 0;
	mem_oom(0);
	EXPECT_NE(tbl_add_row(&tbl, &row), NULL);
	EXPECT_EQ(row, 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_set_cell)
{
	START;

	tbl_t tbl = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_INT, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	char val = 0;
	EXPECT_EQ(tbl_set_cell(NULL, row, field, layout, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_set_cell(&tbl, row, field + 1, layout, &val), 1);
	EXPECT_EQ(tbl_set_cell(&tbl, tbl.rows.cnt, field, layout, &val), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(tbl_set_cell(&tbl, row, field, layout, &val), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_set_cell_str)
{
	START;

	tbl_t tbl = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_set_cell_str(NULL, row, field, layout, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, row, field + 1, layout, STRV_NULL), 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, tbl.rows.cnt, field, layout, STRV_NULL), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_set_cell_str(&tbl, row, field, layout, STRV_NULL), 1);
	tbl.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, row, field, layout, STRV_NULL), 0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, row, field, layout, STRV(" ")), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_get_cell)
{
	START;

	tbl_t tbl = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_get_cell(NULL, row, field), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_get_cell(&tbl, tbl.rows.cnt, field), NULL);
	EXPECT_EQ(tbl_get_cell(&tbl, row, field + 1), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(tbl_get_cell(&tbl, row, field), NULL);

	tbl_free(&tbl);

	END;
}

static int map_fn(tbl_t *tbl, uint row, uint col, const void *data, void *priv)
{
	(void)tbl;
	(void)row;
	(void)col;
	(void)data;
	(void)priv;
	return 0;
}

TEST(tbl_map)
{
	START;

	tbl_t tbl = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_map(NULL, 0, 0, NULL, NULL), 1);
	EXPECT_EQ(tbl_map(&tbl, 0, 0, map_fn, NULL), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_print)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);

	EXPECT_EQ(tbl_print(NULL, DST_BUF(buf)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 0);
	log_set_quiet(0, 0);

	schema_add_layout(&tbl.schema, 1, &layout);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 1);
	EXPECT_STRN(buf, "\n", 1);

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_field)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};

	uint layout, def, field;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV("c1"), 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 3);
	EXPECT_STR(buf, "c1\n");

	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV("c2"), 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 6);
	EXPECT_STR(buf, "c1 c2\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_row)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint c1, c2, row;

	uint layout, def0, def1;

	tbl_init(&tbl, 2, 2, 2, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def0);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def1);
	schema_add_field(&tbl.schema, layout, def0, 0, &c1);
	schema_add_field(&tbl.schema, layout, def1, 0, &c2);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);

	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, c1, layout, STRV_NULL);
	tbl_set_cell_str(&tbl, row, c2, layout, STRV_NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 4);
	EXPECT_STR(buf,
		   " \n"
		   " \n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_int)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_INT, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	int val = 1;
	tbl_set_cell(&tbl, row, field, layout, &val);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 10);
	EXPECT_STR(buf,
		   "    \n"
		   "0x01\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_str_null)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 1, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 0, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, field, layout, STRV_NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 2);
	EXPECT_STR(buf,
		   "\n"
		   "\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_str)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_STR, STRV_NULL, 0, 0, &def);
	schema_add_field(&tbl.schema, layout, def, 0, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, field, layout, STRV("v"));

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 4);
	EXPECT_STR(buf,
		   " \n"
		   "v\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_enum_raw)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_ENUM, STRV_NULL, sizeof(char), 1, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	int val = 1;
	tbl_set_cell(&tbl, row, field, layout, &val);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 6);
	EXPECT_STR(buf,
		   "\n"
		   "0x01\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_enum_val)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;
	char *val;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_ENUM, STRV_NULL, sizeof(char), 2, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	val  = schema_add_val(&tbl.schema, field, STRV("ZERO"));
	*val = 0;
	val  = schema_add_val(&tbl.schema, field, STRV("ONE"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char v = 1;
	tbl_set_cell(&tbl, row, field, layout, &v);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 10);
	EXPECT_STR(buf,
		   "    \n"
		   "ONE \n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_flag_raw)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_FLAG, STRV_NULL, sizeof(char), 2, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char val = 1;
	tbl_set_cell(&tbl, row, field, layout, &val);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 2);
	EXPECT_STR(buf,
		   "\n"
		   "\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_flag_val)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint layout, def, field, row;
	char *val;

	tbl_init(&tbl, 1, 1, 1, ALLOC_STD);
	schema_add_layout(&tbl.schema, 1, &layout);
	schema_add_def(&tbl.schema, FIELD_TYPE_FLAG, STRV_NULL, sizeof(char), 2, &def);
	schema_add_field(&tbl.schema, layout, def, 1, &field);
	schema_map_layout(&tbl.schema, 0);
	val  = schema_add_val(&tbl.schema, field, STRV("Z"));
	*val = 0;
	val  = schema_add_val(&tbl.schema, field, STRV("O"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char v = 1 << 1;
	tbl_set_cell(&tbl, row, field, layout, &v);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 4);
	EXPECT_STR(buf,
		   " \n"
		   "O\n");

	tbl_free(&tbl);

	END;
}

STEST(tbl)
{
	SSTART;

	RUN(tbl_init_free);
	RUN(tbl_init_rows);
	RUN(tbl_add_row);
	RUN(tbl_set_cell);
	RUN(tbl_set_cell_str);
	RUN(tbl_get_cell);
	RUN(tbl_map);
	RUN(tbl_print);
	RUN(tbl_print_field);
	RUN(tbl_print_row);
	RUN(tbl_print_int);
	RUN(tbl_print_str_null);
	RUN(tbl_print_str);
	RUN(tbl_print_enum_raw);
	RUN(tbl_print_enum_val);
	RUN(tbl_print_flag_raw);
	RUN(tbl_print_flag_val);

	SEND;
}
