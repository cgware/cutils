#ifndef ARGS_H
#define ARGS_H

#include "dst.h"
#include "strv.h"

typedef enum opt_type_e {
	OPT_NONE,
	OPT_STR,
	OPT_INT,
	OPT_BOOL,
	OPT_ENUM,
} opt_type_t;

typedef enum opt_opt_e {
	OPT_OPT,
	OPT_REQ,
} opt_opt_t;

typedef struct opt_enum_val_s {
	strv_t param;
	const char *desc;
	void *priv;
} opt_enum_val_t;

typedef struct opt_enum_s {
	const char *name;
	const opt_enum_val_t *vals;
	size_t vals_size;
} opt_enum_t;

typedef struct {
	char short_opt;
	strv_t long_opt;
	opt_type_t type;
	const char *placeholder;
	const char *desc;
	void *value;
	opt_enum_t enums;
	int required;
	int set;
} opt_t;

int args_parse(int argc, const char **argv, opt_t *opts, size_t opts_size, dst_t dst);

#define OPT(short_opt, long_opt, type, placeholder, desc, value, enums, required)                                                          \
	{                                                                                                                                  \
		short_opt, STRVT(long_opt), type, placeholder, desc, value, enums, required, 0                                             \
	}

#endif
