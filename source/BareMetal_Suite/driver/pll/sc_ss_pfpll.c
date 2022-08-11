/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "sc_ss_pfpll_reg.h"
#include "sc_ss_pfpll.h"

#define FBDIV_MIN   16
#define FBDIV_MAX   640
#define FBDIV_MIN_IN_FRAC_MODE  20
#define FBDIV_MAX_IN_FRAC_MODE  320
#define REFDIV_MIN  1
#define REFDIV_MAX  63
#define POSTDIV_MIN  1
#define POSTDIV_MAX  7
#define POSTDIV1_MIN    1
#define POSTDIV1_MAX    7
#define POSTDIV2_MIN    1
#define POSTDIV2_MAX    7

#define VCO_FREQ_MIN    (800*1000*1000U)
#define VCO_FREQ_MAX    (3200*1000*1000U)

/*
 * PLLs sourced from external OSC (24MHz) in ATB
 */

/*
 * Fout = (Fref/RefDiv)*(FbDiv+(Frac/2^24))/PostDiv
 *  PostDiv = PostDiv1 * PostDiv2
 *
 * Some rules shall be followed:
 *  1) PostDiv1 >= PostDiv2
 *  2) 800MHz <= VCO_FREQ <= 3200MHz
 *  3) 16 <= FBDIV <= 640 for INT mode
 *  4) 20 <= FBDIV <= 320 for Frac mode
 *  5) pfd frequency (ref_clk/refdiv) shall be higher than 10MHz for FRAC
 *     mode, and 5MHz for INT mode.
 *
 *  There are four divs (A/B/C/D, in pll wrapper). Their default values are
 *  properly set per integration requirement. Usually, there is no need for
 *  ATB to change them.
 */
void sc_pfpll_program(U32 base, U32 fbdiv, U32 refdiv, U32 frac,
                      U32 postdiv1, U32 div_abcd)
{
    U32 b = base;
    U32 ctrl = readl(b + PLL_CTRL_OFF);
    U32 div = readl(b + PLL_DIV_OFF);
    assert((postdiv1 >= POSTDIV_MIN) &&(postdiv1 <= POSTDIV_MAX));
    assert((refdiv >= REFDIV_MIN) && (refdiv <= REFDIV_MAX));
    assert((fbdiv >= FBDIV_MIN) && (fbdiv <= FBDIV_MAX));

    U32 freq_vco = (SOC_PLL_REFCLK_FREQ / refdiv) * fbdiv;
    assert((freq_vco >= VCO_FREQ_MIN) && (freq_vco <= VCO_FREQ_MAX));

    /* 1> Set MOD_RESET in PLL_SSMOD to 1 (default is 1) to keep modulator
     * in reset mode.*/

    /* disable pll, gate-off outputs, power-down postdiv */
    /* the clock gates need to be off to protect the divs behind them from
     * being malfunctioned by glitch.
     * Once div been feed (i.e, the vco is stable), the div value can be
     * updated on the fly.
     */
    ctrl &= ~BM_PLL_CTRL_PLLEN;
    ctrl &= ~(BM_PLL_CTRL_PLL_DIVD_CG_EN | BM_PLL_CTRL_PLL_DIVC_CG_EN
              | BM_PLL_CTRL_PLL_DIVB_CG_EN | BM_PLL_CTRL_PLL_DIVA_CG_EN
              | BM_PLL_CTRL_PLLPOSTCG_EN | BM_PLL_CTRL_FOUTPOSTDIVEN
              | BM_PLL_CTRL_BYPASS);

    if (0 == frac) {
        ctrl |= BM_PLL_CTRL_INT_MODE;
    } else {
        DBG("%s: PLL(base=0x%x) been set in FRAC mode\n", __FUNCTION__, b);
        ctrl &= ~BM_PLL_CTRL_INT_MODE;
    }

    writel(ctrl, b + PLL_CTRL_OFF);

    writel(FV_PLL_FRAC_FRAC(frac), b + PLL_FRAC_OFF);

    writel(fbdiv, b + PLL_FBDIV_OFF);
    /* update refdiv and postdiv */
    div &= ~(FM_PLL_DIV_REFDIV | FM_PLL_DIV_POSTDIV1);
    div |= FV_PLL_DIV_REFDIV(refdiv) | FV_PLL_DIV_POSTDIV1(postdiv1);
    writel(div, b + PLL_DIV_OFF);
    /* enable PLL, VCO starting... */
    ctrl |= BM_PLL_CTRL_PLLEN;
    writel(ctrl, b + PLL_CTRL_OFF);

    /* max lock time: 250 input clock cycles for freqency lock and 500 cycles
     * for phase lock. For fref=25MHz, REFDIV=1, locktime is 10us and 20us */
    while (!(readl(b + PLL_CTRL_OFF) & BM_PLL_CTRL_LOCK));

    /* power up and upate post div */
    /* 6> Set FOUT4PHASEEN/FOUTPOSTDIVEN to 0x1 to enable PLL VCO clock*/
    /* phase clocks not be used in Kunlun */
    ctrl |= BM_PLL_CTRL_FOUTPOSTDIVEN /*| BM_PLL_CTRL_FOUT4PHASEEN*/;
    writel(ctrl, b + PLL_CTRL_OFF);

    U32 d1 = readl(b + PLL_OUT_DIV_1_OFF);
    U32 d2 = readl(b + PLL_OUT_DIV_2_OFF);
    d1 &= ~(FM_PLL_OUT_DIV_1_DIV_NUM_A | FM_PLL_OUT_DIV_1_DIV_NUM_B);
    d2 &= ~(FM_PLL_OUT_DIV_2_DIV_NUM_C | FM_PLL_OUT_DIV_2_DIV_NUM_D);

    d1 |= FV_PLL_OUT_DIV_1_DIV_NUM_A(DIV_A_V(div_abcd))
          | FV_PLL_OUT_DIV_1_DIV_NUM_B(DIV_B_V(div_abcd));
    d2 |= FV_PLL_OUT_DIV_2_DIV_NUM_C(DIV_C_V(div_abcd))
          | FV_PLL_OUT_DIV_2_DIV_NUM_D(DIV_D_V(div_abcd));
    writel(d1, b + PLL_OUT_DIV_1_OFF);
    writel(d2, b + PLL_OUT_DIV_2_OFF);
    /* these DIVs and CGs is outside sc pll IP, they are in a wrapper */
    ctrl |= BM_PLL_CTRL_PLL_DIVD_CG_EN | BM_PLL_CTRL_PLL_DIVC_CG_EN
            | BM_PLL_CTRL_PLL_DIVB_CG_EN | BM_PLL_CTRL_PLL_DIVA_CG_EN
            | BM_PLL_CTRL_PLLPOSTCG_EN;
    writel(ctrl, b + PLL_CTRL_OFF);

    while (readl(b + PLL_OUT_DIV_1_OFF)
           & (BM_PLL_OUT_DIV_1_DIV_BUSY_A | BM_PLL_OUT_DIV_1_DIV_BUSY_B));

    while (readl(b + PLL_OUT_DIV_2_OFF)
           & (BM_PLL_OUT_DIV_2_DIV_BUSY_C | BM_PLL_OUT_DIV_2_DIV_BUSY_D));
}

bool sc_pfpll_is_enabled(U32 base)
{
    U32 ctrl = readl(base + PLL_CTRL_OFF);

    return !!(ctrl & BM_PLL_CTRL_PLLEN);
}
