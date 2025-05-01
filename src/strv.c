#include "strv.h"

#include "mem.h"

strv_t strv_cstr(const char *cstr)
{
	if (cstr == NULL) {
		return STRV_NULL;
	}

	strv_t strv = {
		.data = cstr,
		.len  = 0,
	};

	while (*cstr++) {
		strv.len++;
	}

	return strv;
}

int strv_eq(strv_t l, strv_t r)
{
	if (!l.data != !r.data || l.len != r.len) {
		return 0;
	}

	if (l.data == NULL && l.len == 0) {
		return 1;
	}

	return mem_cmp(l.data, r.data, l.len) == 0;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int strv_cmp(strv_t l, strv_t r)
{
	if (l.data == NULL && r.data == NULL) {
		return 0;
	} else if (l.data == NULL) {
		return -1;
	} else if (r.data == NULL) {
		return 1;
	}

	int ret = mem_cmp(l.data, r.data, MIN(l.len, r.len));

	if (ret != 0 || l.len == r.len) {
		return ret;
	}

	return l.len < r.len ? -1 : 1;
}

int strv_cmpn(strv_t l, strv_t r, size_t len)
{
	if (l.data == NULL && r.data == NULL) {
		return 0;
	} else if (l.data == NULL) {
		return -1;
	} else if (r.data == NULL) {
		return 1;
	}

	int ret = mem_cmp(l.data, r.data, MIN(MIN(l.len, r.len), len));

	if (ret != 0 || (l.len >= len && r.len >= len)) {
		return ret;
	}

	return l.len < r.len ? -1 : 1;
}

int strv_to_int(strv_t str, int *res)
{
	if (str.data == NULL || str.len < 1) {
		return 1;
	}

	int ret	 = 1;
	int sign = 1;
	size_t i = 0;
	int acc	 = 0;

	if (str.data[0] == '-') {
		sign = -1;
		i++;
	}

	for (; i < str.len; i++) {
		if (str.data[i] < '0' || '9' < str.data[i]) {
			return 1;
		}

		acc = acc * 10 + (str.data[i] - '0');
		ret = 0;
	}

	if (res) {
		*res = sign * acc;
	}

	return ret;
}

int strv_split(strv_t str, char c, strv_t *l, strv_t *r)
{
	if (str.data == NULL) {
		return 1;
	}

	for (size_t i = 0; i < str.len; i++) {
		if (str.data[i] != c) {
			continue;
		}

		if (l) {
			*l = STRVN(str.data, i);
		}

		if (r) {
			*r = STRVN(&str.data[i + 1], str.len - i - 1);
		}

		return 0;
	}

	return 1;
}

int strv_rsplit(strv_t str, char c, strv_t *l, strv_t *r)
{
	if (str.data == NULL) {
		return 1;
	}

	for (size_t i = 1; i <= str.len; i++) {
		if (str.data[str.len - i] != c) {
			continue;
		}

		if (l) {
			*l = STRVN(str.data, str.len - i);
		}

		if (r) {
			*r = STRVN(&str.data[str.len - i + 1], i - 1);
		}

		return 0;
	}

	return 1;
}

size_t strv_print(strv_t str, dst_t dst)
{
	if (str.data == NULL) {
		return 0;
	}

	size_t off = dst.off;

	for (size_t i = 0; i < str.len; i++) {
		const char c = str.data[i];
		switch (c) {
		case '\t': dst.off += dputs(dst, STRV("\\t")); break;
		case '\n': dst.off += dputs(dst, STRV("\\n")); break;
		case '\r': dst.off += dputs(dst, STRV("\\r")); break;
		case '\0': dst.off += dputs(dst, STRV("\\0")); break;
		default: dst.off += dputf(dst, "%c", c); break;
		}
	}

	return dst.off - off;
}
