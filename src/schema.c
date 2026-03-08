#include "schema.h"

#include "log.h"
#include "mem.h"

typedef struct field_map_s {
	byte drop;
	uint id;
} field_map_t;

schema_t *schema_init(schema_t *schema, uint fields_cap, uint layouts_cap, uint strs_cap, alloc_t alloc)
{
	if (schema == NULL) {
		return NULL;
	}

	if (arr_init(&schema->fields, fields_cap, sizeof(schema_field_t), alloc) == NULL ||
	    arr_init(&schema->layouts, layouts_cap, sizeof(schema_layout_t), alloc) == NULL ||
	    arr_init(&schema->members, layouts_cap * 8, sizeof(schema_member_t), alloc) == NULL ||
	    buf_init(&schema->vals, layouts_cap * 8 * (sizeof(u64) + sizeof(size_t)), alloc) == NULL ||
	    arr_init(&schema->field_maps, layouts_cap, sizeof(field_map_t), alloc) == NULL ||
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

	arr_free(&schema->fields);
	arr_free(&schema->layouts);
	arr_free(&schema->members);
	buf_free(&schema->vals);
	arr_free(&schema->field_maps);
	strvbuf_free(&schema->strs);
}

int schema_add_fields(schema_t *schema, schema_field_desc_t *fields, size_t size)
{
	if (schema == NULL) {
		return 1;
	}

	int ret = 0;

	for (uint i = 0; i < size / sizeof(schema_field_desc_t); i++) {
		uint fields_cnt;
		schema_field_t *f = arr_add(&schema->fields, &fields_cnt);
		if (f == NULL) {
			ret = 1;
			continue;
		}

		size_t strs_cnt;
		if (strvbuf_add(&schema->strs, fields[i].name, &strs_cnt)) {
			arr_reset(&schema->fields, fields_cnt);
			ret = 1;
			continue;
		}

		switch (fields[i].type) {
		case SCHEMA_TYPE_STR:
			f->size = sizeof(size_t);
			f->len	= fields[i].name.len;
			break;
		case SCHEMA_TYPE_ENUM:
			f->size = fields[i].size;
			f->len	= fields[i].name.len;
			break;
		case SCHEMA_TYPE_FLAG:
			f->size = sizeof(u8);
			f->len	= fields[i].name.len;
			break;
		default:
			f->size = fields[i].size;
			f->len	= 2 + fields[i].size * 2;
			if (fields[i].name.len > f->len) {
				f->len = fields[i].name.len;
			}
			break;
		}

		f->vals	    = schema->vals.used;
		f->vals_cnt = 0;
		f->type	    = fields[i].type;
		f->name	    = strs_cnt;

		switch (fields[i].type) {
		case SCHEMA_TYPE_ENUM:
		case SCHEMA_TYPE_FLAG:
			for (uint j = 0; j < fields[i].vals_size / sizeof(schema_val_t); j++) {
				size_t vals_cnt;
				u8 tmp = 0;
				if (buf_add(&schema->vals, &tmp, f->size + sizeof(size_t), &vals_cnt)) {
					ret = 1;
					continue;
				}
				u8 *val = buf_get(&schema->vals, vals_cnt);

				if (strvbuf_add(&schema->strs, fields[i].vals[j].str, (size_t *)&val[f->size])) {
					buf_reset(&schema->vals, vals_cnt);
					ret = 1;
					continue;
				}

				if (fields[i].vals[j].str.len > f->len) {
					f->len = fields[i].vals[j].str.len;
				}

				f->vals_cnt++;
				mem_copy(val, f->size, &fields[i].vals[j].val, f->size);
			}
		default: break;
		}
	}

	return ret;
}

const schema_field_t *schema_get_field(const schema_t *schema, uint field)
{
	if (schema == NULL) {
		return NULL;
	}

	const schema_field_t *f = arr_get(&schema->fields, field);
	if (f == NULL) {
		log_error("cutils", "schema", NULL, "failed to get definition: %d", field);
		return NULL;
	}

	return f;
}

int schema_add_layout(schema_t *schema, schema_member_desc_t *members, size_t size, uint *layout)
{
	if (schema == NULL) {
		return 1;
	}

	uint layouts_cnt;
	schema_layout_t *l = arr_add(&schema->layouts, &layouts_cnt);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "failed to add layout");
		return 1;
	}

	l->members     = schema->members.cnt;
	l->members_cnt = 0;
	l->size	       = 0;

	const schema_layout_t *tl = schema_get_layout(schema, 0);

	int ret = 0;

	for (uint i = 0; i < size / sizeof(schema_member_desc_t); i++) {
		const schema_field_t *f = schema_get_field(schema, members[i].field);
		if (f == NULL) {
			log_error("cutils", "schema", NULL, "invalid field: %d", members[i].field);
			ret = 1;
			continue;
		}

		schema_member_t *member = arr_add(&schema->members, NULL);
		if (member == NULL) {
			log_error("cutils", "schema", NULL, "failed to add field");
			ret = 1;
			continue;
		}

		field_map_t *fm = arr_add(&schema->field_maps, NULL);
		if (fm == NULL) {
			log_error("cutils", "schema", NULL, "failed to add field map");
			ret = 1;
			continue;
		}

		member->field = members[i].field;
		member->off   = l->size;

		switch (f->type) {
		case SCHEMA_TYPE_STR: member->size = members[i].size == 0 ? sizeof(size_t) : members[i].size; break;
		default: member->size = members[i].size; break;
		}

		l->members_cnt++;
		l->size += member->size;

		int found = 0;
		uint id;
		for (uint j = tl->members; j < tl->members + tl->members_cnt; j++) {
			const schema_member_t *tm = arr_get(&schema->members, j);
			if (member->field == tm->field) {
				found = 1;
				id    = j;
				break;
			}
		}

		if (found) {
			fm->drop = 0;
			fm->id	 = id;
		} else {
			fm->drop = 1;
		}
	}

	if (layout) {
		*layout = layouts_cnt;
	}

	return ret;
}

const schema_layout_t *schema_get_layout(const schema_t *schema, uint layout)
{
	if (schema == NULL) {
		return NULL;
	}

	const schema_layout_t *l = arr_get(&schema->layouts, layout);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "failed to get layout: %d", layout);
		return NULL;
	}

	return l;
}

const schema_member_t *schema_get_member(const schema_t *schema, uint layout, uint member)
{
	if (schema == NULL) {
		return NULL;
	}

	const schema_layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		return NULL;
	}

	const schema_member_t *m = arr_get(&schema->members, l->members + member);
	if (m == NULL) {
		log_error("cutils", "layout", NULL, "failed to get field");
		return NULL;
	}

	return m;
}

strv_t schema_get_str(const schema_t *schema, size_t str)
{
	if (schema == NULL) {
		return STRV_NULL;
	}

	return strvbuf_get(&schema->strs, str);
}

int schema_set_val(const schema_t *schema, uint layout, uint member, void *data, void *val)
{
	if (schema == NULL) {
		return 1;
	}

	const schema_layout_t *l = arr_get(&schema->layouts, layout);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "invalid layout: %d", layout);
		return 1;
	}

	const field_map_t *map = arr_get(&schema->field_maps, l->members + member);
	if (map == NULL) {
		log_error("cutils", "schema", NULL, "invalid member: %d", member);
		return 1;
	}

	if (map->drop) {
		return 0;
	}

	const schema_member_t *fm = schema_get_member(schema, layout, member);
	const schema_member_t *tm = schema_get_member(schema, 0, map->id);

	void *to_val = (void *)schema_get_val(schema, map->id, data);

	if (to_val == NULL) {
		return 1;
	}

	mem_set(to_val, 0, tm->size);
	mem_copy(to_val, tm->size, val, fm->size);

	return 0;
}

const void *schema_get_val(const schema_t *schema, uint member, const void *data)
{
	if (schema == NULL) {
		return NULL;
	}

	const schema_layout_t *l = schema_get_layout(schema, 0);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "invalid layout: %d", 0);
		return NULL;
	}

	const schema_member_t *m = schema_get_member(schema, 0, l->members + member);
	if (m == NULL) {
		log_error("cutils", "schema", NULL, "invalid member: %d", member);
		return NULL;
	}

	if (data == NULL) {
		return NULL;
	}

	const byte *bytes = data;
	return &bytes[m->off];
}

static int bit_is_set(const byte *data, u8 bit)
{
	return data[bit >> 3] & (1u << (bit & 7));
}

size_t schema_print_val(const schema_t *schema, uint layout, uint member, const void *val, dst_t dst)
{
	if (schema == NULL) {
		return 0;
	}

	const schema_layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		log_error("cutils", "schema", NULL, "invalid layout: %d", layout);
		return 0;
	}

	const schema_member_t *m = schema_get_member(schema, layout, member);
	if (m == NULL) {
		log_error("cutils", "schema", NULL, "invalid field: %d", member);
		return 0;
	}

	const schema_field_t *f = schema_get_field(schema, m->field);
	if (f == NULL) {
		log_error("cutils", "schema", NULL, "invalid field: %d", m->field);
		return 0;
	}

	if (val == NULL) {
		return 0;
	}

	size_t off = dst.off;

#define SUB(a, b) ((a) > (b) ? (a) - (b) : 0)

	strv_t str = STRV_NULL;

	switch (f->type) {
	case SCHEMA_TYPE_ENUM: {
		for (uint i = 0; i < f->vals_cnt; i++) {
			const u8 *v = buf_get(&schema->vals, f->vals + (f->size + sizeof(size_t)) * i);
			if (mem_cmp(val, v, f->size) != 0) {
				continue;
			}

			str = schema_get_str(schema, *(size_t *)(&v[f->size]));
			break;
		}

		break;
	}
	case SCHEMA_TYPE_FLAG: {
		for (uint i = 0; i < f->vals_cnt; i++) {
			const u8 *v = buf_get(&schema->vals, f->vals + (f->size + sizeof(size_t)) * i);
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
		for (uint i = 0; i < m->size; i++) {
			dst.off += dputf(dst, "%02X", bytes[m->size - i - 1]);
		}
	}

	return dst.off - off;
}

size_t schema_print_data(const schema_t *schema, uint layout, const void *data, dst_t dst)
{
	if (schema == NULL) {
		return 0;
	}

	const schema_layout_t *l = schema_get_layout(schema, layout);
	if (l == NULL) {
		return 0;
	}

	size_t off = dst.off;

	const u8 *bytes = data;

	for (uint i = l->members; i < l->members + l->members_cnt; i++) {
		const schema_member_t *m = arr_get(&schema->members, i);
		const schema_field_t *f	 = schema_get_field(schema, m->field);
		strv_t name		 = schema_get_str(schema, f->name);
		dst.off += dputf(dst, "%.*s: ", name.len, name.data);
		dst.off += schema_print_val(schema, layout, i, &bytes[m->off], dst);
		dst.off += dputf(dst, "\n");
	}

	return dst.off - off;
}
