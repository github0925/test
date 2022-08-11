/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */


/*
 *  image_scan: Scan through memory looking for an image
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>

#include "startup.h"

int do_check = 0;

int zero_ok (struct startup_header *shdr)
{
    return (shdr->zero[0] == 0 &&
            shdr->zero[1] == 0 &&
            shdr->zero[2] == 0 );
}

// This calculates the sum of an array of 4byte numbers
int small_checksum(int *iray, long len)
{
    int sum;
    sum = 0;

    while (len > 0) {
        sum += *iray++;
        len -= 4;
    }

    return (sum);
}

// Return the total sum of bytes in memory, len/4 = integer
int checksum (unsigned long addr, unsigned long len)
{
    int  sum = 0;
    char scratch [512];

    while (len >= sizeof(scratch)) {
        memcpy ((unsigned char *) scratch, (unsigned char *)addr, sizeof(scratch));
        sum += small_checksum ((int *)scratch, sizeof(scratch));
        len -= sizeof(scratch);
        addr += sizeof(scratch);
    }

    if (len) {
        memcpy ((unsigned char *)scratch, (unsigned char *)addr, len);
        sum += small_checksum ((int *)scratch, len);
    }

    return (sum);
}

void dump(unsigned char *start, int size)
{
    for (int i = 0; i < size; i++) {
        printf("0x%x ", start[i]);

        if (i % 16 == 0)
            printf("\n");
    }
}

//     Scan 1k boundaries for the image identifier byte and
//     then does a checksum on the image.
unsigned long image_scan (struct startup_header *startup_hdr,
                          unsigned long start,
                          unsigned long end)
{
    unsigned long   lastaddr = -1;
    unsigned short  lastver = 0;

    // We assume that the images will all start on a 4 byte boundary
    for (; start < end; start += 4) {
//        dump((unsigned char*)start, 4);

        memcpy ((unsigned char *)startup_hdr, (unsigned char *)start,
                sizeof(struct startup_header));

        //  No endian issues here since stored "naturally"
        if (startup_hdr->signature != STARTUP_HDR_SIGNATURE)
            continue;

        // There are two checksums, one for the startup hdr
        // and one for the image.  Check both of them.
        if (do_check && checksum (start, startup_hdr->startup_size) != 0) {
            continue;
        }

        if (do_check && checksum (start + startup_hdr->startup_size,
                                  startup_hdr->stored_size - startup_hdr->startup_size) != 0)
            continue;

        // Stash the version and address and continue looking
        // for something newer than we are (jump ahead by
        // startup_hdr.stored_size)
        if (startup_hdr->version > lastver) {
            lastver = startup_hdr->version;
            lastaddr = start;
        }

        start += startup_hdr->stored_size - 4;
    }

    return (lastaddr);
}

