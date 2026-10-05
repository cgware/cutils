#ifndef LOG_H
#define LOG_H

#include "dst.h"
#include "type.h"

#include <stdio.h>

typedef struct log_event_s {
	const char *component;
	const char *file;
	const char *func;
	const char *fmt;
	va_list args;
	int line;
	u64 timestamp;
	int level;
} log_event_t;

typedef void (*log_callback_fn)(log_event_t *ev, void *user);

#define LOG_MAX_CALLBACKS 32

enum {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL
};

#define log_trace(component, ...) log_write(LOG_TRACE, component, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_debug(component, ...) log_write(LOG_DEBUG, component, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_info(component, ...)  log_write(LOG_INFO, component, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_warn(component, ...)  log_write(LOG_WARN, component, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_error(component, ...) log_write(LOG_ERROR, component, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_fatal(component, ...) log_write(LOG_FATAL, component, __FILE__, __func__, __LINE__, __VA_ARGS__)

int log_add_callback(log_callback_fn callback, void *user, int minimum);
int log_add_output(dst_t dst, int minimum, int header, int colors);
int log_add_file(FILE *file, int minimum);
int log_remove_callback(int id);
int log_set_level(int id, int minimum);
int log_set_quiet(int id, int quiet);
int log_set_header(int id, int enabled);
void log_set_location(int enabled);

size_t log_format(dst_t dst, log_event_t *event, int header, int colors, int location);
const char *log_level_str(int level);
int log_write(int level, const char *component, const char *file, const char *function, int line, const char *format, ...);
const char *log_strerror(int errnum);

#endif
