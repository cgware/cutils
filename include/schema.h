#ifndef SCHEMA_H
#define SCHEMA_H

#include "arr.h"
#include "strvbuf.h"

typedef enum field_type_e {
	FIELD_TYPE_UNKNOWN,
	FIELD_TYPE_INT,
	FIELD_TYPE_STR,
	FIELD_TYPE_ENUM,
	FIELD_TYPE_FLAG,
} field_type_t;

typedef struct field_def_s {
	field_type_t type;
	size_t name;
	size_t size;
	size_t len;
	arr_t vals;
} field_def_t;

typedef struct field_s {
	uint def;
	size_t off;
	size_t size;
} field_t;

typedef struct layout_s {
	arr_t fields;
	size_t size;
} layout_t;

typedef struct field_map_s {
	byte drop;
	uint id;
} field_map_t;

typedef struct layout_map_s {
	arr_t field_maps;
} layout_map_t;

typedef struct schema_s {
	arr_t defs;
	arr_t layouts;
	arr_t maps;
	strvbuf_t strs;
} schema_t;

schema_t *schema_init(schema_t *schema, uint defs_cap, uint layouts_cap, uint strs_cap, alloc_t alloc);
void schema_free(schema_t *schema);

field_def_t *schema_add_def(schema_t *schema, field_type_t type, strv_t name, size_t size, uint vals_cap, uint *def);
const field_def_t *schema_get_def(const schema_t *schema, uint def);

int schema_add_layout(schema_t *schema, size_t fields_cap, uint *layout);
const layout_t *schema_get_layout(const schema_t *schema, uint layout);
int schema_map_layout(schema_t *schema, uint layout);

int schema_add_field(schema_t *schema, uint layout, uint def, size_t size, uint *field);
const field_t *schema_get_field(const schema_t *schema, uint layout, uint field);

void *schema_add_val(schema_t *schema, uint def, strv_t str);

int schema_set_val(const schema_t *schema, uint layout, uint field, void *data, void *val);
const void *schema_get_val(const schema_t *schema, uint field, const void *data);

strv_t schema_get_str(const schema_t *schema, size_t str);

size_t schema_print_val(const schema_t *schema, uint layout, uint field, const void *val, dst_t dst);
size_t schema_print_data(const schema_t *schema, uint layout, const void *data, dst_t dst);

#define field_foreach(_layout, _i, _field) arr_foreach(&(_layout)->fields, _i, _field)

#endif
