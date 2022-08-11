/*
 * pll.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: PFPLL driver.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <reg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <trace.h>

#include "lib/reg.h"
#include "pll.h"

#define LOCAL_TRACE 0

/*
 * All PLLs are sc_ss_pfpll which support spread spectrum and
 * fractional mode. PLL 2~7 and PLL_LVDS 1~4 support on the fly
 * frequency adjust.
 *
 * PLL output frequency formula:
 *
 *  Fref = 24 MHz
 *  fbdiv = PLL_FBDIV + PLL_FRAC/pow(2,24)
 *  Fout = Fref / PLL_DIV_REFDIV * fbdiv
 *
 *  Fout_postdiv (root) =  Fout / (PLL_DIV_POSTDIV1 * PLL_DIV_POSTDIV2)
 *  Fout_diva = Fout / (PLL_OUT_DIV_1_DIV_NUM_A + 1)
 *  Fout_divb = Fout / (PLL_OUT_DIV_1_DIV_NUM_B + 1)
 *  Fout_divc = Fout / (PLL_OUT_DIV_1_DIV_NUM_C + 1)
 *  Fout_divd = Fout / (PLL_OUT_DIV_1_DIV_NUM_D + 1)
 */

/* Control register */
#define PLL_CTRL                        0x0
#  define PLL_CTRL_PLLEN                0
#  define PLL_CTRL_INT_MODE             1
#  define PLL_CTRL_FOUTPOSTDIVEN        4
#  define PLL_CTRL_PLLPOSTCG_EN         16
#  define PLL_CTRL_PLL_DIVA_CG_EN       18
#  define PLL_CTRL_PLL_DIVB_CG_EN       19
#  define PLL_CTRL_PLL_DIVC_CG_EN       20
#  define PLL_CTRL_PLL_DIVD_CG_EN       21
#  define PLL_CTRL_LOCK                 31

/* Divide number register */
#define PLL_DIV                         0x4
#  define PLL_DIV_REFDIV                0
#  define PLL_DIV_POSTDIV1              6
#  define PLL_DIV_POSTDIV2              9

/* Feedback divider register. Integer part of feedback division. */
#define PLL_FBDIV                       0x8

/* Fractional portion setting register. Fractional part of feedback division. */
#define PLL_FRAC                        0xc

/* Spread spectrum mode setting register */
#define PLL_SSMOD                       0x18
#  define PLL_SSMOD_MOD_RESET           0
#  define PLL_SSMOD_DISABLE_SSCG    1
#  define PLL_SSMOD_RESETPTR        2
#  define PLL_SSMOD_DOWNSPREAD      3
#  define PLL_SSMOD_SPREAD      4   /*4 ~ 8*/
#  define PLL_SSMOD_DIVVAL      9   /*9 ~ 14*/

/* Outside divider register 1 */
#define PLL_OUT_DIV_1                   0x20
#  define PLL_OUT_DIV_1_DIV_NUM_A       0
#  define PLL_OUT_DIV_1_DIV_BUSY_A      15
#  define PLL_OUT_DIV_1_DIV_NUM_B       16
#  define PLL_OUT_DIV_1_DIV_BUSY_B      31

/* Outside divider register 2 */
#define PLL_OUT_DIV_2                   0x24
#  define PLL_OUT_DIV_2_DIV_NUM_C       0
#  define PLL_OUT_DIV_2_DIV_BUSY_C      15
#  define PLL_OUT_DIV_2_DIV_NUM_D       16
#  define PLL_OUT_DIV_2_DIV_BUSY_D      31

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

static void validate_pll_cfg(const pll_config_t *config)
{
    ASSERT(config->refdiv >= PFPLL_REFDIV_MIN &&
           config->refdiv <= PFPLL_REFDIV_MAX);
    ASSERT(config->postdiv[0] >= config->postdiv[1]);

    for (int i = 0; i < 2; i++) {
        ASSERT(config->postdiv[i] >= PFPLL_POSTDIV_MIN &&
               config->postdiv[i] <= PFPLL_POSTDIV_MAX);
    }

    if (config->integer) {
        ASSERT(!config->spread_spectrum);
        ASSERT(config->fbdiv >= PFPLL_FBDIV_INTEGER_MIN &&
               config->fbdiv <= PFPLL_FBDIV_INTEGER_MAX);
    }
    else {
        ASSERT(config->fbdiv >= PFPLL_FBDIV_FRAC_MIN &&
               config->fbdiv <= PFPLL_FBDIV_FRAC_MAX);
        ASSERT(config->frac >= PFPLL_FRAC_MIN &&
               config->frac <= PFPLL_FRAC_MAX);
    }

    if (config->spread_spectrum) {
        ASSERT(config->spread >= 1 && config->spread <= 31);
    }
}

void pll_config(paddr_t pll_base, const pll_config_t *config)
{
    addr_t base = _ioaddr(pll_base);
    uint32_t val;
    LTRACEF("pll_config: 0x%x, integer %d, ss %d, down %d, spread %d\n",
            (uint32_t)pll_base, config->integer, config->spread_spectrum,
            config->downspread, config->spread);
    LTRACEF("fbdiv 0x%x, refdiv 0x%x, postdiv 0x%x 0x%x\n",
            config->fbdiv, config->refdiv, config->postdiv[0], config->postdiv[1]);
    LTRACEF("fractional 0x%x\n", config->frac);
    LTRACEF("outdiv 0x%x 0x%x 0x%x 0x%x\n",
            config->out_div[0], config->out_div[1],
            config->out_div[2], config->out_div[3]);
    validate_pll_cfg(config);
    /* Configure refdiv, fbdiv and postdiv. */
    writel(config->fbdiv, base + PLL_FBDIV);
    writel((config->refdiv << PLL_DIV_REFDIV) +
           (config->postdiv[0] << PLL_DIV_POSTDIV1) +
           (config->postdiv[1] << PLL_DIV_POSTDIV2),
           base + PLL_DIV);

    if (config->integer) {
        /* Integer mode. */
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1, 1);
    }
    else {
        /* Fractional mode. Configure fbdiv fractional part. */
        writel(config->frac, base + PLL_FRAC);
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1, 0);
    }

    /* Enable PLL. */
    RMWREG32(base + PLL_CTRL, PLL_CTRL_PLLEN, 1, 1);

    /* Wait until PLL is locked. */
    while (!(readl(base + PLL_CTRL) & (1 << PLL_CTRL_LOCK)));

    /* Enable output clocks, including postdiv and outside div a/b/c/d.
     * XXX_CG_EN=1 means turning on the clock.
     */
    int out_div_en[PLL_OUT_DIV_MAX];

    for (pll_out_div_e i = PLL_OUT_DIV_A; i < PLL_OUT_DIV_MAX; i++) {
        out_div_en[i] = (config->out_div[i] > 0);
    }

    val = readl(base + PLL_CTRL);
    val |= (1 << PLL_CTRL_FOUTPOSTDIVEN)
           + (1 << PLL_CTRL_PLLPOSTCG_EN)
           + (out_div_en[PLL_OUT_DIV_A] << PLL_CTRL_PLL_DIVA_CG_EN)
           + (out_div_en[PLL_OUT_DIV_B] << PLL_CTRL_PLL_DIVB_CG_EN)
           + (out_div_en[PLL_OUT_DIV_C] << PLL_CTRL_PLL_DIVC_CG_EN)
           + (out_div_en[PLL_OUT_DIV_D] << PLL_CTRL_PLL_DIVD_CG_EN);
    writel(val, base + PLL_CTRL);
    struct pll_out_div_cfg {
        uint32_t reg;
        uint32_t num_bit;
        uint32_t busy_bit;
    } out_div_cfg[PLL_OUT_DIV_MAX] = {
        [PLL_OUT_DIV_A] = {
            PLL_OUT_DIV_1,
            PLL_OUT_DIV_1_DIV_NUM_A,
            PLL_OUT_DIV_1_DIV_BUSY_A,
        },
        [PLL_OUT_DIV_B] = {
            PLL_OUT_DIV_1,
            PLL_OUT_DIV_1_DIV_NUM_B,
            PLL_OUT_DIV_1_DIV_BUSY_B,
        },
        [PLL_OUT_DIV_C] = {
            PLL_OUT_DIV_2,
            PLL_OUT_DIV_2_DIV_NUM_C,
            PLL_OUT_DIV_2_DIV_BUSY_C,
        },
        [PLL_OUT_DIV_D] = {
            PLL_OUT_DIV_2,
            PLL_OUT_DIV_2_DIV_NUM_D,
            PLL_OUT_DIV_2_DIV_BUSY_D,
        },
    };

    /* Configure outside divider clocks. */
    for (pll_out_div_e i = PLL_OUT_DIV_A; i < PLL_OUT_DIV_MAX; i++) {
        if (out_div_en[i]) {
            /* Configure outside division number. */
            RMWREG32(base + out_div_cfg[i].reg,
                     out_div_cfg[i].num_bit,
                     4,
                     config->out_div[i] - 1);

            /* Wait div busy bit. */
            while (readl(base + out_div_cfg[i].reg) &
                    (1 << out_div_cfg[i].busy_bit));
        }
    }

    /* Enable SS modulator if necessary. */
    /* Modulators may be required for display modules to reduce EMI. */
    if (config->spread_spectrum) {
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_DISABLE_SSCG, 1, 0);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_RESETPTR, 1, 0);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_DOWNSPREAD, 1, config->downspread);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_SPREAD, 5, config->spread);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_DIVVAL, 5, 15);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_MOD_RESET, 1, 0);
        /* TODO - add modulation parameters. */
    }
    else {
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_DISABLE_SSCG, 1, 1);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_RESETPTR, 1, 1);
        RMWREG32(base + PLL_SSMOD, PLL_SSMOD_MOD_RESET, 1, 1);
    }
}

void pll_config_get(paddr_t pll_base, pll_config_t *config)
{
    addr_t base = _ioaddr(pll_base);
    uint32_t val;
    /* get refdiv, fbdiv and postdiv. */
    config->fbdiv = readl(base + PLL_FBDIV);
    val = readl(base + PLL_DIV);
    config->refdiv = (val >> PLL_DIV_REFDIV) & ((1 << 6) - 1);
    config->postdiv[0] = (val >> PLL_DIV_POSTDIV1) & ((1 << 3) - 1);
    config->postdiv[1] = (val >> PLL_DIV_POSTDIV2) & ((1 << 3) - 1);
    val = readl(base + PLL_CTRL);
    config->integer = (val >> PLL_CTRL_INT_MODE) & 1;

    if (!config->integer) {
        /* Fractional mode. get fbdiv fractional part. */
        config->frac = readl(base + PLL_FRAC);
        config->spread_spectrum = (((readl(base + PLL_SSMOD) >>
                                     PLL_SSMOD_MOD_RESET) & 1) == 0);

        if (config->spread_spectrum) {
            config->downspread = ((readl(base + PLL_SSMOD) >> PLL_SSMOD_DOWNSPREAD) &
                                  1);
            config->spread = ((readl(base + PLL_SSMOD) >> PLL_SSMOD_SPREAD) & 0x1f);
        }
    }

    int out_div_en[PLL_OUT_DIV_MAX];

    for (pll_out_div_e i = PLL_OUT_DIV_A; i < PLL_OUT_DIV_MAX; i++) {
        out_div_en[i] = 0;
    }

    val = readl(base + PLL_CTRL);
    out_div_en[PLL_OUT_DIV_A] = val >> PLL_CTRL_PLL_DIVA_CG_EN & 1;
    out_div_en[PLL_OUT_DIV_B] = val >> PLL_CTRL_PLL_DIVB_CG_EN & 1;
    out_div_en[PLL_OUT_DIV_C] = val >> PLL_CTRL_PLL_DIVC_CG_EN & 1;
    out_div_en[PLL_OUT_DIV_D] = val >> PLL_CTRL_PLL_DIVD_CG_EN & 1;
    struct pll_out_div_cfg {
        uint32_t reg;
        uint32_t num_bit;
        uint32_t busy_bit;
    } out_div_cfg[PLL_OUT_DIV_MAX] = {
        [PLL_OUT_DIV_A] = {
            PLL_OUT_DIV_1,
            PLL_OUT_DIV_1_DIV_NUM_A,
            PLL_OUT_DIV_1_DIV_BUSY_A,
        },
        [PLL_OUT_DIV_B] = {
            PLL_OUT_DIV_1,
            PLL_OUT_DIV_1_DIV_NUM_B,
            PLL_OUT_DIV_1_DIV_BUSY_B,
        },
        [PLL_OUT_DIV_C] = {
            PLL_OUT_DIV_2,
            PLL_OUT_DIV_2_DIV_NUM_C,
            PLL_OUT_DIV_2_DIV_BUSY_C,
        },
        [PLL_OUT_DIV_D] = {
            PLL_OUT_DIV_2,
            PLL_OUT_DIV_2_DIV_NUM_D,
            PLL_OUT_DIV_2_DIV_BUSY_D,
        },
    };

    /* get outside divider clocks. */
    for (pll_out_div_e i = PLL_OUT_DIV_A; i < PLL_OUT_DIV_MAX; i++) {
        if (out_div_en[i]) {
            val = readl(base + out_div_cfg[i].reg);
            config->out_div[i] = ((val >> out_div_cfg[i].num_bit) & 0xf) + 1;
        }
    }
}
void stuff_valid_value(pll_config_t *config)
{
    if (!(config->refdiv >= PFPLL_REFDIV_MIN &&
            config->refdiv <= PFPLL_REFDIV_MAX)) {
        config->refdiv = PFPLL_REFDIV_MIN;
    }

    for (int i = 0; i < 2; i++) {
        if (!(config->postdiv[i] >= PFPLL_POSTDIV_MIN &&
                config->postdiv[i] <= PFPLL_POSTDIV_MAX)) {
            config->postdiv[i] = PFPLL_POSTDIV_MIN;
        }
    }

    if (!(config->postdiv[0] >= config->postdiv[1])) {
        config->postdiv[0] = config->postdiv[1];
    }

    if (config->integer) {
        if (config->spread_spectrum) {
            config->spread_spectrum = 0;
        }

        if (!(config->fbdiv >= PFPLL_FBDIV_INTEGER_MIN &&
                config->fbdiv <= PFPLL_FBDIV_INTEGER_MAX)) {
            config->fbdiv = PFPLL_FBDIV_INTEGER_MIN;
        }
    }
    else {
        if (!(config->fbdiv >= PFPLL_FBDIV_FRAC_MIN &&
                config->fbdiv <= PFPLL_FBDIV_FRAC_MAX)) {
            config->fbdiv = PFPLL_FBDIV_FRAC_MIN;
        }

        if (!(config->frac >= PFPLL_FRAC_MIN &&
                config->frac <= PFPLL_FRAC_MAX)) {
            config->frac = PFPLL_FRAC_MIN;
        }
    }
}

