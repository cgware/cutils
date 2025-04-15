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

int strv_cmp(strv_t l, strv_t r)
{
	if (l.data == NULL && r.data == NULL) {
		return 0;
	} else if (l.data == NULL) {
		return -1;
	} else if (r.data == NULL) {
		return 1;
	}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
	int ret = mem_cmp(l.data, r.data, MIN(l.len, r.len));

	if (ret != 0 || l.len == r.len) {
		return ret;
	}

	return l.len < r.len ? -1 : 1;
}

int strv_print(strv_t str, print_dst_t dst)
{
	if (str.data == NULL) {
		return 0;
	}

	int off = dst.off;

	for (size_t i = 0; i < str.len; i++) {
		const char c = str.data[i];
		switch (c) {
		case '\t': dst.off += c_dprintf(dst, "\\t"); break;
		case '\n': dst.off += c_dprintf(dst, "\\n"); break;
		case '\r': dst.off += c_dprintf(dst, "\\r"); break;
		case '\0': dst.off += c_dprintf(dst, "\\0"); break;
		default: dst.off += c_dprintf(dst, "%c", c); break;
		}
	}

	return dst.off - off;
}
