#include "proc.h"

#include "log.h"
#include "mem.h"
#include "platform.h"
#include "test.h"

typedef int (*proc_test_dlsym_fn)(void);

static int proc_test_dlsym_impl(void)
{
	return 7;
}

static proc_test_dlsym_fn proc_test_dlsym = proc_test_dlsym_impl;

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

TEST(proc_setdlsym_rejects_op)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym), 1);

	proc_free(&proc);

	END;
}

TEST(proc_setdlsym_adds_vp_symbol)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 1);

	EXPECT_EQ(proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym), 0);

	proc_free(&proc);

	END;
}

TEST(proc_setdlmain_rejects_op)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_setdlmain(&proc, STRV("sym"), &proc_test_dlsym), 1);

	proc_free(&proc);

	END;
}

TEST(proc_setdlmain_adds_vp_symbol)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 1);

	EXPECT_EQ(proc_setdlmain(&proc, STRV("sym"), &proc_test_dlsym), 0);

	proc_free(&proc);

	END;
}

TEST(proc_dlopen_null_proc)
{
	START;

	void *lib = NULL;

	EXPECT_EQ(proc_dlopen(NULL, STRV("lib"), &lib), 1);

	END;
}

TEST(proc_dlopen_null_name)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlopen(&proc, STRV_NULL, &lib), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlopen_null_lib)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlopen(&proc, STRV("lib"), NULL), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlopen_vp_not_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 1);

	EXPECT_EQ(proc_dlopen(&proc, STRV("missing"), &lib), 1);
	EXPECT_EQ(lib, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_dlopen_vp_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym);

	EXPECT_EQ(proc_dlopen(&proc, STRV("lib"), &lib), 0);
	EXPECT_NE(lib, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_dlmain_null_proc)
{
	START;

	void *lib = NULL;

	EXPECT_EQ(proc_dlmain(NULL, &lib), 1);

	END;
}

TEST(proc_dlmain_null_lib)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlmain(&proc, NULL), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlmain_vp_not_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 1);

	EXPECT_EQ(proc_dlmain(&proc, &lib), 1);
	EXPECT_EQ(lib, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_dlmain_vp_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlmain(&proc, STRV("sym"), &proc_test_dlsym);

	EXPECT_EQ(proc_dlmain(&proc, &lib), 0);
	EXPECT_NE(lib, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_null_proc)
{
	START;

	void *sym = NULL;

	EXPECT_EQ(proc_dlsym(NULL, (void *)"lib", STRV("sym"), &sym), 1);

	END;
}

TEST(proc_dlsym_null_lib)
{
	START;

	proc_t proc = {0};
	void *sym   = NULL;
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlsym(&proc, NULL, STRV("sym"), &sym), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_null_name)
{
	START;

	proc_t proc = {0};
	void *sym   = NULL;
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlsym(&proc, (void *)"lib", STRV_NULL, &sym), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_null_sym)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlsym(&proc, (void *)"lib", STRV("sym"), NULL), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_vp_not_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	void *sym   = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym);
	proc_dlopen(&proc, STRV("lib"), &lib);

	EXPECT_EQ(proc_dlsym(&proc, lib, STRV("missing"), &sym), 1);
	EXPECT_EQ(sym, NULL);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_vp_found)
{
	START;

	proc_t proc	       = {0};
	void *lib	       = NULL;
	void *sym	       = NULL;
	proc_test_dlsym_fn *fn = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym);
	proc_dlopen(&proc, STRV("lib"), &lib);

	EXPECT_EQ(proc_dlsym(&proc, lib, STRV("sym"), &sym), 0);
	fn = sym;
	EXPECT_EQ((*fn)(), 7);

	proc_free(&proc);

	END;
}

TEST(proc_dlsym_vp_main_found)
{
	START;

	proc_t proc	       = {0};
	void *lib	       = NULL;
	void *sym	       = NULL;
	proc_test_dlsym_fn *fn = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlmain(&proc, STRV("sym"), &proc_test_dlsym);
	proc_dlmain(&proc, &lib);

	EXPECT_EQ(proc_dlsym(&proc, lib, STRV("sym"), &sym), 0);
	fn = sym;
	EXPECT_EQ((*fn)(), 7);

	proc_free(&proc);

	END;
}

TEST(proc_dlclose_null_proc)
{
	START;

	EXPECT_EQ(proc_dlclose(NULL, (void *)"lib"), 1);

	END;
}

TEST(proc_dlclose_null_lib)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 0);

	EXPECT_EQ(proc_dlclose(&proc, NULL), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlclose_vp_not_found)
{
	START;

	proc_t proc = {0};
	proc_init(&proc, 0, 1);

	EXPECT_EQ(proc_dlclose(&proc, (void *)"missing"), 1);

	proc_free(&proc);

	END;
}

TEST(proc_dlclose_vp_found)
{
	START;

	proc_t proc = {0};
	void *lib   = NULL;
	proc_init(&proc, 0, 1);
	proc_setdlsym(&proc, STRV("lib"), STRV("sym"), &proc_test_dlsym);
	proc_dlopen(&proc, STRV("lib"), &lib);

	EXPECT_EQ(proc_dlclose(&proc, lib), 0);

	proc_free(&proc);

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
	RUN(proc_setdlsym_rejects_op);
	RUN(proc_setdlsym_adds_vp_symbol);
	RUN(proc_setdlmain_rejects_op);
	RUN(proc_setdlmain_adds_vp_symbol);
	RUN(proc_dlopen_null_proc);
	RUN(proc_dlopen_null_name);
	RUN(proc_dlopen_null_lib);
	RUN(proc_dlopen_vp_not_found);
	RUN(proc_dlopen_vp_found);
	RUN(proc_dlmain_null_proc);
	RUN(proc_dlmain_null_lib);
	RUN(proc_dlmain_vp_not_found);
	RUN(proc_dlmain_vp_found);
	RUN(proc_dlsym_null_proc);
	RUN(proc_dlsym_null_lib);
	RUN(proc_dlsym_null_name);
	RUN(proc_dlsym_null_sym);
	RUN(proc_dlsym_vp_not_found);
	RUN(proc_dlsym_vp_found);
	RUN(proc_dlsym_vp_main_found);
	RUN(proc_dlclose_null_proc);
	RUN(proc_dlclose_null_lib);
	RUN(proc_dlclose_vp_not_found);
	RUN(proc_dlclose_vp_found);

	SEND;
}
