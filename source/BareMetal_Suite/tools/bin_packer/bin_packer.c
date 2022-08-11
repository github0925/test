/************************************************
 * Copyright Semidrive 2020 . All Rights Reserved.
 * **********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void usage(void)
{
    printf("Usage: \n\t./bin_packer if=file1 if=file2 ... if=fileN of=file\n");
}

typedef struct {
    char name[48];
    uint32_t offset;
    uint32_t size;
    uint8_t rsvd[8];
} hdr_entry_t;

int main(int argc, char *argv[])
{
    char *file_i_name = NULL;
    char *file_o_name = NULL;
    FILE *file_i = NULL;
    FILE *file_o = NULL;
    int res = 0;
    unsigned int seek = 0ul;
    unsigned int if_files = 0;

    if (argc < 3) {
        res = -1;
        usage();
        goto cleanup;
    }

    for (unsigned int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "of=", 3)) {
            file_o_name = argv[i] + 3;
        }
    }

    if (file_o_name == NULL) {
        res = -2;
        usage();
        goto cleanup;
    }

    file_o = fopen(file_o_name, "w+b");

    if (NULL == file_o) {
        printf("Opps, failed to open %s\n", file_o_name);
        res = -3;
        goto cleanup;
    }

    for (unsigned int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "if=", 3)) {
            if_files++;
        }
    }

    if (if_files < 1) {
        printf("Opps, too few input files\n");
        res = -4;
        goto cleanup;
    }

    size_t hdr_sz = (if_files * sizeof(hdr_entry_t) + 511ul) / 512ul * 512ul;
    hdr_entry_t *hdr = (hdr_entry_t *) malloc(hdr_sz);
    hdr_entry_t *hdr_save = hdr;

    if (hdr == NULL) {
        res = -4;
        goto cleanup;
    }

    seek = hdr_sz;
    fseek(file_o, seek, SEEK_SET);

    if_files = 0;

    for (unsigned int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "if=", 3)) {
            file_i_name = argv[i] + 3;
            file_i = fopen(file_i_name, "rb");

            if (NULL == file_i) {
                printf("Opps, failed to open %s\n", file_i_name);
                res = -10;
                goto cleanup;
            }

            fseek(file_i, 0L, SEEK_END);
            size_t file_i_sz = ftell(file_i);
            fseek(file_i, 0L, SEEK_SET);
            size_t to_write = (file_i_sz + 15) / 16 * 16;
            uint8_t *buf = (uint8_t *)malloc(to_write);

            if (NULL == buf) {
                res = -11;
                goto cleanup;
            }

            fread(buf, file_i_sz, 1, file_i);

            if (to_write - file_i_sz > 0) {
                memset(buf + file_i_sz, 0, to_write - file_i_sz);
            }

            fwrite(buf, 1, to_write, file_o);

            char *p = file_i_name + strlen(file_i_name);

            for (; p > file_i_name; p--) {
                if (*p == '\\' || *p == '/') {
                    p++;
                    break;
                }
            }

            strncpy(hdr->name, p, sizeof(hdr->name));
            hdr->name[sizeof(hdr->name) - 1] = '\0';
            hdr->offset = seek;
            hdr->size = file_i_sz;
            printf("Packing %s: offset=0x%x, size=%d\n", hdr->name, hdr->offset, hdr->size);
            if_files++;
            seek += to_write;
            free(buf);
            fclose(file_i);
            hdr++;
        }
    }

    printf("%d files %d bytes packed into %s totally\n", if_files, seek, file_o_name);

    fseek(file_o, 0L, SEEK_SET);
    fwrite(hdr_save, 1, hdr_sz, file_o);

cleanup:

    if (NULL != file_o) fclose(file_o);

    if (NULL != hdr_save) free(hdr_save);

    return res;
}
