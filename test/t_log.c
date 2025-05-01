#include "log.h"

#include "test.h"

TEST(log_set_get)
{
	START;

	const log_t *log = log_get();

	EXPECT_EQ(log_set(NULL), log);

	log_set((log_t *)log);

	END;
}

TEST(log_print_header)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 1, 0);

	log_debug("test", "log", NULL, "file test %d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	// clang-format off
	EXPECT_FMT(buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u DEBUG [test:log] test_log_print_header:%u: file test %u\n",
		   &y, &m, &d, &H, &M, &S, &U, &line, &x);
	// clang-format on

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_print_header_tag)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 1, 0);

	log_debug("test", "log", "file", "file test %d", 2);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	// clang-format off
	EXPECT_FMT(buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u DEBUG [test:log] test_log_print_header_tag:%u: [file] file test %u\n",
		   &y, &m, &d, &H, &M, &S, &U, &line, &x);
	// clang-format on

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 2);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_print_header_color)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 1, 1);

	log_debug("test", "log", NULL, "file test %d", 2);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	// clang-format off
	EXPECT_FMT(buf, 9, "\033[90m%4u-%2u-%2u %2u:%2u:%2u.%3u \033[36mDEBUG\033[0m [test:log] \033[90mtest_log_print_header_color:%u:\033[0m file test %u\n",
		   &y, &m, &d, &H, &M, &S, &U, &line, &x);
	// clang-format on

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 2);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_print_no_header)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 0, 0);

	log_debug("test", "log", NULL, "file test %d", 3);

	uint x;

	EXPECT_FMT(buf, 1, "file test %u\n", &x);

	EXPECT_EQ(x, 3);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_print_no_header_tag)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 0, 0);

	log_debug("test", "log", "file", "file test %d", 4);

	uint x;

	EXPECT_FMT(buf, 1, "[file] file test %u\n", &x);

	EXPECT_EQ(x, 4);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_print)
{
	SSTART;
	RUN(log_print_header);
	RUN(log_print_header_tag);
	RUN(log_print_header_color);
	RUN(log_print_no_header);
	RUN(log_print_no_header_tag);
	SEND;
}

TEST(log_level_str)
{
	START;

	EXPECT_STR(log_level_str(LOG_TRACE), "TRACE");
	EXPECT_STR(log_level_str(LOG_DEBUG), "DEBUG");

	END;
}

TEST(log_set_level)
{
	START;

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 0, 0);

	log_t *log = log_set(NULL);
	EXPECT_EQ(log_set_level(cb, LOG_WARN), -1);

	log_set(log);
	EXPECT_EQ(log_set_level(cb, log_set_level(cb, LOG_DEBUG)), LOG_DEBUG);

	log_remove_callback(cb);

	END;
}

TEST(log_set_quiet)
{
	START;

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 0, 0);

	log_t *log = log_set(NULL);
	EXPECT_EQ(log_set_quiet(cb, 0), -1);

	log_set(log);
	EXPECT_EQ(log_set_quiet(cb, log_set_quiet(cb, 1)), 1);

	log_remove_callback(cb);

	END;
}

TEST(log_set_header)
{
	START;

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 0, 0);

	log_t *log = log_set(NULL);
	EXPECT_EQ(log_set_header(cb, 1), -1);

	log_set(log);
	EXPECT_EQ(log_set_header(cb, log_set_header(cb, 0)), 0);

	log_remove_callback(cb);

	END;
}

static size_t print_callback(log_event_t *ev)
{
	(void)ev;
	return 0;
}

TEST(log_add_callback)
{
	START;

	const log_t *log = log_get();

	log_set(NULL);

	EXPECT_EQ(log_add_callback(print_callback, DST_NONE(), LOG_TRACE, 1, 0), -1);

	log_t tmp = {0};
	log_set(&tmp);

	int cb;

	for (int i = 0; i < LOG_MAX_CALLBACKS; i++) {
		EXPECT_EQ(cb = log_add_callback(print_callback, DST_NONE(), LOG_TRACE, 1, 0), i);
	}

	EXPECT_EQ(log_add_callback(print_callback, DST_NONE(), LOG_TRACE, 1, 0), -1);

	log_debug("test", "log", NULL, "trace");

	log_remove_callback(-1);
	log_remove_callback(cb);

	log_set((log_t *)log);

	END;
}

TEST(log_log)
{
	START;

	log_t *log = log_set(NULL);
	EXPECT_EQ(log_log(LOG_TRACE, "test", "log", NULL, 0, NULL, NULL), 1);

	log_t tmp = {0};
	log_set(&tmp);

	EXPECT_EQ(log_log(LOG_TRACE, "test", "log", NULL, 0, NULL, NULL), 1);

	log_set((log_t *)log);

	END;
}

TEST(log_trace)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_TRACE, 1, 0);

	log_trace("test", "log", NULL, "trace%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u TRACE [test:log] test_log_trace:%u: trace%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_debug)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_DEBUG, 1, 0);

	log_debug("test", "log", NULL, "debug%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u DEBUG [test:log] test_log_debug:%u: debug%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_info)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_INFO, 1, 0);

	log_info("test", "log", NULL, "info%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u INFO  [test:log] test_log_info:%u: info%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_warn)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_WARN, 1, 0);

	log_warn("test", "log", NULL, "warn%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u WARN  [test:log] test_log_warn:%u: warn%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_error)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_ERROR, 1, 0);

	log_error("test", "log", NULL, "error%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u ERROR [test:log] test_log_error:%u: error%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_fatal)
{
	START;

	log_t tmp  = {0};
	log_t *log = log_set(&tmp);

	char buf[1024] = {0};

	int cb = log_add_callback(log_std_cb, DST_BUF(buf), LOG_FATAL, 1, 0);

	log_fatal("test", "log", NULL, "fatal%d", 1);
	uint exp_line = __LINE__ - 1;

	uint y, m, d, H, M, S, U, line, x;

	EXPECT_FMT(
		buf, 9, "%4u-%2u-%2u %2u:%2u:%2u.%3u FATAL [test:log] test_log_fatal:%u: fatal%u\n", &y, &m, &d, &H, &M, &S, &U, &line, &x);

	EXPECT_EQ(line, exp_line);
	EXPECT_EQ(x, 1);

	log_remove_callback(cb);
	log_set(log);

	END;
}

TEST(log_strerror)
{
	START;

	EXPECT_STR(log_strerror(-1), "Unknown error");
	EXPECT_STR(log_strerror(0), "No error information");

	END;
}

STEST(log)
{
	SSTART;

	RUN(log_set_get);
	RUN(log_print);
	RUN(log_level_str);
	RUN(log_set_level);
	RUN(log_set_quiet);
	RUN(log_set_header);
	RUN(log_add_callback);
	RUN(log_log);
	RUN(log_trace);
	RUN(log_debug);
	RUN(log_info);
	RUN(log_warn);
	RUN(log_error);
	RUN(log_fatal);
	RUN(log_strerror);

	SEND;
}
