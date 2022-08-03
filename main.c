#include <stdio.h>
#include <stdlib.h>
#include "wasmer.h"

int main(int argc, const char *argv[])
{
    printf("Creating the store...\n");
    wasm_engine_t *engine = wasm_engine_new();
    wasm_store_t *store = wasm_store_new(engine);

    wasm_byte_t buf[BUFSIZ];
    size_t buf_len = 0;

    wasm_module_t *module = NULL;

    // Try to open add.cwasm
    FILE *fp = fopen("add.cwasm", "rb");
    if (fp == NULL)
    {
        // Assumption: No such file or directory
        // If we don't find add.cwasm, try to open add.wasm, compile it, and then serialize the result as add.cwasm
        fp = fopen("add.wasm", "rb");
        if (fp == NULL)
        {
            exit(EXIT_FAILURE);
        }
        size_t nread = 0;

        while ((nread = fread(&buf[buf_len], sizeof(wasm_byte_t), BUFSIZ - buf_len, fp)) != 0)
            buf_len += nread;

        wasm_byte_vec_t wasm_bytes;
        wasm_byte_vec_new(&wasm_bytes, buf_len, buf);

        printf("Compiling module...\n");
        module = wasm_module_new(store, &wasm_bytes);

        wasm_byte_vec_delete(&wasm_bytes);
        fclose(fp);

        wasm_byte_vec_t serialized_module = {
            .data = NULL,
            .size = 0};
        wasm_module_serialize(module, &serialized_module);

        fp = fopen("add.cwasm", "wb");

        size_t serialized_module_cursor = 0;

        while (serialized_module_cursor < serialized_module.size)
        {
            size_t nwritten = fwrite(&serialized_module.data[serialized_module_cursor], sizeof(char), serialized_module.size - serialized_module_cursor, fp);
            serialized_module_cursor += nwritten;
        }

        fflush(fp);
        fclose(fp);
    }
    else
    {
        size_t nread = 0;

        while ((nread = fread(&buf[buf_len], sizeof(wasm_byte_t), BUFSIZ - buf_len, fp)) != 0)
            buf_len += nread;

        wasm_byte_vec_t cwasm_bytes;
        wasm_byte_vec_new(&cwasm_bytes, buf_len, buf);

        module = wasm_module_deserialize(store, &cwasm_bytes);
    }

    if (!module)
    {
        printf("> Error compiling module!\n");
        return 1;
    }

    printf("Creating imports...\n");
    wasm_extern_vec_t import_object = WASM_EMPTY_VEC;

    printf("Instantiating module...\n");
    wasm_instance_t *instance = wasm_instance_new(store, module, &import_object, NULL);

    if (!instance)
    {
        printf("> Error instantiating module!\n");
        return 1;
    }

    printf("Retrieving exports...\n");
    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);

    if (exports.size == 0)
    {
        printf("> Error accessing exports!\n");
        return 1;
    }

    printf("Retrieving the `sum` function...\n");
    wasm_func_t *sum_func = wasm_extern_as_func(exports.data[0]);

    if (sum_func == NULL)
    {
        printf("> Failed to get the `sum` function!\n");
        return 1;
    }

    printf("Calling `sum` function...\n");
    wasm_val_t args_val[2] = {WASM_I32_VAL(3), WASM_I32_VAL(4)};
    wasm_val_t results_val[1] = {WASM_INIT_VAL};
    wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
    wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

    if (wasm_func_call(sum_func, &args, &results))
    {
        printf("> Error calling the `sum` function!\n");

        return 1;
    }

    printf("Results of `sum`: %d\n", results_val[0].of.i32);

    wasm_module_delete(module);
    wasm_extern_vec_delete(&exports);
    wasm_instance_delete(instance);
    wasm_store_delete(store);
    wasm_engine_delete(engine);
}