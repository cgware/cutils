#ifndef SCHEMA_H
#define SCHEMA_H

#include "arr.h"
#include "strvbuf.h"

typedef enum schema_type_e {
	SCHEMA_TYPE_UNKNOWN,
	SCHEMA_TYPE_INT,
	SCHEMA_TYPE_STR,
	SCHEMA_TYPE_ENUM,
	SCHEMA_TYPE_FLAG,
} schema_type_t;

typedef struct schema_val_s {
	u64 val;
	strv_t str;
} schema_val_t;

typedef struct schema_field_desc_s {
	strv_t name;
	size_t size;
	schema_type_t type;
	const schema_val_t *vals;
	size_t vals_size;
} schema_field_desc_t;

typedef struct schema_field_s {
	schema_type_t type;
	size_t name;
	size_t size;
	size_t len;
	uint vals;
	uint vals_cnt;
} schema_field_t;

typedef struct schema_member_desc_s {
	uint field;
	size_t size;
} schema_member_desc_t;

typedef struct schema_member_s {
	uint field;
	size_t off;
	size_t size;
} schema_member_t;

typedef struct schema_layout_s {
	uint members;
	uint members_cnt;
	size_t size;
} schema_layout_t;

typedef struct schema_s {
	arr_t fields;
	arr_t layouts;
	arr_t members;
	buf_t vals;
	arr_t field_maps;
	strvbuf_t strs;
} schema_t;

schema_t *schema_init(schema_t *schema, uint fields_cap, uint layouts_cap, uint strs_cap, alloc_t alloc);
void schema_free(schema_t *schema);

int schema_add_fields(schema_t *schema, schema_field_desc_t *fields, size_t size);
const schema_field_t *schema_get_field(const schema_t *schema, uint def);

int schema_add_layout(schema_t *schema, schema_member_desc_t *members, size_t size, uint *layout);
const schema_layout_t *schema_get_layout(const schema_t *schema, uint layout);

const schema_member_t *schema_get_member(const schema_t *schema, uint layout, uint member);

int schema_set_val(const schema_t *schema, uint layout, uint member, void *data, void *val);
const void *schema_get_val(const schema_t *schema, uint member, const void *data);

strv_t schema_get_str(const schema_t *schema, size_t str);

size_t schema_print_val(const schema_t *schema, uint layout, uint field, const void *val, dst_t dst);
size_t schema_print_data(const schema_t *schema, uint layout, const void *data, dst_t dst);

#endif
