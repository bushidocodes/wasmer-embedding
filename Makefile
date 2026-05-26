CC = clang

ifeq ($(OS),Windows_NT)
  _PF86        := $(shell printenv 'ProgramFiles(x86)' 2>/dev/null | tr -d '\r' | sed 's|\\\\|/|g')
  WASMER_ROOT  := $(_PF86)/Wasmer
  CFLAGS        = -g -D_CRT_SECURE_NO_WARNINGS "-I$(WASMER_ROOT)/include"
  LDFLAGS       =
  LDLIBS        = "$(WASMER_ROOT)/lib/wasmer.dll.lib"
  TARGET        = main.exe
  RM            = rm -f
else
  CFLAGS        = -g -I$(shell $(WASMER_DIR)/bin/wasmer config --includedir)
  LDFLAGS       = -Wl,-rpath,$(shell $(WASMER_DIR)/bin/wasmer config --libdir)
  LDLIBS        = $(shell $(WASMER_DIR)/bin/wasmer config --libs)
  TARGET        = a.out
  RM            = rm -f
endif

$(TARGET): main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

.PHONY: clean
.SILENT: clean
clean:
	$(RM) $(TARGET) main.o *.cwasm
