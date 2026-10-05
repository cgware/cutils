#include "log.h"

#include "cspinlock.h"
#include "cterm.h"
#include "ctime.h"
#include "mem.h"

typedef enum log_sink_kind_e {
	LOG_SINK_NONE,
	LOG_SINK_CALLBACK,
	LOG_SINK_OUTPUT,
	LOG_SINK_FILE
} log_sink_kind_t;

typedef struct log_sink_s {
	log_sink_kind_t kind;
	log_callback_fn callback;
	void *user;
	dst_t dst;
	FILE *file;
	int level;
	int quiet;
	int header;
	int colors;
} log_sink_t;

static log_sink_t sinks[LOG_MAX_CALLBACKS];
static cspinlock_t config_lock = CSPINLOCK_INIT;
static cspinlock_t output_lock = CSPINLOCK_INIT;
static int location_enabled;

static const char *level_strs[]	  = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char *level_colors[] = {"\033[94m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m"};

size_t log_format(dst_t dst, log_event_t *ev, int header, int colors, int location)
{
	if (ev == NULL || ev->level < LOG_TRACE || ev->level > LOG_FATAL || ev->component == NULL || ev->fmt == NULL) {
		return 0;
	}
	size_t start = dst.off;
	if (header) {
		char time[CTIME_BUF_SIZE];
		if (c_time_format_utc(time, sizeof(time), ev->timestamp) != 0) {
			mem_copy(time, sizeof(time), "0000-00-00 00:00:00.000", sizeof(time));
		}
		if (colors) {
			dst.off += dputf(dst,
					 "\033[90m%s\033[0m %s%-5s\033[0m %-16s ",
					 time,
					 level_colors[ev->level],
					 level_strs[ev->level],
					 ev->component);
		} else {
			dst.off += dputf(dst, "%s %-5s %-16s ", time, level_strs[ev->level], ev->component);
		}
	}
	if (location && ev->file != NULL && ev->line > 0) {
		if (colors) {
			dst.off += dputf(dst, "\033[90m");
		}
		dst.off += dputf(dst, "%s:%d", ev->file, ev->line);
		if (ev->func != NULL) {
			dst.off += dputf(dst, ":%s", ev->func);
		}
		if (colors) {
			dst.off += dputf(dst, "\033[0m");
		}
		dst.off += dputf(dst, " ");
	}
	va_list copy;
	va_copy(copy, ev->args);
	dst.off += dputv(dst, ev->fmt, copy);
	va_end(copy);
	dst.off += dputf(dst, "\n");
	return dst.off - start;
}

static size_t file_putv(dst_t dst, const char *format, va_list args)
{
	int length = vfprintf(dst.dst, format, args);
	return length > 0 ? (size_t)length : 0;
}

static int add_sink(log_sink_t sink)
{
	if (sink.level < LOG_TRACE || sink.level > LOG_FATAL) {
		return -1;
	}
	cspinlock_lock(&output_lock);
	cspinlock_lock(&config_lock);
	for (int i = 0; i < LOG_MAX_CALLBACKS; i++) {
		if (sinks[i].kind != LOG_SINK_NONE) {
			continue;
		}
		sinks[i] = sink;
		cspinlock_unlock(&config_lock);
		cspinlock_unlock(&output_lock);
		return i;
	}
	cspinlock_unlock(&config_lock);
	cspinlock_unlock(&output_lock);
	return -1;
}

int log_add_callback(log_callback_fn callback, void *user, int minimum)
{
	if (callback == NULL) {
		return -1;
	}
	return add_sink((log_sink_t){.kind = LOG_SINK_CALLBACK, .callback = callback, .user = user, .level = minimum});
}

int log_add_output(dst_t dst, int minimum, int header, int colors)
{
	if (dst.putv == NULL) {
		return -1;
	}
	return add_sink((log_sink_t){.kind = LOG_SINK_OUTPUT, .dst = dst, .level = minimum, .header = header != 0, .colors = colors != 0});
}

int log_add_file(FILE *file, int minimum)
{
	if (file == NULL) {
		return -1;
	}
	return add_sink((log_sink_t){.kind = LOG_SINK_FILE, .file = file, .level = minimum, .header = 1});
}

int log_remove_callback(int id)
{
	cspinlock_lock(&output_lock);
	cspinlock_lock(&config_lock);
	if (id < 0 || id >= LOG_MAX_CALLBACKS || sinks[id].kind == LOG_SINK_NONE) {
		cspinlock_unlock(&config_lock);
		cspinlock_unlock(&output_lock);
		return 1;
	}
	sinks[id] = (log_sink_t){0};
	cspinlock_unlock(&config_lock);
	cspinlock_unlock(&output_lock);
	return 0;
}

int log_set_level(int id, int minimum)
{
	cspinlock_lock(&config_lock);
	if (id < 0 || id >= LOG_MAX_CALLBACKS || sinks[id].kind == LOG_SINK_NONE || minimum < LOG_TRACE || minimum > LOG_FATAL) {
		cspinlock_unlock(&config_lock);
		return -1;
	}
	int previous	= sinks[id].level;
	sinks[id].level = minimum;
	cspinlock_unlock(&config_lock);
	return previous;
}

int log_set_quiet(int id, int quiet)
{
	cspinlock_lock(&config_lock);
	if (id < 0 || id >= LOG_MAX_CALLBACKS || sinks[id].kind == LOG_SINK_NONE) {
		cspinlock_unlock(&config_lock);
		return -1;
	}
	int previous	= sinks[id].quiet;
	sinks[id].quiet = quiet != 0;
	cspinlock_unlock(&config_lock);
	return previous;
}

int log_set_header(int id, int enabled)
{
	cspinlock_lock(&config_lock);
	if (id < 0 || id >= LOG_MAX_CALLBACKS || sinks[id].kind == LOG_SINK_NONE) {
		cspinlock_unlock(&config_lock);
		return -1;
	}
	int previous	 = sinks[id].header;
	sinks[id].header = enabled != 0;
	cspinlock_unlock(&config_lock);
	return previous;
}

void log_set_location(int enabled)
{
	cspinlock_lock(&config_lock);
	location_enabled = enabled != 0;
	cspinlock_unlock(&config_lock);
}

const char *log_level_str(int level)
{
	return level >= LOG_TRACE && level <= LOG_FATAL ? level_strs[level] : "UNKNOWN";
}

int log_write(int level, const char *component, const char *file, const char *func, int line, const char *fmt, ...)
{
	if (level < LOG_TRACE || level > LOG_FATAL || component == NULL || fmt == NULL) {
		return 1;
	}
	va_list args;
	va_start(args, fmt);
	log_event_t base = {
		.component = component,
		.file	   = file,
		.func	   = func,
		.line	   = line,
		.fmt	   = fmt,
		.timestamp = c_time(),
		.level	   = level,
	};

	for (int i = 0; i < LOG_MAX_CALLBACKS; i++) {
		cspinlock_lock(&config_lock);
		log_sink_t sink = sinks[i];
		int location	= location_enabled;
		cspinlock_unlock(&config_lock);
		if (sink.kind == LOG_SINK_NONE || sink.quiet || level < sink.level) {
			continue;
		}

		log_event_t ev = base;
		va_copy(ev.args, args);
		if (sink.kind == LOG_SINK_CALLBACK) {
			sink.callback(&ev, sink.user);
		} else {
			cspinlock_lock(&output_lock);
			cspinlock_lock(&config_lock);
			sink	 = sinks[i];
			location = location_enabled;
			cspinlock_unlock(&config_lock);
			if (sink.kind == LOG_SINK_OUTPUT && !sink.quiet && level >= sink.level) {
				size_t written = log_format(sink.dst, &ev, sink.header, sink.colors, location);
				cspinlock_lock(&config_lock);
				sinks[i].dst.off += written;
				cspinlock_unlock(&config_lock);
			} else if (sink.kind == LOG_SINK_FILE && !sink.quiet && level >= sink.level) {
				dst_t dst = {.putv = file_putv, .dst = sink.file};
				log_format(dst, &ev, sink.header, c_term_color(sink.file), location);
			}
			cspinlock_unlock(&output_lock);
		}
		va_end(ev.args);
	}
	va_end(args);
	return 0;
}

#define EPERM	     1
#define ENOENT	     2
#define ESRCH	     3
#define EINTR	     4
#define EIO	     5
#define ENXIO	     6
#define E2BIG	     7
#define ENOEXEC	     8
#define EBADF	     9
#define ECHILD	     10
#define EAGAIN	     11
#define ENOMEM	     12
#define EACCES	     13
#define EFAULT	     14
#define ENOTBLK	     15
#define EBUSY	     16
#define EEXIST	     17
#define EXDEV	     18
#define ENODEV	     19
#define ENOTDIR	     20
#define EISDIR	     21
#define EINVAL	     22
#define ENFILE	     23
#define EMFILE	     24
#define ENOTTY	     25
#define ETXTBSY	     26
#define EFBIG	     27
#define ENOSPC	     28
#define ESPIPE	     29
#define EROFS	     30
#define EMLINK	     31
#define EPIPE	     32
#define EDOM	     33
#define ERANGE	     34
#define EDEADLK	     35
#define ENAMETOOLONG 36
#define ENOLCK	     37
#define ENOSYS	     38
#define ENOTEMPTY    39
#define ELOOP	     40

static const char *errors[] = {
	[0]	       = "No error information",
	[EPERM]	       = "Operation not permitted",
	[ENOENT]       = "No such file or directory",
	[ESRCH]	       = "No such process",
	[EINTR]	       = "Interrupted system call",
	[EIO]	       = "I/O error",
	[ENXIO]	       = "No such device or address",
	[E2BIG]	       = "Argument list too long",
	[ENOEXEC]      = "Exec format error",
	[EBADF]	       = "Bad file number",
	[ECHILD]       = "No child processes",
	[EAGAIN]       = "Try again",
	[ENOMEM]       = "Out of memory",
	[EACCES]       = "Permission denied",
	[EFAULT]       = "Bad address",
	[ENOTBLK]      = "Block device required",
	[EBUSY]	       = "Device or resource busy",
	[EEXIST]       = "File exists",
	[EXDEV]	       = "Cross-device link",
	[ENODEV]       = "No such device",
	[ENOTDIR]      = "Not a directory",
	[EISDIR]       = "Is a directory",
	[EINVAL]       = "Invalid argument",
	[ENFILE]       = "File table overflow",
	[EMFILE]       = "Too many open files",
	[ENOTTY]       = "Not a typewriter",
	[ETXTBSY]      = "Text file busy",
	[EFBIG]	       = "File too large",
	[ENOSPC]       = "No space left on device",
	[ESPIPE]       = "Illegal seek",
	[EROFS]	       = "Read-only file system",
	[EMLINK]       = "Too many links",
	[EPIPE]	       = "Broken pipe",
	[EDOM]	       = "Math argument out of domain of func",
	[ERANGE]       = "Math result not representable",
	[EDEADLK]      = "Resource deadlock would occur",
	[ENAMETOOLONG] = "File name too long",
	[ENOLCK]       = "No record locks available",
	[ENOSYS]       = "Invalid system call number",
	[ENOTEMPTY]    = "Directory not empty",
	[ELOOP]	       = "Too many symbolic links encountered",
};

const char *log_strerror(int errnum)
{
	if (errnum < 0 || errnum > (int)(sizeof(errors) / sizeof(const char *))) {
		return "Unknown error";
	}

	return errors[errnum];
}
