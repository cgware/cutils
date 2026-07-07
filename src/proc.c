#include "proc.h"

#include "cproc.h"
#include "log.h"
#include "mem.h"

typedef int (*proc_cmd_fn)(proc_t *proc, strv_t cmd);
typedef int (*proc_gethostname_fn)(proc_t *proc, char *name, size_t len);
typedef strv_t (*proc_getenv_fn)(proc_t *proc, strv_t name);
typedef int (*proc_setenv_fn)(proc_t *proc, strv_t name, strv_t val, int overwrite);
typedef int (*proc_unsetenv_fn)(proc_t *proc, strv_t name);
typedef int (*proc_dlopen_fn)(proc_t *proc, strv_t name, void **lib);
typedef int (*proc_dlmain_fn)(proc_t *proc, void **lib);
typedef int (*proc_dlsym_fn)(proc_t *proc, void *lib, strv_t name, void **sym);
typedef int (*proc_dlclose_fn)(proc_t *proc, void *lib);

typedef struct proc_ops_s {
	proc_cmd_fn cmd;
	proc_gethostname_fn gethostname;
	proc_getenv_fn getenv;
	proc_setenv_fn setenv;
	proc_unsetenv_fn unsetenv;
	proc_dlopen_fn dlopen;
	proc_dlmain_fn dlmain;
	proc_dlsym_fn dlsym;
	proc_dlclose_fn dlclose;
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

static proc_dllib_t *vp_dllib_find(proc_t *proc, strv_t name)
{
	proc_dllib_t *dllib = NULL;
	uint i		    = 0;
	arr_foreach(&proc->dllibs, i, dllib)
	{
		if (!dllib->main && strv_eq(STRVS(dllib->name), name)) {
			return dllib;
		}
	}

	return NULL;
}

static proc_dllib_t *vp_dlmain_find(proc_t *proc)
{
	proc_dllib_t *dllib = NULL;
	uint i		    = 0;
	arr_foreach(&proc->dllibs, i, dllib)
	{
		if (dllib->main) {
			return dllib;
		}
	}

	return NULL;
}

static proc_dllib_t *vp_dllib_find_handle(proc_t *proc, void *handle)
{
	proc_dllib_t *dllib = NULL;
	uint i		    = 0;
	arr_foreach(&proc->dllibs, i, dllib)
	{
		if (dllib->name.data == handle) {
			return dllib;
		}
	}

	return NULL;
}

static proc_dlsym_t *vp_dlsym_find(proc_dllib_t *dllib, strv_t name)
{
	proc_dlsym_t *dlsym = NULL;
	uint i		    = 0;
	arr_foreach(&dllib->dlsyms, i, dlsym)
	{
		if (strv_eq(dlsym->name, name)) {
			return dlsym;
		}
	}

	return NULL;
}

static int vp_dllib_add(proc_t *proc, strv_t name, int main, proc_dllib_t **out)
{
	if (proc->dllibs.data == NULL && arr_init(&proc->dllibs, 1, sizeof(proc_dllib_t), ALLOC_STD) == NULL) {
		return 1;
	}

	proc_dllib_t dllib = {0};
	dllib.name	    = strn(name.data, name.len, name.len + 1);
	dllib.main	    = main;
	if (dllib.name.data == NULL) {
		return 1;
	}

	if (arr_init(&dllib.dlsyms, 1, sizeof(proc_dlsym_t), ALLOC_STD) == NULL) {
		str_free(&dllib.name);
		return 1;
	}

	if (arr_addv(&proc->dllibs, &dllib, NULL)) {
		arr_free(&dllib.dlsyms);
		str_free(&dllib.name);
		return 1;
	}

	if (out) {
		*out = arr_get(&proc->dllibs, proc->dllibs.cnt - 1);
	}

	return 0;
}

int op_dlopen(proc_t *proc, strv_t name, void **lib)
{
	(void)proc;
	return cproc_dlopen(name.data, lib) ? 1 : 0;
}

int vp_dlopen(proc_t *proc, strv_t name, void **lib)
{
	proc_dllib_t *dllib = vp_dllib_find(proc, name);
	if (dllib == NULL) {
		*lib = NULL;
		return 1;
	}

	*lib = dllib->name.data;
	return 0;
}

int op_dlmain(proc_t *proc, void **lib)
{
	(void)proc;
	return cproc_dlmain(lib) ? 1 : 0;
}

int vp_dlmain(proc_t *proc, void **lib)
{
	proc_dllib_t *dllib = vp_dlmain_find(proc);
	if (dllib == NULL) {
		*lib = NULL;
		return 1;
	}

	*lib = dllib->name.data;
	return 0;
}

int op_dlsym(proc_t *proc, void *lib, strv_t name, void **sym)
{
	(void)proc;
	return cproc_dlsym(lib, name.data, sym) ? 1 : 0;
}

int vp_dlsym(proc_t *proc, void *lib, strv_t name, void **sym)
{
	proc_dllib_t *dllib = vp_dllib_find_handle(proc, lib);
	if (dllib == NULL) {
		*sym = NULL;
		return 1;
	}

	proc_dlsym_t *dlsym = vp_dlsym_find(dllib, name);
	if (dlsym == NULL) {
		*sym = NULL;
		return 1;
	}

	*sym = dlsym->sym;
	return 0;
}

int op_dlclose(proc_t *proc, void *lib)
{
	(void)proc;
	return cproc_dlclose(lib) ? 1 : 0;
}

int vp_dlclose(proc_t *proc, void *lib)
{
	return vp_dllib_find_handle(proc, lib) == NULL;
}

static const proc_ops_t s_proc_ops[] = {
	[0] =
		{
			.cmd	     = op_cmd,
			.gethostname = op_gethostname,
			.getenv	     = op_getenv,
			.setenv	     = op_setenv,
			.unsetenv    = op_unsetenv,
			.dlopen	     = op_dlopen,
			.dlmain      = op_dlmain,
			.dlsym	     = op_dlsym,
			.dlclose     = op_dlclose,
		},
	[1] =
		{
			.cmd	     = vp_cmd,
			.gethostname = vp_gethostname,
			.getenv	     = vp_getenv,
			.setenv	     = vp_setenv,
			.unsetenv    = vp_unsetenv,
			.dlopen	     = vp_dlopen,
			.dlmain      = vp_dlmain,
			.dlsym	     = vp_dlsym,
			.dlclose     = vp_dlclose,
		},
};

proc_t *proc_init(proc_t *proc, size_t buf_size, int virt)
{
	if (proc == NULL) {
		return NULL;
	}

	if (virt) {
		if (buf_size > 0) {
			proc->buf = strz(buf_size);
			if (proc->buf.data == NULL) {
				return NULL;
			}
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
		if (proc->dllibs.data != NULL) {
			proc_dllib_t *dllib = NULL;
			uint i		    = 0;
			arr_foreach(&proc->dllibs, i, dllib)
			{
				str_free(&dllib->name);
				arr_free(&dllib->dlsyms);
			}
			arr_free(&proc->dllibs);
		}
	}
}

int proc_cmd(proc_t *proc, strv_t cmd)
{
	if (proc == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].cmd(proc, cmd);
}

int proc_gethostname(proc_t *proc, char *name, size_t len)
{
	if (proc == NULL || name == NULL || len == 0) {
		return 1;
	}

	return s_proc_ops[proc->virt].gethostname(proc, name, len);
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

int proc_setdlsym(proc_t *proc, strv_t lib, strv_t name, void *sym)
{
	if (proc == NULL || !proc->virt || lib.data == NULL || name.data == NULL || sym == NULL) {
		return 1;
	}

	proc_dllib_t *dllib = vp_dllib_find(proc, lib);
	if (dllib == NULL && vp_dllib_add(proc, lib, 0, &dllib)) {
		return 1;
	}

	proc_dlsym_t *dlsym = vp_dlsym_find(dllib, name);
	if (dlsym != NULL) {
		dlsym->sym = sym;
		return 0;
	}

	proc_dlsym_t add = {.name = name, .sym = sym};
	return arr_addv(&dllib->dlsyms, &add, NULL);
}

int proc_setdlmain(proc_t *proc, strv_t name, void *sym)
{
	if (proc == NULL || !proc->virt || name.data == NULL || sym == NULL) {
		return 1;
	}

	proc_dllib_t *dllib = vp_dlmain_find(proc);
	if (dllib == NULL && vp_dllib_add(proc, STRV(""), 1, &dllib)) {
		return 1;
	}

	proc_dlsym_t *dlsym = vp_dlsym_find(dllib, name);
	if (dlsym != NULL) {
		dlsym->sym = sym;
		return 0;
	}

	proc_dlsym_t add = {.name = name, .sym = sym};
	return arr_addv(&dllib->dlsyms, &add, NULL);
}

int proc_dlopen(proc_t *proc, strv_t name, void **lib)
{
	if (proc == NULL || name.data == NULL || lib == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].dlopen(proc, name, lib);
}

int proc_dlmain(proc_t *proc, void **lib)
{
	if (proc == NULL || lib == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].dlmain(proc, lib);
}

int proc_dlsym(proc_t *proc, void *lib, strv_t name, void **sym)
{
	if (proc == NULL || lib == NULL || name.data == NULL || sym == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].dlsym(proc, lib, name, sym);
}

int proc_dlclose(proc_t *proc, void *lib)
{
	if (proc == NULL || lib == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].dlclose(proc, lib);
}
