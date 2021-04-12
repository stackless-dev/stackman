

CPPFLAGS += -Istackman $(PLATFORMFLAGS)
CFLAGS += -fPIC -g $(PLATFORMFLAGS)
CXXFLAGS += -fPIC -g $(PLATFORMFLAGS)
LDFLAGS += -L$(LIB) -g $(PLATFORMFLAGS)

# add flag to disable Intel CET
NO_CET := $(shell ./disable_cet $(CC))
CFLAGS += $(NO_CET)
CXXFLAGS += $(NO_CET)

OLDCC := $(CC)
ifdef PLATFORM_PREFIX
CC = $(PLATFORM_PREFIX)-gcc
CXX = $(PLATFORM_PREFIX)-g++
LD = $(PLATFORM_PREFIX)-ld
AR = $(PLATFORM_PREFIX)-ar
endif
# run c preprocessor with any cflags to get cross compilation result, then run regular compile in native
ABI := $(shell tools/abiname.sh "$(CC)" "$(CFLAGS)")
ifndef ABI
$(error Could not determine platform)
endif

LIB := lib/$(ABI)

all: $(LIB)/libstackman.a

# echo the abiname, for build tools.
.PHONY: abiname
abiname:
	@echo $(ABI)

obj = stackman/stackman.o stackman/stackman_s.o


$(LIB)/libstackman.a: lib $(obj)
	$(info ABI is $(ABI))
	$(AR) $(ARFLAGS) -s $@ $(obj)

.PHONY: lib clean
lib:
	mkdir -p $(LIB) bin

clean:
	rm -f stackman/*.o tests/*.o
	rm -f bin/* 
	rm -rf tmp tools/tmp

DEBUG = #-DDEBUG_DUMP

.PHONY: test tests

test: tests
	$(EMULATOR) bin/test
	$(EMULATOR) bin/test_cc
	$(EMULATOR) bin/test_static
	$(EMULATOR) bin/test_asm
	@echo "*** All test suites passed ***"

tests: bin/test
tests: bin/test_cc
tests: bin/test_static
tests: bin/test_asm
tests: LDLIBS := -lstackman

bin/test: tests/test.o $(LIB)/libstackman.a
	$(CC) $(LDFLAGS) -static -o $@ $< ${DEBUG} $(LDLIBS)

bin/test_cc: tests/test_cc.o $(LIB)/libstackman.a
	$(CXX) $(LDFLAGS) -static -o $@ $< ${DEBUG} $(LDLIBS)

bin/test_static: tests/test_static.o
	$(CC) $(LDFLAGS) -static -o $@ $^ ${DEBUG}

bin/test_asm: tests/test_asm.o tests/test_asm_s.o
	$(CC) $(LDFLAGS) -static -o $@ $^ ${DEBUG}
