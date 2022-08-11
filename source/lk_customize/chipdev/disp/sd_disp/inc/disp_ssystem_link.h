/*
* disp_ssystem_link.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __DISP_SSYTEM_LINK_H__
#define __DISP_SSYTEM_LINK_H__

enum {
    DSP_CLK_PLL_LVDS1 = 0x0,
    DSP_CLK_PLL_LVDS2 = 0x1,
    DSP_CLK_PLL_LVDS3 = 0x2,
    DSP_CLK_PLL_LVDS4 = 0x3,
    DSP_CLK_PLL_DISP  = 0x4
};

struct disp_pll_clk{
    int pll_id;
    int clk;
};

struct disp_pll_clks {
    int count;
    struct disp_pll_clk *pll_clks;
};

int disp_link_init(uint8_t *dc_pll, struct display_resource *disp_res, unsigned int res_num);
int disp_get_clks(struct disp_pll_clks* pclks);
#endif
