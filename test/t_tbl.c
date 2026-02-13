#include "tbl.h"

#include "alloc.h"
#include "dst.h"
#include "log.h"
#include "mem.h"
#include "test.h"
/*
TEST(tbl_init_free)
{
	START;

	tbl_t tbl = {0};

	EXPECT_EQ(tbl_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(tbl_init(&tbl, 1, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(tbl_init(&tbl, 1, 1, ALLOC_STD), &tbl);

	tbl_free(&tbl);
	tbl_free(NULL);

	END;
}

TEST(tbl_init_rows)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_INT, 1, NULL);

	EXPECT_EQ(tbl_init_rows(NULL, 0, ALLOC_STD), 1);
	mem_oom(1);
	EXPECT_EQ(tbl_init_rows(&tbl, 1, ALLOC_STD), 1);
	mem_oom(0);
	EXPECT_EQ(tbl_init_rows(&tbl, 1, ALLOC_STD), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_row)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_INT, 1, NULL);
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

TEST(tbl_get_cell)
{
	START;

	tbl_t tbl = {0};
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_INT, 1, &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_get_cell(NULL, field, row), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_get_cell(&tbl, tbl.layout.fields.cnt, row), NULL);
	EXPECT_EQ(tbl_get_cell(&tbl, field, tbl.rows.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(tbl_get_cell(&tbl, field, row), NULL);

	tbl_free(&tbl);

	END;
}

TEST(tbl_set_cell_str)
{
	START;

	tbl_t tbl = {0};
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_STR, 1, &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_set_cell_str(NULL, field, row, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, tbl.layout.fields.cnt, row, STRV_NULL), 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, field, tbl.rows.cnt, STRV_NULL), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_set_cell_str(&tbl, field, row, STRV_NULL), 1);
	tbl.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, field, row, STRV_NULL), 0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, field, row, STRV(" ")), 0);
	const field_t *c = layout_get_field(&tbl.layout, field);
	EXPECT_EQ(c->len, 1);

	tbl_free(&tbl);

	END;
}

TEST(tbl_print)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};

	tbl_init(&tbl, 1, 1, ALLOC_STD);

	EXPECT_EQ(tbl_print(NULL, DST_BUF(buf)), 0);
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

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV("c1"), FIELD_TYPE_STR, 1, NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 3);
	EXPECT_STR(buf, "c1\n");

	layout_add_field(&tbl.layout, STRV("c2"), FIELD_TYPE_STR, 1, NULL);

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

	tbl_init(&tbl, 2, 2, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_STR, 1, &c1);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_STR, 1, &c2);
	tbl_init_rows(&tbl, 1, ALLOC_STD);

	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, c1, STRV_NULL);
	tbl_set_cell_str(&tbl, row, c2, STRV_NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 8);
	EXPECT_STR(buf,
		   "   \n"
		   "   \n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_int)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_INT, sizeof(int), &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	int *val = tbl_get_cell(&tbl, row, field);
	*val	 = 1;

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 22);
	EXPECT_STR(buf,
		   "          \n"
		   "0x00000001\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_str_null)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_STR, 1, &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, field, STRV_NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 4);
	EXPECT_STR(buf,
		   " \n"
		   " \n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_str)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_STR, 1, &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, field, STRV("v"));

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
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_ENUM, sizeof(char), &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char *val = tbl_get_cell(&tbl, row, field);
	*val	  = 1;

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 10);
	EXPECT_STR(buf,
		   "    \n"
		   "0x01\n");

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_enum_val)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};
	uint field, row;
	char *val;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_ENUM, sizeof(char), &field);
	val  = layout_add_enum(&tbl.layout, field, STRV("ZERO"));
	*val = 0;
	val  = layout_add_enum(&tbl.layout, field, STRV("ONE"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	val  = tbl_get_cell(&tbl, row, field);
	*val = 1;

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
	uint field, row;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_FLAG, sizeof(char), &field);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char *val = tbl_get_cell(&tbl, row, field);
	*val	  = 1;

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
	uint field, row;
	char *val;

	tbl_init(&tbl, 1, 1, ALLOC_STD);
	layout_add_field(&tbl.layout, STRV_NULL, FIELD_TYPE_FLAG, sizeof(char), &field);
	val  = layout_add_flag(&tbl.layout, field, STRV("Z"));
	*val = 0;
	val  = layout_add_flag(&tbl.layout, field, STRV("O"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	val  = tbl_get_cell(&tbl, row, field);
	*val = 1 << 1;

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 6);
	EXPECT_STR(buf,
		   "  \n"
		   "O \n");

	tbl_free(&tbl);

	END;
}

STEST(tbl)
{
	SSTART;

	RUN(tbl_init_free);
	RUN(tbl_init_rows);
	RUN(tbl_add_row);
	RUN(tbl_get_cell);
	RUN(tbl_set_cell_str);
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
*/
