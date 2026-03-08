#include "schema.h"

#include "alloc.h"
#include "dst.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(schema_init_free)
{
	START;

	schema_t schema = {0};

	EXPECT_EQ(schema_init(NULL, 1, 1, 1, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(schema_init(&schema, 1, 1, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(schema_init(&schema, 1, 1, 1, ALLOC_STD), &schema);

	schema_free(&schema);
	schema_free(NULL);

	END;
}

TEST(schema_add_def)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint def;
	EXPECT_EQ(schema_add_def(NULL, FIELD_TYPE_INT, STRV_NULL, 0, 0, NULL), NULL);
	mem_oom(1);
	schema.strs.used = schema.strs.size;
	EXPECT_EQ(schema_add_def(&schema, FIELD_TYPE_INT, STRV_NULL, 0, 0, NULL), NULL);
	schema.strs.used = 0;
	schema.defs.cnt	 = schema.defs.cap;
	EXPECT_EQ(schema_add_def(&schema, FIELD_TYPE_INT, STRV_NULL, 0, 0, NULL), NULL);
	schema.defs.cnt = 0;
	mem_oom(0);
	EXPECT_NE(schema_add_def(&schema, FIELD_TYPE_INT, STRV_NULL, 0, 0, &def), NULL);
	EXPECT_EQ(def, 0);
	const field_def_t *d = schema_get_def(&schema, def);
	EXPECT_EQ(d->len, 2);
	EXPECT_NE(schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), 0, 0, &def), NULL);
	d = schema_get_def(&schema, def);
	EXPECT_EQ(d->len, 3);

	schema_free(&schema);

	END;
}

TEST(schema_add_def_str)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	EXPECT_NE(schema_add_def(&schema, FIELD_TYPE_STR, STRV_NULL, 0, 0, NULL), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_add_def_enum)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(schema_add_def(&schema, FIELD_TYPE_ENUM, STRV_NULL, 0, 1, NULL), NULL);
	mem_oom(0);
	EXPECT_NE(schema_add_def(&schema, FIELD_TYPE_ENUM, STRV_NULL, 0, 1, NULL), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_add_def_flag)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(schema_add_def(&schema, FIELD_TYPE_FLAG, STRV_NULL, 0, 1, NULL), NULL);
	mem_oom(0);
	EXPECT_NE(schema_add_def(&schema, FIELD_TYPE_FLAG, STRV_NULL, 0, 1, NULL), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_get_def)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV_NULL, 0, 0, &def);

	EXPECT_EQ(schema_get_def(NULL, def), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_def(&schema, schema.defs.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_def(&schema, def), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_add_val_enum)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_ENUM, STRV_NULL, 0, 1, &def);
	EXPECT_EQ(schema_add_val(NULL, def, STRV_NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_add_val(&schema, schema.defs.cnt, STRV_NULL), NULL);
	log_set_quiet(0, 0);
	field_def_t *d = (field_def_t *)schema_get_def(&schema, def);
	mem_oom(1);
	d->vals.cnt = d->vals.cap;
	EXPECT_EQ(schema_add_val(&schema, def, STRV_NULL), NULL);
	d->vals.cnt	 = 0;
	schema.strs.used = schema.strs.size;
	EXPECT_EQ(schema_add_val(&schema, def, STRV_NULL), NULL);
	schema.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(d->vals.cnt, 0);
	EXPECT_NE(schema_add_val(&schema, def, STRV_NULL), NULL);
	EXPECT_NE(schema_add_val(&schema, def, STRV(" ")), NULL);
	EXPECT_EQ(d->len, 1);

	schema_free(&schema);

	END;
}

TEST(schema_add_val_flag)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_FLAG, STRV_NULL, 0, 1, &def);
	EXPECT_EQ(schema_add_val(NULL, def, STRV_NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_add_val(&schema, schema.defs.cnt, STRV_NULL), NULL);
	log_set_quiet(0, 0);
	field_def_t *d = (field_def_t *)schema_get_def(&schema, def);
	mem_oom(1);
	d->vals.cnt = d->vals.cap;
	EXPECT_EQ(schema_add_val(&schema, def, STRV_NULL), NULL);
	d->vals.cnt	 = 0;
	schema.strs.used = schema.strs.size;
	EXPECT_EQ(schema_add_val(&schema, def, STRV_NULL), NULL);
	schema.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(d->vals.cnt, 0);
	EXPECT_NE(schema_add_val(&schema, def, STRV_NULL), NULL);
	EXPECT_NE(schema_add_val(&schema, def, STRV(" ")), NULL);
	EXPECT_EQ(d->len, 1);

	schema_free(&schema);

	END;
}

TEST(schema_add_layout)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	EXPECT_EQ(schema_add_layout(NULL, 0, NULL), 1);
	mem_oom(1);
	schema.layouts.cnt = schema.layouts.cap;
	EXPECT_EQ(schema_add_layout(&schema, 1, &layout), 1);
	schema.layouts.cnt = 0;
	EXPECT_EQ(schema_add_layout(&schema, 1, &layout), 1);
	mem_oom(0);
	EXPECT_EQ(schema_add_layout(&schema, 1, &layout), 0);

	schema_free(&schema);

	END;
}

TEST(schema_get_layout)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	EXPECT_EQ(schema_get_layout(NULL, schema.layouts.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_layout(&schema, schema.layouts.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_layout(&schema, layout), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_map_layout)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	EXPECT_EQ(schema_map_layout(NULL, schema.layouts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_map_layout(&schema, schema.layouts.cnt), 1);
	log_set_quiet(0, 0);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	log_set_quiet(0, 1);
	EXPECT_EQ(schema_map_layout(&schema, schema.layouts.cnt), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	schema.maps.cnt = schema.maps.cap;
	EXPECT_EQ(schema_map_layout(&schema, layout), 1);
	schema.maps.cnt = 0;
	mem_oom(0);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);
	schema_add_field(&schema, layout, def, sizeof(int), NULL);

	mem_oom(1);
	EXPECT_EQ(schema_map_layout(&schema, layout), 1);
	mem_oom(0);
	EXPECT_EQ(schema_map_layout(&schema, layout), 0);

	schema_free(&schema);

	END;
}

TEST(schema_map_layout_drop)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_add_layout(&schema, 1, NULL);
	uint layout;
	schema_add_layout(&schema, 1, &layout);

	log_set_quiet(0, 1);
	EXPECT_EQ(schema_map_layout(&schema, schema.layouts.cnt), 1);
	log_set_quiet(0, 0);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);
	schema_add_field(&schema, layout, def, sizeof(int), NULL);

	EXPECT_EQ(schema_map_layout(&schema, layout), 0);

	schema_free(&schema);

	END;
}

TEST(schema_add_field)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	EXPECT_EQ(schema_add_field(NULL, 0, 0, 0, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_add_field(&schema, schema.layouts.cnt, 0, 0, NULL), 1);
	EXPECT_EQ(schema_add_field(&schema, layout, schema.defs.cnt, 0, NULL), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	layout_t *l   = (layout_t *)schema_get_layout(&schema, layout);
	l->fields.cnt = l->fields.cap;
	EXPECT_EQ(schema_add_field(&schema, layout, def, sizeof(int), NULL), 1);
	l->fields.cnt = 0;
	mem_oom(0);
	uint field;
	EXPECT_EQ(schema_add_field(&schema, layout, def, sizeof(int), &field), 0);
	EXPECT_EQ(field, 0);

	schema_free(&schema);

	END;
}

TEST(schema_add_field_str)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_STR, STRV("def"), sizeof(int), 1, &def);

	EXPECT_EQ(schema_add_field(&schema, layout, def, sizeof(int), NULL), 0);

	schema_free(&schema);

	END;
}

TEST(schema_get_field)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	const layout_t *l = schema_get_layout(&schema, layout);

	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	EXPECT_EQ(schema_get_field(NULL, schema.layouts.cnt, l->fields.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_field(&schema, schema.layouts.cnt, l->fields.cnt), NULL);
	EXPECT_EQ(schema_get_field(&schema, layout, l->fields.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_field(&schema, layout, field), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_get_str)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	EXPECT_EQ(schema_get_str(NULL, schema.strs.size).data, NULL);

	schema_free(&schema);

	END;
}

TEST(schema_set_val)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	uint field;
	EXPECT_EQ(schema_add_field(&schema, layout, def, sizeof(int), &field), 0);

	schema_map_layout(&schema, layout);

	int data = 0;
	int val	 = 1;

	EXPECT_EQ(schema_set_val(NULL, schema.layouts.cnt, 1, NULL, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_set_val(&schema, schema.layouts.cnt, 1, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, 1, NULL, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(schema_set_val(&schema, layout, field, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, field, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, field, &data, &val), 0);
	EXPECT_EQ(data, val);

	schema_free(&schema);

	END;
}

TEST(schema_set_val_drop)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);
	log_set_quiet(0, 1);
	schema_map_layout(&schema, layout);
	log_set_quiet(0, 0);
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);
	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	schema_map_layout(&schema, layout);

	int data = 0;

	EXPECT_EQ(schema_set_val(&schema, layout, field, &data, NULL), 0);

	schema_free(&schema);

	END;
}

TEST(schema_get_val)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_val(&schema, 1, NULL), NULL);
	log_set_quiet(0, 0);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	uint field;
	EXPECT_EQ(schema_add_field(&schema, layout, def, sizeof(int), &field), 0);

	schema_map_layout(&schema, layout);

	int data = 1;

	EXPECT_EQ(schema_get_val(NULL, 1, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_val(&schema, 1, NULL), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_val(&schema, field, &data), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_print_val)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	schema_map_layout(&schema, layout);

	EXPECT_EQ(schema_print_val(NULL, schema.layouts.cnt, 1, NULL, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_val(&schema, schema.layouts.cnt, 1, NULL, DST_NONE()), 0);
	EXPECT_EQ(schema_print_val(&schema, layout, 1, NULL, DST_NONE()), 0);
	field_t *f   = (field_t *)schema_get_field(&schema, layout, def);
	uint tmp_def = f->def;
	f->def	     = schema.defs.cnt;
	EXPECT_EQ(schema_print_val(&schema, layout, field, NULL, DST_NONE()), 0);
	f->def = tmp_def;
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_val(&schema, layout, field, NULL, DST_NONE()), 0);

	schema_free(&schema);

	END;
}

TEST(schema_print_val_int)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	schema_map_layout(&schema, layout);

	int val = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, field, &val, DST_BUF(buf)), 10);
	EXPECT_STR(buf, "0x00000001");

	schema_free(&schema);

	END;
}

TEST(schema_print_val_enum)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_ENUM, STRV("def"), sizeof(int), 1, &def);
	int *v = schema_add_val(&schema, def, STRV("val0"));
	*v     = 0;
	v      = schema_add_val(&schema, def, STRV("val1"));
	*v     = 1;

	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	schema_map_layout(&schema, layout);

	int val = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, field, &val, DST_BUF(buf)), 4);
	EXPECT_STR(buf, "val1");

	schema_free(&schema);

	END;
}

TEST(schema_print_val_flag)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_FLAG, STRV("def"), 8, 1, &def);
	u8 *v = schema_add_val(&schema, def, STRV("val0"));
	*v    = 0;
	v     = schema_add_val(&schema, def, STRV("val1"));
	*v    = 1;

	uint field;
	schema_add_field(&schema, layout, def, 8, &field);

	schema_map_layout(&schema, layout);

	u8 val = 1 << 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, field, &val, DST_BUF(buf)), 4);
	EXPECT_STR(buf, "val1");

	schema_free(&schema);

	END;
}

TEST(schema_print_data)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, 1, &layout);

	uint def;
	schema_add_def(&schema, FIELD_TYPE_INT, STRV("def"), sizeof(int), 1, &def);

	uint field;
	schema_add_field(&schema, layout, def, sizeof(int), &field);

	schema_map_layout(&schema, layout);

	int data = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_data(NULL, schema.layouts.cnt, NULL, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_data(&schema, schema.layouts.cnt, NULL, DST_NONE()), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(schema_print_data(&schema, layout, NULL, DST_NONE()), 0);
	EXPECT_EQ(schema_print_data(&schema, layout, &data, DST_BUF(buf)), 16);
	EXPECT_STR(buf, "def: 0x00000001\n")

	schema_free(&schema);

	END;
}

STEST(schema)
{
	SSTART;

	RUN(schema_init_free);
	RUN(schema_add_def);
	RUN(schema_add_def_str);
	RUN(schema_add_def_enum);
	RUN(schema_add_def_flag);
	RUN(schema_get_def);
	RUN(schema_add_val_enum);
	RUN(schema_add_val_flag);
	RUN(schema_add_layout);
	RUN(schema_get_layout);
	RUN(schema_map_layout);
	RUN(schema_map_layout_drop);
	RUN(schema_add_field);
	RUN(schema_add_field_str);
	RUN(schema_get_field);
	RUN(schema_get_str);
	RUN(schema_set_val);
	RUN(schema_set_val_drop);
	RUN(schema_get_val);
	RUN(schema_print_val);
	RUN(schema_print_val_int);
	RUN(schema_print_val_enum);
	RUN(schema_print_val_flag);
	RUN(schema_print_data);

	SEND;
}
