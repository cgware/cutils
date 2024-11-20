CUTILS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CUTILS_SRC := $(CUTILS_DIR)src/
CUTILS_C := $(wildcard $(CUTILS_SRC)*.c)
CUTILS_H := $(wildcard $(CUTILS_SRC)*.h)
CUTILS_HEADERS := $(wildcard $(CUTILS_DIR)include/*.h)
CUTILS_HEADERS += $(CBASE_HEADERS)

CUTILS_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/cutils
CUTILS_INTDIR := $(CUTILS_OUTDIR)/int/
CUTILS_OBJ := $(patsubst $(CUTILS_SRC)%.c,$(CUTILS_INTDIR)%.o,$(CUTILS_C))
CUTILS_GCDA := $(patsubst %.o,%.gcda,$(CUTILS_OBJ))

CUTILS_INCLUDES := -I$(CUTILS_DIR)include/ $(CBASE_INCLUDES)

CUTILS := $(CUTILS_OUTDIR)/cutils.a

.PHONY: cutils
cutils: $(CUTILS)

$(CUTILS): $(CUTILS_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CUTILS_OBJ)

$(CUTILS_INTDIR)%.o: $(CUTILS_SRC)%.c $(CUTILS_H) $(CUTILS_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CUTILS_SRC) $(CUTILS_INCLUDES) $(CFLAGS) -o $@ $<

CUTILS_TEST_SRC := $(CUTILS_DIR)test/
CUTILS_TEST_C := $(wildcard $(CUTILS_TEST_SRC)*.c)
CUTILS_TEST_H := $(wildcard $(CUTILS_TEST_SRC)*.h)
CUTILS_TEST_HEADERS := $(CUTILS_HEADERS) $(CTEST_HEADERS)

CUTILS_TEST_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/cutils_test
CUTILS_TEST_INTDIR := $(CUTILS_TEST_OUTDIR)/int/
CUTILS_TEST_OBJ := $(patsubst $(CUTILS_TEST_SRC)%.c,$(CUTILS_TEST_INTDIR)%.o,$(CUTILS_TEST_C))
CUTILS_TEST_GCDA := $(patsubst %.o,%.gcda,$(CUTILS_TEST_OBJ))

CUTILS_TEST_INCLUDES := $(CUTILS_INCLUDES) $(CTEST_INCLUDES) 
CUTILS_TEST_LIBS := $(CUTILS) $(CTEST) $(CBASE)

CUTILS_TEST := $(CUTILS_TEST_OUTDIR)/cutils_test

.PHONY: cutils_test
cutils_test: $(CUTILS_TEST)
	@rm -rf $(CUTILS_GCDA) $(CUTILS_TEST_GCDA)
	@$(CUTILS_TEST)

$(CUTILS_TEST): $(CUTILS_TEST_OBJ) $(CUTILS_TEST_LIBS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CUTILS_TEST_OBJ) $(patsubst %,-L%,$(dir $(CUTILS_TEST_LIBS))) $(patsubst %,-l:%,$(notdir $(CUTILS_TEST_LIBS)))

$(CUTILS_TEST_INTDIR)%.o: $(CUTILS_TEST_SRC)%.c $(CUTILS_TEST_H) $(CUTILS_TEST_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CUTILS_TEST_SRC) $(CUTILS_TEST_INCLUDES) $(CFLAGS) -o $@ $<
