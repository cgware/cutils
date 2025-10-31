#include "proc.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(proc_init_free)
{
	START;

	proc_t proc = {0};

	EXPECT_EQ(proc_init(NULL, 0, 0), NULL);
	mem_oom(1);
	EXPECT_EQ(proc_init(&proc, 1, 1), NULL);
	mem_oom(0);
	EXPECT_EQ(proc_init(&proc, 1, 1), &proc);

	proc_free(NULL);
	proc_free(&proc);

	END;
}

TEST(proc_cmd)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 4, 1);

	EXPECT_EQ(proc_cmd(NULL, STRV_NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(proc_cmd(&op, STRV_NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(proc_cmd(&vp, STRV_NULL), 1);
	EXPECT_EQ(proc_cmd(&op, STRV("true")), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(proc_cmd(&op, STRV("false")), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(proc_cmd(&vp, STRV("true")), 0);
	EXPECT_STRN(vp.buf.data, "true\n", vp.buf.len);
	EXPECT_EQ(proc_cmd(&vp, STRV("false")), 0);
	EXPECT_STRN(vp.buf.data,
		    "true\n"
		    "false\n",
		    vp.buf.len);

	proc_free(&op);
	proc_free(&vp);

	END;
}

STEST(proc)
{
	SSTART;

	RUN(proc_init_free);
	RUN(proc_cmd);

	SEND;
}
