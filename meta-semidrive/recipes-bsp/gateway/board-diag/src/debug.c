/*
 * debug.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "debug.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isprint(c)  in_range(c, 0x20, 0x7f)

void hexdump8_ex(const void *ptr, size_t len, u_int64_t disp_addr)
{
    caddr_t address = (caddr_t)ptr;
    size_t count;
    size_t i;
    const char *addr_fmt = ((disp_addr + len) > 0xFFFFFFFF)
                           ? "0x%016llx: "
                           : "0x%08llx: ";

    for (count = 0 ; count < len; count += 16) {
        printf(addr_fmt, disp_addr + count);

        for (i = 0; i < MIN(len - count, 16); i++) {
            printf("%02hhx ", *(const unsigned char *)(address + i));
        }

        for (; i < 16; i++) {
            printf("   ");
        }

        printf("|");

        for (i = 0; i < MIN(len - count, 16); i++) {
            char c = ((const char *)address)[i];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
        address += 16;
    }
}

void hexdump8(const void *ptr, size_t len)
{
    hexdump8_ex(ptr, len, (u_int64_t)((caddr_t)ptr));
}
