#include "strbuf.h"

#include <memory.h>

strbuf_t *strbuf_init(strbuf_t *buf, size_t size, alloc_t alloc)
{
	if (buf == NULL) {
		return NULL;
	}

	if (buf_init(&buf->buf, size, alloc) == NULL) {
		return NULL;
	}

	buf->cnt = 0;

	return buf;
}

void strbuf_free(strbuf_t *buf)
{
	if (buf == NULL) {
		return;
	}

	buf_free(&buf->buf);
	buf->cnt = 0;
}

int strbuf_add(strbuf_t *buf, const char *str, uint8_t len, uint *index)
{
	if (buf == NULL) {
		return 1;
	}

	if (buf_add(&buf->buf, &len, sizeof(len), NULL) || buf_add(&buf->buf, str, sizeof(char) * len, NULL)) {
		return 1;
	}

	if (index) {
		*index = buf->cnt;
	}

	buf->cnt++;
	return 0;
}

int strbuf_get_index(const strbuf_t *buf, const char *str, uint8_t len, uint *index)
{
	if (buf == 0) {
		return 1;
	}

	size_t i = 0;
	size_t start;
	size_t blen;
	uint id = 0;

	strbuf_foreach(buf, i, start, blen)
	{
		if (len == blen && memcmp(&buf->buf.data[start], str, len) == 0) {
			if (index) {
				*index = id;
			}
			return 0;
		}

		id++;
	}

	return 1;
}
