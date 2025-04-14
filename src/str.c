#include "str.h"

#include "cstr.h"
#include "log.h"
#include "mem.h"

str_t str_null()
{
	return (str_t){0};
}

str_t strz(size_t size)
{
	char *data = mem_alloc(size);
	if (data != NULL && size > 0) {
		data[0] = '\0';
	}

	return (str_t){
		.data = data,
		.size = size,
		.len  = 0,
		.ref  = 0,
	};
}

str_t strc(const char *cstr, size_t len)
{
	return (str_t){
		.data = cstr,
		.size = 0,
		.len  = len,
		.ref  = 1,
	};
}

str_t strs(str_t str)
{
	return strc(str.data, str.len);
}

str_t strn(const char *cstr, size_t len, size_t size)
{
	if (cstr == NULL) {
		return (str_t){0};
	}

	if (len + 1 > size) {
		return (str_t){0};
	}

	char *data = mem_alloc(size);
	if (data != NULL) {
		mem_copy(data, size, cstr, len);
		data[len] = '\0';
	}

	return (str_t){
		.data = data,
		.size = size,
		.len  = len,
		.ref  = 0,
	};
}

str_t strv(const char *fmt, va_list args)
{
	str_t str   = {0};
	int size = cstrv(NULL, 0, fmt, args);
	if (size < 0) {
		return (str_t){0};
	}

	str.size = size + 1;
	str.data = mem_alloc(str.size);
	if (str.data == NULL) {
		return (str_t){0};
	}

	str.len = cstrv((char *)str.data, str.size, fmt, args);
	str.ref = 0;
	return str;
}

str_t strf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	str_t ret = strv(fmt, args);
	va_end(args);
	return ret;
}

str_t strb(const char *buf, size_t size, size_t len)
{
	return (str_t){
		.data = buf,
		.size = size,
		.len  = len,
		.ref  = 1,
	};
}

str_t strr()
{
	return strc(NULL, 0);
}

void str_free(str_t *str)
{
	if (str == NULL) {
		return;
	}

	if (!str->ref) {
		mem_free((char *)str->data, str->size);
	}

	str->data = NULL;
	str->size = 0;
	str->len  = 0;
}

void str_zero(str_t *str)
{
	if (str == NULL) {
		return;
	}

	if (!str->ref) {
		cstr_zero((char *)str->data, str->size);
	}

	str->len = 0;
}

int str_resize(str_t *str, size_t size)
{
	if (str == NULL || str->ref) {
		return 1;
	}

	if (size <= str->size) {
		return 0;
	}

	const char *data = mem_realloc((char *)str->data, size, str->size);
	if (data == NULL) {
		return 1;
	}

	str->data = data;
	str->size = size;
	return 0;
}

str_t *str_cat(str_t *str, strv_t src)
{
	if (str == NULL || src.data == NULL) {
		return NULL;
	}

	if (str->ref && str->size == 0) {
		return NULL;
	}

	if (!str->ref && str_resize(str, str->len + src.len + 1)) {
		return NULL;
	}

	str->len = cstr_cat((char *)str->data, str->size, str->len, src.data, src.len);
	return str;
}

int str_cmpn(str_t str, strv_t src, size_t len)
{
	return cstr_cmpn(str.data, str.len, src.data, src.len, len);
}

int str_cmp(str_t str, strv_t src)
{
	return cstr_cmp(str.data, str.len, src.data, src.len);
}

int str_chr(str_t str, str_t *l, str_t *r, char c)
{
	const char *res = cstr_chr(str.data, c);

	if (!res || res >= str.data + str.len) {
		return 1;
	}

	const char *str_data = str.data;
	size_t str_len	     = str.len;

	if (l != NULL) {
		*l = (str_t){
			.data = str_data,
			.size = 0,
			.len  = (size_t)(res - str_data),
			.ref  = 1,
		};
	}

	if (r != NULL) {
		*r = (str_t){
			.data = res + 1,
			.size = 0,
			.len  = (size_t)(str_data + str_len - (res + 1)),
			.ref  = 1,
		};
	}

	return 0;
}

int str_cstr(str_t str, str_t *l, str_t *r, const char *s, size_t s_len)
{
	const char *res = cstr_cstr(str.data, s);

	if (!res || res >= str.data + str.len) {
		return 1;
	}

	const char *str_data = str.data;
	size_t str_len	     = str.len;

	if (l != NULL) {
		*l = (str_t){
			.data = str_data,
			.size = 0,
			.len  = (size_t)(res - str_data),
			.ref  = 1,
		};
	}

	if (r != NULL) {
		*r = (str_t){
			.data = res + s_len,
			.size = 0,
			.len  = (size_t)(str_data + str_len - (res + s_len)),
			.ref  = 1,
		};
	}

	return 0;
}

str_t str_cpy(str_t src)
{
	str_t copy = strz(src.len + 1);
	str_cpyd(src, &copy);
	return copy;
}

int str_cpyd(str_t src, str_t *dst)
{
	if (dst == NULL || dst->size < src.len + 1) {
		return 1;
	}

	void *data = cstr_cpy((char *)dst->data, dst->size, src.data, src.len);

	if (data == NULL) {
		return 1;
	}

	dst->data = data;
	dst->len  = src.len;

	((char *)dst->data)[dst->len] = '\0';

	return 0;
}

int str_to_upper(str_t str, str_t *dst)
{
	if (dst == NULL || dst->size < str.len + 1) {
		return 1;
	}

	int d = 'a' - 'A';

	char *data = (char *)dst->data;
	for (size_t i = 0; i < str.len; i++) {
		data[i] = str.data[i] - (str.data[i] >= 'a' && str.data[i] <= 'z') * d;
	}
	data[str.len] = '\0';
	dst->len      = str.len;
	return 0;
}

static int append(str_t *str, const char *data, size_t len)
{
	if (str->ref && str->size == 0) {
		if (str->data) {
			return 1;
		}

		str->data = data;
		str->len  = len;
		return 0;
	}

	if (str_cat(str, STRVN(data, len)) == NULL) {
		return 1;
	}

	return 0;
}

int str_split(str_t str, char c, str_t *l, str_t *r)
{
	const char *res = cstr_chr(str.data, c);

	if (!res || res >= str.data + str.len) {
		return 1;
	}

	if (l && append(l, str.data, res - str.data)) {
		return 1;
	}

	if (r && append(r, res + 1, str.len - (res + 1 - str.data))) {
		return 1;
	}

	return 0;
}

int str_rsplit(str_t str, char c, str_t *l, str_t *r)
{
	const char *res = cstr_rchr(str.data, c);

	if (!res || res >= str.data + str.len) {
		return 1;
	}

	if (l && append(l, str.data, res - str.data)) {
		return 1;
	}

	if (r && append(r, res + 1, str.len - (res + 1 - str.data))) {
		return 1;
	}

	return 0;
}

int str_replace(str_t *str, str_t from, str_t to)
{
	if (str == NULL) {
		return 0;
	}

	int found = 0;
	str->len  = cstr_replace((char *)str->data, str->size, str->len, from.data, from.len, to.data, to.len, &found);
	return found;
}

int str_replaces(str_t *str, const str_t *from, const str_t *to, size_t cnt)
{
	if (from == NULL || to == NULL) {
		return 0;
	}

	int found = 0;
	for (size_t i = 0; i < cnt; i++) {
		found |= str_replace(str, from[i], to[i]);
	}

	return found;
}

int str_rreplaces(str_t *str, const str_t *from, const str_t *to, size_t cnt)
{
	int ret	  = 0;
	int found = 0;
	do {
		ret |= found = str_replaces(str, from, to, cnt);
	} while (found);
	return ret;
}

int str_subreplace(str_t *dst, size_t start, size_t end, strv_t str)
{
	if (dst == NULL || dst->data == NULL) {
		return 1;
	}

	if (dst->len + str.len - (end - start) > dst->size) {
		log_error("cutils", "str", NULL, "destination too small: %d/%d", dst->size, dst->len + str.len - (end - start));
		return 1;
	}

	char *data = (char *)dst->data;

	mem_replace(&data[start], dst->size - start, dst->len - start, str.data, end - start, str.len);

	dst->len += str.len - (end - start);

	return 0;
}
