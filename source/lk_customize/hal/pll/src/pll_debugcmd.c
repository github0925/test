/*
 * pll_debugcmd.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: PFPLL debug command
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

/* Script generated PLL configuration table. */
#include "pll_cfg.h"
struct pll_name {
    /* PLL name */
    char name[20];
    /* PLL resource Id. */
    uint32_t    resid;

    /* PLL register address. Use resource address if paddr is 0. */
    paddr_t     paddr;
};

static struct pll_name g_pll[PLL_MAX] = {
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
static void dumpcfg(pll_config_t *config)
{
    dprintf(CRITICAL, "integer %d, ss %d, down %d, spread %d\n",
            config->integer, config->spread_spectrum, config->downspread,
            config->spread);
    dprintf(CRITICAL, "fbdiv 0x%x, refdiv 0x%x, postdiv 0x%x 0x%x\n",
            config->fbdiv, config->refdiv, config->postdiv[0], config->postdiv[1]);
    dprintf(CRITICAL, "fractional 0x%x\n", config->frac);
    dprintf(CRITICAL, "outdiv 0x%x 0x%x 0x%x 0x%x\n",
            config->out_div[0], config->out_div[1],
            config->out_div[2], config->out_div[3]);
}
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

static int cmd_enable_ss(int argc, const cmd_args *argv)
{
    uint32_t resid = 0;

    if (argc != 4) {
        dprintf(CRITICAL, "enabless : pllname down/center spread(1~31)\n");
        return -1;
    }

    resid = get_resid_by_name(argv[1].str);

    if (!resid) {
        dprintf(CRITICAL, "no such pll %s\n", argv[1].str);
        return -1;
    }

    pll_handle_t pll;
    pll_config_t config = {0};
    bool down = argv[2].b;
    int spread = argv[3].u;
    pll =  hal_pll_create_handle(resid);

    //ASSERT(pll);
    if (pll == (pll_handle_t)0) {
        dprintf(CRITICAL, "pll res 0x%x not belong this domain\n", resid);
        return -1;
    }

    hal_pll_get_config(pll, &config);

    if (config.integer) {
        dprintf(CRITICAL, "pll is integer, will force set to frac\n");
        //hal_pll_delete_handle(pll);
        //return -1;
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

static int cmd_disable_ss(int argc, const cmd_args *argv)
{
    uint32_t resid = 0;

    if (argc != 2) {
        dprintf(CRITICAL, "disabless : pllname\n");
        return -1;
    }

    resid = get_resid_by_name(argv[1].str);

    if (!resid) {
        dprintf(CRITICAL, "no such pll %s\n", argv[1].str);
        return -1;
    }

    pll_handle_t pll;
    pll_config_t config = {0};
    pll =  hal_pll_create_handle(resid);

    //ASSERT(pll);
    if (pll == (pll_handle_t)0) {
        dprintf(CRITICAL, "pll res 0x%x not belong this domain\n", resid);
        return -1;
    }

    hal_pll_get_config(pll, &config);
    config.spread_spectrum = false;
    hal_pll_config(pll, &config);
    hal_pll_delete_handle(pll);
    return 0;
}
static int cmd_dumpcfg(int argc, const cmd_args *argv)
{
    uint32_t resid = 0;

    if (argc != 2) {
        dprintf(CRITICAL, "dumpcfg : pllname\n");
        return -1;
    }

    resid = get_resid_by_name(argv[1].str);

    if (!resid) {
        dprintf(CRITICAL, "no such pll %s\n", argv[1].str);
        return -1;
    }

    pll_handle_t pll;
    pll_config_t config = {0};
    pll =  hal_pll_create_handle(resid);

    //ASSERT(pll);
    if (pll == (pll_handle_t)0) {
        dprintf(CRITICAL, "pll res 0x%x not belong this domain\n", resid);
        return -1;
    }

    hal_pll_get_config(pll, &config);
    dumpcfg(&config);
    hal_pll_delete_handle(pll);
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("enabless", "enabless : pllname down/center spread(1~31)", (console_cmd)&cmd_enable_ss)
STATIC_COMMAND("disabless", "disabless : pllname", (console_cmd)&cmd_disable_ss)
STATIC_COMMAND("dumpcfg", "dumpcfg : pllname", (console_cmd)&cmd_dumpcfg)
STATIC_COMMAND_END(plldebugcmd);
#endif
