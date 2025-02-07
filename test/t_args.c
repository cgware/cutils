#include "args.h"

#include "log.h"
#include "test.h"

TEST(args_parse_no_opts_short)
{
	START;

	char buf[256] = {0};

	{
		const char *argv[] = {"test"};
		EXPECT_EQ(args_parse(1, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	}

	{
		const char *argv[] = {"test", "a"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n");
	}

	{
		const char *argv[] = {"test", "-"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown option: -\n");
	}

	{
		const char *argv[] = {"test", "-a"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown option: -a\n");
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n");
	}

	{
		opt_t opts[] = {
			OPT('a', NULL, OPT_NONE, NULL, NULL, NULL, {0}, OPT_OPT),
		};
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a                                \n");
	}

	END;
}

TEST(args_parse_no_opts_long)
{
	START;

	char buf[256] = {0};

	{
		const char *argv[] = {"test", "--"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown option: --\n");
	}

	{
		const char *argv[] = {"test", "--a"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown option: --a\n");
	}

	{
		const char *argv[] = {"test", "--help"};
		EXPECT_EQ(args_parse(2, argv, NULL, 0, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n");
	}

	{
		opt_t opts[] = {
			OPT(0, "a", OPT_NONE, NULL, NULL, NULL, {0}, OPT_OPT),
		};
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "      --a                           \n");
	}
	END;
}

TEST(args_parse_no_param)
{
	START;

	char buf[256] = {0};

	const char *val = "";
	opt_t opts[]	= {
		   OPT('a', "a", OPT_STR, "<a>", "A", &val, {0}, OPT_OPT),
	   };

	{
		const char *argv[] = {"test", "-a"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "No <a> specified for -a\n");
	}

	{
		const char *argv[] = {"test", "-a", "-b"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "No <a> specified for -a\n");
	}

	{
		const char *argv[] = {"test", "--a"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "No <a> specified for --a\n");
	}

	END;
}

TEST(args_parse_none)
{
	START;

	char buf[256] = {0};

	opt_t opts[] = {
		OPT('a', "a", OPT_NONE, NULL, NULL, NULL, {0}, OPT_OPT),
	};

	{
		const char *argv[] = {"test", "-a"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	}

	{
		const char *argv[] = {"test", "--a"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	}

	{
		const char *val;
		opt_t optsv[] = {
			OPT('a', "a", OPT_NONE, NULL, NULL, &val, {0}, OPT_OPT),
		};
		const char *argv[] = {"test", "-a", "val"};
		EXPECT_EQ(args_parse(3, argv, optsv, sizeof(optsv), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a                           \n");
	}

	END;
}

TEST(args_parse_str)
{
	START;

	char buf[256] = {0};

	const char *val = "";

	opt_t opts[] = {
		OPT('a', "a", OPT_STR, "<a>", "A", &val, {0}, OPT_OPT),
	};

	{
		const char *argv[] = {"test", "-a", "val"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_STR(val, "val");
		val = "";
	}

	{
		const char *argv[] = {"test", "--a", "val"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_STR(val, "val");
		val = "";
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a          <a>              A (default: )\n");
	}

	END;
}

TEST(args_parse_int)
{
	START;

	char buf[256] = {0};

	int val = 1;

	opt_t opts[] = {
		OPT('a', "a", OPT_INT, "<a>", "A", &val, {0}, OPT_OPT),
	};

	{
		const char *argv[] = {"test", "-a", "2"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 2);
		val = 1;
	}

	{
		const char *argv[] = {"test", "--a", "2"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 2);
		val = 1;
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a          <a>              A (default: 1)\n");
	}

	END;
}

TEST(args_parse_bool)
{
	START;

	char buf[256] = {0};

	int val = 1;

	opt_t opts[] = {
		OPT('a', "a", OPT_BOOL, "<a>", "A", &val, {0}, OPT_OPT),
	};

	{
		const char *argv[] = {"test", "-a", "0"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 0);
		val = 1;
	}

	{
		const char *argv[] = {"test", "--a", "0"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 0);
		val = 1;
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a          <a>              A (default: 1)\n");
	}

	END;
}

TEST(args_parse_enum)
{
	START;

	char buf[256] = {0};

	int val = 1;

	const opt_enum_val_t enums[] = {
		[0] = {"a", "A"},
		[1] = {"b", "B"},
		[2] = {"c", "C"},
	};

	const opt_enum_t enums_desc = {
		.name	   = "Enums",
		.vals	   = enums,
		.vals_size = sizeof(enums),
	};

	opt_t opts[] = {
		OPT('a', "a", OPT_ENUM, "<a>", "A", &val, enums_desc, OPT_OPT),
	};

	{
		const char *argv[] = {"test", "-a", "d"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown <a> specified for -a: 'd'\n");
	}

	{
		const char *argv[] = {"test", "-a", "c"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 2);
		val = 1;
	}

	{
		const char *argv[] = {"test", "--a", "d"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Unknown <a> specified for --a: 'd'\n");
	}

	{
		const char *argv[] = {"test", "--a", "c"};
		EXPECT_EQ(args_parse(3, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		EXPECT_EQ(val, 2);
		val = 1;
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a          <a>              A (default: b)\n"
			   "\n"
			   "Enums\n"
			   "  a          = A\n"
			   "  b          = B\n"
			   "  c          = C\n");
	}

	END;
}

TEST(args_parse_other)
{
	START;

	char buf[256] = {0};

	const char *val = "";

	opt_t opts[] = {
		OPT('a', "a", OPT_STR, "<a>", "A", &val, {0}, OPT_REQ),
	};

	{
		const char *argv[] = {"test"};
		EXPECT_EQ(args_parse(1, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf, "Missing required option: -a / --a\n");
	}

	{
		const char *argv[] = {"test", "-h"};
		EXPECT_EQ(args_parse(2, argv, opts, sizeof(opts), PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
		EXPECT_STR(buf,
			   "Usage: test [options]\n"
			   "\n"
			   "Options\n"
			   "  -h, --help                        Print usage information and exit\n"
			   "  -a, --a          <a>              A (required)\n");
	}

	{
		const char *val;
		opt_t optsv[] = {
			OPT('a', "a", -1, NULL, NULL, &val, {0}, OPT_OPT),
		};
		const char *argv[] = {"test", "-a", "val"};
		log_set_quiet(0, 1);
		EXPECT_EQ(args_parse(3, argv, optsv, sizeof(optsv), PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
		log_set_quiet(0, 0);
	}

	END;
}

STEST(args)
{
	SSTART;

	RUN(args_parse_no_opts_short);
	RUN(args_parse_no_opts_long);
	RUN(args_parse_no_param);
	RUN(args_parse_none);
	RUN(args_parse_str);
	RUN(args_parse_int);
	RUN(args_parse_bool);
	RUN(args_parse_enum);
	RUN(args_parse_other);

	SEND;
}
