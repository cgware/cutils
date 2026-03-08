#include "schema.h"

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

TEST(schema_add_fields)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, 0, SCHEMA_TYPE_INT, NULL, 0};

	EXPECT_EQ(schema_add_fields(NULL, NULL, 0), 1);
	mem_oom(1);
	schema.strs.used = schema.strs.size;
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 1);
	schema.strs.used  = 0;
	schema.fields.cnt = schema.fields.cap;
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 1);
	schema.fields.cnt = 0;
	mem_oom(0);
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 0);

	const schema_field_t *f = schema_get_field(&schema, 0);
	EXPECT_EQ(f->len, 2);

	schema_field_desc_t fields2 = {STRV("field"), 0, SCHEMA_TYPE_INT, NULL, 0};
	EXPECT_EQ(schema_add_fields(&schema, &fields2, sizeof(fields2)), 0);
	f = schema_get_field(&schema, 1);
	EXPECT_EQ(f->len, 5);

	schema_free(&schema);

	END;
}

TEST(schema_add_fields_str)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, 0, SCHEMA_TYPE_STR, NULL, 0};

	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 0);

	schema_free(&schema);

	END;
}

TEST(schema_add_fields_enum)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 3, 1, 2, ALLOC_STD);

	schema_val_t vals[]	   = {{0, STRV_NULL}};
	schema_field_desc_t fields = {STRV_NULL, 0, SCHEMA_TYPE_ENUM, vals, sizeof(vals)};

	mem_oom(1);
	schema.vals.used = schema.vals.size;
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 1);
	schema.vals.used = 0;
	schema.strs.used = schema.strs.size - sizeof(size_t);
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 1);
	schema.strs.used = 0;
	mem_oom(0);
	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 0);

	schema_free(&schema);

	END;
}

TEST(schema_add_fields_flag)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, 0, SCHEMA_TYPE_FLAG, NULL, 0};

	EXPECT_EQ(schema_add_fields(&schema, &fields, sizeof(fields)), 0);

	schema_free(&schema);

	END;
}

TEST(schema_get_field)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, 0, SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	EXPECT_EQ(schema_get_field(NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_field(&schema, schema.fields.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_field(&schema, 0), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_add_layout)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 4, 4, 3, ALLOC_STD);

	uint layout;
	EXPECT_EQ(schema_add_layout(NULL, NULL, 0, NULL), 1);
	mem_oom(1);
	schema.layouts.cnt = schema.layouts.cap;
	EXPECT_EQ(schema_add_layout(&schema, NULL, 0, &layout), 1);
	schema.layouts.cnt = 0;
	mem_oom(0);
	EXPECT_EQ(schema_add_layout(&schema, NULL, 0, &layout), 0);

	schema_member_desc_t members[] = {
		{0, sizeof(int)},
		{1, 0},
	};

	log_set_quiet(0, 1);
	EXPECT_EQ(schema_add_layout(&schema, members, sizeof(members), &layout), 1);
	log_set_quiet(0, 0);

	schema_field_desc_t fields[] = {
		{STRV_NULL, 1, SCHEMA_TYPE_INT, NULL, 0},
		{STRV_NULL, 0, SCHEMA_TYPE_STR, NULL, 0},
	};
	schema_add_fields(&schema, fields, sizeof(fields));

	mem_oom(1);
	schema.members.cnt = schema.members.cap;
	EXPECT_EQ(schema_add_layout(&schema, members, sizeof(members), &layout), 1);
	schema.members.cnt    = 0;
	schema.field_maps.cnt = schema.field_maps.cap;
	EXPECT_EQ(schema_add_layout(&schema, members, sizeof(members), &layout), 1);
	schema.field_maps.cnt = 0;
	mem_oom(0);
	EXPECT_EQ(schema_add_layout(&schema, members, sizeof(members), &layout), 0);

	schema_free(&schema);

	END;
}

TEST(schema_get_layout)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	uint layout;
	schema_add_layout(&schema, NULL, 0, &layout);

	EXPECT_EQ(schema_get_layout(NULL, schema.layouts.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_layout(&schema, schema.layouts.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_layout(&schema, layout), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_map_layout_drop)
{
	START;

	schema_t schema = {0};
	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_add_layout(&schema, NULL, 0, NULL);

	schema_field_desc_t fields = {STRV_NULL, 1, SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, 1};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	schema_free(&schema);

	END;
}

TEST(schema_get_member)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, 1, SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, 1};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	EXPECT_EQ(schema_get_member(NULL, schema.layouts.cnt, schema.members.cnt), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_member(&schema, schema.layouts.cnt, schema.members.cnt), NULL);
	EXPECT_EQ(schema_get_member(&schema, layout, schema.members.cnt), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_member(&schema, layout, 0), NULL);

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

	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int data = 0;
	int val	 = 1;

	EXPECT_EQ(schema_set_val(NULL, schema.layouts.cnt, 1, NULL, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_set_val(&schema, schema.layouts.cnt, 1, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, 1, NULL, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(schema_set_val(&schema, layout, 0, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, 0, NULL, NULL), 1);
	EXPECT_EQ(schema_set_val(&schema, layout, 0, &data, &val), 0);
	EXPECT_EQ(data, val);

	schema_free(&schema);

	END;
}

TEST(schema_set_val_drop)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	log_set_quiet(0, 1);
	schema_add_layout(&schema, NULL, 0, &layout);
	log_set_quiet(0, 0);

	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int data = 0;

	EXPECT_EQ(schema_set_val(&schema, layout, 0, &data, NULL), 0);

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

	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int data = 1;

	EXPECT_EQ(schema_get_val(NULL, 1, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_get_val(&schema, 1, NULL), NULL);
	log_set_quiet(0, 0);
	EXPECT_NE(schema_get_val(&schema, 0, &data), NULL);

	schema_free(&schema);

	END;
}

TEST(schema_print_val)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	EXPECT_EQ(schema_print_val(NULL, schema.layouts.cnt, 1, NULL, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_val(&schema, schema.layouts.cnt, 1, NULL, DST_NONE()), 0);
	EXPECT_EQ(schema_print_val(&schema, layout, 1, NULL, DST_NONE()), 0);
	schema_member_t *m = (schema_member_t *)schema_get_member(&schema, layout, 0);
	uint tmp_def	   = m->field;
	m->field	   = schema.fields.cnt;
	EXPECT_EQ(schema_print_val(&schema, layout, 0, NULL, DST_NONE()), 0);
	m->field = tmp_def;
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_val(&schema, layout, 0, NULL, DST_NONE()), 0);

	schema_free(&schema);

	END;
}

TEST(schema_print_val_int)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int val = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, 0, &val, DST_BUF(buf)), 10);
	EXPECT_STR(buf, "0x00000001");

	schema_free(&schema);

	END;
}

TEST(schema_print_val_enum)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_val_t vals[] = {
		{0, STRV("val0")},
		{1, STRV("val1")},
	};
	schema_field_desc_t fields = {STRV_NULL, sizeof(int), SCHEMA_TYPE_ENUM, vals, sizeof(vals)};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int val = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, 0, &val, DST_BUF(buf)), 4);
	EXPECT_STR(buf, "val1");

	schema_free(&schema);

	END;
}

TEST(schema_print_val_flag)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_val_t vals[] = {
		{0, STRV("val0")},
		{1, STRV("val1")},
	};
	schema_field_desc_t fields = {STRV_NULL, 1, SCHEMA_TYPE_FLAG, vals, sizeof(vals)};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, 8};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	u8 val = 1 << 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_val(&schema, layout, 0, &val, DST_BUF(buf)), 4);
	EXPECT_STR(buf, "val1");

	schema_free(&schema);

	END;
}

TEST(schema_print_data)
{
	START;

	schema_t schema = {0};

	schema_init(&schema, 1, 1, 1, ALLOC_STD);

	schema_field_desc_t fields = {STRV("field"), sizeof(int), SCHEMA_TYPE_INT, NULL, 0};
	schema_add_fields(&schema, &fields, sizeof(fields));

	uint layout;
	schema_member_desc_t members = {0, sizeof(int)};
	schema_add_layout(&schema, &members, sizeof(members), &layout);

	int data = 1;

	char buf[256] = {0};

	EXPECT_EQ(schema_print_data(NULL, schema.layouts.cnt, NULL, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(schema_print_data(&schema, schema.layouts.cnt, NULL, DST_NONE()), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(schema_print_data(&schema, layout, NULL, DST_NONE()), 0);
	EXPECT_EQ(schema_print_data(&schema, layout, &data, DST_BUF(buf)), 18);
	EXPECT_STR(buf, "field: 0x00000001\n")

	schema_free(&schema);

	END;
}

STEST(schema)
{
	SSTART;

	RUN(schema_init_free);
	RUN(schema_add_fields);
	RUN(schema_add_fields_str);
	RUN(schema_add_fields_enum);
	RUN(schema_add_fields_flag);
	RUN(schema_get_field);
	RUN(schema_add_layout);
	RUN(schema_get_layout);
	RUN(schema_map_layout_drop);
	RUN(schema_get_member);
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
