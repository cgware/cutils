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

	EXPECT_EQ(schema_init(NULL, 0, 0, 0, ALLOC_STD), NULL);
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

	SEND;
}
