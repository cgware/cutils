#include "log.h"

#include "test.h"

#include <string.h>

TEST(log_print)
{
	START;
	char plain[256]	  = {0};
	char colored[256] = {0};
	char message[64]  = {0};
	int plain_id	  = log_add_output(DST_BUF(plain), LOG_DEBUG, 1, 0);
	int color_id	  = log_add_output(DST_BUF(colored), LOG_DEBUG, 1, 1);
	int message_id	  = log_add_output(DST_BUF(message), LOG_DEBUG, 0, 0);
	EXPECT_EQ(plain_id >= 0 && color_id >= 0 && message_id >= 0, 1);
	log_debug("test", "value %d", 7);
	uint y, m, d, H, M, S, U, x;
	EXPECT_FMT(plain, 8, "%4u-%2u-%2u %2u:%2u:%2u.%3u DEBUG test             value %u\n", &y, &m, &d, &H, &M, &S, &U, &x);
	EXPECT_EQ(x, 7);
	EXPECT_FMT(colored,
		   8,
		   "\033[90m%4u-%2u-%2u %2u:%2u:%2u.%3u\033[0m \033[36mDEBUG\033[0m test             value %u\n",
		   &y,
		   &m,
		   &d,
		   &H,
		   &M,
		   &S,
		   &U,
		   &x);
	EXPECT_STR(message, "value 7\n");
	EXPECT_EQ(log_remove_callback(plain_id), 0);
	EXPECT_EQ(log_remove_callback(color_id), 0);
	EXPECT_EQ(log_remove_callback(message_id), 0);
	END;
}

TEST(log_file)
{
	START;
	FILE *file = tmpfile();
	if (file == NULL) {
		END;
	}
	char buffer[256] = {0};
	int output_id	 = log_add_output(DST_BUF(buffer), LOG_INFO, 1, 0);
	int file_id	 = log_add_file(file, LOG_INFO);
	EXPECT_EQ(output_id >= 0 && file_id >= 0, 1);
	log_info("test", "shared %d", 5);
	fflush(file);
	rewind(file);
	char from_file[256] = {0};
	fgets(from_file, sizeof(from_file), file);
	EXPECT_STR(buffer, from_file);
	EXPECT_EQ(log_remove_callback(output_id), 0);
	EXPECT_EQ(log_remove_callback(file_id), 0);
	fclose(file);
	END;
}

TEST(log_output_controls)
{
	START;
	char buffer[256] = {0};
	int id		 = log_add_output(DST_BUF(buffer), LOG_INFO, 0, 0);
	EXPECT_EQ(id >= 0, 1);
	log_info("test", "first");
	EXPECT_STR(buffer, "first\n");
	EXPECT_EQ(log_set_header(id, 1), 0);
	EXPECT_EQ(log_set_level(id, LOG_WARN), LOG_INFO);
	log_info("test", "filtered");
	EXPECT_STR(buffer, "first\n");
	int quiet = log_set_quiet(0, 1);
	log_warn("test", "second");
	EXPECT_EQ(strstr(buffer, "WARN  test             second\n") != NULL, 1);
	EXPECT_EQ(log_set_quiet(id, 1), 0);
	size_t length = strlen(buffer);
	log_error("test", "quiet");
	log_set_quiet(0, quiet);
	EXPECT_EQ(strlen(buffer), length);
	log_remove_callback(id);
	END;
}

TEST(log_location)
{
	START;
	char plain[256]	  = {0};
	char colored[256] = {0};
	int plain_id	  = log_add_output(DST_BUF(plain), LOG_INFO, 0, 0);
	int color_id	  = log_add_output(DST_BUF(colored), LOG_INFO, 0, 1);
	log_set_location(1);
	int line = __LINE__ + 1;
	log_info("test", "located");
	char expected[160];
	snprintf(expected, sizeof(expected), "%s:%d:%s located\n", __FILE__, line, __func__);
	EXPECT_STR(plain, expected);
	snprintf(expected, sizeof(expected), "\033[90m%s:%d:%s\033[0m located\n", __FILE__, line, __func__);
	EXPECT_STR(colored, expected);
	log_set_location(0);
	log_remove_callback(plain_id);
	log_remove_callback(color_id);
	END;
}

typedef struct capture_s {
	int id;
	int count;
	int level;
	char message[128];
	char file[128];
	char function[64];
	int line;
} capture_t;

static void capture_event(log_event_t *ev, void *user)
{
	capture_t *capture = user;
	capture->count++;
	capture->level = ev->level;
	capture->line  = ev->line;
	snprintf(capture->file, sizeof(capture->file), "%s", ev->file);
	snprintf(capture->function, sizeof(capture->function), "%s", ev->func);
	va_list args;
	va_copy(args, ev->args);
	vsnprintf(capture->message, sizeof(capture->message), ev->fmt, args);
	va_end(args);
	if (capture->count == 1) {
		log_set_level(capture->id, LOG_ERROR);
	}
}

TEST(log_callback)
{
	START;
	capture_t capture = {0};
	capture.id	  = log_add_callback(capture_event, &capture, LOG_INFO);
	EXPECT_EQ(capture.id >= 0, 1);
	log_debug("test", "filtered");
	EXPECT_EQ(capture.count, 0);
	int line = __LINE__ + 1;
	log_info("test", "first %d", 1);
	EXPECT_EQ(capture.count, 1);
	EXPECT_EQ(capture.level, LOG_INFO);
	EXPECT_EQ(capture.line, line);
	EXPECT_STR(capture.file, __FILE__);
	EXPECT_STR(capture.function, __func__);
	EXPECT_STR(capture.message, "first 1");
	log_info("test", "filtered again");
	EXPECT_EQ(capture.count, 1);
	int quiet = log_set_quiet(0, 1);
	log_error("test", "second %d", 2);
	log_set_quiet(0, quiet);
	EXPECT_EQ(capture.count, 2);
	EXPECT_STR(capture.message, "second 2");
	log_remove_callback(capture.id);
	END;
}

static void count_event(log_event_t *ev, void *user)
{
	(void)ev;
	int *count = user;
	(*count)++;
}

typedef struct remove_state_s {
	int target;
	int calls;
} remove_state_t;

static void remove_later(log_event_t *ev, void *user)
{
	(void)ev;
	remove_state_t *state = user;
	state->calls++;
	log_remove_callback(state->target);
}

TEST(log_remove_during_callback)
{
	START;
	remove_state_t state = {0};
	int count	     = 0;
	int first	     = log_add_callback(remove_later, &state, LOG_INFO);
	state.target	     = log_add_callback(count_event, &count, LOG_INFO);
	EXPECT_EQ(first >= 0 && state.target > first, 1);
	log_info("test", "remove next");
	EXPECT_EQ(state.calls, 1);
	EXPECT_EQ(count, 0);
	log_remove_callback(first);
	END;
}

typedef struct replace_state_s {
	int target;
	int replacement;
	int calls;
} replace_state_t;

static void replace_later(log_event_t *ev, void *user)
{
	(void)ev;
	replace_state_t *state = user;
	state->calls++;
	log_remove_callback(state->target);
	state->replacement = log_add_callback(count_event, &state->calls, LOG_INFO);
}

TEST(log_replace_during_callback)
{
	START;
	replace_state_t state = {.replacement = -1};
	int old_calls	      = 0;
	int first	      = log_add_callback(replace_later, &state, LOG_INFO);
	state.target	      = log_add_callback(count_event, &old_calls, LOG_INFO);
	EXPECT_EQ(first >= 0 && state.target > first, 1);
	log_info("test", "replace next");
	EXPECT_EQ(state.replacement, state.target);
	EXPECT_EQ(state.calls, 2);
	EXPECT_EQ(old_calls, 0);
	log_remove_callback(first);
	log_remove_callback(state.replacement);
	END;
}

TEST(log_controls)
{
	START;
	int count = 0;
	int id	  = log_add_callback(count_event, &count, LOG_INFO);
	EXPECT_EQ(id >= 0, 1);
	EXPECT_EQ(log_set_quiet(id, 1), 0);
	log_info("test", "quiet");
	EXPECT_EQ(count, 0);
	EXPECT_EQ(log_set_quiet(id, 0), 1);
	EXPECT_EQ(log_set_level(id, LOG_ERROR), LOG_INFO);
	log_info("test", "filtered");
	EXPECT_EQ(count, 0);
	int quiet = log_set_quiet(0, 1);
	log_error("test", "called");
	log_set_quiet(0, quiet);
	EXPECT_EQ(count, 1);
	EXPECT_EQ(log_set_header(id, 0), 0);
	EXPECT_EQ(log_remove_callback(id), 0);
	EXPECT_EQ(log_remove_callback(id), 1);
	EXPECT_EQ(log_set_level(id, LOG_INFO), -1);
	EXPECT_EQ(log_set_quiet(id, 1), -1);
	EXPECT_EQ(log_set_header(id, 1), -1);
	EXPECT_EQ(log_remove_callback(-1), 1);
	EXPECT_EQ(log_set_level(-1, LOG_INFO), -1);
	EXPECT_EQ(log_set_quiet(-1, 1), -1);
	EXPECT_EQ(log_set_header(-1, 1), -1);
	END;
}

TEST(log_registry)
{
	START;
	EXPECT_EQ(log_add_callback(NULL, NULL, LOG_INFO), -1);
	EXPECT_EQ(log_add_output(DST_NONE(), LOG_INFO, 1, 0), -1);
	EXPECT_EQ(log_add_file(NULL, LOG_INFO), -1);
	EXPECT_EQ(log_add_callback(count_event, NULL, -1), -1);
	int count = 0;
	int ids[LOG_MAX_CALLBACKS];
	while (count < LOG_MAX_CALLBACKS) {
		int id = log_add_callback(count_event, NULL, LOG_TRACE);
		if (id < 0) {
			break;
		}
		ids[count++] = id;
	}
	EXPECT_EQ(count, LOG_MAX_CALLBACKS - 1);
	EXPECT_EQ(log_add_callback(count_event, NULL, LOG_TRACE), -1);
	for (int i = 0; i < count; i++) {
		log_remove_callback(ids[i]);
	}
	END;
}

TEST(log_write_invalid)
{
	START;
	EXPECT_EQ(log_format(DST_NONE(), NULL, 0, 0, 0), 0);
	EXPECT_EQ(log_write(LOG_INFO, "test", __FILE__, __func__, __LINE__, NULL), 1);
	EXPECT_EQ(log_write(-1, "test", __FILE__, __func__, __LINE__, "invalid"), 1);
	EXPECT_EQ(log_write(LOG_INFO, NULL, __FILE__, __func__, __LINE__, "invalid"), 1);
	EXPECT_STR(log_level_str(LOG_TRACE), "TRACE");
	EXPECT_STR(log_level_str(LOG_FATAL), "FATAL");
	EXPECT_STR(log_level_str(-1), "UNKNOWN");
	END;
}

static void format_invalid_time(log_event_t *ev, void *user)
{
	char *buffer  = user;
	ev->timestamp = UINT64_MAX;
	log_format(DST_BUFN(buffer, 128), ev, 1, 0, 0);
}

TEST(log_format_failure)
{
	START;
	char buffer[128] = {0};
	int id		 = log_add_callback(format_invalid_time, buffer, LOG_INFO);
	log_info("test", "bad time");
	EXPECT_STR(buffer, "0000-00-00 00:00:00.000 INFO  test             bad time\n");
	log_remove_callback(id);
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
	RUN(log_print);
	RUN(log_file);
	RUN(log_output_controls);
	RUN(log_location);
	RUN(log_callback);
	RUN(log_remove_during_callback);
	RUN(log_replace_during_callback);
	RUN(log_controls);
	RUN(log_registry);
	RUN(log_write_invalid);
	RUN(log_format_failure);
	RUN(log_strerror);
	SEND;
}
