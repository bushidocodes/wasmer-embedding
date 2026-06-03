#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "wasmer.h"

static int tests_run    = 0;
static int tests_failed = 0;

#define CHECK(cond, label) do { \
    tests_run++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", (label)); \
        tests_failed++; \
    } else { \
        printf("PASS: %s\n", (label)); \
    } \
} while (0)

static int32_t call_sum(wasm_func_t *sum, int32_t a, int32_t b)
{
    wasm_val_t args[2]   = {WASM_I32_VAL(a), WASM_I32_VAL(b)};
    wasm_val_t result[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args_vec   = WASM_ARRAY_VEC(args);
    wasm_val_vec_t result_vec = WASM_ARRAY_VEC(result);
    if (wasm_func_call(sum, &args_vec, &result_vec))
        return INT32_MIN;
    return result[0].of.i32;
}

int main(void)
{
    FILE *fp = fopen("add.wasm", "rb");
    if (!fp) { fprintf(stderr, "cannot open add.wasm\n"); return 1; }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_size <= 0) { fprintf(stderr, "bad file size\n"); fclose(fp); return 1; }

    wasm_byte_t *buf = malloc((size_t)file_size);
    if (!buf) { fprintf(stderr, "out of memory\n"); fclose(fp); return 1; }
    size_t len = fread(buf, 1, (size_t)file_size, fp);
    fclose(fp);

    wasm_engine_t   *engine   = wasm_engine_new();
    wasm_store_t    *store    = wasm_store_new(engine);
    wasm_byte_vec_t  bytes;
    wasm_byte_vec_new(&bytes, len, buf);
    free(buf);
    wasm_module_t   *module   = wasm_module_new(store, &bytes);
    wasm_byte_vec_delete(&bytes);

    if (!module) { fprintf(stderr, "failed to compile module\n"); return 1; }

    wasm_extern_vec_t imports  = WASM_EMPTY_VEC;
    wasm_instance_t  *instance = wasm_instance_new(store, module, &imports, NULL);
    if (!instance) { fprintf(stderr, "failed to instantiate module\n"); return 1; }

    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    wasm_func_t *sum = wasm_extern_as_func(exports.data[0]);

    CHECK(call_sum(sum,  3,  4) ==  7,  "sum(3, 4) == 7");
    CHECK(call_sum(sum,  0,  0) ==  0,  "sum(0, 0) == 0");
    CHECK(call_sum(sum, -1,  1) ==  0,  "sum(-1, 1) == 0");
    CHECK(call_sum(sum, 10, -3) ==  7,  "sum(10, -3) == 7");
    CHECK(call_sum(sum, INT32_MAX, 0) == INT32_MAX, "sum(INT32_MAX, 0) == INT32_MAX");
    CHECK(call_sum(sum, INT32_MIN, 0) == INT32_MIN, "sum(INT32_MIN, 0) == INT32_MIN");

    wasm_extern_vec_delete(&exports);
    wasm_instance_delete(instance);
    wasm_module_delete(module);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    printf("\n%d/%d tests passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
