#include <stdio.h>
#include <stdlib.h>
#include "wasmer.h"

int main(int argc, const char *argv[])
{
    printf("Creating the store...\n");
    wasm_engine_t *engine = wasm_engine_new();
    wasm_store_t *store = wasm_store_new(engine);

    int rc = 0;
    wasm_module_t *module = NULL;
    wasm_instance_t *instance = NULL;
    wasm_extern_vec_t exports = WASM_EMPTY_VEC;

    // Try to open add.cwasm
    FILE *fp = fopen("add.cwasm", "rb");
    if (fp == NULL)
    {
        // Assumption: No such file or directory
        // If we don't find add.cwasm, try to open add.wasm, compile it, and then serialize the result as add.cwasm
        fp = fopen("add.wasm", "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "> Error: could not open add.wasm\n");
            rc = 1;
            goto cleanup;
        }

        fseek(fp, 0, SEEK_END);
        long wasm_file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (wasm_file_size < 0)
        {
            fprintf(stderr, "> Error: ftell failed on add.wasm\n");
            fclose(fp);
            rc = 1;
            goto cleanup;
        }

        wasm_byte_t *buf = malloc((size_t)wasm_file_size);
        if (!buf)
        {
            fprintf(stderr, "> Error: out of memory\n");
            fclose(fp);
            rc = 1;
            goto cleanup;
        }
        size_t buf_len = fread(buf, sizeof(wasm_byte_t), (size_t)wasm_file_size, fp);

        wasm_byte_vec_t wasm_bytes;
        wasm_byte_vec_new(&wasm_bytes, buf_len, buf);
        free(buf);

        printf("Compiling module...\n");
        module = wasm_module_new(store, &wasm_bytes);

        wasm_byte_vec_delete(&wasm_bytes);
        fclose(fp);

        wasm_byte_vec_t serialized_module = {
            .data = NULL,
            .size = 0};
        wasm_module_serialize(module, &serialized_module);

        fp = fopen("add.cwasm", "wb");
        if (fp == NULL)
        {
            fprintf(stderr, "Warning: could not create add.cwasm, skipping cache write\n");
        }
        else
        {
            size_t serialized_module_cursor = 0;

            while (serialized_module_cursor < serialized_module.size)
            {
                size_t nwritten = fwrite(&serialized_module.data[serialized_module_cursor], sizeof(char), serialized_module.size - serialized_module_cursor, fp);
                if (nwritten == 0) {
                    fprintf(stderr, "Warning: fwrite failed, cwasm cache may be corrupt\n");
                    break;
                }
                serialized_module_cursor += nwritten;
            }

            fflush(fp);
            fclose(fp);
        }

        wasm_byte_vec_delete(&serialized_module);
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        long cwasm_file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (cwasm_file_size < 0)
        {
            fprintf(stderr, "> Error: ftell failed on add.cwasm\n");
            fclose(fp);
            rc = 1;
            goto cleanup;
        }

        wasm_byte_t *buf = malloc((size_t)cwasm_file_size);
        if (!buf)
        {
            fprintf(stderr, "> Error: out of memory\n");
            fclose(fp);
            rc = 1;
            goto cleanup;
        }
        size_t buf_len = fread(buf, sizeof(wasm_byte_t), (size_t)cwasm_file_size, fp);
        fclose(fp);

        wasm_byte_vec_t cwasm_bytes;
        wasm_byte_vec_new(&cwasm_bytes, buf_len, buf);
        free(buf);

        module = wasm_module_deserialize(store, &cwasm_bytes);
        wasm_byte_vec_delete(&cwasm_bytes);
    }

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
