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

	size_t size = 1;
	void *mem   = alloc_alloc(&alloc, size);

	EXPECT_EQ(alloc_realloc(&alloc, &mem, &size, 2), 0);
	EXPECT_EQ(size, 2);

	alloc_free(&alloc, mem, size);

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
