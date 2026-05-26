CC = clang

ifeq ($(OS),Windows_NT)
  _PF86        := $(shell printenv 'ProgramFiles(x86)' 2>/dev/null | tr -d '\r' | sed 's|\\\\|/|g')
  WASMER_ROOT  := $(_PF86)/Wasmer
  CFLAGS        = -g -D_CRT_SECURE_NO_WARNINGS "-I$(WASMER_ROOT)/include"
  LDFLAGS       =
  LDLIBS        = "$(WASMER_ROOT)/lib/wasmer.dll.lib"
  TARGET        = main.exe
  WAT2WASM_TOOL = wat2wasm_tool.exe
  RM            = rm -f
else
  CFLAGS        = -g -I$(shell $(WASMER_DIR)/bin/wasmer config --includedir)
  LDFLAGS       = -Wl,-rpath,$(shell $(WASMER_DIR)/bin/wasmer config --libdir)
  LDLIBS        = $(shell $(WASMER_DIR)/bin/wasmer config --libs)
  TARGET        = a.out
  WAT2WASM_TOOL = wat2wasm_tool
  RM            = rm -f
endif

$(TARGET): main.c add.wasm
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ main.c
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

add.wasm: add.wat $(WAT2WASM_TOOL)
	./$(WAT2WASM_TOOL) add.wat add.wasm

$(WAT2WASM_TOOL): wat2wasm_tool.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

.PHONY: clean
.SILENT: clean
clean:
	$(RM) $(TARGET) $(WAT2WASM_TOOL) add.wasm *.cwasm
