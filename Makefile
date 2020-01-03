CPPFLAGS += -Isrc
CFLAGS += -fPIC
LDFLAGS += -Lbin

all: bin/libstackman.a

obj = src/stackman.o src/stackman_s.o


bin/libstackman.a: bin $(obj)
	$(AR) $(ARFLAGS) -s $@ $(obj)

bin:
	mkdir bin

clean:
	rm -f src/*.o tests/*.o
	rm -f bin/*

DEBUG = #-DDEBUG_DUMP

.PHONY: test tests

test: tests
	bin/test
	bin/test_static
	bin/test_asm
	@echo "*** All test suites passed ***"

tests: bin/test
tests: bin/test_static
tests: bin/test_asm
tests: LDLIBS := -lstackman

bin/test: tests/test.o bin/libstackman.a
	$(CC) $(LDFLAGS) -o $@ $< ${DEBUG} $(LDLIBS)

bin/test_static: tests/test_static.o
	$(CC) $(LDFLAGS) -o $@ $^ ${DEBUG}

bin/test_asm: tests/test_asm.o tests/test_asm_s.o
	$(CC) $(LDFLAGS) -o $@ $^ ${DEBUG}
