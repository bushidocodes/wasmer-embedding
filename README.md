# wasmer-embedding

Demonstrates three approaches to embedding a WebAssembly module in a C program
using the [Wasmer](https://wasmer.io) C API. The module is a trivial `sum(i32, i32) → i32`
function defined in [add.wat](add.wat).

## Approaches

| Source file | How the module is loaded |
|---|---|
| [main.c](main.c) | Reads `add.wasm` from disk, compiles it, and serializes the result to `add.cwasm` for fast reuse on subsequent runs. |
| [main_embed.c](main_embed.c) | The WAT source is embedded as a C string literal. No external files are needed at runtime. |
| [main_wat_external.c](main_wat_external.c) | Reads `add.wat` from disk and converts it to binary via `wat2wasm()` at runtime. |

## Prerequisites

- **clang**
- **Wasmer** — install with the official script (Linux/macOS) or winget (Windows):

```bash
# Linux / macOS
curl https://get.wasmer.io -sSfL | sh

# Windows
winget install Wasmer.Wasmer
```

## Building

### Linux / macOS

```bash
export WASMER_DIR="$HOME/.wasmer"
make
```

This builds `a.out` (the `main.c` example). `add.wasm` is generated from `add.wat`
as part of the build.

### Windows

```bat
make
```

Wasmer is located automatically via the `ProgramFiles(x86)` environment variable.
The build produces `main.exe` and copies `wasmer.dll` into the project directory.

### Building a specific example

```bash
make main_embed        # builds main_embed   (Linux) / main_embed.exe   (Windows)
make main_wat_external # builds main_wat_external (Linux) / main_wat_external.exe (Windows)
make all               # builds all three examples
```

## Running

```bash
# Linux
./a.out

# Windows
./main.exe
```

Expected output (all three examples produce the same result):

```
Creating the store...
Compiling module...
Creating imports...
Instantiating module...
Retrieving exports...
Retrieving the `sum` function...
Calling `sum` function...
Results of `sum`: 7
```

## Testing

```bash
make test
```

Builds and runs `test_sum` against `add.wasm` with six input cases covering
normal values, negative addends, and 32-bit integer boundaries.

```
PASS: sum(3, 4) == 7
PASS: sum(0, 0) == 0
PASS: sum(-1, 1) == 0
PASS: sum(10, -3) == 7
PASS: sum(INT32_MAX, 0) == INT32_MAX
PASS: sum(INT32_MIN, 0) == INT32_MIN

6/6 tests passed
```

## Project structure

```
add.wat              # WebAssembly Text source for the sum module
main.c               # Example 1: load .wasm binary with .cwasm caching
main_embed.c         # Example 2: WAT embedded as a C string literal
main_wat_external.c  # Example 3: load .wat file and convert at runtime
wat2wasm_tool.c      # Build utility: converts .wat → .wasm via the Wasmer C API
test_sum.c           # Unit tests for the sum function
Makefile
```

## CI

[![CI](https://github.com/bushidocodes/wasmer-embedding/actions/workflows/ci.yml/badge.svg)](https://github.com/bushidocodes/wasmer-embedding/actions/workflows/ci.yml)

Tests run automatically on every push and pull request via GitHub Actions
(`ubuntu-latest`, clang, Wasmer installed from the official script).
