#ifndef PROC_H
#define PROC_H

#include "str.h"

typedef struct proc_s {
	str_t buf;
	int virt;
} proc_t;

proc_t *proc_init(proc_t *proc, size_t buf_size, int virt);
void proc_free(proc_t *proc);

int proc_cmd(proc_t *proc, strv_t cmd);

#endif
