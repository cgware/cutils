#include "proc.h"

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
	EXPECT_EQ(proc_cmd(&op, STRV_NULL), 1);
	EXPECT_EQ(proc_cmd(&vp, STRV_NULL), 1);
	EXPECT_EQ(proc_cmd(&op, STRV("echo")), 0);
	EXPECT_EQ(proc_cmd(&vp, STRV("echo0")), 0);
	EXPECT_STRN(vp.buf.data, "echo0\n", vp.buf.len);
	EXPECT_EQ(proc_cmd(&vp, STRV("echo1")), 0);
	EXPECT_STRN(vp.buf.data,
		    "echo0\n"
		    "echo1\n",
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
