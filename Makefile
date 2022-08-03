CC = clang
CFLAGS = -g -I$(shell $(WASMER_DIR)/bin/wasmer config --includedir)
LDFLAGS = -Wl,-rpath,$(shell $(WASMER_DIR)/bin/wasmer config --libdir)
LDLIBS = $(shell $(WASMER_DIR)/bin/wasmer config --libs)

a.out: main.c
	clang ${CFLAGS} ${LDFLAGS} ${LDLIBS} -o $@ $^

.PHONY: clean
.SILENT: clean
clean:
	rm -f a.out main.o
