#include "log.h"

#include "file.h"
#include "platform.h"

#include <string.h>

#define DEFAULT_QUIET 0

static log_t *s_log;

static const char *level_strs[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static const char *level_colors[] = {"\033[94m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m"};

int log_std_cb(log_event_t *ev)
{
	const char *tag_s = "";
	const char *tag_e = "";
	const char *tag	  = "";
	if (ev->tag != NULL) {
		tag_s = "[";
		tag_e = "] ";
		tag   = ev->tag;
	}

	int off = ev->print.off;

	if (ev->header) {
		if (ev->colors) {
			ev->print.off += c_dprintf(ev->print,
						   "%s %s%-5s\033[0m [%s:%s] \033[90m%s:%d:\033[0m %s%s%s",
						   ev->time,
						   level_colors[ev->level],
						   level_strs[ev->level],
						   ev->pkg,
						   ev->file,
						   ev->func,
						   ev->line,
						   tag_s,
						   tag,
						   tag_e);
		} else {
			ev->print.off += c_dprintf(ev->print,
						   "%s %-5s [%s:%s] %s:%d: %s%s%s",
						   ev->time,
						   level_strs[ev->level],
						   ev->pkg,
						   ev->file,
						   ev->func,
						   ev->line,
						   tag_s,
						   tag,
						   tag_e);
		}
	} else {
		ev->print.off += c_dprintf(ev->print, "%s%s%s", tag_s, tag, tag_e);
	}

	ev->print.off += c_dprintv(ev->print, ev->fmt, ev->ap);
	ev->print.off += c_dprintf(ev->print, "\n");

	return ev->print.off - off;
}

log_t *log_set(log_t *log)
{
	log_t *cur = s_log;

	s_log = log;

	return cur;
}

const log_t *log_get()
{
	return s_log;
}

const char *log_level_str(int level)
{
	return level_strs[level];
}

int log_set_level(int cb, int level)
{
	if (s_log == NULL || cb >= LOG_MAX_CALLBACKS || s_log->callbacks[cb].log == NULL) {
		return -1;
	}

	const int cur = s_log->callbacks[cb].level;

	s_log->callbacks[cb].level = level;

	return cur;
}

int log_set_quiet(int cb, int quiet)
{
	if (s_log == NULL || cb >= LOG_MAX_CALLBACKS || s_log->callbacks[cb].log == NULL) {
		return -1;
	}

	const int cur = s_log->callbacks[cb].quiet;

	s_log->callbacks[cb].quiet = quiet;

	return cur;
}

int log_set_header(int cb, int enable)
{
	if (s_log == NULL || cb >= LOG_MAX_CALLBACKS || s_log->callbacks[cb].log == NULL) {
		return -1;
	}

	const int header = s_log->callbacks[cb].header;

	s_log->callbacks[cb].header = enable;

	return header;
}

int log_add_callback(log_cb log, print_dst_t print, int level, int header, int colors)
{
	if (s_log == NULL) {
		return -1;
	}

	for (int i = 0; i < LOG_MAX_CALLBACKS; i++) {
		if (s_log->callbacks[i].log) {
			continue;
		}

		s_log->callbacks[i] = (log_callback_t){
			.log	= log,
			.print	= print,
			.level	= level,
			.header = header,
			.colors = colors,
			.quiet	= 0,
		};

		return i;
	}

	return -1;
}

int log_remove_callback(int cb)
{
	if (s_log == NULL || cb < 0 || cb >= LOG_MAX_CALLBACKS || s_log->callbacks[cb].log == NULL) {
		return 1;
	}

	s_log->callbacks[cb].log = NULL;

	return 0;
}

static int init_event(log_event_t *ev, print_dst_t print, int header, int colors)
{
	if (!ev->time[0]) {
		c_time_str(ev->time);
	}

	ev->print  = print;
	ev->header = header;
	ev->colors = colors;

	return 0;
}

int log_log(int level, const char *pkg, const char *file, const char *func, int line, const char *tag, const char *fmt, ...)
{
	if (s_log == NULL || file == NULL || fmt == NULL) {
		return 1;
	}

	log_event_t ev = {
		.pkg   = pkg,
		.file  = file,
		.func  = func,
		.tag   = tag,
		.fmt   = fmt,
		.line  = line,
		.level = level,
	};

	for (int i = 0; i < LOG_MAX_CALLBACKS && s_log->callbacks[i].log; i++) {
		log_callback_t *cb = &s_log->callbacks[i];
		if (cb->quiet || level < cb->level) {
			continue;
		}

		init_event(&ev, cb->print, cb->header, cb->colors);
		va_start(ev.ap, fmt);
		cb->print.off += cb->log(&ev);
		va_end(ev.ap);
	}

	return 0;
}

const char *log_strerror(int errnum)
{
#if defined C_WIN
	static char buf[256] = {0};
	strerror_s(buf, sizeof(buf), errnum);
	return buf;
#else
	return strerror(errnum);
#endif
}
