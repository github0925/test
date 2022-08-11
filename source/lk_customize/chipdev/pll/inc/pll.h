/*
 * pll.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: PFPLL driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef _PLL_H
#define _PLL_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef enum pll {
    PLL_INVALID = -1,

    /* safety */
    PLL1 = 0,
    PLL2,

    /* secure */
    PLL3,
    PLL4,
    PLL5,
    PLL6,
    PLL7,

    /* disp */
    PLL_DISP,
    PLL_LVDS1,
    PLL_LVDS2,
    PLL_LVDS3,
    PLL_LVDS4,

    /* soc */
    PLL_CPU1A,
    PLL_CPU1B,
    PLL_CPU2,
    PLL_GPU1,
    PLL_GPU2,
    PLL_VPU,
    PLL_VSN,
    PLL_HPI,
    PLL_HIS,
    PLL_DDR,

    PLL_MAX,
} pll_e;

/* Outside divider A/B/C/D. */
typedef enum pll_out_div {
    PLL_OUT_DIV_A = 0,
    PLL_OUT_DIV_B,
    PLL_OUT_DIV_C,
    PLL_OUT_DIV_D,
    PLL_OUT_DIV_MAX,
} pll_out_div_e;

typedef struct pll_config {
    /* PLL I */
    pll_e       pll;

    /* True if PFPLL works in integer mode, otherwise fractional
     * mode.
     */
    bool        integer;

    /* True to enable spread spectrum which slightly jitters PLL
     * output frequency to reduce EMI. The SS modulator works only
     * in fractional mode.
     */
    bool        spread_spectrum;
    /* True to using downspread, false for center-spread*/
    bool    downspread;
    /* spread :1~31, 0.1% ~ 3.1% */
    int     spread;

    /* Reference divide value [5:0]: 1~63. */
    uint32_t    refdiv;

    /* Integer part of feedback divide value [11:0].
     * 16~640 in integer mode, 20~320 in fractional mode.
     */
    uint32_t    fbdiv;

    /* postdiv1 & postdiv2 [2:0]: 1~7
     *  postdiv1 = PLL_DIV_CFG(root)
     *  postdiv2 = POSTDIV
     */
    uint32_t    postdiv[2];

    /* Fractional part of feedback divide value [23:0]. Only for
     * fraction mode. fbdiv.frac = (float)frac / pow(2,24).
     */
    uint32_t    frac;

    /* Outside divider a/b/c/d. */
    uint32_t    out_div[PLL_OUT_DIV_MAX];
} pll_config_t;

void pll_config(paddr_t pll_base, const pll_config_t *config);
void pll_config_get(paddr_t pll_base, pll_config_t *config);
void stuff_valid_value(pll_config_t *config);

#endif /* _PLL_H */

