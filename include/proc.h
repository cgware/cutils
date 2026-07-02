#ifndef PROC_H
#define PROC_H

#include "str.h"

typedef struct proc_s {
	str_t buf;
	str_t env;
	str_t hostname;
	int virt;
} proc_t;

proc_t *proc_init(proc_t *proc, size_t buf_size, int virt);
void proc_free(proc_t *proc);

int proc_cmd(proc_t *proc, strv_t cmd);
strv_t proc_getenv(proc_t *proc, strv_t name);
int proc_setenv(proc_t *proc, strv_t name, strv_t val, int overwrite);
int proc_unsetenv(proc_t *proc, strv_t name);
int proc_gethostname(proc_t *proc, char *name, size_t len);

#endif
