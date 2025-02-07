#include "args.h"

#include "log.h"

#include <stdlib.h>
#include <string.h>

static int print_usage_line(const opt_t *opt, print_dst_t dst)
{
	int off = dst.off;

	if (opt->short_opt) {
		dst.off += c_dprintf(dst, "  -%c", opt->short_opt);
	} else {
		dst.off += c_dprintf(dst, "    ");
	}

	if (opt->long_opt) {
		dst.off += c_dprintf(dst, opt->short_opt ? ", " : "  ");
		dst.off += c_dprintf(dst, "--%-10s", opt->long_opt);
	} else {
		dst.off += c_dprintf(dst, "    %-10s", "");
	}

	dst.off += c_dprintf(dst, " %-16s %s", opt->placeholder ? opt->placeholder : "", opt->desc ? opt->desc : "");

	if (opt->required) {
		dst.off += c_dprintf(dst, " (required)\n");
	} else {
		switch (opt->type) {
		case OPT_STR: dst.off += c_dprintf(dst, " (default: %s)\n", *(const char **)opt->value); break;
		case OPT_INT: dst.off += c_dprintf(dst, " (default: %d)\n", *(int *)opt->value); break;
		case OPT_BOOL: dst.off += c_dprintf(dst, " (default: %d)\n", *(int *)opt->value ? 1 : 0); break;
		case OPT_ENUM: dst.off += c_dprintf(dst, " (default: %s)\n", opt->enums.vals[*(int *)opt->value].param); break;
		default: dst.off += c_dprintf(dst, "\n"); break;
		}
	}

	return dst.off - off;
}

static int print_usage(const char *name, const opt_t *opts, size_t opts_size, print_dst_t dst)
{
	int off = dst.off;

	dst.off += c_dprintf(dst,
			     "Usage: %s [options]\n"
			     "\n"
			     "Options\n",
			     name);

	opt_t help = OPT('h', "help", OPT_NONE, NULL, "Print usage information and exit", NULL, {0}, OPT_OPT);
	dst.off += print_usage_line(&help, dst);

	size_t opts_len = opts_size / sizeof(opt_t);
	for (size_t i = 0; i < opts_len; i++) {
		dst.off += print_usage_line(&opts[i], dst);
	}

	for (size_t i = 0; i < opts_len; i++) {
		if (opts[i].type != OPT_ENUM) {
			continue;
		}

		dst.off += c_dprintf(dst, "\n%s\n", opts[i].enums.name);
		size_t vals_len = opts[i].enums.vals_size / sizeof(opt_enum_val_t);
		for (size_t j = 0; j < vals_len; j++) {
			dst.off += c_dprintf(dst, "  %-10s = %s\n", opts[i].enums.vals[j].param, opts[i].enums.vals[j].desc);
		}
	}

	return dst.off - off;
}

static int print_no_param(const opt_t *opt, int opt_long, print_dst_t dst)
{
	int off = dst.off;

	if (opt_long) {
		dst.off += c_dprintf(dst, "No %s specified for --%s\n", opt->placeholder, opt->long_opt);

	} else {
		dst.off += c_dprintf(dst, "No %s specified for -%c\n", opt->placeholder, opt->short_opt);
	}

	return dst.off - off;
}

static int print_unknown_param(const opt_t *opt, int opt_long, const char *param, print_dst_t dst)
{
	int off = dst.off;

	if (opt_long) {
		dst.off += c_dprintf(dst, "Unknown %s specified for --%s: '%s'\n", opt->placeholder, opt->long_opt, param);

	} else {
		dst.off += c_dprintf(dst, "Unknown %s specified for -%c: '%s'\n", opt->placeholder, opt->short_opt, param);
	}

	return dst.off - off;
}

static opt_t *find_short_opt(opt_t *opts, size_t opts_size, char opt)
{
	size_t opts_len = opts_size / sizeof(opt_t);
	for (size_t i = 0; i < opts_len; i++) {
		if (opts[i].short_opt && opts[i].short_opt == opt) {
			return &opts[i];
		}
	}

	return NULL;
}

static opt_t *find_long_opt(opt_t *opts, size_t opts_size, const char *opt)
{
	size_t opts_len = opts_size / sizeof(opt_t);
	for (size_t i = 0; i < opts_len; i++) {
		if (opts[i].long_opt && strcmp(opts[i].long_opt, opt) == 0) {
			return &opts[i];
		}
	}

	return NULL;
}

static int find_enum_val(const opt_enum_val_t *vals, size_t vals_size, const char *param)
{
	size_t vals_len = vals_size / sizeof(opt_enum_val_t);
	for (size_t i = 0; i < vals_len; i++) {
		if (strcmp(vals[i].param, param) == 0) {
			return (int)i;
		}
	}

	return -1;
}

static int parse_param(opt_t *opt, int opt_long, const char *param, print_dst_t dst)
{
	switch (opt->type) {
	case OPT_NONE:
		*(const char **)opt->value = param;
		opt->set		   = 1;
		break;
	case OPT_STR:
		*(const char **)opt->value = param;
		opt->set		   = 1;
		break;
	case OPT_INT:
		*(int *)opt->value = strtol(param, NULL, 10);
		opt->set	   = 1;
		break;
	case OPT_BOOL:
		*(int *)opt->value = strtol(param, NULL, 10) ? 1 : 0;
		opt->set	   = 1;
		break;
	case OPT_ENUM:
		*(int *)opt->value = find_enum_val(opt->enums.vals, opt->enums.vals_size, param);
		if (*(int *)opt->value == -1) {
			dst.off += print_unknown_param(opt, opt_long, param, dst);
			return 1;
		}
		opt->set = 1;
		break;
	default: log_error("cutils", "args", NULL, "unknown type: %d", opt->type); break;
	}

	return 0;
}

int args_parse(int argc, const char **argv, opt_t *opts, size_t opts_size, print_dst_t dst)
{
	opt_t *opt   = NULL;
	int opt_long = 0;

	for (int i = 1; i < argc; i++) {
		if (opt && opt->value) {
			if (opt_long || argv[i][0] != '-') {
				if (parse_param(opt, opt_long, argv[i], dst)) {
					return 1;
				}
				opt = NULL;
				continue;
			}

			dst.off += print_no_param(opt, opt_long, dst);
			return 1;
		}

		if (argv[i][0] != '-') {
			dst.off += print_usage(argv[0], opts, opts_size, dst);
			return 1;
		}

		if (argv[i][1] == '-') {
			if (strcmp(&argv[i][2], "help") == 0) {
				dst.off += print_usage(argv[0], opts, opts_size, dst);
				return 1;
			}
			opt	 = find_long_opt(opts, opts_size, &argv[i][2]);
			opt_long = 1;
		} else if (argv[i][1] == 'h') {
			dst.off += print_usage(argv[0], opts, opts_size, dst);
			return 1;
		} else {
			opt	 = find_short_opt(opts, opts_size, argv[i][1]);
			opt_long = 0;
		}

		if (opt == NULL) {
			dst.off += c_dprintf(dst, "Unknown option: %s\n", argv[i]);
			return 1;
		}
	}

	if (opt && opt->value) {
		dst.off += print_no_param(opt, opt_long, dst);
		return 1;
	}

	size_t opts_len = opts_size / sizeof(opt_t);
	for (size_t i = 0; i < opts_len; i++) {
		if (opts[i].required && !opts[i].set) {
			dst.off += c_dprintf(dst, "Missing required option: -%c / --%s\n", opts[i].short_opt, opts[i].long_opt);
			return 1;
		}
	}

	return 0;
}
