#include "cbuf.h"

int cbuf_set_le(void *buf, size_t off, const void *val, size_t size)
{
	u8 *dst	      = (u8 *)buf + off;
	const u8 *src = val;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
	}
#else
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[size - i - 1];
	}
#endif

	return 0;
}

int cbuf_set_u8le(void *buf, size_t off, u8 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> (i * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u16le(void *buf, size_t off, u16 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> (i * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u32le(void *buf, size_t off, u32 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> (i * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u64le(void *buf, size_t off, u64 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> (i * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_be(void *buf, size_t off, const void *val, size_t size)
{
	u8 *dst	      = (u8 *)buf + off;
	const u8 *src = val;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[size - i - 1];
	}
#else
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
	}
#endif

	return 0;
}

int cbuf_set_u8be(void *buf, size_t off, u8 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> ((sizeof(val) - i - 1) * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u16be(void *buf, size_t off, u16 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> ((sizeof(val) - i - 1) * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u32be(void *buf, size_t off, u32 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> ((sizeof(val) - i - 1) * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_set_u64be(void *buf, size_t off, u64 val)
{
	u8 *dst = (u8 *)buf + off;
	for (size_t i = 0; i < sizeof(val); i++) {
		dst[i] = (val >> ((sizeof(val) - i - 1) * 8)) & 0xFF;
	}
	return 0;
}

int cbuf_get_le(const void *buf, size_t off, void *val, size_t size)
{
	u8 *dst	      = val;
	const u8 *src = (u8 *)buf + off;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
	}
#else
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[size - i - 1];
	}
#endif

	return 0;
}

int cbuf_get_u8le(const void *buf, size_t off, u8 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u8)src[i] << (i * 8);
	}
	return 0;
}

int cbuf_get_u16le(const void *buf, size_t off, u16 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u16)src[i] << (i * 8);
	}
	return 0;
}

int cbuf_get_u32le(const void *buf, size_t off, u32 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u32)src[i] << (i * 8);
	}
	return 0;
}

int cbuf_get_u64le(const void *buf, size_t off, u64 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u64)src[i] << (i * 8);
	}
	return 0;
}

int cbuf_get_be(const void *buf, size_t off, void *val, size_t size)
{
	u8 *dst	      = val;
	const u8 *src = (u8 *)buf + off;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[size - i - 1];
	}
#else
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
	}
#endif

	return 0;
}

int cbuf_get_u8be(const void *buf, size_t off, u8 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u8)src[i] << ((sizeof(*val) - i - 1) * 8);
	}
	return 0;
}

int cbuf_get_u16be(const void *buf, size_t off, u16 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u16)src[i] << ((sizeof(*val) - i - 1) * 8);
	}
	return 0;
}

int cbuf_get_u32be(const void *buf, size_t off, u32 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u32)src[i] << ((sizeof(*val) - i - 1) * 8);
	}
	return 0;
}

int cbuf_get_u64be(const void *buf, size_t off, u64 *val)
{
	u8 *src = (u8 *)buf + off;
	*val	= 0;
	for (size_t i = 0; i < sizeof(*val); i++) {
		*val |= (u64)src[i] << ((sizeof(*val) - i - 1) * 8);
	}
	return 0;
}

int cbuf_write_le(void *buf, size_t *off, const void *val, size_t size)
{
	cbuf_set_le(buf, *off, val, size);
	*off += size;
	return 0;
}

int cbuf_write_u8le(void *buf, size_t *off, u8 val)
{
	cbuf_set_u8le(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u16le(void *buf, size_t *off, u16 val)
{
	cbuf_set_u16le(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u32le(void *buf, size_t *off, u32 val)
{
	cbuf_set_u32le(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u64le(void *buf, size_t *off, u64 val)
{
	cbuf_set_u64le(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_be(void *buf, size_t *off, const void *val, size_t size)
{
	cbuf_set_be(buf, *off, val, size);
	*off += size;
	return 0;
}

int cbuf_write_u8be(void *buf, size_t *off, u8 val)
{
	cbuf_set_u8be(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u16be(void *buf, size_t *off, u16 val)
{
	cbuf_set_u16be(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u32be(void *buf, size_t *off, u32 val)
{
	cbuf_set_u32be(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_write_u64be(void *buf, size_t *off, u64 val)
{
	cbuf_set_u64be(buf, *off, val);
	*off += sizeof(val);
	return 0;
}

int cbuf_read_le(const void *buf, size_t *off, void *val, size_t size)
{
	cbuf_get_le(buf, *off, val, size);
	*off += size;
	return 0;
}

int cbuf_read_u8le(const void *buf, size_t *off, u8 *val)
{
	cbuf_get_u8le(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u16le(const void *buf, size_t *off, u16 *val)
{
	cbuf_get_u16le(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u32le(const void *buf, size_t *off, u32 *val)
{
	cbuf_get_u32le(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u64le(const void *buf, size_t *off, u64 *val)
{
	cbuf_get_u64le(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_be(const void *buf, size_t *off, void *val, size_t size)
{
	cbuf_get_be(buf, *off, val, size);
	*off += size;
	return 0;
}

int cbuf_read_u8be(const void *buf, size_t *off, u8 *val)
{
	cbuf_get_u8be(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u16be(const void *buf, size_t *off, u16 *val)
{
	cbuf_get_u16be(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u32be(const void *buf, size_t *off, u32 *val)
{
	cbuf_get_u32be(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}

int cbuf_read_u64be(const void *buf, size_t *off, u64 *val)
{
	cbuf_get_u64be(buf, *off, val);
	*off += sizeof(*val);
	return 0;
}
