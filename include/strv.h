#ifndef STRV_H
#define STRV_H

#include "dst.h"

strv_t strv_cstr(const char *cstr);

int strv_eq(strv_t l, strv_t r);
int strv_cmp(strv_t l, strv_t r);
int strv_cmpn(strv_t l, strv_t r, size_t len);

int strv_to_int(strv_t str, int *res);

int strv_split(strv_t str, char c, strv_t *l, strv_t *r);
int strv_rsplit(strv_t str, char c, strv_t *l, strv_t *r);

size_t strv_print(strv_t str, dst_t dst);

#endif
