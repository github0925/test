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
#include <arch/defines.h>
#if ARCH_ARM64
#include <arch/arm64.h>
#include <arch/arm64/mmu.h>
#else
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#endif
#include <arch/ops.h>
#include <platform/interrupts.h>

#include <dev/i2c.h>

#include "__regs_base.h"
#include "rstgen_hal.h"
#include "clkgen_hal.h"
#include "target_res.h"

#include "ckgen_init.h"
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

#include "dcf.h"
#if WITH_SMP

#if SMP_MAX_CPUS > 1
#ifndef GENERIC_TIMER
#error  "must using generic timer if enable smp & cpus > 1; please config SYSTEM_TIMER as generic in project.mk\n"
#endif
#endif

#include <bits.h>
#include "rstgen_hal.h"
#include "res.h"
#include "scr_hal.h"
#endif
#include <common/arm64/platform.h>
#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

#define TSGEN_BASE APB_SYS_CNT_RW_BASE
#define CNTCR 0 /* counter control */

#define GENERIC_TIMER_FREQ (CKGEN_SYS_CNT)

extern void timer_init_early(void);

#if WITH_SMP
#define SMC_PSCI_CPU_ON (0xc4000003)

extern void psci_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3);
static void cpu1_clock_gate_disable(uint32_t resid)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    ret = hal_clock_enable(handle, resid);
    ASSERT(ret);
    hal_clock_release_handle(handle);
}

static void setup_cpu1_reset_addr(int corenum, uint64_t addr)
{
    scr_handle_t handle_39_30;
    scr_handle_t handle_29_2;
    scr_signal_t hi_res_id, lo_res_id;
    bool ret = false;

    switch (corenum) {
        case 0:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr0_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr0_29_2;
            break;

        case 1:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr1_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr1_29_2;
            break;

        case 2:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr2_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr2_29_2;
            break;

        case 3:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr3_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr3_29_2;
            break;

        case 4:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr4_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr4_29_2;
            break;

        case 5:
            hi_res_id = SCR_SEC__L31__cpu1_rvbaraddr5_39_30;
            lo_res_id = SCR_SEC__L31__cpu1_rvbaraddr5_29_2;
            break;

        default:
            ASSERT(0);
            break;
    }

    handle_39_30 = hal_scr_create_handle(hi_res_id);
    handle_29_2  = hal_scr_create_handle(lo_res_id);
    hal_scr_get(handle_39_30);
    hal_scr_get(handle_29_2);
    ret = hal_scr_set(handle_39_30, (uint32_t)(addr >> 30));
    ASSERT(ret);
    ret = hal_scr_set(handle_29_2, ((uint32_t)addr & BIT_MASK(30)) >> 2);
    ASSERT(ret);
    hal_scr_delete_handle(handle_39_30);
    hal_scr_delete_handle(handle_29_2);
}

void smcc_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3)
{
    smc(arg0, arg1, arg2, arg3, 0, 0, 0, 0);
}

void psci_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3)
{
    bool ret = true;
    void *handle = NULL;
    int cpunum = arg1;
    paddr_t pbootbase = arg2;
    /* Set CPU1 boot address. */
    pbootbase &= (1ull << 40) - 1;
    setup_cpu1_reset_addr(cpunum, pbootbase);
    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);

    /* Disable clock gating. */
    if (cpunum == 0) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE0_WARM);
        ASSERT(ret);
    }
    else if (cpunum == 1) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE1_WARM);
        ASSERT(ret);
    }
    else if (cpunum == 2) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE2_WARM);
        ASSERT(ret);
    }
    else if (cpunum == 3) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE3_WARM);
        ASSERT(ret);
    }
    else if (cpunum == 4) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE4_WARM);
        ASSERT(ret);
    }
    else if (cpunum == 5) {
        cpu1_clock_gate_disable(RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5);
        ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_CPU1_CORE5_WARM);
        ASSERT(ret);
    }

    hal_rstgen_release_handle(handle);
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

static bool is_support_psci(void)
{
    static int isfirst = 1;
    static bool issupport = true;

    if (isfirst) {
        uint64_t ret = smc(SMCCC_ARCH_FEATURES, SMCCC_ARCH_FEATURES, 0, 0,
                           0, 0, 0, 0);

        if (ret != SMC_OK) {
            issupport = false;
        }
    }

    return issupport;
}

static int reset_gic(void)
{
    void *handle = NULL;
    bool ret = true;
    ret = hal_rstgen_creat_handle(&handle, RES_MODULE_RST_SEC_CPU1_CORE0_WARM);
    ASSERT(ret);
    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(handle);
    ASSERT(ret);

    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_GIC4);
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
    uart_init_early();

#ifdef SUPPORT_VIRT_UART
    vuart_init();
#endif
    setup_gic_clk();
    /* If SMCCC is ready, no permission to reset GIC */
    uint64_t ret = smc(SMCCC_ARCH_FEATURES, SMCCC_ARCH_FEATURES, 0, 0,
                       0, 0, 0, 0);

    if (ret != SMC_OK) {
        reset_gic();
    }

    arm_gic_init_early();
#if GENERIC_TIMER
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
        //smc
        if (is_support_psci()) {
            smcc_call(SMC_PSCI_CPU_ON, i << 8, MEMBASE, 0);
        }
        else {
            //native
#ifdef WITH_KERNEL_VM
            psci_call(psci_call_num, i, MEMBASE + KERNEL_LOAD_OFFSET, 0);
#else
            psci_call(psci_call_num, i, MEMBASE, 0);
#endif
        }
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

