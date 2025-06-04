#include "proc.h"

#include <stdlib.h>

typedef int (*proc_cmd_fn)(proc_t *proc, strv_t cmd);

typedef struct proc_ops_s {
	proc_cmd_fn cmd;
} proc_ops_t;

int op_cmd(proc_t *proc, strv_t cmd)
{
	(void)proc;
	return system(cmd.data);
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

static const proc_ops_t s_proc_ops[] = {
	[0] =
		{
			.cmd = op_cmd,
		},
	[1] =
		{
			.cmd = vp_cmd,
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
	}
}

int proc_cmd(proc_t *proc, strv_t cmd)
{
	if (proc == NULL) {
		return 1;
	}

	return s_proc_ops[proc->virt].cmd(proc, cmd);
}
