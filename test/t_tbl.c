#include "alloc.h"
#include "dst.h"
#include "tbl.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(tbl_init_free)
{
	START;

	tbl_t tbl = {0};

	EXPECT_EQ(tbl_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(tbl_init(&tbl, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(tbl_init(&tbl, 1, ALLOC_STD), &tbl);

	tbl_free(&tbl);
	tbl_free(NULL);

	END;
}

TEST(tbl_add_col)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	EXPECT_EQ(tbl_add_col(NULL, STRV_NULL, TBL_COL_TYPE_INT, 0, ALLOC_STD, NULL), 1);
	mem_oom(1);
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 0, ALLOC_STD, &col), 1);
	tbl.strs.used = 0;
	tbl.cols.cnt  = tbl.cols.cap;
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 0, ALLOC_STD, &col), 1);
	tbl.cols.cnt = 0;
	mem_oom(0);
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 0, ALLOC_STD, &col), 0);
	EXPECT_EQ(col, 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_col_str)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 0, ALLOC_STD, &col), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_col_enum)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	mem_oom(1);
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_ENUM, 0, ALLOC_STD, &col), 1);
	mem_oom(0);
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_ENUM, 0, ALLOC_STD, &col), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_col_flag)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	mem_oom(1);
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_FLAG, 0, ALLOC_STD, &col), 1);
	mem_oom(0);
	EXPECT_EQ(tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_FLAG, 0, ALLOC_STD, &col), 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_get_col)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 0, ALLOC_STD, &col);

	EXPECT_EQ(tbl_get_col(NULL, col), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_get_col(&tbl, tbl.cols.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(tbl_get_col(&tbl, col), NULL);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_enum)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_ENUM, 0, ALLOC_STD, &col);
	EXPECT_EQ(tbl_add_enum(NULL, col, STRV_NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_add_enum(&tbl, tbl.cols.cnt, STRV_NULL), NULL);
	log_set_quiet(0, 0);
	tbl_col_t *c = (tbl_col_t *)tbl_get_col(&tbl, col);
	mem_oom(1);
	c->vals.cnt = c->vals.cap;
	EXPECT_EQ(tbl_add_enum(&tbl, col, STRV_NULL), NULL);
	c->vals.cnt   = 0;
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_add_enum(&tbl, col, STRV_NULL), NULL);
	tbl.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(c->vals.cnt, 0);
	EXPECT_NE(tbl_add_enum(&tbl, col, STRV_NULL), NULL);
	EXPECT_NE(tbl_add_enum(&tbl, col, STRV(" ")), NULL);
	EXPECT_EQ(c->len, 2);

	tbl_free(&tbl);

	END;
}

TEST(tbl_add_flag)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	uint col;
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_FLAG, 0, ALLOC_STD, &col);
	EXPECT_EQ(tbl_add_flag(NULL, col, STRV_NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_add_flag(&tbl, tbl.cols.cnt, STRV_NULL), NULL);
	log_set_quiet(0, 0);
	tbl_col_t *c = (tbl_col_t *)tbl_get_col(&tbl, col);
	mem_oom(1);
	c->vals.cnt = c->vals.cap;
	EXPECT_EQ(tbl_add_flag(&tbl, col, STRV_NULL), NULL);
	c->vals.cnt   = 0;
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_add_flag(&tbl, col, STRV_NULL), NULL);
	tbl.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(c->vals.cnt, 0);
	EXPECT_NE(tbl_add_flag(&tbl, col, STRV_NULL), NULL);
	EXPECT_NE(tbl_add_flag(&tbl, col, STRV_NULL), NULL);
	EXPECT_EQ(c->len, 2);

	tbl_free(&tbl);

	END;
}

TEST(tbl_init_rows)
{
	START;

	tbl_t tbl = {0};

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 1, ALLOC_STD, NULL);

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

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 1, ALLOC_STD, NULL);
	tbl_init_rows(&tbl, 1, ALLOC_STD);

	uint row;
	EXPECT_EQ(tbl_add_row(NULL, &row), 1);
	mem_oom(1);
	tbl.rows.cnt = tbl.rows.cap;
	EXPECT_EQ(tbl_add_row(&tbl, &row), 1);
	tbl.rows.cnt = 0;
	mem_oom(0);
	EXPECT_EQ(tbl_add_row(&tbl, &row), 0);
	EXPECT_EQ(row, 0);

	tbl_free(&tbl);

	END;
}

TEST(tbl_get_cell)
{
	START;

	tbl_t tbl = {0};
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, 1, ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_get_cell(NULL, col, row), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_get_cell(&tbl, tbl.cols.cnt, row), NULL);
	EXPECT_EQ(tbl_get_cell(&tbl, col, tbl.rows.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(tbl_get_cell(&tbl, col, row), NULL);

	tbl_free(&tbl);

	END;
}

TEST(tbl_set_cell_str)
{
	START;

	tbl_t tbl = {0};
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 1, ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);

	EXPECT_EQ(tbl_set_cell_str(NULL, col, row, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, tbl.cols.cnt, row, STRV_NULL), 1);
	EXPECT_EQ(tbl_set_cell_str(&tbl, col, tbl.rows.cnt, STRV_NULL), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	tbl.strs.used = tbl.strs.size;
	EXPECT_EQ(tbl_set_cell_str(&tbl, col, row, STRV_NULL), 1);
	tbl.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, col, row, STRV_NULL), 0);
	EXPECT_EQ(tbl_set_cell_str(&tbl, col, row, STRV(" ")), 0);
	const tbl_col_t *c = tbl_get_col(&tbl, col);
	EXPECT_EQ(c->len, 1);

	tbl_free(&tbl);

	END;
}

TEST(tbl_print)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};

	tbl_init(&tbl, 1, ALLOC_STD);

	EXPECT_EQ(tbl_print(NULL, DST_BUF(buf)), 0);
	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 1);
	EXPECT_STRN(buf, "\n", 1);

	tbl_free(&tbl);

	END;
}

TEST(tbl_print_col)
{
	START;

	tbl_t tbl     = {0};
	char buf[128] = {0};

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV("c1"), TBL_COL_TYPE_STR, 1, ALLOC_STD, NULL);

	EXPECT_EQ(tbl_print(&tbl, DST_BUF(buf)), 3);
	EXPECT_STR(buf, "c1\n");

	tbl_add_col(&tbl, STRV("c2"), TBL_COL_TYPE_STR, 1, ALLOC_STD, NULL);

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

	tbl_init(&tbl, 2, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 1, ALLOC_STD, &c1);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 1, ALLOC_STD, &c2);
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
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_INT, sizeof(int), ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	int *val = tbl_get_cell(&tbl, row, col);
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
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 1, ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, col, STRV_NULL);

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
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_STR, 1, ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	tbl_set_cell_str(&tbl, row, col, STRV("v"));

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
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_ENUM, sizeof(char), ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char *val = tbl_get_cell(&tbl, row, col);
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
	uint col, row;
	char *val;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_ENUM, sizeof(char), ALLOC_STD, &col);
	val  = tbl_add_enum(&tbl, col, STRV("ZERO"));
	*val = 0;
	val  = tbl_add_enum(&tbl, col, STRV("ONE"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	val  = tbl_get_cell(&tbl, row, col);
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
	uint col, row;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_FLAG, sizeof(char), ALLOC_STD, &col);
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	char *val = tbl_get_cell(&tbl, row, col);
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
	uint col, row;
	char *val;

	tbl_init(&tbl, 1, ALLOC_STD);
	tbl_add_col(&tbl, STRV_NULL, TBL_COL_TYPE_FLAG, sizeof(char), ALLOC_STD, &col);
	val  = tbl_add_flag(&tbl, col, STRV("Z"));
	*val = 0;
	val  = tbl_add_flag(&tbl, col, STRV("O"));
	*val = 1;
	tbl_init_rows(&tbl, 1, ALLOC_STD);
	tbl_add_row(&tbl, &row);
	val  = tbl_get_cell(&tbl, row, col);
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
	RUN(tbl_add_col);
	RUN(tbl_add_col_str);
	RUN(tbl_add_col_enum);
	RUN(tbl_add_col_flag);
	RUN(tbl_get_col);
	RUN(tbl_add_enum);
	RUN(tbl_add_flag);
	RUN(tbl_init_rows);
	RUN(tbl_add_row);
	RUN(tbl_get_cell);
	RUN(tbl_set_cell_str);
	RUN(tbl_print);
	RUN(tbl_print_col);
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
