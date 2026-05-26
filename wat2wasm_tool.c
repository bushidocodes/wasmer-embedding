#include <stdio.h>
#include <stdlib.h>
#include "wasmer.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.wat output.wasm\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = malloc(size);
    fread(buf, 1, size, fp);
    fclose(fp);

    wasm_byte_vec_t wat, wasm;
    wasm_byte_vec_new(&wat, size, buf);
    free(buf);

    wat2wasm(&wat, &wasm);
    wasm_byte_vec_delete(&wat);

    if (!wasm.data) {
        fprintf(stderr, "wat2wasm failed\n");
        return 1;
    }

    fp = fopen(argv[2], "wb");
    if (!fp) {
        fprintf(stderr, "Cannot open %s for writing\n", argv[2]);
        wasm_byte_vec_delete(&wasm);
        return 1;
    }

    fwrite(wasm.data, 1, wasm.size, fp);
    fclose(fp);
    wasm_byte_vec_delete(&wasm);
    return 0;
}
