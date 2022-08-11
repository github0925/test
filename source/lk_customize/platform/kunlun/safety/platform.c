/*
 * Copyright (c) 2019 Semidrive Inc.
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
#include <lib/reg.h>
#include <sys/types.h>

#include <platform.h>
#include <dev/uart.h>

#include <arch/arm.h>
#include <arch/arm/mpu.h>
#include <arch/ops.h>
#include <platform/interrupts.h>

#include <assert.h>
#include "__regs_base.h"
#include "target_res.h"
#include <bits.h>

#include <clkgen_hal.h>
#include <rstgen_hal.h>
#if ENABLE_SD_DMA
#include <dma_hal.h>
#endif
#include <scr_hal.h>
#include <target_port.h>
#include "hal_port.h"
#include "pll_hal.h"
#if !SUPPORT_DIL2_INIT
#include <sdrpc.h>
#endif
#include "dcf.h"
#include <common/arm/platform.h>

#if DDR_ENTER_SELF
#include <lk/init.h>
void str_enter(void);
#endif
#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_hw.h"
#endif

#include <sys_diagnosis.h>

#include "image_cfg.h"
#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

#define REMAP_SZ (512 * 1024)
#define TCMA_BASE 0x4c0000
#define TCMA_SIZE (1024*64)
#define TCMB_BASE 0x4b0000
#define TCMB_SIZE (1024*64)

extern void tcm_init(uint32_t tcmA_base, uint32_t tcmB_base, uint32_t tcmA_size,
                     uint32_t tcmB_size);

extern void timer_init_early(void);


/************ Remap function implementation ************/
#define RSTGEN_GENERAL_REG(n)   ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_SAF_BASE + RSTGEN_GENERAL_REG(1))
#define REMAP_DONE              (0x52454d50)   /* 'REMP' */
#define ROMC_STICKY_REG         (APB_ROMC1_BASE + 0x34) //safe romc1

static void platform_core_reset(void)
{
    /* Reset R5 core. */
    addr_t phy_addr = 0;
    int32_t idx = 0;
    int ret;

    ret = res_get_info_by_id(RES_CORE_RST_SAF_CR5_SAF_SW, &phy_addr, &idx);
    ASSERT(!ret);
    writel(readl(LOCKSTEP_SCR_ADDR) | (0x3 << LOCKSTEP_SCR_BIT),
           LOCKSTEP_SCR_ADDR);
    /* Need to pass 2 as idx to find correct register addr */
    rstgen_core_reset(phy_addr, idx);

    while (1);
}

static void platform_gic_reset(void)
{
    /* Reset GIC to be writable. */
    addr_t phy_addr = 0;
    int32_t idx = 0;
    int ret;
    ret = res_get_info_by_id(RES_MODULE_RST_SAF_GIC1, &phy_addr, &idx);
    ASSERT(!ret);
    rstgen_module_ctl(phy_addr, idx, false);
    rstgen_module_ctl(phy_addr, idx, true);
}

static void vector_remap(addr_t vector_base)
{
    /* base address must be 4 KB aligned */
    ASSERT((vector_base & 0xFFF) == 0);
    dprintf(INFO, "Remapping ARM vector to 0x%lx!\n", vector_base);
    writel(REMAP_DONE, _ioaddr(RSTGEN_REMAP_STATUS_REG));
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_handle_t handle;
    handle = hal_scr_create_handle(
                 SCR_SAFETY__L31__remap_cr5_saf_ar_addr_offset_19_0);
    ASSERT(handle);
    hal_scr_set(handle, vector_base >> 12);
    hal_scr_delete_handle(handle);
    handle = hal_scr_create_handle(
                 SCR_SAFETY__L31__remap_cr5_saf_ar_remap_ovrd_en);
    ASSERT(handle);
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(_ioaddr(ROMC_STICKY_REG), 0, 1, 1);
}

static void platform_remap(void)
{
    if (readl(_ioaddr(RSTGEN_REMAP_STATUS_REG)) != REMAP_DONE) {
        vector_remap(MEMBASE);
        platform_core_reset();
    }
    else {
        writel(0, _ioaddr(RSTGEN_REMAP_STATUS_REG));
    }
}

void reset_safety_cr5(uint32_t entry)
{
    vector_remap(entry);
    platform_core_reset();
}
/************ Remap function implementation end ************/

static void fpu_init(void)
{
#if ARM_WITH_VFP==1
    volatile uint32_t fpsid;
    /* Enable FPU. */
    arm_fpu_set_enable(true);
    __asm__ volatile("VMRS %0, FPSID" : "=r"(fpsid));
    dprintf(INFO, "FPSID 0x%x\n", fpsid);
#endif
}

static void platform_mpu_init(void)
{
    int region = 0;
    mpu_enable(false);
    mpu_add_region(region++, 0x0, REMAP_SZ, MPU_REGION_NO_ACCESS);
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
#if SDPE_STAT_MEMBASE && SDPE_STAT_MEMSIZE
    mpu_add_region(region++, SDPE_STAT_MEMBASE, SDPE_STAT_MEMSIZE,
                   MPU_REGION_NORMAL_NONCACHEABLE);
#endif
#ifdef SUPPORT_SDPE_RPC
    mpu_add_region(region++, SAF_SDPE_RPC_MEMBASE, SAF_SDPE_RPC_MEMSIZE,
                   MPU_REGION_NORMAL_NONCACHEABLE);
#endif
#ifdef ENABLE_FASMAVM
    mpu_add_region(region++, 0xa00000,
                   0x100000, MPU_REGION_DEVICE);
#endif
#if OSPI_DIRECT_ACCESS
    addr_t flash_start_map = 0;
    int32_t dummy;
    res_get_info_by_id(RES_OSPI_OSPI1, &flash_start_map, &dummy);
    mpu_add_region(region++, flash_start_map, 0x4000000, MPU_REGION_NORMAL_RO);
#endif
    //protect vector table
    mpu_add_region(region++, 0, 0x40, MPU_REGION_NORMAL_RO);
    mpu_add_region(region++, MEMBASE, 0x40, MPU_REGION_NORMAL_RO);
    region = platform_mpu_r5_common(region);
    ASSERT(region <= 16);

    if (region != 0) {
        mpu_enable(true);
    }
}

void platform_early_init(void)
{
    platform_remap();
    tcm_init(TCMA_BASE, TCMB_BASE, TCMA_SIZE, TCMB_SIZE);
    //enable gic IP
    platform_gic_reset();
    uart_port_init();

    uart_init_early();

#ifdef SUPPORT_VIRT_UART
    vuart_init();
#endif

#if (CONFIG_USE_SYS_PROPERTY == 0) && (!defined(SUPPORT_DIL2_INIT))
    sdrpc_notify_msg(NULL, 0xff, NULL);
#ifndef WITH_APPLICATION_EARLY_APP
    sdrpc_notify_msg(NULL, COM_DC_STATUS, NULL);
#endif
#endif
    arm_gic_init_early();
    timer_init_early();
    dcf_early_init();
    fpu_init();
    platform_mpu_init();

    dprintf(INFO, "------------------------------\n\n");
    dprintf(INFO, "Welcome to Semidrive Safety\n\n");
    dprintf(INFO, "Build on: %s - %s\n\n", __DATE__, __TIME__);
    dprintf(INFO, "------------------------------\n\n");
    dprintf(INFO, "platform_early_init done\n");
}

void platform_init(void)
{
#if SUPPORT_BOARDINFO
    init_hwid();
#endif

    uart_init();

#if ENABLE_SD_DMA
    hal_dma_init();
#endif
#if !SUPPORT_DIL2_INIT
    sysd_start(SEM1);
#endif
}

static void str_enter_app(uint level)
{
#if DDR_ENTER_SELF
    printf("enter str_enter_app\n");
    str_enter();
    return;
#endif
}

#if DDR_ENTER_SELF
LK_INIT_HOOK(str_enter, str_enter_app, LK_INIT_LEVEL_TARGET + 1);
#endif
