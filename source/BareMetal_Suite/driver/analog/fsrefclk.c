/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "analog.h"
#include <scr/scr.h>

typedef struct {
    fsrefclk_sel_e fsrc;
    U32 scr_base;
    U32 bit_src_sel;
    U32 bit_xtal_cg;
} fsrc_ctrl_t;

static const fsrc_ctrl_t fsrc_ctrl[] = {
    {
        FSRC_SAF, APB_SCR_SAF_BASE,
        SCR_SAF_FSREFCLK_SAF_SRC_SEL_1_0_L16_START_BIT,
        SCR_SAF_FSREFCLK_SAF_XTAL_CG_EN_L16_START_BIT
    },
    {
        FSRC_SEC, APB_SCR_SEC_BASE,
        SCR_SEC_FSREFCLK_SEC_SRC_SEL_1_0_L16_START_BIT,
        SCR_SEC_FSREFCLK_SEC_XTAL_CG_EN_L16_START_BIT
    },
    {
        FSRC_HIS, APB_SCR_SEC_BASE,
        SCR_SEC_FSREFCLK_HIS_SRC_SEL_1_0_L16_START_BIT,
        SCR_SEC_FSREFCLK_HIS_XTAL_CG_EN_L16_START_BIT
    },
    {
        FSRC_HPI, APB_SCR_SEC_BASE,
        SCR_SEC_FSREFCLK_HPI_SRC_SEL_1_0_L16_START_BIT,
        SCR_SEC_FSREFCLK_HPI_XTAL_CG_EN_L16_START_BIT
    },
    {
        FSRC_DDR, APB_SCR_SEC_BASE,
        SCR_SEC_DDR_SS_FSREFCLK_SRC_SEL_1_0_L16_START_BIT,
        SCR_SEC_DDR_SS_FSREFCLK_XTAL_CG_EN_L16_START_BIT,
    }
};

osc_out_sel_e ana_osc24_get_output_sel(U32 fsrc)
{
    osc_out_sel_e sel = SEL_XTAL1;
    assert(fsrc < ARRAY_SZ(fsrc_ctrl));

    const fsrc_ctrl_t *p = &fsrc_ctrl[fsrc];
    /* src_sel:
     *      00  XTAL_SAF
     *      01  XTAL_AP
     *      10  RC
     */
    U32 src_sel_0 = scr_bit_get(p->scr_base, L16, p->bit_src_sel);
    U32 src_sel_1 = scr_bit_get(p->scr_base, L16, p->bit_src_sel + 1);

    if (src_sel_1) {
        sel = SEL_OSC;
    } else if (src_sel_0) {
        sel = SEL_XTAL2;
    } else {
        sel = SEL_XTAL1;
    }

    return sel;
}

void ana_osc24_output_sel(U32 fsrc, osc_out_sel_e sel)
{
    assert(fsrc < ARRAY_SZ(fsrc_ctrl));

    const fsrc_ctrl_t *p = &fsrc_ctrl[fsrc];
    /* src_sel:
     *      00  XTAL_SAF
     *      01  XTAL_AP
     *      10  RC
     */
    U32 src_sel_0 = scr_bit_get(p->scr_base, L16, p->bit_src_sel);
    U32 src_sel_1 = scr_bit_get(p->scr_base, L16, p->bit_src_sel + 1);
    osc_out_sel_e cur_sel = 0;

    /*                      glitch-less
     *             osc --------|1\
     *                         | |
     *   xtal1---|0\           | |------ output
     *           | |--(gate)---| |
     *   xtal2---|1/           |0/
     *          src_sel_0       src_sel_1
     */

    if (src_sel_1) {
        cur_sel = SEL_OSC;
    } else if (src_sel_0) {
        cur_sel = SEL_XTAL2;
    } else {
        cur_sel = SEL_XTAL1;
    }

    /* 1>Set FS_RC_EN, FS_XTAL_EN both to 0x0. Or 1 for function safety mode */

    if (sel == cur_sel) {
        DBG("%s: sel is %d already, no need to switch\n", __FUNCTION__);
    } else {
        if (SEL_OSC == sel) {      /* to osc */
            scr_bit_set(p->scr_base, L16, p->bit_src_sel + 1);
        } else {    /* to xtal1/2 */
            /* switch to osc */
            scr_bit_set(p->scr_base, L16, p->bit_src_sel + 1);
            /* gate off xtal */
            scr_bit_clr(p->scr_base, L16, p->bit_xtal_cg);

            /* select xtal1/2 */
            if (SEL_XTAL2 == sel) {
                scr_bit_set(p->scr_base, L16, p->bit_src_sel);
            } else {
                scr_bit_clr(p->scr_base, L16, p->bit_src_sel);
            }

            /* gate on xtal */
            scr_bit_set(p->scr_base, L16, p->bit_xtal_cg);
            /* switch to xtal. */
            /* SRC_SEL[1] switch only works when both path 0/1 have clocks */
            scr_bit_clr(p->scr_base, L16, p->bit_src_sel + 1);
        }
    }
}
