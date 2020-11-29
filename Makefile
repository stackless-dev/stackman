

CPPFLAGS += -Isrc $(PLATFORM)
CFLAGS += -fPIC -g $(PLATFORM)
CXXFLAGS += -fPIC -g $(PLATFORM)
LDFLAGS += -L$(LIB) -g $(PLATFORM)

# add flag to disable Intel CET
NO_CET := $(shell ./disable_cet $(CC))
CFLAGS += $(NO_CET)
CXXFLAGS += $(NO_CET)

# run c preprocessor with any cflags to get cross compilation result, then run regular compile in native
ABI := $(shell mkdir -p bin; $(CC) -E $(CFLAGS) $(CPPFLAGS) -o bin/get_abi.c get_abi.c && $(CC) -o bin/get_abi bin/get_abi.c && bin/get_abi)
ifndef ABI
$(error Could not determine platform)
else
$(info ABI is $(ABI))
endif

LIB := lib/$(ABI)

all: $(LIB)/libstackman.a

obj = src/stackman.o src/stackman_s.o


$(LIB)/libstackman.a: lib $(obj)
	$(AR) $(ARFLAGS) -s $@ $(obj)

.PHONY: lib clean
lib:
	mkdir -p $(LIB)

clean:
	rm -f src/*.o tests/*.o
	rm -f bin/*

DEBUG = #-DDEBUG_DUMP

.PHONY: test tests

test: tests
	bin/test
	bin/test_cc
	bin/test_static
	bin/test_asm
	@echo "*** All test suites passed ***"

tests: bin/test
tests: bin/test_cc
tests: bin/test_static
tests: bin/test_asm
tests: LDLIBS := -lstackman

bin/test: tests/test.o $(LIB)/libstackman.a
	$(CC) $(LDFLAGS) -o $@ $< ${DEBUG} $(LDLIBS)

bin/test_cc: tests/test_cc.o $(LIB)/libstackman.a
	$(CXX) $(LDFLAGS) -o $@ $< ${DEBUG} $(LDLIBS)

bin/test_static: tests/test_static.o
	$(CC) $(LDFLAGS) -o $@ $^ ${DEBUG}

bin/test_asm: tests/test_asm.o tests/test_asm_s.o
	$(CC) $(LDFLAGS) -o $@ $^ ${DEBUG}
