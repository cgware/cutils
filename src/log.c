#include "log.h"

#include "platform.h"

#define DEFAULT_QUIET 0

static log_t *s_log;

static const char *level_strs[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static const char *level_colors[] = {"\033[94m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m"};

size_t log_std_cb(log_event_t *ev)
{
	const char *tag_s = "";
	const char *tag_e = "";
	const char *tag	  = "";
	if (ev->tag != NULL) {
		tag_s = "[";
		tag_e = "] ";
		tag   = ev->tag;
	}

	size_t off = ev->dst.off;

	if (ev->header) {
		if (ev->colors) {
			ev->dst.off += dputf(ev->dst,
					     "\033[90m%s %s%-5s\033[0m [%s:%s] \033[90m%s:%d:\033[0m %s%s%s",
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
			ev->dst.off += dputf(ev->dst,
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
		ev->dst.off += dputf(ev->dst, "%s%s%s", tag_s, tag, tag_e);
	}

	ev->dst.off += dputv(ev->dst, ev->fmt, ev->ap);
	ev->dst.off += dputs(ev->dst, STRV("\n"));

	return ev->dst.off - off;
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

int log_add_callback(log_cb log, dst_t dst, int level, int header, int colors)
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
			.dst	= dst,
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

static int init_event(log_event_t *ev, dst_t dst, int header, int colors)
{
	if (!ev->time[0]) {
		c_time_str(ev->time);
	}

	ev->dst	   = dst;
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

		init_event(&ev, cb->dst, cb->header, cb->colors);
		va_start(ev.ap, fmt);
		cb->dst.off += cb->log(&ev);
		va_end(ev.ap);
	}

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
