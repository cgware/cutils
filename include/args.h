#ifndef ARGS_H
#define ARGS_H

#include "print.h"

typedef enum param_e {
	PARAM_NONE,
	PARAM_INT,
	PARAM_STR,
	PARAM_MODE,
	PARAM_SWITCH,
} param_t;

typedef struct mode_s {
	char c;
	const char *desc;
} mode_t;

typedef struct mode_desc_s {
	const char *name;
	const mode_t *modes;
	size_t len;
} mode_desc_t;

#define ARG(_c, _l, _param, _name, _desc, _handler) {_l, _name, _desc, _handler, _param, _c}

typedef int (*param_handler_fn)(const char *param, void *ret);
typedef struct arg_s {
	const char *l;
	const char *name;
	const char *desc;
	param_handler_fn handler;
	param_t param;
	char c;
} arg_t;

int args_usage(const char *name, const char *description, print_dst_t dst);
int args_handle(const char *name, const char *description, const arg_t *args, size_t args_size, const mode_desc_t *modes, size_t modes_size,
		int argc, const char **argv, void **params, print_dst_t dst);

#endif
