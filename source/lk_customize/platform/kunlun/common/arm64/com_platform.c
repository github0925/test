/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: common function for arm
*
*/

#include <common/arm64/platform.h>
#include <arch/arm64.h>
#include <arch/arm64/mmu.h>
#include <arch/mmu.h>
#include <kernel/vm.h>

#ifdef WITH_KERNEL_VM
#include <lk/init.h>

extern addr_t _nocacheable_start;
extern addr_t _nocacheable_end;
static void vm_init_nocache(uint level)
{
    int ret;

    if ((ret = arch_mmu_map(&vmm_get_kernel_aspace()->arch_aspace,
                            (addr_t)&_nocacheable_start,
                            (paddr_t)(&_nocacheable_start - KERNEL_BASE),
                            (&_nocacheable_start - &_nocacheable_end) / PAGE_SIZE,
                            ARCH_MMU_FLAG_UNCACHED)) != 0) {
        panic("Could not map loader into new space %d\n", ret);
    }
}

LK_INIT_HOOK(nocache, vm_init_nocache, LK_INIT_LEVEL_VM);
#endif


