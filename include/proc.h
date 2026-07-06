#ifndef PROC_H
#define PROC_H

#include "arr.h"
#include "str.h"

typedef struct proc_dlsym_s {
	strv_t name;
	void *sym;
} proc_dlsym_t;

typedef struct proc_dllib_s {
	str_t name;
	arr_t dlsyms;
} proc_dllib_t;

typedef struct proc_s {
	str_t buf;
	str_t env;
	str_t hostname;
	arr_t dllibs;
	int virt;
} proc_t;

proc_t *proc_init(proc_t *proc, size_t buf_size, int virt);
void proc_free(proc_t *proc);

int proc_cmd(proc_t *proc, strv_t cmd);
strv_t proc_getenv(proc_t *proc, strv_t name);
int proc_setenv(proc_t *proc, strv_t name, strv_t val, int overwrite);
int proc_unsetenv(proc_t *proc, strv_t name);
int proc_gethostname(proc_t *proc, char *name, size_t len);
int proc_setdlsym(proc_t *proc, strv_t lib, strv_t name, void *sym);
int proc_dlopen(proc_t *proc, strv_t name, void **lib);
int proc_dlsym(proc_t *proc, void *lib, strv_t name, void **sym);
int proc_dlclose(proc_t *proc, void *lib);

#endif
