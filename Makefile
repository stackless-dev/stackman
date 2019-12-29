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
	@echo "*** All test suites passed ***"

tests: bin/test
tests: LDLIBS := -lstackman

bin/test: tests/test.o bin/libstackman.a
	$(CC) $(LDFLAGS) -o $@ $< ${DEBUG} $(LDLIBS)
