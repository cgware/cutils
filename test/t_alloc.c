#include "alloc.h"

#include "test.h"

TEST(alloc_alloc)
{
	START;

	alloc_t alloc = ALLOC_STD;

	void *mem = alloc_alloc(&alloc, 1);

	alloc_free(&alloc, mem, 1);

	END;
}

TEST(alloc_realloc)
{
	START;

	alloc_t alloc = ALLOC_STD;

	void *mem = alloc_alloc(&alloc, 1);

	mem = alloc_realloc(&alloc, mem, 1, 2);

	alloc_free(&alloc, mem, 2);

	END;
}

TEST(alloc_free)
{
	START;

	alloc_t alloc = ALLOC_STD;

	void *mem = alloc_alloc(&alloc, 1);

	alloc_free(&alloc, mem, 1);

	END;
}

STEST(alloc)
{
	SSTART;

	RUN(alloc_alloc);
	RUN(alloc_realloc);
	RUN(alloc_free);

	SEND;
}
