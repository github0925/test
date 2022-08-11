/*
 * Copyright (c) 2015 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <reg.h>
#include <sys/types.h>
#include <kernel/vm.h>
#include <platform.h>
#include <dev/uart.h>
#if ARCH_ARM64
#include <arch/arm64.h>
#include <arch/arm64/mmu.h>
#else
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#endif
#include <arch/ops.h>
#include <arm_gic_hal.h>
#include <platform/interrupts.h>

#include <dev/i2c.h>

#include "__regs_base.h"
#include "rstgen_hal.h"
#include "clkgen_hal.h"
#include "target_res.h"

#if GENERIC_TIMER
#include <dev/timer/arm_generic.h>
#endif

#if SUPPORT_SD_DISPLAY
#include <platform/sd_display.h>
#endif

#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif

#if ENABLE_DW_USB
#include <dev/usbc.h>
#endif

#if HAVE_RSTGEN
#include <rstgen_program.h>
#endif

#include "dcf.h"
#include <common/arm64/platform.h>

#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

#define TSGEN_BASE APB_SYS_CNTRW_BASE
#define GENERIC_TIMER_FREQ (24000000)
#define TIMER_FREQ (24000000)

extern void timer_init_early(void);

#if WITH_SMP

extern void psci_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3);
void psci_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3)
{
    int cpunum = arg1;
    vaddr_t iobase;
    paddr_t pbootbase = arg2;
#ifdef WITH_KERNEL_VM
    iobase = (vaddr_t)paddr_to_kvaddr(APB_SCR_SEC_BASE);
#else
    iobase = (vaddr_t)APB_SCR_SEC_BASE;
#endif
    writel(pbootbase >> 2, iobase + (cpunum << 13));
#if HAVE_RSTGEN
    rstgen_sec_module_rst(cpunum +
                          RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_0_INDEX,
                          1);
#endif
}
#endif

#ifdef WITH_KERNEL_VM
struct mmu_initial_mapping mmu_initial_mappings[] = {
    // XXX needs to be filled in

    /* Note: mapping entry should be 1MB alignment (address and size will be masked to 1MB boundaries in arch/arm/arm/start.S) */
    /* mcusys (peripherals) */
    {
        .phys = 0,
        .virt = PERIPHERAL_BASE_VIRT,
        .size = 0x40000000,
        .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
        .name = "mcusys"
    },
    /* ram */
    {
        .phys = 0x40000000,
        .virt = KERNEL_BASE + 0X40000000,
        .size = (30ULL * 1024 * 1024 * 1024),
        .flags = 0,
        .name = "ram"
    },

    /* null entry to terminate the list */
    {0}
};

static pmm_arena_t arena = {
    .name = "dram",
    .base = MEMBASE,
    .size = MEMSIZE,
    .flags = PMM_ARENA_FLAG_KMAP,
};
#endif

static int reset_gic(void)
{
    void *handle = NULL;
    bool ret = true;
    ret = hal_rstgen_creat_handle(&handle, RES_MODULE_RST_SEC_CPU2_SS);
    ASSERT(ret);
    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(handle);
    ASSERT(ret);
    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_GIC5);
    ASSERT(ret);
    hal_rstgen_release_handle(handle);
    return 0;
}

static void setup_gic_clk(void)
{
    void *handle = NULL;
    bool ret = false;
    clkgen_app_ip_cfg_t clk = {4, 0, 0};
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SEC_GIC4_GIC5, &clk);
    ASSERT(ret);
    hal_clock_release_handle(handle);
}

void platform_early_init(void)
{
    setup_gic_clk();
    uint64_t ret = smc(SMCCC_ARCH_FEATURES, SMCCC_ARCH_FEATURES, 0, 0,
                       0, 0, 0, 0);

    if (ret != SMC_OK) {
        hal_arm_gic_igroup_init(NULL);
        reset_gic();
    }
    arm_gic_init_early();

    uart_init_early();

#ifdef SUPPORT_VIRT_UART
    vuart_init();
#endif

#if GENERIC_TIMER
#define CNTCR 0 /*counter control*/
    /*enable tsgen*/
    uint32_t val;
#ifdef WITH_KERNEL_VM
    vaddr_t addr = (vaddr_t)paddr_to_kvaddr(TSGEN_BASE + CNTCR);
#else
    vaddr_t addr = (TSGEN_BASE + CNTCR);
#endif
    val = readl(addr);
    val |= 1 << 0;
    writel(val, addr);
    arm_generic_timer_init(GENERIC_TIMER_INT, GENERIC_TIMER_FREQ);
#endif
#if SDRV_TIMER
    timer_init_early();
#endif
    //i2c_init_early();
#if ARM_WITH_VFP == 1
    fpu_init();
#endif
#ifdef WITH_KERNEL_VM
    pmm_add_arena(&arena);
#endif
#if WITH_SMP
    /* boot the secondary cpus using the Power State Coordintion Interface */
    ulong psci_call_num = 0x84000000 + 3; /* SMC32 CPU_ON */
#if ARCH_ARM64
    psci_call_num += 0x40000000; /* SMC64 */
#endif

    for (uint i = 1; i < SMP_MAX_CPUS; i++) {
#ifdef WITH_KERNEL_VM
        psci_call(psci_call_num, i, MEMBASE + KERNEL_LOAD_OFFSET, 0);
#else
        psci_call(psci_call_num, i, MEMBASE, 0);
#endif
    }

#endif
}

void platform_init(void)
{
    uart_init();

    //i2c_init();
    dcf_init();
#if SUPPORT_SD_DISPLAY
    /* install display */
    sd_display_create();
#endif
#if ENABLE_DW_USB
    usbc_init();
#endif
#if ENABLE_SD_DMA
    hal_dma_init();
#endif
}
