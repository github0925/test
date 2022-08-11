/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "ckgen_reg.h"
#include "ckgen.h"

#define CKGEN_POLLs    10000u

/*
 * IP clk slice
 * clkin0-7 -> CG -> pre_div((3 bits) -> post_div((6bits)
 *
 * clkin0 by default
 */

void ckgen_ip_slice_gate(U32 base, U32 slice_id)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_IP_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);
    /* 1> Set pre_en to 1'b0 to turn off the clock */
    ctl &= ~BM_CKGEN_IP_SLICE_CTL_CG_EN;
    writel(ctl, ctl_addr);
    U32 tms = 0;

    while ((readl(ctl_addr) & BM_CKGEN_IP_SLICE_CTL_CG_EN)
           && (tms++ < CKGEN_POLLs));
}

void ckgen_ip_slice_ungate(U32 base, U32 slice_id)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_IP_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);
    /* 1> Set pre_en to 1'b1 to turn on the clock */
    ctl |= BM_CKGEN_IP_SLICE_CTL_CG_EN;
    writel(ctl, ctl_addr);
    U32 tms = 0;

    while ((!(readl(ctl_addr) & BM_CKGEN_IP_SLICE_CTL_CG_EN))
           && (tms++ < CKGEN_POLLs));
}

void ckgen_ip_slice_cfg(U32 base, U32 slice_id,
                        U32 src_sel, U32 pre_div, U32 post_div)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_IP_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);

    /* 1> Set pre_en to 1'b0 to turn off the clock */
    ctl &= ~BM_CKGEN_IP_SLICE_CTL_CG_EN;
    writel(ctl, ctl_addr);
    U32 tms = 0;

    while ((readl(ctl_addr) & BM_CKGEN_IP_SLICE_CTL_CG_EN)
           && (tms++ < CKGEN_POLLs));

    /* 2> Change pre_mux_sel (non glitchless) to select clock source */
    ctl &= ~FM_CKGEN_IP_SLICE_CTL_CLK_SRC_SEL;
    ctl |= FV_CKGEN_IP_SLICE_CTL_CLK_SRC_SEL(src_sel);
    writel(ctl, ctl_addr);

    /* 3> Set pre_en to 1'b1 to turn on the clock */
    ctl |= BM_CKGEN_IP_SLICE_CTL_CG_EN;
    writel(ctl, ctl_addr);
    tms = 0;

    while ((!(readl(ctl_addr) & BM_CKGEN_IP_SLICE_CTL_CG_EN))
           && (tms++ < CKGEN_POLLs));

    /* 4>   a. Set pre_div_num/post_div_num */
    ctl &= ~(FM_CKGEN_IP_SLICE_CTL_POST_DIV_NUM
             | FM_CKGEN_IP_SLICE_CTL_PRE_DIV_NUM);
    ctl |= (FV_CKGEN_IP_SLICE_CTL_POST_DIV_NUM(post_div)
            | FV_CKGEN_IP_SLICE_CTL_PRE_DIV_NUM(pre_div));
    writel(ctl, ctl_addr);

    /* 'BUSY' will not asserted if un-changed 'div' written */
    /*      b. Check the corresponding busy bit. */
    tms = 0;

    while ((readl(ctl_addr) & (BM_CKGEN_IP_SLICE_CTL_POST_BUSY
                               | BM_CKGEN_IP_SLICE_CTL_PRE_BUSY))
           && (tms++ < CKGEN_POLLs));
}

/*
 * Bus clk slice
 *
 * clk_in0-7 -> CG -> pre_div_a((3 bit) -> |\
 *                                         || |---((post_div--------->
 * clk_in0-7 -> CG -> pre_div_b((3 bit) -> |/                |
 *                                    pre_a_b_sel
 *                                                      ||MNPQ DIV|---> divs
 *
 * a_clk_in0 by default.
 *
 * SW program sequence to change from a to b.
 */
void ckgen_bus_slice_cfg(U32 base, U32 slice_id,
                         U32 path, U32 src_sel, U32 pre_div)
{
    U32 ctl_addr = base +  SOC_CKGEN_REG_MAP(CKGEN_BUS_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);

    U32 tms = 0;

    if (PATH_A == path) {
        /* B is being selected now */
        assert(ctl & BM_CKGEN_BUS_SLICE_CTL_A_B_SEL);
        /* 1> Set pre_en to 1'b0 to turn off path. */
        ctl &= ~BM_CKGEN_BUS_SLICE_CTL_CG_EN_A;
        writel(ctl, ctl_addr);

        while ((readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_CG_EN_A)
               && (tms++ < CKGEN_POLLs));

        /* 2> Change pre_mux_sel to select clock source.*/
        ctl &= ~FM_CKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A;
        ctl |= FV_CKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A(src_sel);
        writel(ctl, ctl_addr);
        /* 3> Set pre_en to 1'b1 to turn on path. */
        ctl |= BM_CKGEN_BUS_SLICE_CTL_CG_EN_A;
        writel(ctl, ctl_addr);
        tms = 0;

        while ((!(readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_CG_EN_A))
               && (tms++ < CKGEN_POLLs));

        /* 4>   a. Set pre_div_num */
        ctl &= ~FM_CKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A;
        ctl |= FV_CKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A(pre_div);
        writel(ctl, ctl_addr);
        /*      b. Check the corresponding busy bit. */
        tms = 0;

        while ((readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_PRE_BUSY_A)
               && (tms++ < CKGEN_POLLs));
    } else {
        /* A is being selected now */
        assert(!(ctl & BM_CKGEN_BUS_SLICE_CTL_A_B_SEL));
        /* 1> Set pre_en to 1'b0 to turn off path. */
        ctl &= ~BM_CKGEN_BUS_SLICE_CTL_CG_EN_B;
        writel(ctl, ctl_addr);
        tms = 0;

        while ((readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_CG_EN_B)
               && (tms++ < CKGEN_POLLs));

        /* 2> Change pre_mux_sel to select clock source.*/
        ctl &= ~FM_CKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B;
        ctl |= FV_CKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B(src_sel);
        writel(ctl, ctl_addr);
        /* 3> Set pre_en to 1'b1 to turn on path. */
        ctl |= BM_CKGEN_BUS_SLICE_CTL_CG_EN_B;
        writel(ctl, ctl_addr);
        tms = 0;

        while ((!(readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_CG_EN_B))
               && (tms++ < CKGEN_POLLs));

        /* 4>   a. Set pre_div_num */
        ctl &= ~FM_CKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B;
        ctl |= FV_CKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B(pre_div);
        writel(ctl, ctl_addr);
        /*      b. Check the corresponding busy bit. */
        tms = 0;

        while ((readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_PRE_BUSY_B)
               && (tms++ < CKGEN_POLLs));
    }
}

void ckgen_bus_slice_postdiv_update(U32 base, U32 slice_id, U32 post_div)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_BUS_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);
    U32 tms = 0;

    /* 6>   a. Change post_div_num */
    ctl &= ~FM_CKGEN_BUS_SLICE_CTL_POST_DIV_NUM;
    ctl |= FV_CKGEN_BUS_SLICE_CTL_POST_DIV_NUM(post_div);
    writel(ctl, ctl_addr);

    /*      b. Check the corresponding busy bit. */
    while ((readl(ctl_addr) & BM_CKGEN_BUS_SLICE_CTL_POST_BUSY)
           && (tms++ < CKGEN_POLLs));
}

void ckgen_bus_slice_switch(U32 base, U32 slice_id)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_BUS_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);

    /* 5> toggle pre_a_b_sel. This mux is glitch-less.
     *  Note: Both clock from path a and b should be active when swithing.
     */
    if (ctl & BM_CKGEN_BUS_SLICE_CTL_A_B_SEL) {
        ctl &= ~BM_CKGEN_BUS_SLICE_CTL_A_B_SEL;
    } else {
        ctl |= BM_CKGEN_BUS_SLICE_CTL_A_B_SEL;
    }

    writel(ctl, ctl_addr);
}

/*
 * Core clock slice
 * Derived from Bus clk slice without pre_div nor mnpq divs.
 *
 * a_clk_in0 by default
 */

void ckgen_core_slice_cfg(U32 base, U32 slice_id, U32 path, U32 src_sel)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_CORE_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);
    U32 tms = 0;

    if (PATH_A == path) {
        /* B is being selected now */
        assert(ctl & BM_CKGEN_CORE_SLICE_CTL_A_B_SEL);
        /* 1> Set pre_en to 1'b0 to turn off path. */
        ctl &= ~BM_CKGEN_CORE_SLICE_CTL_CG_EN_A;
        writel(ctl, ctl_addr);

        while ((readl(ctl_addr) & BM_CKGEN_CORE_SLICE_CTL_CG_EN_A)
               && (tms++ < CKGEN_POLLs));

        /* 2> Change pre_mux_sel to select clock source.*/
        ctl &= ~FM_CKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A;
        ctl |= FV_CKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A(src_sel);
        writel(ctl, ctl_addr);
        /* 3> Set pre_en to 1'b1 to turn on path. */
        ctl |= BM_CKGEN_CORE_SLICE_CTL_CG_EN_A;
        writel(ctl, ctl_addr);
        tms = 0;

        while ((!(readl(ctl_addr) & BM_CKGEN_CORE_SLICE_CTL_CG_EN_A))
               && (tms++ < CKGEN_POLLs));
    } else {
        /* A is being selected now */
        assert(!(ctl & BM_CKGEN_CORE_SLICE_CTL_A_B_SEL));
        /* 1> Set pre_en to 1'b0 to turn off path. */
        ctl &= ~BM_CKGEN_CORE_SLICE_CTL_CG_EN_B;
        writel(ctl, ctl_addr);

        while ((readl(ctl_addr) & BM_CKGEN_CORE_SLICE_CTL_CG_EN_B)
               && (tms++ < CKGEN_POLLs));

        /* 2> Change pre_mux_sel to select clock source.*/
        ctl &= ~FM_CKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B;
        ctl |= FV_CKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B(src_sel);
        writel(ctl, ctl_addr);
        /* 3> Set pre_en to 1'b1 to turn on path. */
        ctl |= BM_CKGEN_CORE_SLICE_CTL_CG_EN_B;
        writel(ctl, ctl_addr);
        tms = 0;

        while ((!(readl(ctl_addr) & BM_CKGEN_CORE_SLICE_CTL_CG_EN_B))
               && (tms++ < CKGEN_POLLs));
    }
}

void ckgen_core_slice_postdiv_update(U32 base, U32 slice_id, U32 post_div)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_CORE_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);

    /* 6>   a. Change post_div_num */
    ctl &= ~FM_CKGEN_CORE_SLICE_CTL_POST_DIV_NUM;
    ctl |= FV_CKGEN_CORE_SLICE_CTL_POST_DIV_NUM(post_div);
    writel(ctl, ctl_addr);
    /*      b. Check the corresponding busy bit. */
    U32 tms = 0;

    while ((readl(ctl_addr) & BM_CKGEN_CORE_SLICE_CTL_POST_BUSY)
           && (tms++ < CKGEN_POLLs));
}

void ckgen_core_slice_switch(U32 base, U32 slice_id)
{
    U32 ctl_addr = base + SOC_CKGEN_REG_MAP(CKGEN_CORE_SLICE_CTL_OFF(slice_id));
    U32 ctl = readl(ctl_addr);

    /* 5> toggle pre_a_b_sel. This mux is glitch-less.
     *  Note: Both clock from path a and be should be active when swithing.
     */
    if (ctl & BM_CKGEN_CORE_SLICE_CTL_A_B_SEL) {
        ctl &= ~BM_CKGEN_CORE_SLICE_CTL_A_B_SEL;
    } else {
        ctl |= BM_CKGEN_CORE_SLICE_CTL_A_B_SEL;
    }

    writel(ctl, ctl_addr);
}

void ckgen_cg_en(U32 base, U32 id)
{
    U32 a = base + SOC_CKGEN_REG_MAP(CKGEN_LP_GATING_EN_OFF(id));
    U32 v = readl(a);
    /* 1>Set sw_dis to 0x0 to enable the clk */
    v &= ~BM_CKGEN_LP_GATING_EN_SW_GATING_DIS;
    writel(v, a);
    /* SW_GATING_EN (force_en actually) is for debug purpose */
}

void ckgen_cg_dis(U32 base, U32 id)
{
    U32 a = base + SOC_CKGEN_REG_MAP(CKGEN_LP_GATING_EN_OFF(id));
    U32 v = readl(a);
    /* 2> Set sw_dis to 0x1 to disable the clk  */
    v |= BM_CKGEN_LP_GATING_EN_SW_GATING_DIS;
    writel(v, a);
}

void ckgen_uuu_slice_cfg(U32 base, U32 slice_id, U32 src_sel, U32 div_mnpq)
{
    assert((UUU_SEL_CKGEN_SOC == src_sel) || (UUU_SEL_PLL == src_sel));
    U32 a = base + SOC_CKGEN_REG_MAP(CKGEN_UUU_SLICE_OFF(slice_id));
    U32 v = readl(a);
    /* 5> Set Div_mnpq before changing the clock source */
    v &= ~(FM_CKGEN_UUU_SLICE_M_DIV_NUM | FM_CKGEN_UUU_SLICE_N_DIV_NUM
           | FM_CKGEN_UUU_SLICE_P_DIV_NUM | FM_CKGEN_UUU_SLICE_Q_DIV_NUM);
    v |= div_mnpq;
    writel(v, a);
    /* 6> Set clk_sel[0] to 0x1 to set clk_out_1/2/3 source from PLL_x.
     * 7> Set clk_sel[1] to 0x1 to set clk_out_0 source from PLL_x
     */
    v &= ~FM_CKGEN_UUU_SLICE_UUU_SEL;
    v |= FV_CKGEN_UUU_SLICE_UUU_SEL(src_sel);
    writel(v, a);
}
