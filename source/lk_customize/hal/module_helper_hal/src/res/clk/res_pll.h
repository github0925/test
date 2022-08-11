#include <pll_hal.h>
#include <bits.h>
/* Division number limits. */
#define PFPLL_REFDIV_MIN                1
#define PFPLL_REFDIV_MAX                63

#define PFPLL_POSTDIV_MIN               1
#define PFPLL_POSTDIV_MAX               7

#define PFPLL_FBDIV_INTEGER_MIN         16
#define PFPLL_FBDIV_INTEGER_MAX         640
#define PFPLL_FBDIV_FRAC_MIN            20
#define PFPLL_FBDIV_FRAC_MAX            320

#define PFPLL_FRAC_MIN                  0
#define PFPLL_FRAC_MAX                  (int)BIT_MASK(24)

static pll_config_t pll_cfg_1064M = {    // DDR-2133
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 133,
    .frac = 0,
};
static pll_config_t pll_cfg_1083M = {
    .integer = false,
    .spread_spectrum = false,
    .refdiv = 2,
    .fbdiv = 90,
    .frac = 5315022,
};
static pll_config_t pll_cfg_1328M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 166,
    .frac = 0,
};
static pll_config_t pll_cfg_1456M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 182,
    .frac = 0,
};
static pll_config_t pll_cfg_1474dot56M = {
    .integer = false,
    .spread_spectrum = false,
    .refdiv = 2,
    .fbdiv = 122,
    .frac = 14763950,
};

static pll_config_t pll_cfg_1496M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 187,
    .frac = 0,
};
static pll_config_t pll_cfg_1592M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 199,
    .frac = 0,
};

static pll_config_t pll_cfg_1600M = {    // For DDR-800
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 200,
    .frac = 0,
};
static pll_config_t pll_cfg_1656M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 207,
    .frac = 0,
};
static pll_config_t pll_cfg_1800M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 2,
    .fbdiv = 150,
    .frac = 0,
};

static pll_config_t pll_cfg_1896M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 237,
    .frac = 0,
};
static pll_config_t pll_cfg_2000M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 250,
    .frac = 0,
};
static pll_config_t pll_cfg_2079M = {
    .integer = false,
    .spread_spectrum = false,
    .refdiv = 2,
    .fbdiv = 173,
    .frac = 4194304,
};

static pll_config_t pll_cfg_2128M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 3,
    .fbdiv = 266,
    .frac = 0,
};
static pll_config_t pll_cfg_2664M = {
    .integer = true,
    .spread_spectrum = false,
    .refdiv = 2,
    .fbdiv = 222,
    .frac = 0,
};

static pll_config_t *pll_config_database[] = {
    &pll_cfg_1064M,
    &pll_cfg_1083M,
    &pll_cfg_1328M,
    &pll_cfg_1456M,
    &pll_cfg_1474dot56M,
    &pll_cfg_1496M,
    &pll_cfg_1592M,
    &pll_cfg_1600M,
    &pll_cfg_1656M,
    &pll_cfg_1800M,
    &pll_cfg_1896M,
    &pll_cfg_2000M,
    &pll_cfg_2079M,
    &pll_cfg_2128M,
    &pll_cfg_2664M,
};

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

const pll_config_t *find_config_in_database(struct clk *clk, int plldiv,
        unsigned long freq)
{
    int i;
    int size = clk->pll.ratetable_size;

    for (i = 0; i < size; i++)
        if (calculate_pll_rate(clk->pll.ratetable[i], plldiv) == freq) {
            return clk->pll.ratetable[i];
        }

    return NULL;
}

static int set_pll_endis(unsigned long resid, int plldiv, bool isenable)
{
//TODO
    return 0;
}
static unsigned long get_pll_rate(struct clk *clk,
                                  unsigned long prate, bool bymonitor)
{
    pll_handle_t pll;
    unsigned long rate = 0;
    pll_config_t conf = {0};
    int plldiv = clk->pll.plldiv;
    //int ret=0;
    pll =  hal_pll_create_handle(clk->resid);

    if (pll == (pll_handle_t)0) {
        return 0;
    }

    hal_pll_get_config(pll, &conf);

    if (bymonitor) {
        rate = calculate_pll_rate(&conf, plldiv);
    }
    else {
        if (plldiv == PLL_DUMMY_ROOT) {
            rate = calculate_pll_rate(&conf, plldiv);
        }
        else if (plldiv == PLL_ROOT) {
            rate = prate / conf.postdiv[0] / conf.postdiv[1];
        }
        else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
            if (conf.out_div[plldiv - 1] != 0) {
                rate = prate / conf.out_div[plldiv - 1];
            }
            else {
                rate = 0;
            }
        }
    }

    hal_pll_delete_handle(pll);
    return rate;
}

static int set_pll_rate(struct clk *clk,
                        unsigned long prate, unsigned long rate)
{
    const pll_config_t *config;
    pll_config_t cur = {0};
    int div;
    int plldiv = clk->pll.plldiv;
    pll_handle_t pll =  hal_pll_create_handle(clk->resid);

    if (pll == (pll_handle_t)0) {
        return 0;
    }

    hal_pll_get_config(pll, &cur);
    stuff_valid_value(&cur);

    if (plldiv == PLL_DUMMY_ROOT) { //will ignore the prate
        if (clk->pll.moreprecise) {
            config = &clk->pll.config;
            ASSERT(rate == calculate_pll_rate(&clk->pll.config, PLL_DUMMY_ROOT));
        }
        else {
            config = find_config_in_database(clk, plldiv, rate);
        }

        ASSERT(config != NULL);
        cur.integer = config->integer;
        cur.frac = config->frac;

        //cur.spread_spectrum = config->spread_spectrum;
        if (cur.spread_spectrum) {
            //cur.downspread = config->downspread;
            //cur.spread = config->spread;
            if (cur.integer) {
                cur.integer = false;
                cur.frac = 0;
            }
        }

        cur.refdiv = config->refdiv;
        cur.fbdiv = config->fbdiv;
    }
    else if (plldiv == PLL_ROOT) {
        int i, j;
        int post0div_max = 7;
        int post1div_max = 7;
        unsigned int div = 0;
        div = prate / rate;

        for (i = 1; i <= post0div_max; i++) {
            for (j = 1; j <= post1div_max; j++) {
                if (i < j) { continue; } //post0 >= post1

                if ((i * j) == (int)div) {
                    goto found;
                }
            }
        }

        ASSERT(i <= post0div_max && j <= post1div_max);
        ASSERT(i >= j);
found:
        cur.postdiv[0] = i;
        cur.postdiv[1] = j;
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        div = prate / rate;
        cur.out_div[plldiv - 1] = div;
    }

    hal_pll_config(pll, &cur);
    hal_pll_delete_handle(pll);
    return 0;
}
static bool is_valid_pll_config(struct clk *clk, pll_config_t *conf)
{
    if (clk->pll.plldiv == PLL_DUMMY_ROOT) { // check the pll limit
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
    }

    return true;
}
unsigned long get_pll_dummy_root_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int size = clk->pll.ratetable_size;
    int i;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, rate;
    unsigned long diff;
    ASSERT(clk->pll.plldiv == PLL_DUMMY_ROOT);

    if (clk->pll.moreprecise) {
        pll_config_t conf = {0};
        int fbdiv = 1, refdiv = 1;
        int refdiv_max;
        int fbdiv_max, fbdiv_min;
        pll_handle_t pll =  hal_pll_create_handle(clk->resid);

        if (pll == (pll_handle_t)0) {
            return 0;
        }

        hal_pll_get_config(pll, &clk->pll.config);
        hal_pll_delete_handle(pll);
        stuff_valid_value(&clk->pll.config);
        conf.integer = clk->pll.config.integer;
        conf.frac = clk->pll.config.frac;
        conf.spread_spectrum = clk->pll.config.spread_spectrum;
        conf.downspread = clk->pll.config.downspread;
        conf.spread = clk->pll.config.spread;
        bestrate = calculate_pll_rate(&clk->pll.config, clk->pll.plldiv);

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

                if (!is_valid_pll_config(clk, &conf)) { continue; }

                rate = calculate_pll_rate(&conf, clk->pll.plldiv);

                //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d\n", clk->name, freq, rate, i);
                if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) { continue; }

                diff = abs_clk(freq, rate);

                //dprintf(CRITICAL, "pll %s req %lu round get rate %lu diff %lu \n", clk->name, freq, rate, diff);
                if (diff == 0) {
                    clk->pll.config = conf;
                    return freq;
                }

                if (diff < bestratediff) {
                    bestratediff = diff;
                    bestrate = rate;
                    clk->pll.config = conf;
                }
            }
        }

        conf = clk->pll.config;

        if (bestrate > freq && conf.integer == 1) {
            conf.fbdiv--;
        }

        conf.frac = ((freq / 1000 * conf.refdiv / 24) - conf.fbdiv * 1000) *
                    16777216 / 1000;

        if (conf.frac) {
            conf.integer = 0;
        }

        if (!is_valid_pll_config(clk, &conf)) { return bestrate; }

        rate = calculate_pll_rate(&conf, clk->pll.plldiv);

        if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) { return bestrate; }

        //printf("bestrate %lu freq %lu frac %u\n", rate, freq, conf.frac);
        clk->pll.config = conf;
        bestrate = rate;
        //printf("bestrate %lu freq %lu frac %u name %s\n", bestrate, freq, conf.frac, clk->name);
        return bestrate;
    }
    else {
        for (i = 0; i < size; i++) {
            rate = calculate_pll_rate(clk->pll.ratetable[i], clk->pll.plldiv);

            //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d\n", clk->name, freq, rate, i);
            if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) { continue; }

            diff = abs_clk(freq, rate);

            //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d diff %lu \n", clk->name, freq, rate, i, diff);
            if (diff == 0) {
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestrate = rate;
            }
        }
    }

    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, *prate);
    return bestrate;
}

static unsigned long get_pll_root_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i, j;
    int postdiv0div_max = 7;
    int postdiv1div_max = 7;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    struct clk *p = NULL;

    if (pindex != -1) {
        p = clk->parents[pindex];
    }

    maxdiv = MIN(clk->maxdiv, postdiv0div_max * postdiv1div_max);
    maxdiv = MIN(UINT32_MAX / freq, maxdiv);
    mindiv = MAX(clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
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

            prate_cur = *prate;
            rate = res_clk_div_round_rate(clk, p, i * j, &prate_cur, freq);

            if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

            diff = abs_clk(freq, prate_cur / (i * j));

            if (diff == 0) {
                *prate = prate_cur;
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestprate = prate_cur;
                bestdiv = (i * j);
            }
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}


static unsigned long get_pll_div_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    int div_max = 16;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    struct clk *p = NULL;

    if (pindex != -1) {
        p = clk->parents[pindex];
    }

    maxdiv = MIN(clk->maxdiv, div_max);
    maxdiv = MIN(UINT32_MAX / freq, maxdiv);
    mindiv = MAX(clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = 1; i <= (int)maxdiv; i++) {
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / i);

        if (diff == 0) {
            *prate = prate_cur;
            return freq;
        }

        if (diff < bestratediff) {
            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = i;
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}

static unsigned long get_pll_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int plldiv = clk->pll.plldiv;

    if (plldiv == PLL_DUMMY_ROOT) { // ignore prate
        return get_pll_dummy_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv == PLL_ROOT) {
        return get_pll_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        return get_pll_div_round_rate_clk(clk, pindex, prate, freq);
    }
    else {
        //could not be here
        ASSERT(0);
    }

    return 0;
}
static pll_config_t *pll_1_cfg[] = {
    &pll_cfg_2000M,
};

static pll_config_t *pll_2_cfg[] = {
    &pll_cfg_1600M,
};

static pll_config_t *pll_3_cfg[] = {
    &pll_cfg_1800M,
};

static pll_config_t *pll_4_cfg[] = {
    &pll_cfg_2000M,
};

static pll_config_t *pll_5_cfg[] = {
    &pll_cfg_1592M,
};

static pll_config_t *pll_6_cfg[] = {
    &pll_cfg_1474dot56M,
};

static pll_config_t *pll_7_cfg[] = {
    &pll_cfg_1083M,
};

static pll_config_t *pll_lvds1234_disp_cfg[] = {
    &pll_cfg_2079M,
};

static pll_config_t *pll_cpu1a_cfg[] = {
    &pll_cfg_1656M,
};
static pll_config_t *pll_cpu1b_cfg[] = {
    &pll_cfg_1328M,
};
static pll_config_t *pll_cpu2_cfg[] = {
    &pll_cfg_1496M,
};
static pll_config_t *pll_gpu1_cfg[] = {
    &pll_cfg_1600M,
};
static pll_config_t *pll_gpu2_cfg[] = {
    &pll_cfg_1456M,
};
static pll_config_t *pll_vpu_cfg[] = {
    &pll_cfg_2664M,
};
static pll_config_t *pll_vsn_cfg[] = {
    &pll_cfg_1496M,
};
static pll_config_t *pll_hpi_cfg[] = {
    &pll_cfg_1896M,
};
static pll_config_t *pll_his_cfg[] = {
    &pll_cfg_2000M,
};

#define PLL_RES_ITEMS   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL1, RES_PLL_PLL1, "PLL1", pll_1_cfg),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL2, RES_PLL_PLL2, "PLL2", pll_2_cfg),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL3, RES_PLL_PLL3, "PLL3", pll_3_cfg),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL4, RES_PLL_PLL4, "PLL4", pll_4_cfg),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL5,RES_PLL_PLL5, "PLL5", pll_5_cfg),    \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL6, RES_PLL_PLL6, "PLL6", pll_6_cfg),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL7, RES_PLL_PLL7, "PLL7", pll_7_cfg),   \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DISP, RES_PLL_PLL_DISP, "PLL_DISP", pll_lvds1234_disp_cfg), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS1, RES_PLL_PLL_LVDS1, "PLL_LVDS1", pll_lvds1234_disp_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS2, RES_PLL_PLL_LVDS2, "PLL_LVDS2", pll_lvds1234_disp_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS3, RES_PLL_PLL_LVDS3, "PLL_LVDS3", pll_lvds1234_disp_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS4, RES_PLL_PLL_LVDS4, "PLL_LVDS4", pll_lvds1234_disp_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1A, RES_PLL_PLL_CPU1A, "PLL_CPU1A", pll_cpu1a_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1B, RES_PLL_PLL_CPU1B, "PLL_CPU1B", pll_cpu1b_cfg),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU2, RES_PLL_PLL_CPU2, "PLL_CPU2", pll_cpu2_cfg), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU1, RES_PLL_PLL_GPU1, "PLL_GPU1", pll_gpu1_cfg), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU2, RES_PLL_PLL_GPU2, "PLL_GPU2", pll_gpu2_cfg), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VPU, RES_PLL_PLL_VPU, "PLL_VPU", pll_vpu_cfg),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VSN, RES_VDSP_PLL_VSN, "PLL_VSN", pll_vsn_cfg),   \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HPI, RES_PLL_PLL_HPI, "PLL_HPI", pll_hpi_cfg),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HIS, RES_PLL_PLL_HIS, "PLL_HIS", pll_his_cfg),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DDR, RES_DDR_CFG_DDR_CFG, "PLL_DDR", pll_config_database),
