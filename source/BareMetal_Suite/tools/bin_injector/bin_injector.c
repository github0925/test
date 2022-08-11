/************************************************
 * Copyright Semidrive 2020 . All Rights Reserved.
 * **********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void usage(void)
{
    printf("Usage: \n\t./bin_injector of=file1 if=file2 seek=offset_in_byte\n"
           "\tbin_injector of=file inj=offset:b inj=offset2:b\n");
}

typedef struct {
    unsigned int off;
    unsigned char v;
} inj_t;

int main(int argc, char *argv[])
{
    char *file_i_name = NULL;
    char *file_o_name = NULL;
    FILE *file_i = NULL;
    FILE *file_o = NULL;
    int res = 0;
    unsigned int seek = 0ul;
    char *buf = NULL;
#define MAX_INJ     256
    inj_t inj[MAX_INJ];
    unsigned int inj_n = 0;

    if (argc < 3) {
        res = -1;
        usage();
        goto cleanup;
    }

    for (unsigned int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "if=", 3)) {
            file_i_name = argv[i] + 3;
        } else if (0 == strncmp(argv[i], "of=", 3)) {
            file_o_name = argv[i] + 3;
        } else if (0 == strncmp(argv[i], "seek=", 5)) {
            seek = strtoul(argv[i] + 5, NULL, 0);
        } else if (0 == strncmp(argv[i], "inj=", 4)) {
            char *p = argv[i];

            while (*p != ':') p++;

            *p = '\0';
            inj[inj_n].off = strtoul(argv[i] + 4, NULL, 0);
            inj[inj_n].v = strtoul(p + 1, NULL, 0);
            //printf("inj=%d:0x%02x\n", inj[inj_n].off, inj[inj_n].v);
            inj_n++;

            if (inj_n >= MAX_INJ) {
                printf("Opps, too many inj\n");
                return -1;
            }
        }
    }

    if ((file_o_name == NULL)
        || ((file_i_name == NULL) && (inj_n == 0))) {
        res = -2;
        usage();
        goto cleanup;
    }

    file_o = fopen(file_o_name, "rw+b");

    if (NULL == file_o) {
        printf("Opps, failed to open %s\n", file_o_name);
        res = -3;
        goto cleanup;
    }

    fseek(file_o, 0L, SEEK_END);
    size_t file_o_sz = ftell(file_o);
    fseek(file_o, seek, SEEK_SET);

    if (NULL != file_i_name) {
        file_i = fopen(file_i_name, "rb");

        if (NULL == file_i) {
            printf("Opps, failed to open %s\n", file_i_name);
            res = -4;
            goto cleanup;
        }

        fseek(file_i, 0L, SEEK_END);
        size_t file_i_sz = ftell(file_i);
        fseek(file_i, 0L, SEEK_SET);

        if (file_i_sz > (file_o_sz - seek)
            || (seek > file_o_sz)) {
            printf("Opps, too big input file\n");
            res = -5;
            goto cleanup;
        }

        buf = (char *)malloc(file_i_sz);

        if (NULL == buf) {
            res = -10;
            goto cleanup;
        }

        fread(buf, file_i_sz, 1, file_i);
        fwrite(buf, file_i_sz, 1, file_o);
    }

    for (int i = 0; i < inj_n; i++) {
        if (inj[i].off != 0) {
            if (inj[i].off > file_o_sz) {
                res = -11;
                printf("Opps, invalid inj=%d0x:%x\n", inj[i].off, inj[i].v);
                goto cleanup;
            }

            fseek(file_o, inj[i].off, SEEK_SET);
            fwrite(&(inj[i].v), 1, 1, file_o);
        }
    }

cleanup:

    if (NULL != buf) free(buf);

    if (NULL != file_i) fclose(file_i);

    if (NULL != file_o) fclose(file_o);

    return res;
}
