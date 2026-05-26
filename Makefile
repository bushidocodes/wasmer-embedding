CC = clang

ifeq ($(OS),Windows_NT)
  ifdef WASMER_DIR
    # Explicit override — e.g. script-based install: iwr https://win.wasmer.io | iex
    WASMER_ROOT   := $(subst \,/,$(WASMER_DIR))
  else
    # Default: winget install location (winget install Wasmer.Wasmer)
    _PF86         := $(shell printenv 'ProgramFiles(x86)' 2>/dev/null | tr -d '\r' | sed 's|\\\\|/|g')
    WASMER_ROOT   := $(_PF86)/Wasmer
  endif
  CFLAGS           = -g -D_CRT_SECURE_NO_WARNINGS "-I$(WASMER_ROOT)/include"
  LDFLAGS          =
  LDLIBS           = "$(WASMER_ROOT)/lib/wasmer.dll.lib"
  TARGET           = main.exe
  EMBED_TARGET     = main_embed.exe
  WAT_EXT_TARGET   = main_wat_external.exe
  TEST_TARGET      = test_sum.exe
  WAT2WASM_TOOL    = wat2wasm_tool.exe
  RM               = rm -f
else
  CFLAGS           = -g -I$(shell $(WASMER_DIR)/bin/wasmer config --includedir)
  LDFLAGS          = -Wl,-rpath,$(shell $(WASMER_DIR)/bin/wasmer config --libdir)
  LDLIBS           = $(shell $(WASMER_DIR)/bin/wasmer config --libs)
  TARGET           = a.out
  EMBED_TARGET     = main_embed
  WAT_EXT_TARGET   = main_wat_external
  TEST_TARGET      = test_sum
  WAT2WASM_TOOL    = wat2wasm_tool
  RM               = rm -f
endif

.PHONY: all
all: $(TARGET) $(EMBED_TARGET) $(WAT_EXT_TARGET)

$(TARGET): main.c add.wasm
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ main.c
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

# Friendly aliases so `make main_embed` / `make main_wat_external` work on all platforms
.PHONY: main_embed main_wat_external
main_embed: $(EMBED_TARGET)
main_wat_external: $(WAT_EXT_TARGET)

$(EMBED_TARGET): main_embed.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

$(WAT_EXT_TARGET): main_wat_external.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

$(TEST_TARGET): test_sum.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

add.wasm: add.wat $(WAT2WASM_TOOL)
	./$(WAT2WASM_TOOL) add.wat add.wasm

$(WAT2WASM_TOOL): wat2wasm_tool.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
ifeq ($(OS),Windows_NT)
	cp "$(WASMER_ROOT)/lib/wasmer.dll" .
endif

.PHONY: test
test: $(TEST_TARGET) add.wasm
	./$(TEST_TARGET)

.PHONY: clean
.SILENT: clean
clean:
	$(RM) $(TARGET) $(EMBED_TARGET) $(WAT_EXT_TARGET) $(TEST_TARGET) $(WAT2WASM_TOOL) add.wasm *.cwasm
