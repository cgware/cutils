#include "log.h"
#include "mem.h"
#include "test.h"

STEST(alloc);
STEST(args);
STEST(arr);
STEST(buf);
STEST(dict);
STEST(fs);
STEST(list);
STEST(loc);
STEST(log);
STEST(mem);
STEST(path);
STEST(proc);
STEST(str);
STEST(strbuf);
STEST(strv);
STEST(strvbuf);
STEST(tree);
STEST(type);

TEST(cutils)
{
	SSTART;
	RUN(alloc);
	RUN(args);
	RUN(arr);
	RUN(buf);
	RUN(dict);
	RUN(fs);
	RUN(list);
	RUN(loc);
	RUN(log);
	RUN(mem);
	RUN(path);
	RUN(proc);
	RUN(str);
	RUN(strbuf);
	RUN(strv);
	RUN(strvbuf);
	RUN(tree);
	RUN(type);
	SEND;
}

int main()
{
	c_print_init();

	log_t log = {0};
	log_set(&log);
	log_add_callback(log_std_cb, DST_STD(), LOG_WARN, 1, 1);

	t_init();

	t_run(test_cutils, 1);

	int ret = t_finish();

	mem_print(DST_STD());

	if (mem_check()) {
		ret = 1;
	}

	return ret;
}
