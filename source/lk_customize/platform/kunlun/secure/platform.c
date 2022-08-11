/*
 * platform.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Secure R5 platform initialization codes.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <debug.h>
#include <dev/uart.h>
#include <platform/interrupts.h>
#include <reg.h>
#include <sys/types.h>
#include <dev/i2c.h>
#include <arch/arm/tcm.h>
#include <arch/arm/mpu.h>

#include "__regs_base.h"
#include "earlycopy.h"
#include "lib/reg.h"
#include "rstgen_hal.h"
#include "scr_hal.h"
#include "target_res.h"
#include "dcf.h"
#include "image_cfg.h"
#include <common/arm/platform.h>
#include <sys_diagnosis.h>

#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif
#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

#define RSTGEN_GENERAL_REG(n) ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_SEC_BASE + RSTGEN_GENERAL_REG(1))
#define REMAP_DONE (0x52454d50) /* 'REMP' */

#define ROMC_STICKY_REG (APB_ROMC2_BASE + 0x34)

extern void timer_init_early(void);

static void sec_core_reset(void)
{
    /* Reset R5 core. */
    void *handle;
    int ret;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);

    writel(readl(LOCKSTEP_SCR_ADDR)|(0x3 << LOCKSTEP_SCR_BIT), LOCKSTEP_SCR_ADDR);
    hal_rstgen_core_reset(handle, RES_CORE_RST_SEC_CR5_SEC_SW);
    // hal_rstgen_release_handle(handle);

    while (1) {
        arch_idle();
    }
}

static void gic_reset(void)
{
    void *handle;
    int ret;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);
    hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_GIC2);
    hal_rstgen_release_handle(handle);
}

/*
 * Remap core vector, and reset the core.
 */
void platform_cpu_reset(addr_t img_base)
{
    /* base address must be 4 KB aligned */
    ASSERT((img_base & 0xFFF) == 0);
    dprintf(INFO, "Remapping ARM vector to 0x%lx!\n", img_base);
    writel(REMAP_DONE, _ioaddr(RSTGEN_REMAP_STATUS_REG));
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_handle_t handle;
    handle = hal_scr_create_handle(
                        SCR_SEC__L31__remap_cr5_sec_ar_addr_offset_19_0);
    ASSERT(handle);
    hal_scr_set(handle, img_base >> 12);
    hal_scr_delete_handle(handle);
    handle = hal_scr_create_handle(
                        SCR_SEC__L31__remap_cr5_sec_ar_remap_ovrd_en);
    ASSERT(handle);
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(_ioaddr(ROMC_STICKY_REG), 0, 1, 1);
    sec_core_reset();
}

/*
 * Remap AXI space 0~512KB to real vector address.
 */
static void platform_remap(void)
{
    if (readl(_ioaddr(RSTGEN_REMAP_STATUS_REG)) != REMAP_DONE) {
        platform_cpu_reset(MEMBASE);
    }
    else {
        writel(0, _ioaddr(RSTGEN_REMAP_STATUS_REG));
    }
}

static void platform_mpu_init(void)
{
    int region = 0;
    mpu_enable(false);
#ifdef ENABLE_SDRV_SPINOR
    addr_t cospi_trigger_address;
    uint32_t cospi_trigger_range;
    /* config mpu for disable ospi sarm address cache */
    cospi_trigger_range = 1 << (7 + 2);
    cospi_trigger_address = OSPI1_BASE + 0x4000000 - cospi_trigger_range;
    mpu_add_region(region++, cospi_trigger_address,
                   cospi_trigger_range, MPU_REGION_DEVICE);
    cospi_trigger_address = OSPI2_BASE + 0x4000000 - cospi_trigger_range;
    mpu_add_region(region++, cospi_trigger_address,
                   cospi_trigger_range, MPU_REGION_DEVICE);

#endif
    region = init_shm_domain_area(region);
    region = platform_mpu_r5_common(region);

    mpu_add_region(region++, CE2_VCE1_BASE, 0x2000, MPU_REGION_NORMAL_NONCACHEABLE);   /*CE2_VCE1 8K*/
    mpu_add_region(region++, SAF_SEC_MEMBASE + SAF_SEC_MEMSIZE - 0x4000, 0x4000, MPU_REGION_NORMAL_NONCACHEABLE);  /*HSM share-mem with safety*/

    ASSERT(region <= 16);

    if (region != 0) {
        mpu_enable(true);
    }

    return;
}

__WEAK void os_platform_early_init(void)
{
}

void platform_early_init(void)
{
    uart_init_early();

#ifdef SUPPORT_VIRT_UART
    vuart_init();
#endif

    platform_remap();
    /* Enable TCM internal access. */
    tcm_enable(R5_SEC_TCMA_BASE, R5_SEC_TCMB_BASE, true);
    /* Copy code and data into TCM. */
    platform_earlycopy();
    /* Release GIC from reset and initialize GIC. Clocks of GIC
     * 1~3 are enabled by default.
     */
    gic_reset();
    arm_gic_init_early();
#if ARM_WITH_VFP
    arm_fpu_set_enable(true);
#endif
#if SDRV_TIMER
    timer_init_early();
#else
#error "No timer defined for R5 platform"
#endif
    platform_mpu_init();
    os_platform_early_init();
}

void platform_init(void)
{
    uart_init();

#if ENABLE_SD_DMA
    hal_dma_init();
#endif
    sysd_start(SEM2);
    dcf_init();
}

