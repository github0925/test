/*
* demo.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: test entry for fpga demo
*
* Revision History:
* -----------------
* 011, 11/24/2018 chenqing create this file
*/

#include <sys/types.h>
#include <stdio.h>
#include <arch/arch_ops.h>
#include <lk/init.h>
#include <kernel/thread.h>
#include "config.h"
#include <string.h>
#include <platform/interrupts.h>
#include <kernel/vm.h>


void test_entry(uint level)
{
    do {
        //should add test code here
        dprintf(SPEW, "fpga demo\n");

    }
    while (0);
    return;
}

//Hook func could be called according to your need by simply modify
//params like LK_INIT_LEVEL_TARGET_EARLY
LK_INIT_HOOK(demo, test_entry, LK_INIT_LEVEL_TARGET);
//LK_INIT_HOOK_FLAGS(demo, test_entry, LK_INIT_LEVEL_THREADING, LK_INIT_FLAG_ALL_CPUS);

