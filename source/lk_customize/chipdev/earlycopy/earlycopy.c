/*
 * earlycopy.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Copy sections from Load Address to Link Address.
 *
 * Revision History:
 * -----------------
 */
#include <stdint.h>
#include <string.h>
#include <trace.h>
#include <arch/ops.h>

#include "earlycopy.h"

#define LOCAL_TRACE     0

/**
 * Early copy is required when a section has different load address and link
 * address, e.g., TCM code and data sections are loaded into DDR/SHRAM, and
 * must be copied to TCM before they could be accessed.
 *
 * To enable early copy feature, create earlycopy.ld in target folder based
 * on earlycopy.ld.example, then add following lines to target rules.mk.
 *
 * MODULE_DEPS += chipdev/earlycopy
 * EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/earlycopy.ld
 *
 * Note that early copy is not "physics relocation", which have the same
 * LMA to VMA offset for all sections. The early copy sections have different
 * LMA to VMA offsets.
 */
struct early_copy_section {
    uint32_t    vma;    /* virtual memory address */
    uint32_t    size;   /* section size */
    uint32_t    lma;    /* load memory address */
};

extern struct early_copy_section __earlycopy_start;
extern struct early_copy_section __earlycopy_end;

void platform_earlycopy(void)
{
    struct early_copy_section *p;

    for (p = &__earlycopy_start; p < &__earlycopy_end; p++) {
        LTRACEF_LEVEL(3, "vma 0x%x, lma 0x%x, size %d\n",
            p->vma, p->lma, p->size);

        memcpy((void *)p->vma, (void *)p->lma, p->size);
        arch_clean_cache_range((addr_t)p->vma, p->size);
    }
}
