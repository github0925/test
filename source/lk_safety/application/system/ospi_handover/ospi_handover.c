/*
* wdg_init.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg system init .
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
#include <arch.h>
#include "clkgen_hal.h"
#include "rstgen_hal.h"
#include <include/lib/sdrv_common_reg.h>
#include <res.h>

#if OSPI_HANDOVER_SPI

#include <app.h>
#include <stdio.h>
#include <lib/console.h>
#include "spi_hal_master.h"
#include "res.h"
#include "chip_res.h"
//#include "hal_dio.h"

#include <scr_hal.h>
#include <target_port.h>
#include "hal_port.h"
#include "pll_hal.h"
#include <kernel/event.h>
#endif

#define ROMC_STICKY_OFFSET        0x34
#define ROMC_STICKY_REG    (APB_ROMC1_BASE + ROMC_STICKY_OFFSET)
#define STICKY_SAFE_RPC_DSEL        2
#define STICKY_SAFE_RPC_DSEL_WIDTH  1

static addr_t ospic_addr,rstgenc_addr;

static void ospi_modules_rst(int idx)
{
    if(ospic_addr)
    {
        res_get_info_by_id(RES_MODULE_RST_SAF_OSPI1, &ospic_addr, &idx);
    }

    rstgen_module_ctl(ospic_addr, idx, false);
    rstgen_module_ctl(ospic_addr, idx, true);
}

void soc_global_rst(void)
{
    int idx = 0;
    if(!rstgenc_addr)
    {
        res_get_info_by_id(RES_GLOBAL_RST_SAF_RST_EN, &rstgenc_addr, &idx);
    }

    rstgen_global_rst_enable(rstgenc_addr,1);
    rstgen_sw_self_rst(rstgenc_addr,false);
    rstgen_sw_self_rst(rstgenc_addr,true);

}

#if OSPI_HANDOVER_SPI
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

static void spi2_clk_init(void)
{
	 bool ret = true;

    static void *g_saf_handle;
	setup_pll(RES_PLL_PLL2);
	hal_saf_clock_set_default();
	ret = hal_clock_creat_handle(&g_saf_handle);
    if (!ret) {
        printf("cmd_clkgen creat handle failed\n");
        return ;
    }
	//hal_clock_ip_init(handle, CFG_CKGEN_SAF_BASE, &ip[i]);	
	ret = hal_clock_enable(g_saf_handle,RES_GATING_EN_SAF_SPI2);
	hal_clock_release_handle(g_saf_handle);
}
#endif

void ospi_handover_entry(void)
{
    int idx = 0;
    res_get_info_by_id(RES_GLOBAL_RST_SAF_RST_EN, &rstgenc_addr, &idx);
    res_get_info_by_id(RES_MODULE_RST_SAF_OSPI1, &ospic_addr, &idx);
    dprintf(ALWAYS,"%s handover ospi to secure core E\n", __func__);

    __asm__ volatile("cpsid if"); //disable interrupt to avoid crash while ospi handover

    ospi_modules_rst(idx);
#if 0
    //hal_saf_clock_set_default();
    //writel(0x3FFF,  APB_SCR_SAF_BASE + (0x200 <<10));
    //writel(0x7FFF,  APB_SCR_SAF_BASE + (0x204 <<10));
    //writel(0xEFFF,  APB_SCR_SAF_BASE + (0x208 <<10));
    //writel(0x7FF,  APB_SCR_SAF_BASE + (0x20C <<10));
#else
    RMWREG32(APB_SCR_SAF_BASE + (0x200 <<10), 3, 1, 1);
    RMWREG32(APB_SCR_SAF_BASE + (0x208 <<10), 13, 1, 1);
	#if OSPI_HANDOVER_SPI
	
	spi2_clk_init();
    RMWREG32(APB_SCR_SAF_BASE + (0x208 <<10), 7, 1, 1);
	#endif
#endif
    // RMWREG32(ROMC_STICKY_REG, STICKY_SAFE_RPC_DSEL, STICKY_SAFE_RPC_DSEL_WIDTH, 1);

    sdrv_common_reg_set_value(SDRV_REG_STATUS,SDRV_REG_STATUS_HANDOVER_DONE,SDRV_REG_STATUS_HANDOVER_DONE);

    while(sdrv_common_reg_get_value(SDRV_REG_BOOTREASON,0x0000000F) != HALT_REASON_SW_RESET);

    sdrv_common_reg_set_value(SDRV_REG_BOOTREASON,0,0);

    soc_global_rst();

    dprintf(ALWAYS,"%s X\n", __func__);
    return;
}
