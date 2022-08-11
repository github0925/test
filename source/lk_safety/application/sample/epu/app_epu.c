/*
* app_wdg.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch/ops.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

static void pl_status(u32 status, bool flush)
{
    *(volatile u32*)0x20000 |= status;
    if(flush)
      arch_clean_cache_range(0x20000,0x4);
}

static int epu_handle(void)
{
    printf("epu_handle enter!\n");
    return 0;
}

static void epu_entry(const struct app_descriptor *app, void *args)
{
    pl_status(0x20000, true);
    epu_handle();
}
APP_START(app_epu)
.flags = 0,
.entry=epu_entry,
APP_END
