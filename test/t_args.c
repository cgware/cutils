#include "args.h"

#include "test.h"

#define NAME "test_cutils"
#define DESC "cutils tests"

TEST(args_usage)
{
	START;

	char buf[128] = {0};
	EXPECT_EQ(args_usage(NAME, DESC, PRINT_DST_BUF(buf, sizeof(buf), 0)), 91);

	EXPECT_STR(buf,
		   "cutils tests\n"
		   "\n"
		   "Usage\n"
		   "  test_cutils [options]\n"
		   "\n"
		   "Run 'test_cutils --help' for more information\n");

	END;
}

static int handle_dir(const char *param, void *ret)
{
	(void)param;
	(void)ret;
	return 0;
}

TEST(args_handle)
{
	START;

	arg_t args[] = {
		[0] = ARG('T', "test", PARAM_NONE, "", "Run tests", NULL),
		[1] = ARG('B', "dir", PARAM_STR, "<dir>", "Set directory", handle_dir),
		[2] = ARG('S', "dir", PARAM_STR, "<dir>", "Set directory", NULL),
		[3] = ARG('D', "debug", PARAM_SWITCH, "<0/1>", "Turn on/off debug messages (default: 0)", NULL),
		[4] = ARG('M', "mode", PARAM_MODE, "<mode>", "Set mode (default: A)", NULL),
	};

	mode_t mode_modes[] = {
		[0] = {.c = 'A', .desc = "A mode"},
		[1] = {.c = 'B', .desc = "B mode"},
	};

	mode_desc_t modes[] = {
		{.name = "Modes", .modes = mode_modes, .len = 2},
	};

	char *dir = NULL;
	int debug = 0;

	void *params[] = {
		[2] = &dir,
		[3] = &debug,
	};

	char buf[512] = {0};

	const char *argv[] = {NAME, "-T", "-B", "./", "-S", "./", "-M", "B", "--debug", "1"};

	EXPECT_EQ(args_handle(NAME,
			      DESC,
			      args,
			      sizeof(args),
			      modes,
			      sizeof(modes),
			      sizeof(argv) / sizeof(char *),
			      argv,
			      params,
			      PRINT_DST_BUF(buf, sizeof(buf), 0)),
		  0);
	EXPECT_STR(buf, "");

	EXPECT_EQ(debug, 1);

	const char *argv_err[] = {NAME, "-D", "0", "-D", "2", "-G", "1"};

	EXPECT_EQ(args_handle(NAME,
			      DESC,
			      args,
			      sizeof(args),
			      modes,
			      sizeof(modes),
			      sizeof(argv_err) / sizeof(char *),
			      argv_err,
			      params,
			      PRINT_DST_BUF(buf, sizeof(buf), 0)),
		  1);

	EXPECT_STR(buf,
		   "cutils tests\n"
		   "\n"
		   "Usage\n"
		   "  test_cutils [options]\n"
		   "\n"
		   "Run 'test_cutils --help' for more information\n");

	const char *argv_help[] = {NAME, "--help"};

	EXPECT_EQ(args_handle(NAME,
			      DESC,
			      args,
			      sizeof(args),
			      modes,
			      sizeof(modes),
			      sizeof(argv_help) / sizeof(char *),
			      argv_help,
			      params,
			      PRINT_DST_BUF(buf, sizeof(buf), 0)),
		  2);

	EXPECT_STR(buf,
		   "cutils tests\n"
		   "\n"
		   "Usage\n"
		   "  test_cutils [options]\n"
		   "\n"
		   "Options\n"
		   "  -T --test                    Run tests\n"
		   "  -B --dir          <dir>      Set directory\n"
		   "  -S --dir          <dir>      Set directory\n"
		   "  -D --debug        <0/1>      Turn on/off debug messages (default: 0)\n"
		   "  -M --mode         <mode>     Set mode (default: A)\n"
		   "\n"
		   "Modes\n"
		   "  A = A mode\n"
		   "  B = B mode\n");

	const char *argv_d[] = {NAME, "-D"};

	EXPECT_EQ(args_handle(NAME,
			      DESC,
			      args,
			      sizeof(args),
			      modes,
			      sizeof(modes),
			      sizeof(argv_d) / sizeof(char *),
			      argv_d,
			      params,
			      PRINT_DST_BUF(buf, sizeof(buf), 0)),
		  2);

	EXPECT_STR(buf,
		   "cutils tests\n"
		   "\n"
		   "Usage\n"
		   "  test_cutils [options]\n"
		   "\n"
		   "Options\n"
		   "  -T --test                    Run tests\n"
		   "  -B --dir          <dir>      Set directory\n"
		   "  -S --dir          <dir>      Set directory\n"
		   "  -D --debug        <0/1>      Turn on/off debug messages (default: 0)\n"
		   "  -M --mode         <mode>     Set mode (default: A)\n"
		   "\n"
		   "Modes\n"
		   "  A = A mode\n"
		   "  B = B mode\n");

	const char *argv_h[] = {NAME, "-H"};

	EXPECT_EQ(args_handle(NAME,
			      DESC,
			      args,
			      sizeof(args),
			      modes,
			      sizeof(modes),
			      sizeof(argv_h) / sizeof(char *),
			      argv_h,
			      params,
			      PRINT_DST_BUF(buf, sizeof(buf), 0)),
		  2);

	EXPECT_STR(buf,
		   "cutils tests\n"
		   "\n"
		   "Usage\n"
		   "  test_cutils [options]\n"
		   "\n"
		   "Options\n"
		   "  -T --test                    Run tests\n"
		   "  -B --dir          <dir>      Set directory\n"
		   "  -S --dir          <dir>      Set directory\n"
		   "  -D --debug        <0/1>      Turn on/off debug messages (default: 0)\n"
		   "  -M --mode         <mode>     Set mode (default: A)\n"
		   "\n"
		   "Modes\n"
		   "  A = A mode\n"
		   "  B = B mode\n");

	END;
}

STEST(args)
{
	SSTART;

	RUN(args_usage);
	RUN(args_handle);

	SEND;
}
