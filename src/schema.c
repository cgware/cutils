#include "schema.h"

#include "arr.h"
#include "dst.h"
#include "log.h"
#include "mem.h"
#include "strvbuf.h"

schema_t *schema_init(schema_t *schema, uint defs_cap, uint layouts_cap, uint strs_cap, alloc_t alloc)
{
	if (schema == NULL) {
		return NULL;
	}

	if (arr_init(&schema->defs, defs_cap, sizeof(field_def_t), alloc) == NULL ||
	    arr_init(&schema->layouts, layouts_cap, sizeof(layout_t), alloc) == NULL ||
	    arr_init(&schema->maps, layouts_cap, sizeof(layout_map_t), alloc) == NULL ||
	    strvbuf_init(&schema->strs, strs_cap, 16, alloc) == NULL) {
		return NULL;
	}

	return schema;
}

void schema_free(schema_t *schema)
{
	if (schema == NULL) {
		return;
	}

	layout_t *layout;
	uint i = 0;
	arr_foreach(&schema->layouts, i, layout)
	{
		arr_free(&layout->fields);
	}

	field_def_t *def;
	i = 0;
	arr_foreach(&schema->defs, i, def)
	{
		switch (def->type) {
		case FIELD_TYPE_ENUM:
		case FIELD_TYPE_FLAG: arr_free(&def->vals); break;
		default: break;
		}
	}

	layout_map_t *map;
	i = 0;
	arr_foreach(&schema->maps, i, map)
	{
		arr_free(&map->field_maps);
	}

	arr_free(&schema->defs);
	arr_free(&schema->layouts);
	arr_free(&schema->maps);
	strvbuf_free(&schema->strs);
}

field_def_t *schema_add_def(schema_t *schema, field_type_t type, strv_t name, size_t size, uint vals_cap, uint *def)
{
	if (schema == NULL) {
		return NULL;
	}

	uint defs_cnt;
	field_def_t *d = arr_add(&schema->defs, &defs_cnt);
	if (d == NULL) {
		return NULL;
	}

	size_t strs_cnt;
	if (strvbuf_add(&schema->strs, name, &strs_cnt)) {
		arr_reset(&schema->defs, defs_cnt);
		return NULL;
	}

	int vals;
	switch (type) {
	case FIELD_TYPE_STR:
		d->size = sizeof(size_t);
		d->len	= name.len;
		vals	= 0;
		break;
	case FIELD_TYPE_ENUM:
		d->size = size;
		d->len	= name.len;
		vals	= 1;
		break;
	case FIELD_TYPE_FLAG:
		d->size = sizeof(u8);
		d->len	= name.len;
		vals	= 1;
		break;
	default:
		d->size = size;
		d->len	= 2 + size * 2;
		if (name.len > d->len) {
			d->len = name.len;
		}
		vals = 0;
		break;
	}

	if (vals && arr_init(&d->vals, vals_cap, d->size + sizeof(size_t), schema->layouts.alloc) == NULL) {
		arr_reset(&schema->defs, defs_cnt);
		strvbuf_reset(&schema->strs, strs_cnt);
		return NULL;
	}

	d->type = type;
	d->name = strs_cnt;

	if (def) {
		*def = defs_cnt;
	}

	return d;
}

const field_def_t *schema_get_def(const schema_t *schema, uint def)
{
	if (schema == NULL) {
		return NULL;
	}

	const field_def_t *d = arr_get(&schema->defs, def);
	if (d == NULL) {
		log_error("cutils", "schema", NULL, "failed to get definition: %d", def);
		return NULL;
	}

	return d;
}

int schema_add_layout(schema_t *schema, size_t fields_cap, uint *layout)
{
	if (schema == NULL) {
		return 1;
	}

	uint layouts_cnt;
	layout_t *l = arr_add(&schema->layouts, &layouts_cnt);
	if (l == NULL) {
		return 1;
	}

	if (arr_init(&l->fields, fields_cap, sizeof(field_t), schema->layouts.alloc) == NULL) {
		arr_reset(&schema->layouts, layouts_cnt);
		return 1;
	}

	l->size = 0;

	if (layout) {
		*layout = layouts_cnt;
	}

	return 0;
}

const layout_t *schema_get_layout(const schema_t *schema, uint layout)
{
	if (schema == NULL) {
		return NULL;
	}

	const layout_t *l = arr_get(&schema->layouts, layout);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "failed to get layout: %d", layout);
		return NULL;
	}

	return l;
}

int schema_map_layout(schema_t *schema, uint layout)
{
	if (schema == NULL) {
		return 1;
	}

	const layout_t *fl = schema_get_layout(schema, layout);
	if (fl == NULL) {
		return 1;
	}

	const layout_t *tl = schema_get_layout(schema, 0);
	if (tl == NULL) {
		return 1;
	}

	layout_map_t *map = arr_add(&schema->maps, NULL);
	if (map == NULL) {
		return 1;
	}

	if (arr_init(&map->field_maps, fl->fields.cnt, sizeof(field_map_t), schema->layouts.alloc) == NULL) {
		return 1;
	}

	int ret = 0;
	const field_t *ff;
	uint i = 0;
	arr_foreach(&fl->fields, i, ff)
	{
		field_map_t *fm = arr_add(&map->field_maps, NULL);
		if (fm == NULL) {
			ret = 1;
			continue;
		}

		int found = 0;
		const field_t *tf;
		uint j = 0;
		arr_foreach(&tl->fields, j, tf)
		{
			if (ff->def == tf->def) {
				found = 1;
				break;
			}
		}

		if (found) {
			fm->drop = 0;
			fm->id	 = j;
		} else {
			fm->drop = 1;
		}
	}

	if (ret) {
		return 1;
	}

	return 0;
}

int schema_add_field(schema_t *schema, uint layout, uint def, size_t size, uint *id)
{
	if (schema == NULL) {
		return 1;
	}

	layout_t *l = (layout_t *)schema_get_layout(schema, layout);
	if (l == NULL) {
		return 1;
	}

	const field_def_t *d = schema_get_def(schema, def);
	if (d == NULL) {
		return 1;
	}

	uint fields_cnt;
	field_t *field = arr_add(&l->fields, &fields_cnt);
	if (field == NULL) {
		return 1;
	}

	field->def = def;
	field->off = l->size;

	switch (d->type) {
	case FIELD_TYPE_STR: field->size = size == 0 ? sizeof(size_t) : size; break;
	default: field->size = size; break;
	}

	l->size += field->size;

	if (id) {
		*id = fields_cnt;
	}

	return 0;
}

const field_t *schema_get_field(const schema_t *schema, uint layout, uint field)
{
	if (schema == NULL) {
		return NULL;
	}

	const layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		return NULL;
	}

	const field_t *f = arr_get(&l->fields, field);
	if (f == NULL) {
		log_error("cutils", "layout", NULL, "failed to get field");
		return NULL;
	}

	return f;
}

void *schema_add_val(schema_t *schema, uint def, strv_t str)
{
	if (schema == NULL) {
		return NULL;
	}

	field_def_t *d = (field_def_t *)schema_get_def(schema, def);
	if (d == NULL) {
		return NULL;
	}

	uint vals_cnt;
	u8 *val = arr_add(&d->vals, &vals_cnt);
	if (val == NULL) {
		return NULL;
	}

	if (strvbuf_add(&schema->strs, str, (size_t *)&val[d->size])) {
		arr_reset(&d->vals, vals_cnt);
		return NULL;
	}

	if (str.len > d->len) {
		d->len = str.len;
	}

	return val;
}

strv_t schema_get_str(const schema_t *schema, size_t str)
{
	if (schema == NULL) {
		return STRV_NULL;
	}

	return strvbuf_get(&schema->strs, str);
}

int schema_set_val(const schema_t *schema, uint layout, uint field, void *data, void *val)
{
	if (schema == NULL) {
		return 1;
	}

	const layout_map_t *lm = arr_get(&schema->maps, layout);
	const field_map_t *fm  = arr_get(&lm->field_maps, field);

	if (fm->drop) {
		return 0;
	}

	const field_t *ff = schema_get_field(schema, layout, field);
	const field_t *tf = schema_get_field(schema, 0, fm->id);

	void *to_val = (void *)schema_get_val(schema, fm->id, data);

	mem_set(to_val, 0, tf->size);
	mem_copy(to_val, tf->size, val, ff->size);

	return 0;
}

const void *schema_get_val(const schema_t *schema, uint field, const void *data)
{
	if (schema == NULL) {
		return NULL;
	}

	const layout_t *l = schema_get_layout(schema, 0);
	if (l == NULL) {
		return NULL;
	}

	const field_t *f = schema_get_field(schema, 0, field);
	if (f == NULL) {
		return NULL;
	}

	const byte *bytes = data;
	return &bytes[f->off];
}

static int bit_is_set(const byte *data, u8 bit)
{
	return data[bit >> 3] & (1u << (bit & 7));
}

size_t schema_print_val(const schema_t *schema, uint layout, uint field, const void *val, dst_t dst)
{
	if (schema == NULL) {
		return 0;
	}

	const layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		return 0;
	}

	const field_t *f = schema_get_field(schema, layout, field);
	if (f == NULL) {
		return 0;
	}

	const field_def_t *d = schema_get_def(schema, f->def);
	if (d == NULL) {
		return 0;
	}

	size_t off = dst.off;

#define SUB(a, b) ((a) > (b) ? (a) - (b) : 0)

	strv_t str = STRV_NULL;

	switch (d->type) {
	case FIELD_TYPE_ENUM: {
		uint i = 0;
		const u8 *v;
		arr_foreach(&d->vals, i, v)
		{
			if (mem_cmp(val, v, d->size) != 0) {
				continue;
			}

			str = schema_get_str(schema, *(size_t *)(&v[d->size]));
			break;
		}

		break;
	}
	case FIELD_TYPE_FLAG: {
		uint i = 0;
		const u8 *v;
		arr_foreach(&d->vals, i, v)
		{
			if (!bit_is_set(val, *v)) {
				continue;
			}

			str = schema_get_str(schema, *(size_t *)(&v[sizeof(u8)]));
			dst.off += dputf(dst, "%.*s", str.len, str.data);
		}

		str = STRV("");

		break;
	}
	default: break;
	}

	if (str.data) {
		dst.off += strv_print(str, dst);
	} else {
		const byte *bytes = val;
		dst.off += dputf(dst, "0x");
		for (uint i = 0; i < f->size; i++) {
			dst.off += dputf(dst, "%02X", bytes[f->size - i - 1]);
		}
	}

	return dst.off - off;
}

size_t schema_print_data(const schema_t *schema, uint layout, const void *data, dst_t dst)
{
	if (schema == NULL) {
		return 0;
	}

	const layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		return 0;
	}

	size_t off = dst.off;

	const u8 *bytes = data;

	const field_t *f;
	uint i = 0;
	field_foreach(l, i, f)
	{
		const field_def_t *d = schema_get_def(schema, f->def);
		strv_t name	     = schema_get_str(schema, d->name);
		dst.off += dputf(dst, "%.*s: ", name.len, name.data);
		dst.off += schema_print_val(schema, layout, i, &bytes[f->off], dst);
		dst.off += dputf(dst, "\n");
	}

	return dst.off - off;
}

void layout_map_free(layout_map_t *map)
{
	if (map == NULL) {
		return;
	}

	arr_free(&map->field_maps);
}
