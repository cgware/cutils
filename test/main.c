#include "file.h"
#include "log.h"
#include "mem.h"
#include "test.h"

STEST(alloc);
STEST(args);
STEST(arr);
STEST(cstr);
STEST(ctime);
STEST(dict);
STEST(file);
STEST(list);
STEST(log);
STEST(mem);
STEST(path);
STEST(str);
STEST(tree);
STEST(type);

TEST(cutils)
{
	SSTART;
	RUN(alloc);
	RUN(args);
	RUN(arr);
	RUN(cstr);
	RUN(ctime);
	RUN(dict);
	RUN(file);
	RUN(list);
	RUN(log);
	RUN(mem);
	RUN(path);
	RUN(str);
	RUN(tree);
	RUN(type);
	SEND;
}

int main()
{
	log_t log = {0};
	log_set(&log);
	log_add_callback(log_std_cb, PRINT_DST_FILE(stderr), LOG_WARN, 1, 1);

	t_init();

	t_run(test_cutils, 1);

	int ret = t_finish();

	mem_print(PRINT_DST_STD());

	if (mem_check()) {
		ret = 1;
	}

	return ret;
}
