#include <stdio.h>
#include <string.h>
#include "wasmer.h"

int main(int argc, const char *argv[])
{
    const char *wat_string =
        "(module\n"
        "  (type $sum_t (func (param i32 i32) (result i32)))\n"
        "  (func $sum_f (type $sum_t) (param $x i32) (param $y i32) (result i32)\n"
        "    local.get $x\n"
        "    local.get $y\n"
        "    i32.add)\n"
        "  (export \"sum\" (func $sum_f)))";

    wasm_byte_vec_t wat;
    wasm_byte_vec_new(&wat, strlen(wat_string), wat_string);
    wasm_byte_vec_t wasm_bytes;
    wat2wasm(&wat, &wasm_bytes);
    wasm_byte_vec_delete(&wat);

    printf("Creating the store...\n");
    wasm_engine_t *engine = wasm_engine_new();
    wasm_store_t *store = wasm_store_new(engine);

    int rc = 0;
    wasm_module_t *module = NULL;
    wasm_instance_t *instance = NULL;
    wasm_extern_vec_t exports = WASM_EMPTY_VEC;

    printf("Compiling module...\n");
    module = wasm_module_new(store, &wasm_bytes);
    wasm_byte_vec_delete(&wasm_bytes);

    if (!module)
    {
        fprintf(stderr, "> Error compiling module!\n");
        rc = 1;
        goto cleanup;
    }

    printf("Creating imports...\n");
    wasm_extern_vec_t import_object = WASM_EMPTY_VEC;

    printf("Instantiating module...\n");
    instance = wasm_instance_new(store, module, &import_object, NULL);

    if (!instance)
    {
        fprintf(stderr, "> Error instantiating module!\n");
        rc = 1;
        goto cleanup;
    }

    printf("Retrieving exports...\n");
    wasm_instance_exports(instance, &exports);

    if (exports.size == 0)
    {
        fprintf(stderr, "> Error accessing exports!\n");
        rc = 1;
        goto cleanup;
    }

    printf("Retrieving the `sum` function...\n");
    wasm_func_t *sum_func = wasm_extern_as_func(exports.data[0]);

    if (sum_func == NULL)
    {
        fprintf(stderr, "> Failed to get the `sum` function!\n");
        rc = 1;
        goto cleanup;
    }

    printf("Calling `sum` function...\n");
    wasm_val_t args_val[2] = {WASM_I32_VAL(3), WASM_I32_VAL(4)};
    wasm_val_t results_val[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
    wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

    if (wasm_func_call(sum_func, &args, &results))
    {
        fprintf(stderr, "> Error calling the `sum` function!\n");
        rc = 1;
        goto cleanup;
    }

    printf("Results of `sum`: %d\n", results_val[0].of.i32);

cleanup:
    wasm_extern_vec_delete(&exports);
    if (instance)
        wasm_instance_delete(instance);
    if (module)
        wasm_module_delete(module);
    wasm_store_delete(store);
    wasm_engine_delete(engine);
    return rc;
}
