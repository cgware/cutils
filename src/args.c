#include "args.h"

#include "log.h"
#include "strv.h"

static int print_usage_line(const opt_t *opt, print_dst_t dst)
{
	int off = dst.off;

	if (opt->short_opt) {
		dst.off += c_dprintf(dst, "  -%c", opt->short_opt);
	} else {
		dst.off += c_dprintf(dst, "    ");
	}

	if (opt->long_opt.data) {
		dst.off += c_dprintf(dst, opt->short_opt ? ", " : "  ");
		dst.off += c_dprintf(dst, "--%-10.*s", (int)opt->long_opt.len, opt->long_opt.data);
	} else {
		dst.off += c_dprintf(dst, "    %-10s", "");
	}

	dst.off += c_dprintf(dst, " %-16s %s", opt->placeholder ? opt->placeholder : "", opt->desc ? opt->desc : "");

	if (opt->required) {
		dst.off += c_dprintf(dst, " (required)\n");
	} else {
		switch (opt->type) {
		case OPT_STR: {
			strv_t val = *(strv_t *)opt->value;
			dst.off += c_dprintf(dst, " (default: %.*s)\n", (int)val.len, val.data);
			break;
		}
		case OPT_INT: dst.off += c_dprintf(dst, " (default: %d)\n", *(int *)opt->value); break;
		case OPT_BOOL: dst.off += c_dprintf(dst, " (default: %d)\n", *(int *)opt->value ? 1 : 0); break;
		case OPT_ENUM: {
			strv_t param = opt->enums.vals[*(int *)opt->value].param;
			dst.off += c_dprintf(dst, " (default: %.*s)\n", (int)param.len, param.data);
			break;
		}
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
			strv_t param = opts[i].enums.vals[j].param;
			dst.off += c_dprintf(dst, "  %-10.*s = %s\n", (int)param.len, param.data, opts[i].enums.vals[j].desc);
		}
	}

	return dst.off - off;
}

static int print_no_param(const opt_t *opt, int opt_long, print_dst_t dst)
{
	int off = dst.off;

	if (opt_long) {
		dst.off += c_dprintf(dst, "No %s specified for --%.*s\n", opt->placeholder, (int)opt->long_opt.len, opt->long_opt.data);

	} else {
		dst.off += c_dprintf(dst, "No %s specified for -%c\n", opt->placeholder, opt->short_opt);
	}

	return dst.off - off;
}

static int print_unknown_param(const opt_t *opt, int opt_long, strv_t param, print_dst_t dst)
{
	int off = dst.off;

	if (opt_long) {
		dst.off += c_dprintf(dst,
				     "Unknown %s specified for --%.*s: '%.*s'\n",
				     opt->placeholder,
				     (int)opt->long_opt.len,
				     opt->long_opt.data,
				     (int)param.len,
				     param.data);

	} else {
		dst.off += c_dprintf(
			dst, "Unknown %s specified for -%c: '%.*s'\n", opt->placeholder, opt->short_opt, (int)param.len, param.data);
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

static opt_t *find_long_opt(opt_t *opts, size_t opts_size, strv_t opt)
{
	size_t opts_len = opts_size / sizeof(opt_t);
	for (size_t i = 0; i < opts_len; i++) {
		if (opts[i].long_opt.data && strv_eq(opts[i].long_opt, opt)) {
			return &opts[i];
		}
	}

	return NULL;
}

static int find_enum_val(const opt_enum_val_t *vals, size_t vals_size, strv_t param)
{
	size_t vals_len = vals_size / sizeof(opt_enum_val_t);
	for (size_t i = 0; i < vals_len; i++) {
		if (strv_eq(vals[i].param, param)) {
			return (int)i;
		}
	}

	return -1;
}

static int parse_param(opt_t *opt, int opt_long, strv_t param, print_dst_t dst)
{
	switch (opt->type) {
	case OPT_NONE:
		*(strv_t *)opt->value = param;
		opt->set	      = 1;
		break;
	case OPT_STR:
		*(strv_t *)opt->value = param;
		opt->set	      = 1;
		break;
	case OPT_INT:
		if (strv_to_int(param, opt->value)) {
			return 1;
		}
		opt->set = 1;
		break;
	case OPT_BOOL:
		if (param.len != 1) {
			return 1;
		}

		switch (param.data[0]) {
		case '0': *(int *)opt->value = 0; break;
		case '1': *(int *)opt->value = 1; break;
		default: return 1;
		}
		opt->set = 1;
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
				if (parse_param(opt, opt_long, strv_cstr(argv[i]), dst)) {
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
			if (strv_eq(strv_cstr(&argv[i][2]), STRV("help"))) {
				dst.off += print_usage(argv[0], opts, opts_size, dst);
				return 1;
			}
			opt	 = find_long_opt(opts, opts_size, strv_cstr(&argv[i][2]));
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
			dst.off += c_dprintf(dst,
					     "Missing required option: -%c / --%.*s\n",
					     opts[i].short_opt,
					     (int)opts[i].long_opt.len,
					     opts[i].long_opt.data);
			return 1;
		}
	}

	return 0;
}
