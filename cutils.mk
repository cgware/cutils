CUTILS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CUTILS_SRC := $(wildcard $(CUTILS_DIR)src/*.c)
CUTILS_INC := $(wildcard $(CUTILS_DIR)src/*.h)
CUTILS_INCLUDE := $(wildcard $(CUTILS_DIR)include/*.h)
CUTILS_INCLUDE += $(CTEST_INCLUDE)

CUTILS_OUTDIR := $(BUILDDIR)bin/cutils/$(ARCH)-$(CONFIG)
CUTILS_INTDIR := $(CUTILS_OUTDIR)/int/
CUTILS_OBJ := $(patsubst $(CUTILS_DIR)%.c,$(CUTILS_INTDIR)%.o,$(CUTILS_SRC))
CUTILS_GCDA := $(patsubst %.o,%.gcda,$(CUTILS_OBJ))

CUTILS_INCLUDES := -I$(CUTILS_DIR)src/ -I$(CUTILS_DIR)include/ -I$(SRCDIR)deps/cbase/include/

CUTILS_NAME := cutils.a
CUTILS := $(CUTILS_OUTDIR)/$(CUTILS_NAME)

.PHONY: cutils
cutils: $(CUTILS)

$(CUTILS): $(CTEST) $(CUTILS_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CUTILS_OBJ)

$(CUTILS_INTDIR)%.o: $(CUTILS_DIR)%.c $(CUTILS_INC) $(CUTILS_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CUTILS_INCLUDES) $(CFLAGS) -o $@ $<

CUTILS_TEST_SRC := $(wildcard $(CUTILS_DIR)test/*.c)
CUTILS_TEST_INC := $(wildcard $(CUTILS_DIR)test/*.h)
CUTILS_TEST_INCLUDE := $(wildcard include/*.h)
CUTILS_TEST_INCLUDE += $(CUTILS_INCLUDE)

CUTILS_TEST_OUTDIR := $(BUILDDIR)bin/cutils_test/$(ARCH)-$(CONFIG)
CUTILS_TEST_INTDIR := $(CUTILS_TEST_OUTDIR)/int/
CUTILS_TEST_OBJ := $(patsubst $(CUTILS_DIR)%.c,$(CUTILS_TEST_INTDIR)%.o,$(CUTILS_TEST_SRC))
CUTILS_TEST_GCDA := $(patsubst %.o,%.gcda,$(CUTILS_TEST_OBJ))

CUTILS_TEST_INCLUDES := -I$(CUTILS_DIR)test/ -I$(CUTILS_DIR)include/ -I$(SRCDIR)deps/cbase/include/ -I$(SRCDIR)deps/ctest/include/
CUTILS_TEST_LIBS := -L$(CUTILS_OUTDIR) -l:$(CUTILS_NAME) -L$(CTEST_OUTDIR) -l:$(CTEST_NAME) -L$(CBASE_OUTDIR) -l:$(CBASE_NAME)

CUTILS_TEST := $(CUTILS_TEST_OUTDIR)/cutils_test

.PHONY: cutils_test
cutils_test: $(CUTILS_TEST)
	@rm -rf $(CUTILS_GCDA) $(CUTILS_TEST_GCDA)
	@$(CUTILS_TEST)

$(CUTILS_TEST): $(CUTILS) $(CUTILS_TEST_OBJ)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CUTILS_TEST_OBJ) $(CUTILS_TEST_LIBS)

$(CUTILS_TEST_INTDIR)%.o: $(CUTILS_DIR)%.c $(CUTILS_TEST_INC) $(CUTILS_TEST_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CUTILS_TEST_INCLUDES) $(CFLAGS) -o $@ $<
