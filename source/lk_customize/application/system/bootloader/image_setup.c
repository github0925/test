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
 * image_setup: Setup an existing IPL header for execution
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <lib/reg.h>

#include "startup.h"

void dump_ram(unsigned char *start, int size)
{
    for (int i = 0; i < size; i++) {
        if (i % 16 == 0)
            dprintf(DEFAULT_LOGLEVEL, "\n");

        dprintf(DEFAULT_LOGLEVEL, "%x ", start[i]);
    }

    dprintf(DEFAULT_LOGLEVEL, "\n");
}


int image_setup (struct startup_header *startup_hdr, unsigned long addr)
{
    unsigned long   ram_addr;

    // Copy the data from the address into our structure in memory
    memcpy ((unsigned char *)startup_hdr, (unsigned char *)addr,
            sizeof(struct startup_header));

    // get ram_addr and patch startup with the images physical
    // location.  Startup will handle the rest ...
    ram_addr = startup_hdr->ram_paddr + startup_hdr->paddr_bias;
    startup_hdr->imagefs_paddr = addr + startup_hdr->startup_size -
                                 startup_hdr->paddr_bias;

#if WITH_KERNEL_VM
    ram_addr = _ioaddr((paddr_t)(ram_addr));
#endif
    //Copy startup to ram_addr.
    memcpy((unsigned char *)ram_addr, (unsigned char *)startup_hdr,
           sizeof(struct startup_header));
//    dump_ram((unsigned char*)ram_addr,0x20);

    memcpy ((unsigned char *)(ram_addr + sizeof(struct startup_header)),
            (unsigned char *)(addr + sizeof(struct startup_header)),
            (startup_hdr->startup_size - sizeof(struct startup_header)));

#if WITH_KERNEL_VM
//    dump_ram((unsigned char*)(_ioaddr((paddr_t)startup_hdr->startup_vaddr)),0x20);
#else
//    dump_ram((unsigned char*)startup_hdr->startup_vaddr, 0x20);
#endif

    // All set now for image_start
    return (0);
}

