/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#ifndef _SD_SMC_H_
#define _SD_SMC_H_

#include <stdint.h>
#include <arch/ops.h>
#include <asm.h>

/* SMC64: OEM Service Calls */
#define SMC_NR_LAUNCH_NON_IMG   0xc3000001
#define SMC_NR_SWITCH_NON_SEC   0xc3000002

extern void arch_flush_dcache_all(void);

struct pt_regs {
    unsigned long regs[31];
};

/*
 * void smc_call(arg0, arg1...arg7)
 *
 * issue the secure monitor call
 *
 * x0~x7: input arguments
 * x0~x3: output arguments
 */

static inline void smc_call(struct pt_regs *args)
{
    __asm__ volatile(
        "ldr x0, %0\n"
        "ldr x1, %1\n"
        "ldr x2, %2\n"
        "ldr x3, %3\n"
        "ldr x4, %4\n"
        "ldr x5, %5\n"
        "ldr x6, %6\n"
        "smc    #0\n"
        "str x0, %0\n"
        "str x1, %1\n"
        "str x2, %2\n"
        "str x3, %3\n"
        : "+m" (args->regs[0]), "+m" (args->regs[1]),
        "+m" (args->regs[2]), "+m" (args->regs[3])
        : "m" (args->regs[4]), "m" (args->regs[5]),
        "m" (args->regs[6])
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17");
}

static unsigned long switch_non_sec(void)
{
    struct pt_regs regs;
    regs.regs[0] = SMC_NR_SWITCH_NON_SEC;

    arch_flush_dcache_all();
    DSB;
    ISB;
    smc_call(&regs);
    return regs.regs[0];
}

static unsigned long boot_nonsec_img(paddr_t entry, paddr_t dtb)
{
    struct pt_regs regs;
    regs.regs[0] = SMC_NR_LAUNCH_NON_IMG;
    regs.regs[1] = entry;
    regs.regs[2] = dtb;

    DSB;
    ISB;
    /* Disable el2 MMU/DCACHE/ICACHE during smc handler */
    smc_call(&regs);
    /* Should never return */
    return regs.regs[0];
}

#endif
