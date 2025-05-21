#ifndef STRVBUF_H
#define STRVBUF_H

#include "buf.h"
#include "strv.h"
#include "type.h"

typedef buf_t strvbuf_t;

strvbuf_t *strvbuf_init(strvbuf_t *buf, uint cap, size_t len, alloc_t alloc);
void strvbuf_free(strvbuf_t *buf);

void strvbuf_reset(strvbuf_t *buf, size_t used);

int strvbuf_add(strvbuf_t *buf, strv_t strv, size_t *off);
strv_t strvbuf_get(const strvbuf_t *buf, size_t off);

int strvbuf_set(strvbuf_t *buf, size_t off, strv_t strv);
int strvbuf_app(strvbuf_t *buf, size_t off, strv_t strv);

#endif
