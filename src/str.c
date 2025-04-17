#include "str.h"

#include "cstr.h"
#include "log.h"
#include "mem.h"

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
	};
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
	};
}

str_t strv(const char *fmt, va_list args)
{
	str_t str = {0};
	int size  = cstrv(NULL, 0, fmt, args);
	if (size < 0) {
		return (str_t){0};
	}

	str.size = size + 1;
	str.data = mem_alloc(str.size);
	if (str.data == NULL) {
		return (str_t){0};
	}

	str.len = cstrv(str.data, str.size, fmt, args);
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

void str_free(str_t *str)
{
	if (str == NULL) {
		return;
	}

	mem_free(str->data, str->size);

	str->data = NULL;
	str->size = 0;
	str->len  = 0;
}

void str_zero(str_t *str)
{
	if (str == NULL) {
		return;
	}

	mem_set(str->data, 0, str->size);
	str->len = 0;
}

int str_resize(str_t *str, size_t size)
{
	if (str == NULL) {
		return 1;
	}

	if (size <= str->size) {
		return 0;
	}

	char *data = mem_realloc(str->data, size, str->size);
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

	if (str_resize(str, str->len + src.len + 1)) {
		return NULL;
	}

	str->len = cstr_cat(str->data, str->size, str->len, src.data, src.len);
	return str;
}

int str_to_upper(strv_t str, str_t *dst)
{
	if (dst == NULL || dst->size < str.len + 1) {
		return 1;
	}

	int d = 'a' - 'A';

	char *data = dst->data;
	for (size_t i = 0; i < str.len; i++) {
		data[i] = str.data[i] - (str.data[i] >= 'a' && str.data[i] <= 'z') * d;
	}
	data[str.len] = '\0';
	dst->len      = str.len;
	return 0;
}

int str_replace(str_t *str, strv_t from, strv_t to, int *found)
{
	if (str == NULL || from.data == NULL || str->len < from.len) {
		return 1;
	}

	if (found) {
		*found = 0;
	}

	for (size_t i = 0; str->len >= from.len && i <= str->len - from.len; i++) {
		if (strv_cmpn(STRVN(&str->data[i], str->len), from, from.len)) {
			continue;
		}

		if (to.data == NULL) {
			continue;
		}

		if (found) {
			*found = 1;
		}

		if (to.len < from.len) {
			for (size_t j = i + to.len, k = i + from.len; k <= str->len; j++, k++) {
				str->data[j] = str->data[k];
			}
		} else if (to.len > from.len) {
			if (str->len + to.len - from.len >= str->size) {
				return 1;
			}
			for (size_t j = str->len + to.len - from.len, k = str->len; k >= i + from.len; j--, k--) {
				str->data[j] = str->data[k];
			}
		}

		for (size_t j = 0; j < to.len; j++) {
			str->data[i + j] = to.data[j];
		}

		str->len += to.len - from.len;
		i = i + to.len - 1;
	}

	return 0;
}

int str_replaces(str_t *str, const strv_t *from, const strv_t *to, size_t cnt, int *found)
{
	if (from == NULL || to == NULL) {
		return 1;
	}

	for (size_t i = 0; i < cnt; i++) {
		if (str_replace(str, from[i], to[i], found)) {
			return 1;
		}
	}

	return 0;
}

int str_rreplaces(str_t *str, const strv_t *from, const strv_t *to, size_t cnt)
{
	int found = 0;

	do {
		if (str_replaces(str, from, to, cnt, &found)) {
			return 1;
		}
	} while (found);

	return 0;
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

	mem_replace(&dst->data[start], dst->size - start, dst->len - start, str.data, end - start, str.len);

	dst->len += str.len - (end - start);

	return 0;
}
