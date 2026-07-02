#include "proc.h"

#include "cproc.h"
#include "log.h"
#include "mem.h"

typedef int (*proc_cmd_fn)(proc_t *proc, strv_t cmd);
typedef strv_t (*proc_getenv_fn)(proc_t *proc, strv_t name);
typedef int (*proc_setenv_fn)(proc_t *proc, strv_t name, strv_t val, int overwrite);
typedef int (*proc_unsetenv_fn)(proc_t *proc, strv_t name);
typedef int (*proc_gethostname_fn)(proc_t *proc, char *name, size_t len);

typedef struct proc_ops_s {
	proc_cmd_fn cmd;
	proc_getenv_fn getenv;
	proc_setenv_fn setenv;
	proc_unsetenv_fn unsetenv;
	proc_gethostname_fn gethostname;
} proc_ops_t;

static int env_name_valid(strv_t name)
{
	if (name.data == NULL || name.len == 0) {
		return 0;
	}

	for (size_t i = 0; i < name.len; i++) {
		if (name.data[i] == '=' || name.data[i] == '\n') {
			return 0;
		}
	}

	return 1;
}

static int env_val_valid(strv_t val)
{
	if (val.data == NULL) {
		return 0;
	}

	for (size_t i = 0; i < val.len; i++) {
		if (val.data[i] == '\n') {
			return 0;
		}
	}

	return 1;
}

static int env_find(proc_t *proc, strv_t name, size_t *start, size_t *end, strv_t *val)
{
	strv_t env = STRVS(proc->env);
	for (size_t off = 0; off < env.len;) {
		size_t line_start = off;
		while (off < env.len && env.data[off] != '\n') {
			off++;
		}

		size_t line_end = off;
		strv_t line	= STRVN(&env.data[line_start], line_end - line_start);
		strv_t key	= STRV_NULL;
		strv_t value	= STRV_NULL;

		strv_lsplit(line, '=', &key, &value);
		if (value.data != NULL && strv_eq(key, name)) {
			if (start) {
				*start = line_start;
			}
			if (end) {
				*end = off < env.len ? off + 1 : off;
			}
			if (val) {
				*val = value;
			}
			return 1;
		}

		if (off < env.len) {
			off++;
		}
	}

	return 0;
}

int op_cmd(proc_t *proc, strv_t cmd)
{
	(void)proc;
	int ret = cproc_system(cmd.data);
	if (ret) {
		log_error("cutils", "proc", NULL, "\"%.*s\": exited with code %d", cmd.len, cmd.data, ret);
	}
	return ret;
}

int vp_cmd(proc_t *proc, strv_t cmd)
{
	if (cmd.data == NULL) {
		return 1;
	}

	str_cat(&proc->buf, cmd);
	str_cat(&proc->buf, STRV("\n"));

	return 0;
}

strv_t op_getenv(proc_t *proc, strv_t name)
{
	(void)proc;
	return strv_cstr(cproc_getenv(name.data));
}

strv_t vp_getenv(proc_t *proc, strv_t name)
{
	strv_t val = STRV_NULL;
	env_find(proc, name, NULL, NULL, &val);
	return val;
}

int op_setenv(proc_t *proc, strv_t name, strv_t val, int overwrite)
{
	(void)proc;
	return cproc_setenv(name.data, val.data, overwrite) ? 1 : 0;
}

int vp_setenv(proc_t *proc, strv_t name, strv_t val, int overwrite)
{
	size_t start = 0;
	size_t end   = 0;
	int exists   = env_find(proc, name, &start, &end, NULL);
	if (exists && !overwrite) {
		return 0;
	}

	if (exists) {
		size_t line_len = name.len + 1 + val.len + 1;
		str_t line	= strz(line_len + 1);
		if (line.data == NULL) {
			return 1;
		}

		str_cat(&line, name);
		str_cat(&line, STRV("="));
		str_cat(&line, val);
		str_cat(&line, STRV("\n"));

		int ret = str_subreplace(&proc->env, start, end, STRVS(line));
		str_free(&line);
		return ret;
	}

	if (proc->env.data == NULL) {
		proc->env = strz(name.len + 1 + val.len + 1 + 1);
		if (proc->env.data == NULL) {
			return 1;
		}
	}

	int ret = 0;
	ret |= str_cat(&proc->env, name) == NULL;
	ret |= str_cat(&proc->env, STRV("=")) == NULL;
	ret |= str_cat(&proc->env, val) == NULL;
	ret |= str_cat(&proc->env, STRV("\n")) == NULL;
	return ret;
}

int op_unsetenv(proc_t *proc, strv_t name)
{
	(void)proc;
	return cproc_unsetenv(name.data) ? 1 : 0;
}

int vp_unsetenv(proc_t *proc, strv_t name)
{
	size_t start = 0;
	size_t end   = 0;
	if (!env_find(proc, name, &start, &end, NULL)) {
		return 0;
	}

	return str_subreplace(&proc->env, start, end, STRV(""));
}

int op_gethostname(proc_t *proc, char *name, size_t len)
{
	(void)proc;
	return cproc_gethostname(name, len);
}

int vp_gethostname(proc_t *proc, char *name, size_t len)
{
	if (proc->hostname.data == NULL || proc->hostname.len + 1 > len) {
		return 1;
	}

	mem_copy(name, len, proc->hostname.data, proc->hostname.len);
	name[proc->hostname.len] = '\0';

	return 0;
}

static const proc_ops_t s_proc_ops[] = {
	[0] =
		{
			.cmd	     = op_cmd,
			.getenv	     = op_getenv,
			.setenv	     = op_setenv,
			.unsetenv    = op_unsetenv,
			.gethostname = op_gethostname,
		},
	[1] =
		{
			.cmd	     = vp_cmd,
			.getenv	     = vp_getenv,
			.setenv	     = vp_setenv,
			.unsetenv    = vp_unsetenv,
			.gethostname = vp_gethostname,
		},
};

proc_t *proc_init(proc_t *proc, size_t buf_size, int virt)
{
	if (proc == NULL) {
		return NULL;
	}

	if (virt) {
		proc->buf = strz(buf_size);
		if (buf_size > 0 && proc->buf.data == NULL) {
			return NULL;
		}
	}
	proc->virt = virt;

	return proc;
}

void proc_free(proc_t *proc)
{
	if (proc == NULL) {
		return;
	}

	if (proc->virt) {
		str_free(&proc->buf);
		str_free(&proc->env);
		str_free(&proc->hostname);
	}
}

int proc_cmd(proc_t *proc, strv_t cmd)
{
	if (proc == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].cmd(proc, cmd);
}

strv_t proc_getenv(proc_t *proc, strv_t name)
{
	if (proc == NULL || !env_name_valid(name)) {
		return STRV_NULL;
	}

	return s_proc_ops[proc->virt].getenv(proc, name);
}

int proc_setenv(proc_t *proc, strv_t name, strv_t val, int overwrite)
{
	if (proc == NULL || !env_name_valid(name) || !env_val_valid(val)) {
		return 1;
	}

	return s_proc_ops[proc->virt].setenv(proc, name, val, overwrite);
}

int proc_unsetenv(proc_t *proc, strv_t name)
{
	if (proc == NULL || !env_name_valid(name)) {
		return 1;
	}

	return s_proc_ops[proc->virt].unsetenv(proc, name);
}

int proc_gethostname(proc_t *proc, char *name, size_t len)
{
	if (proc == NULL || name == NULL || len == 0) {
		return 1;
	}

	return s_proc_ops[proc->virt].gethostname(proc, name, len);
}
