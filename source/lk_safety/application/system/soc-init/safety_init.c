/*
 * Copyright (c) 2019 Semidrive Inc.
 *
 */
#include <app.h>
#include <assert.h>
#include <lib/console.h>
#include <lk/init.h>
#include <stdlib.h>

#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_hw.h"
#endif

#include "chip_res.h"
#include "clkgen_hal.h"
#include "cpu_hal.h"
#include "dcf.h"
#include "handover_res.h"
#include "hal_port.h"
#include "image_cfg.h"
#include "lib/reg.h"
#include "lib/reboot.h"
#include "lib/sdrv_common_reg.h"
#include "pll_hal.h"
#include "rstgen_hal.h"

#ifdef ENABLE_RTC
#include "rtc_drv.h"
#endif

#define TSGEN_BASE APB_SYS_CNT_RW_BASE
#define CNTCR 0 /* counter control */

extern void soc_init(uint level);

const uint32_t default_clk_gating_enabled_table[] = {
    // RES_GATING_EN_SAF_BIPC_ENET1,
    // RES_GATING_EN_SAF_XTAL_SAF,
    RES_GATING_EN_SAF_RPC_SAF,
    // RES_GATING_EN_SAF_EIC_SAF,
    // RES_GATING_EN_SAF_SEM2,
    RES_GATING_EN_SAF_SEM1,
    // RES_GATING_EN_SAF_RSTGEN_SAF,
    // RES_GATING_EN_SAF_CKGEN_SAF,
    // RES_GATING_EN_SAF_PLL2,
    // RES_GATING_EN_SAF_PLL1,
    // RES_GATING_EN_SAF_PVT_SENS_SAF,
    // RES_GATING_EN_SAF_SCR_SAF,
    // RES_GATING_EN_SAF_GPIO1,
    // RES_GATING_EN_SAF_CAN4,
    // RES_GATING_EN_SAF_CAN3,
    // RES_GATING_EN_SAF_CAN2,
    // RES_GATING_EN_SAF_CAN1,
    // RES_GATING_EN_SAF_PWM2,
    // RES_GATING_EN_SAF_PWM1,
    RES_GATING_EN_SAF_TIMER2,
    RES_GATING_EN_SAF_TIMER1,
    // RES_GATING_EN_SAF_TBU13,
    // RES_GATING_EN_SAF_TBU12,
    // RES_GATING_EN_SAF_TBU11,
    RES_GATING_EN_SAF_I2S_SC2,
    RES_GATING_EN_SAF_I2S_SC1,
    // RES_GATING_EN_SAF_UART8,
    // RES_GATING_EN_SAF_UART7,
    // RES_GATING_EN_SAF_UART6,
    // RES_GATING_EN_SAF_UART5,
    // RES_GATING_EN_SAF_UART4,
    // RES_GATING_EN_SAF_UART3,
    // RES_GATING_EN_SAF_UART2,
    // RES_GATING_EN_SAF_UART1,
    // RES_GATING_EN_SAF_SPI4,
    // RES_GATING_EN_SAF_SPI3,
    // RES_GATING_EN_SAF_SPI2,
    // RES_GATING_EN_SAF_SPI1,
    RES_GATING_EN_SAF_I2C4,
    RES_GATING_EN_SAF_I2C3,
    RES_GATING_EN_SAF_I2C2,
    RES_GATING_EN_SAF_I2C1,
    RES_GATING_EN_SAF_CE1,
    // RES_GATING_EN_SAF_EFUSEC,
    // RES_GATING_EN_SAF_ROMC1,
    // RES_GATING_EN_SAF_IRAM1,
    RES_GATING_EN_SAF_OSPI1,
#ifdef ENABLE_ETHERNET1
    RES_GATING_EN_SAF_ENET1_TIMER_SEC,
    RES_GATING_EN_SAF_ENET1_PHY_REF,
    RES_GATING_EN_SAF_ENTE1_TX,
#endif
    // RES_GATING_EN_SAF_DMA1,
    // RES_GATING_EN_SAF_RTC_PCLK,
    // RES_GATING_EN_SAF_MAC_RDC,
    // RES_GATING_EN_SAF_NOC_MAINB,
    // RES_GATING_EN_SAF_SAF_PLAT_NOC,
    // RES_GATING_EN_SAF_SAF_PLAT,
    // RES_GATING_EN_SAF_GIC1,
};

const uint32_t default_rst_ip_module[] = {
    RES_MODULE_RST_SAF_SEM2,
    RES_MODULE_RST_SAF_SEM1,
    RES_MODULE_RST_SAF_CANFD4,
    RES_MODULE_RST_SAF_CANFD3,
    RES_MODULE_RST_SAF_CANFD2,
    RES_MODULE_RST_SAF_CANFD1,
    RES_MODULE_RST_SAF_I2S_SC2,
    RES_MODULE_RST_SAF_I2S_SC1,
    RES_MODULE_RST_SAF_ENET1,
#if !OSPI_DIRECT_ACCESS
    RES_MODULE_RST_SAF_OSPI1,
#endif
};

static void wait_ap_rst_done(void)
{
    while (!(readl(APB_SCR_SAF_BASE + (0x104 << 10)) & 0x8000000));
}

static void platform_handover(void)
{
    unsigned i;

    for (i = 0; i < handover_list.handover_num; i++) {
        RMWREG32(handover_list.handover_info[i].op_addr,
                 handover_list.handover_info[i].op_bit, 1, 1);
    }

#if (CONFIG_USE_SYS_PROPERTY == 0) && (!defined(SUPPORT_DIL2_INIT))
    sdrpc_notify_msg(NULL, COM_HANDOVER_STATUS, NULL);
#endif
}

static void setup_pll(uint32_t resid)
{
    pll_handle_t pll;
    pll =  hal_pll_create_handle(resid);

    //ASSERT(pll);
    if (pll == (pll_handle_t)0) {
        dprintf(CRITICAL, "pll res 0x%x not belong this domain\n", resid);
        return;
    }

    hal_pll_config(pll, NULL);
    hal_pll_delete_handle(pll);
}

static void platform_clk_init(void)
{
    void *handle;
    bool ret = 0;
    setup_pll(RES_PLL_PLL2);
    hal_saf_clock_set_default();
    setup_pll(RES_PLL_PLL1);
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        dprintf(ALWAYS, "clock handle create fail\n");
        return;
    }

    for (uint32_t i = 0;
            i < sizeof(default_clk_gating_enabled_table) / sizeof(
                default_clk_gating_enabled_table[0]); i++) {
        ret = hal_clock_enable(handle, default_clk_gating_enabled_table[i]);

        if (!ret) {
            dprintf(ALWAYS, "clock gating enable fail. IP gate RES:0x%x\n",
                    default_clk_gating_enabled_table[i]);
        }
    }

    hal_clock_release_handle(handle);
}


#ifdef ENABLE_RTC
static void platform_rtc_init(void)
{
    rtc_init(APB_RTC1_BASE);
    rtc_enable(APB_RTC1_BASE);
    /* rtc_lock(APB_RTC1_BASE); */
    rtc_init(APB_RTC2_BASE);
    rtc_enable(APB_RTC2_BASE);
    /* rtc_lock(APB_RTC2_BASE); */
}
#endif

#ifdef ENABLE_ETHERNET1
void eth1_scr_init(void)
{
    /* selects the PHY interface of the ethernet 1 as RGMII */
    RMWREG32(_ioaddr(APB_SCR_SAF_BASE + (0x610 << 10)), 3, 3, 1);
}
#endif

static void platform_modules_rst(void)
{
    /* Release other ips' rst signal. */
    // addr_t rst_phy_addr = 0;
    addr_t module_phy_addr = 0;
    int32_t idx = 0;
    int ret;
    // ASSERT(!res_get_info_by_id(RES_GLOBAL_RST_SAF_RST_EN, &rst_phy_addr, &idx));

    for (uint32_t i = 0;
            i < sizeof(default_rst_ip_module) / sizeof(default_rst_ip_module[0]);
            i++) {
        ret = res_get_info_by_id(default_rst_ip_module[i], &module_phy_addr, &idx);

        if (0 == ret) {
            rstgen_module_ctl(module_phy_addr, idx, false);
            rstgen_module_ctl(module_phy_addr, idx, true);
        }
    }
}


#if OSPI_DIRECT_ACCESS
#include <spi_nor.h>
void xip_init(void)
{
    addr_t ospic_reg = 0;
    int32_t dummy;
    res_get_info_by_id(RES_OSPI_REG_OSPI1, &ospic_reg, &dummy);
    spi_nor_reset_slave(ospic_reg);
}
#endif

static void safety_init(uint32_t level)
{
    wait_ap_rst_done();
    platform_clk_init();
#if OSPI_DIRECT_ACCESS
    xip_init();
#endif
#ifdef ENABLE_ETHERNET1
    eth1_scr_init();
#endif
    /* before this line should not invoke
     * ANY OS API.
    */
    /* enable tsgen */
    RMWREG32(_ioaddr(TSGEN_BASE + CNTCR), 0, 1, 1);
    platform_modules_rst();
#ifdef ENABLE_RTC
    /* should call it before handover */
    platform_rtc_init();
#endif
    platform_handover();
}

#if SUPPORT_NEXT_OS
#include "reboot_service.h"
static void reboot_saf(uint32_t level)
{
    rb_arg arg = {0};
    arg.entry = SAF_MEMBASE;
    arg.sz = SAF_MEMSIZE;
    reboot_module(RB_SAF_M, RB_RB_OPC, &arg);
}

static void safety_os_entry(const struct app_descriptor *app, void *args)
{
    reboot_saf(0);
}

APP_START(safety_os)
.flags = 0,
.entry = safety_os_entry,
APP_END
#endif

LK_INIT_HOOK(safety_init, safety_init, LK_INIT_LEVEL_PLATFORM - 1);
//LK_INIT_HOOK(reboot_saf, reboot_saf, LK_INIT_LEVEL_TARGET);
