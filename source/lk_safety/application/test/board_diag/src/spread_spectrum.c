/*
 * spread_spectrum.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <debug.h>
#include <stdbool.h>
#include <sys/types.h>
#include <trace.h>
#include "chip_res.h"
#include "res.h"
#include "pll.h"
#include "pll_hal.h"
#include <macros.h>
#include <string.h>
#include "board_start.h"

/* Script generated PLL configuration table. */
#include "pll_cfg.h"
struct pll_name {
    /* PLL name */
    char name[20];
    /* PLL resource Id. */
    uint32_t resid;
    /* PLL register address. Use resource address if paddr is 0. */
    paddr_t paddr;
};

const static struct pll_name g_pll[PLL_MAX] = {
    [PLL1] = {"PLL1", RES_PLL_PLL1, 0},
    [PLL2] = {"PLL2", RES_PLL_PLL2, 0},
    [PLL3] = {"PLL3", RES_PLL_PLL3, 0},
    [PLL4] = {"PLL4", RES_PLL_PLL4, 0},
    [PLL5] = {"PLL5", RES_PLL_PLL5, 0},
    [PLL6] = {"PLL6", RES_PLL_PLL6, 0},
    [PLL7] = {"PLL7", RES_PLL_PLL7, 0},
    [PLL_DISP] = {"PLL_DISP", RES_PLL_PLL_DISP, 0},
    [PLL_LVDS1] = {"PLL_LVDS1", RES_PLL_PLL_LVDS1, 0},
    [PLL_LVDS2] = {"PLL_LVDS2", RES_PLL_PLL_LVDS2, 0},
    [PLL_LVDS3] = {"PLL_LVDS3", RES_PLL_PLL_LVDS3, 0},
    [PLL_LVDS4] = {"PLL_LVDS4", RES_PLL_PLL_LVDS4, 0},
    [PLL_CPU1A] = {"PLL_CPU1A", RES_PLL_PLL_CPU1A, 0},
    [PLL_CPU1B] = {"PLL_CPU1B", RES_PLL_PLL_CPU1B, 0},
    [PLL_CPU2] = {"PLL_CPU2", RES_PLL_PLL_CPU2, 0},
    [PLL_GPU1] = {"PLL_GPU1", RES_PLL_PLL_GPU1, 0},
    [PLL_GPU2] = {"PLL_GPU2", RES_PLL_PLL_GPU2, 0},
    [PLL_VPU] = {"PLL_VPU", RES_PLL_PLL_VPU, 0},
    [PLL_VSN] = {"PLL_VSN", RES_VDSP_PLL_VSN, 0},
    [PLL_HPI] = {"PLL_HPI", RES_PLL_PLL_HPI, 0},
    [PLL_HIS] = {"PLL_HIS", RES_PLL_PLL_HIS, 0},
    [PLL_DDR] = {"PLL_DDR", RES_DDR_CFG_DDR_CFG, APB_PLL_DDR_BASE }
};

static uint32_t get_resid_by_name(const char *name)
{
    for (pll_e i = 0; i < PLL_MAX; i++) {
        if (strcmp(name, g_pll[i].name) == 0)
            return g_pll[i].resid;
    }

    return 0;
}

static int cmd_enable_ss(const char *pll_name, bool direction, int value)
{
    uint32_t resid = 0;
    resid = get_resid_by_name(pll_name);

    pll_handle_t pll;
    pll_config_t config = {0};
    bool down = direction;
    int spread = value;
    pll =  hal_pll_create_handle(resid);

    if (pll == (pll_handle_t)0) {
        dprintf(CRITICAL, "pll res 0x%x not belong this domain\n", resid);
        return -1;
    }

    hal_pll_get_config(pll, &config);

    if (config.integer) {
        dprintf(CRITICAL, "pll is integer, will force set to frac\n");
        config.integer = false;
        config.frac = 0;
    }

    config.spread_spectrum = true;
    config.downspread = down;
    config.spread = spread;
    hal_pll_config(pll, &config);
    hal_pll_delete_handle(pll);
    return 0;
}

void spread_spectrum_ops(void)
{
    static uint16_t timers = 0;
    static bool checkRet = false;
    if (timers < 300) {
        timers++;
    }
    else {
        if(checkRet != true){
           cmd_enable_ss("PLL1", 1, 20);
           cmd_enable_ss("PLL2", 1, 20);
           cmd_enable_ss("PLL3", 1, 20);
           cmd_enable_ss("PLL4", 1, 20);
           checkRet = true;
         }
    }
}
