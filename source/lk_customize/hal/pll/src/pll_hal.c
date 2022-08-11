/*
 * pll_hal.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: PFPLL HAL.
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
#define abs_clk(a, b)   \
    ((a > b) ? (a-b):(b-a))

#define LOCAL_TRACE 0

/* Script generated PLL configuration table. */
#include "pll_cfg.h"

struct pll_res {
    /* PLL resource Id. */
    uint32_t    resid;

    /* PLL register address. Use resource address if paddr is 0. */
    paddr_t     paddr;
};

static struct pll_res g_pll_res[PLL_MAX] = {
    [PLL1] = { RES_PLL_PLL1 },
    [PLL2] = { RES_PLL_PLL2 },
    [PLL3] = { RES_PLL_PLL3 },
    [PLL4] = { RES_PLL_PLL4 },
    [PLL5] = { RES_PLL_PLL5 },
    [PLL6] = { RES_PLL_PLL6 },
    [PLL7] = { RES_PLL_PLL7 },
    [PLL_DISP] = { RES_PLL_PLL_DISP },
    [PLL_LVDS1] = { RES_PLL_PLL_LVDS1 },
    [PLL_LVDS2] = { RES_PLL_PLL_LVDS2 },
    [PLL_LVDS3] = { RES_PLL_PLL_LVDS3 },
    [PLL_LVDS4] = { RES_PLL_PLL_LVDS4 },
    [PLL_CPU1A] = { RES_PLL_PLL_CPU1A },
    [PLL_CPU1B] = { RES_PLL_PLL_CPU1B },
    [PLL_CPU2] = { RES_PLL_PLL_CPU2 },
    [PLL_GPU1] = { RES_PLL_PLL_GPU1 },
    [PLL_GPU2] = { RES_PLL_PLL_GPU2 },
    [PLL_VPU] = { RES_PLL_PLL_VPU },
    [PLL_VSN] = { RES_VDSP_PLL_VSN },
    [PLL_HPI] = { RES_PLL_PLL_HPI },
    [PLL_HIS] = { RES_PLL_PLL_HIS },
    [PLL_DDR] = { RES_DDR_CFG_DDR_CFG, APB_PLL_DDR_BASE }
};

/* Find PLL from resource ID. */
static pll_e res2pll(uint32_t resid)
{
    for (pll_e i = 0; i < PLL_MAX; i++) {
        if (g_pll_res[i].resid == resid) {
            return i;
        }
    }

    return PLL_INVALID;
}

static const pll_config_t *get_config(pll_e pll)
{
    const pll_config_t *config = NULL;

    for (size_t i = 0; i < sizeof(pll_configs) / sizeof(pll_configs[0]); i++) {
        if (pll_configs[i].pll == pll) {
            config = &pll_configs[i];
            break;
        }
    }

    return config;
}

pll_handle_t hal_pll_create_handle(uint32_t resid)
{
    addr_t      pll_addr;
    int32_t     index;
    pll_e       pll;

    if (!res_get_info_by_id(resid, &pll_addr, &index) &&
            (pll = res2pll(resid)) != PLL_INVALID) {
        /* The PLL resource is valid. */
        //LTRACEF("pll handle created, resid 0x%x, pll %d\n", resid, pll);
        return (pll_handle_t)resid;
    }
    else {
        //LTRACEF("Can not create pll handle, resid 0x%x\n", resid);
        return (pll_handle_t)0;
    }
}

void hal_pll_delete_handle(pll_handle_t handle)
{
    return;
}

void hal_pll_config(pll_handle_t handle, const pll_config_t *config)
{
    uint32_t            resid = (uint32_t)handle;
    pll_e               pll;
    addr_t              pll_addr;
    int32_t             index;

    if (res_get_info_by_id(resid, &pll_addr, &index) != 0 ||
            (pll = res2pll(resid)) == PLL_INVALID) {
        dprintf(CRITICAL, "invalid pll resource 0x%x\n", resid);
        return;
    }

    if (!config) {
        config = get_config(pll);

        if (!config) {
            dprintf(CRITICAL, "no config for pll %d\n", pll);
            return;
        }
    }

    /* Override PLL address if necessary. */
    if (g_pll_res[pll].paddr) {
        pll_addr = g_pll_res[pll].paddr;
    }

    LTRACEF("pll_init pll %d, addr 0x%x\n", pll, (uint32_t)pll_addr);
    pll_config(pll_addr, config);
}


int hal_pll_get_config(pll_handle_t handle, pll_config_t *config)
{
    uint32_t            resid = (uint32_t)handle;
    pll_e               pll;
    addr_t              pll_addr;
    int32_t             index;

    if (!config) { return -1; }

    if (res_get_info_by_id(resid, &pll_addr, &index) != 0 ||
            (pll = res2pll(resid)) == PLL_INVALID) {
        dprintf(CRITICAL, "invalid pll resource 0x%x\n", resid);
        return 0;
    }

    /* Override PLL address if necessary. */
    if (g_pll_res[pll].paddr) {
        pll_addr = g_pll_res[pll].paddr;
    }

    config->pll = pll;
    pll_config_get(pll_addr, config);
    return 0;
}
static unsigned long calculate_pll_rate(pll_config_t *config, int plldiv)
{
    /*2^24*/
#define FRACM (16777216)
    unsigned long tmp = 0;
    unsigned long freq = 0;

    if (config->integer == 0) {
        tmp = config->frac * (1000) / (FRACM / 1000);
    }

    freq = (tmp + (unsigned long)(config->fbdiv * (1000000))) * ((
                24) / config->refdiv);

    if (plldiv == PLL_ROOT) {//root
        freq = freq / config->postdiv[0] / config->postdiv[1];
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        freq = freq / config->out_div[plldiv - 1];
    }
    else if (plldiv == PLL_DUMMY_ROOT) {
        return freq;
    }
    else {
        freq = 0;
    }

    return freq;
}

static bool is_valid_pll_config(pll_config_t *conf)
{
    // check the pll limit
    //fvco 800MHZ~3200MHZ
    unsigned long fvco;
    unsigned pfd;
    fvco = calculate_pll_rate(conf, PLL_DUMMY_ROOT);

    if (fvco < 800000000 || fvco > 3200000000) {
        return false;
    }

    //pfd >10Mhz in fac mode. and >5Mhz in int mode
    pfd = 24000000 / conf->refdiv;

    if (conf->integer && pfd <= 5000000) {
        return false;
    }

    if (conf->integer == 0 && pfd <= 10000000) {
        return false;
    }

    if (conf->integer == 0 && conf->frac >= 16777216) {
        return false;
    }

    return true;
}

unsigned long hal_pll_calcurate(unsigned long freq, int plldiv,
                                pll_config_t *config)
{
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    pll_config_t conf;
    conf = *config;

    if (plldiv == PLL_DUMMY_ROOT) {
        int fbdiv = 1, refdiv = 1;
        int refdiv_max;
        int fbdiv_max, fbdiv_min;

        if (conf.integer) {
            fbdiv_min = 16;
            fbdiv_max = 640;
            refdiv_max = 4;
        }
        else {
            fbdiv_min = 20;
            fbdiv_max = 320;
            refdiv_max = 2;
        }

        freq = MIN(freq, 3200000000);
        freq = MAX(freq, 800000000);

        for (refdiv = 1; refdiv <= refdiv_max; refdiv++) {
            int fbdivmin, fbdivmax;
            int fbtmp = ((freq * refdiv / 24) - (conf.frac * 1000000) / 16777216) /
                        1000000;
            fbdivmin = MAX(fbtmp - 2, fbdiv_min);
            fbdivmax = MIN(fbtmp + 2, fbdiv_max);

            for (fbdiv = fbdivmin; fbdiv <= fbdivmax; fbdiv++) {
                conf.fbdiv = fbdiv;
                conf.refdiv = refdiv;

                if (!is_valid_pll_config(&conf)) { continue; }

                rate = calculate_pll_rate(&conf, plldiv);
                diff = abs_clk(freq, rate);

                //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d diff %lu \n", clk->name, freq, rate, i, diff);
                if (diff == 0) {
                    config->fbdiv = conf.fbdiv;
                    config->refdiv = conf.refdiv;
                    return freq;
                }

                if (diff < bestratediff) {
                    bestratediff = diff;
                    bestrate = rate;
                    config->fbdiv = conf.fbdiv;
                    config->refdiv = conf.refdiv;
                }
            }
        }

        conf = *config;

        if (bestrate > freq && conf.integer == 1) {
            conf.fbdiv--;
        }

        conf.frac = ((freq / 1000 * conf.refdiv / 24) - conf.fbdiv * 1000) *
                    16777216 / 1000;

        if (conf.frac) {
            conf.integer = 0;
        }

        if (!is_valid_pll_config(&conf)) { return bestrate; }

        rate = calculate_pll_rate(&conf, plldiv);
        //printf("bestrate %lu freq %lu frac %u\n", rate, freq, conf.frac);
        *config = conf;
        bestrate = rate;
    }
    else if (plldiv == PLL_ROOT) {
        int i, j;
        int postdiv0div_max = 7;
        int postdiv1div_max = 7;
        maxdiv = postdiv0div_max * postdiv1div_max;
        maxdiv = MIN(UINT32_MAX / freq, maxdiv);
        mindiv = 1;
        //initial value of bestdiv is the fix div or current div.
        bestdiv = mindiv;

        for (i = 1; i <= postdiv0div_max; i++) {
            for (j = 1; j <= postdiv1div_max; j++) {
                if (((i * j) < (int)mindiv) || ((i * j) > (int)maxdiv)) {
                    break;
                }

                if (i < j) { //post0 >= post1
                    break;
                }

                rate = hal_pll_calcurate(i * j * freq, PLL_DUMMY_ROOT, &conf);
                diff = abs_clk(freq, rate / (i * j));

                if (diff == 0) {
                    *config = conf;
                    config->postdiv[0] = i;
                    config->postdiv[1] = j;
                    return freq;
                }

                if (diff < bestratediff) {
                    bestratediff = diff;
                    bestdiv = (i * j);
                    bestrate = rate / bestdiv;
                    *config = conf;
                    config->postdiv[0] = i;
                    config->postdiv[1] = j;
                }
            }
        }
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        int div_max  = 16;
        int i;
        maxdiv = div_max;
        maxdiv = MIN(UINT32_MAX / freq, maxdiv);
        mindiv = 1;
        maxdiv = MAX(maxdiv, mindiv);
        //initial value of bestdiv is the fix div or current div.
        bestdiv = mindiv;

        for (i = mindiv; i <= (int)maxdiv; i++) {
            rate = hal_pll_calcurate(i * freq, PLL_DUMMY_ROOT, &conf);
            diff = abs_clk(freq, rate / i);

            if (diff == 0) {
                *config = conf;
                config->out_div[plldiv - 1] = i;
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestdiv = i;
                bestrate = rate / bestdiv;
                *config = conf;
                config->out_div[plldiv - 1] = i;
            }
        }
    }

    return bestrate;
}

unsigned long hal_pll_set_rate(pll_handle_t handle, unsigned long freq,
                               int plltype)
{
    pll_config_t conf;
    unsigned long rate;
    hal_pll_get_config(handle, &conf);
    stuff_valid_value(&conf);
    rate = hal_pll_calcurate(freq, plltype, &conf);
    hal_pll_config(handle, &conf);
    return rate;
}

