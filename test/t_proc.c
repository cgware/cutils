#include "proc.h"

#include "log.h"
#include "mem.h"
#include "platform.h"
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

TEST(proc_getenv_invalid)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_getenv(NULL, STRV("A")).data, NULL);
	EXPECT_EQ(proc_getenv(&proc, STRV_NULL).data, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_getenv_vp_found)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);
	proc_setenv(&proc, STRV("A"), STRV("1"), 1);
	proc_setenv(&proc, STRV("B"), STRV("2"), 1);

	strv_t val = proc_getenv(&proc, STRV("B"));
	EXPECT_STRN(val.data, "2", val.len);

	proc_free(&proc);

	END;
}

TEST(proc_getenv_vp_not_found)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);
	proc.env = STR("A=1\nB\n");

	EXPECT_EQ(proc_getenv(&proc, STRV("B")).data, NULL);
	EXPECT_EQ(proc_getenv(&proc, STRV("C")).data, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_getenv_op_vp_found)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 1, 1);

	proc_setenv(&op, STRV("CUTILS_PROC_TEST"), STRV("value"), 1);
	proc_setenv(&vp, STRV("CUTILS_PROC_TEST"), STRV("value"), 1);

	strv_t oval = proc_getenv(&op, STRV("CUTILS_PROC_TEST"));
	strv_t vval = proc_getenv(&vp, STRV("CUTILS_PROC_TEST"));
	EXPECT_EQ(strv_eq(oval, vval), 1);

	proc_free(&op);
	proc_free(&vp);

	END;
}

TEST(proc_getenv_op_vp_not_found)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 1, 1);

	proc_unsetenv(&op, STRV("CUTILS_PROC_TEST_NONE"));
	proc_unsetenv(&vp, STRV("CUTILS_PROC_TEST_NONE"));

	strv_t oval = proc_getenv(&op, STRV("CUTILS_PROC_TEST_NONE"));
	strv_t vval = proc_getenv(&vp, STRV("CUTILS_PROC_TEST_NONE"));
	EXPECT_EQ(strv_eq(oval, vval), 1);

	proc_free(&op);
	proc_free(&vp);

	END;
}

TEST(proc_setenv_invalid)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	EXPECT_EQ(proc_setenv(NULL, STRV("A"), STRV("1"), 1), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV_NULL, STRV("1"), 1), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV(""), STRV("1"), 1), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A=B"), STRV("1"), 1), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV_NULL, 1), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("1\n"), 1), 1);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_vp_set)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("1"), 1), 0);
	strv_t val = proc_getenv(&proc, STRV("A"));
	EXPECT_STRN(val.data, "1", val.len);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_vp_overwrite)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	proc_setenv(&proc, STRV("A"), STRV("1"), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("2"), 1), 0);
	strv_t val = proc_getenv(&proc, STRV("A"));
	EXPECT_STRN(val.data, "2", val.len);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_vp_no_overwrite)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	proc_setenv(&proc, STRV("A"), STRV("1"), 1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("2"), 0), 0);
	strv_t val = proc_getenv(&proc, STRV("A"));
	EXPECT_STRN(val.data, "1", val.len);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_vp_oom_set)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	mem_oom(1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("1"), 1), 1);
	mem_oom(0);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_vp_oom_overwrite)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);
	proc_setenv(&proc, STRV("A"), STRV("1"), 1);

	mem_oom(1);
	EXPECT_EQ(proc_setenv(&proc, STRV("A"), STRV("2"), 1), 1);
	mem_oom(0);

	proc_free(&proc);

	END;
}

TEST(proc_setenv_op_vp)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 1, 1);

	proc_unsetenv(&op, STRV("CUTILS_PROC_SETENV"));
	proc_unsetenv(&vp, STRV("CUTILS_PROC_SETENV"));

	EXPECT_EQ(proc_setenv(&op, STRV("CUTILS_PROC_SETENV"), STRV("value"), 1),
		  proc_setenv(&vp, STRV("CUTILS_PROC_SETENV"), STRV("value"), 1));
	EXPECT_EQ(strv_eq(proc_getenv(&op, STRV("CUTILS_PROC_SETENV")), proc_getenv(&vp, STRV("CUTILS_PROC_SETENV"))), 1);

	proc_free(&op);
	proc_free(&vp);

	END;
}

TEST(proc_unsetenv_invalid)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	EXPECT_EQ(proc_unsetenv(NULL, STRV("A")), 1);
	EXPECT_EQ(proc_unsetenv(&proc, STRV_NULL), 1);
	EXPECT_EQ(proc_unsetenv(&proc, STRV("")), 1);
	EXPECT_EQ(proc_unsetenv(&proc, STRV("A=B")), 1);

	proc_free(&proc);

	END;
}

TEST(proc_unsetenv_vp_set)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	proc_setenv(&proc, STRV("A"), STRV("1"), 1);
	EXPECT_EQ(proc_unsetenv(&proc, STRV("A")), 0);
	EXPECT_EQ(proc_getenv(&proc, STRV("A")).data, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_unsetenv_vp_not_found)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 1, 1);

	EXPECT_EQ(proc_unsetenv(&proc, STRV("A")), 0);

	proc_free(&proc);

	END;
}

TEST(proc_unsetenv_op_vp)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 1, 1);

	proc_setenv(&op, STRV("CUTILS_PROC_UNSETENV"), STRV("value"), 1);
	proc_setenv(&vp, STRV("CUTILS_PROC_UNSETENV"), STRV("value"), 1);

	EXPECT_EQ(proc_unsetenv(&op, STRV("CUTILS_PROC_UNSETENV")), proc_unsetenv(&vp, STRV("CUTILS_PROC_UNSETENV")));
	EXPECT_EQ(strv_eq(proc_getenv(&op, STRV("CUTILS_PROC_UNSETENV")), proc_getenv(&vp, STRV("CUTILS_PROC_UNSETENV"))), 1);

	proc_free(&op);
	proc_free(&vp);

	END;
}

TEST(proc_gethostname_invalid)
{
	START;

	proc_t proc = {0};
	char name[1] = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_gethostname(NULL, name, sizeof(name)), 1);
	EXPECT_EQ(proc_gethostname(&proc, NULL, sizeof(name)), 1);
	EXPECT_EQ(proc_gethostname(&proc, name, 0), 1);

	proc_free(&proc);

	END;
}

TEST(proc_gethostname_vp_copy)
{
	START;

	proc_t proc = {0};
	char name[5] = {0};
	proc_init(&proc, 1, 1);
	proc.hostname = STR("host");

	EXPECT_EQ(proc_gethostname(&proc, name, sizeof(name)), 0);
	EXPECT_STR(name, "host");

	proc_free(&proc);

	END;
}

TEST(proc_gethostname_vp_too_small)
{
	START;

	proc_t proc = {0};
	char name[4] = {0};
	proc_init(&proc, 1, 1);
	proc.hostname = STR("host");

	EXPECT_EQ(proc_gethostname(&proc, name, sizeof(name)), 1);

	proc_free(&proc);

	END;
}

TEST(proc_gethostname_op_vp)
{
	START;

	proc_t op = {0};
	proc_t vp = {0};
	char oname[256] = {0};
	char vname[256] = {0};
	proc_init(&op, 0, 0);
	proc_init(&vp, 1, 1);

	EXPECT_EQ(proc_gethostname(&op, oname, sizeof(oname)), 0);
	vp.hostname = strn(oname, strv_cstr(oname).len, sizeof(oname));
	EXPECT_EQ(proc_gethostname(&vp, vname, sizeof(vname)), 0);
	EXPECT_STR(vname, oname);

	proc_free(&op);
	proc_free(&vp);

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
#ifdef C_WIN
	EXPECT_EQ(proc_cmd(&op, STRV("cmd /c exit 0")), 0);
#else
	EXPECT_EQ(proc_cmd(&op, STRV("true")), 0);
#endif
	log_set_quiet(0, 1);
#ifdef C_WIN
	EXPECT_EQ(proc_cmd(&op, STRV("cmd /c exit 1")), 1);
#else
	EXPECT_EQ(proc_cmd(&op, STRV("false")), 1);
#endif
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
	RUN(proc_getenv_invalid);
	RUN(proc_getenv_vp_found);
	RUN(proc_getenv_vp_not_found);
	RUN(proc_getenv_op_vp_found);
	RUN(proc_getenv_op_vp_not_found);
	RUN(proc_setenv_invalid);
	RUN(proc_setenv_vp_set);
	RUN(proc_setenv_vp_overwrite);
	RUN(proc_setenv_vp_no_overwrite);
	RUN(proc_setenv_vp_oom_set);
	RUN(proc_setenv_vp_oom_overwrite);
	RUN(proc_setenv_op_vp);
	RUN(proc_unsetenv_invalid);
	RUN(proc_unsetenv_vp_set);
	RUN(proc_unsetenv_vp_not_found);
	RUN(proc_unsetenv_op_vp);
	RUN(proc_gethostname_invalid);
	RUN(proc_gethostname_vp_copy);
	RUN(proc_gethostname_vp_too_small);
	RUN(proc_gethostname_op_vp);

	SEND;
}
