#include "strv.h"

#include "mem.h"

size_t strv_len(strv_t str)
{
	if (str.data == NULL) {
		return 0;
	}

	size_t len = 0;
	while (*str.data++) {
		len++;
	}
	return len;
}

int strv_eq(strv_t l, strv_t r)
{
	if (l.data == NULL || r.data == NULL || l.len != r.len) {
		return 0;
	}

	return mem_cmp(l.data, r.data, l.len) == 0;
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
