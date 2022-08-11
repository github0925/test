//*****************************************************************************
//
// clkgen_drv.c - Driver for the clkgen Module.
//
// Copyright (c) 2019  SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <trace.h>
#include <reg.h>
#include "__regs_base.h"
#include "irq_v.h"
#include "clkgen_drv.h"

#define LOCAL_TRACE 0
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
//*****************************************************************************
//
//! reg_poll_value .
//!
//! \reg is the reg address of the rstgen module.
//! \start is register start bit
//! \width is register start bit to end bit number
//! \value is write to register value
//! \retrycount is polling count
//! This function is polling reg status .
//!
//! \return true when polling status equal value else return false
//
//*****************************************************************************
static bool reg_poll_value(vaddr_t reg, int start, int width,
                           uint32_t value, int retrycount)
{
    uint32_t v = 0;
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "start:%d width:%d value:%d retrycount:%d\n", start, width, value,
                  retrycount);

    do {
        v = readl(reg);

        if (((v >> start) & ((1 << width) - 1)) == value)
            return true;

        spin(1);
    } while (--retrycount);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "reg_poll_value is error \n");
    return false;
}

static bool reg_poll_clear(vaddr_t reg, int start, int width,
                           uint32_t pollvalue, uint32_t setvalue, int retrycount)
{
    uint32_t v = 0;
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "start:%d width:%d value:%d set:%d retrycount:%d\n", start, width,
                  pollvalue,
                  setvalue, retrycount);

    do {
        v = readl(reg);

        if (((v >> start) & ((1 << width) - 1)) == pollvalue)
            break;
    } while (--retrycount);

    if (retrycount == 0) {
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "reg_poll_value is error \n");
        return false;
    }

    RMWREG32(reg, start, width, setvalue);
    return true;
}

//*****************************************************************************
//
//! clkgen_get_default_config .
//!
//! \def_cfg ckgen default config
//!
//!This function is get ckgen default config.
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_get_default_config(clkgen_default_cfg_t *def_cfg)
{
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_get_default_config start...............\n");
    def_cfg->src_sel_mask = 2;
    def_cfg->safety_mode = true;
    return true;
}


//*****************************************************************************
//
//! fsrefclk clk select .
//!
//! \param base   clkgen peripheral base address
//! \scr_idx please reference clkgen_scr_idx
//! \src_sel_mask please reference fsrefclk_clkgen_src_sel
//! \safety_mode is need enable safety mode
//!
//!This function is fsrefclk select.
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_fsrefclk_sel(vaddr_t base, uint32_t scr_idx,
                         uint32_t src_sel_mask, bool safety_mode)
{
    vaddr_t scr_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;

    // Check the arguments.
    if ((base != SCR_SAF_BASE)
            && (base != SCR_SEC_BASE)) {
        LTRACEF("base paramenter error \n");
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_fsrefclk_sel scr_idx:0x%x,src_sel_mask:0x%x,safety_mode:0x%x\n",
                  scr_idx, src_sel_mask, safety_mode);
    scr_base_addr = base + CLKGEN_FSREFCLK_OFF(scr_idx);
    //clear all
    reg_read = readl(scr_base_addr);
    reg_write = reg_read & (~(CLKGEN_FSREFCLK_FS_RC_EN(1) |
                              CLKGEN_FSREFCLK_FS_XTAL_EN(1)));
    writel(reg_write, scr_base_addr);
    //select clk src:xtal_saf = 0x0U,xtal_ap = 0x1U,rc_24m = 0x2U,
    reg_read = readl(scr_base_addr);
    reg_write = reg_read | CLKGEN_FSREFCLK_SCR_SEL(src_sel_mask);
    writel(reg_write, scr_base_addr);

    //enable safety mode
    if (safety_mode) {
        reg_read = readl(scr_base_addr);
        reg_write = reg_read | (CLKGEN_FSREFCLK_FS_RC_EN(1) |
                                CLKGEN_FSREFCLK_FS_XTAL_EN(1));
        writel(reg_write, scr_base_addr);
    }

    //enable xtal cg gate
    reg_read = readl(scr_base_addr);
    reg_write = reg_read | CLKGEN_FSREFCLK_XTAL_CG_EN(1);
    writel(reg_write, scr_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_gating_enable .
//!
//! \param base   clkgen peripheral base address
//! \gating_idx please reference clkgen_gating_idx
//! \bool enable
//!
//!This function is ckgen ip slice gating enable .
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_gating_enable(vaddr_t base, uint16_t gating_idx, bool enable)
{
    vaddr_t gating_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    gating_base_addr = base + CLKGEN_LP_GATING_EN_OFF(gating_idx);
    //unlock gating register
    reg_read = readl(gating_base_addr);
    reg_write = reg_read & (~(CLKGEN_LP_GATING_EN_GATING_LOCK_MASK));
    writel(reg_write, gating_base_addr);
    //set sw disable
    reg_read = readl(gating_base_addr);

    if (enable) {
        reg_write = reg_read & (~(CLKGEN_LP_GATING_EN_SW_GATING_DIS_MASK));
        writel(reg_write, gating_base_addr);
    }
    else {
        reg_write = reg_read | CLKGEN_LP_GATING_EN_SW_GATING_DIS_MASK;
        writel(reg_write, gating_base_addr);
    }

    return true;
}

//*****************************************************************************
//
//! clkgen_ip_slice_set .
//!
//! \param base   clkgen peripheral base address
//! \scr_idx please reference clkgen_scr_idx
//! \src_sel_mask
//!
//!This function is select ip clk
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_ip_slice_set(vaddr_t base, uint8_t ip_slice_idx,
                         uint8_t clk_src_sel, uint16_t pre_div, uint16_t post_div)
{
    bool ret = false;
    vaddr_t ip_slice_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    ip_slice_base_addr = base + CLKGEN_IP_SLICE_CTL_OFF(ip_slice_idx);
    //clear pre_en
    reg_read = readl(ip_slice_base_addr);

    if ((reg_read & CLKGEN_IP_SLICE_CTL_CG_EN_MASK) != 0) {
        reg_write = reg_read & (~CLKGEN_IP_SLICE_CTL_CG_EN_MASK);
        writel(reg_write, ip_slice_base_addr);
        reg_poll_clear(ip_slice_base_addr, CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT,
                       1, 1, 0, 100);
    }

    //select clk src
    reg_read = readl(ip_slice_base_addr);
    reg_write = (reg_read & (~CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL_MASK)) |
                CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL(clk_src_sel);
    writel(reg_write, ip_slice_base_addr);
    //set pre_en
    reg_read = readl(ip_slice_base_addr);
    reg_write = reg_read | CLKGEN_IP_SLICE_CTL_CG_EN_MASK;
    writel(reg_write, ip_slice_base_addr);
    reg_poll_clear(ip_slice_base_addr, CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT,
                   1, 1, 0, 100);
    //set pre_div
    reg_read = readl(ip_slice_base_addr);
    reg_write = (reg_read & (~CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM_MASK)) |
                CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM(pre_div);
    writel(reg_write, ip_slice_base_addr);
    //wait pre_upd_ack is 0
    ret = reg_poll_value(ip_slice_base_addr,
                         CLKGEN_IP_SLICE_CTL_PRE_BUSY_SHIFT, 1, 0, 100);
    //set post_div
    reg_read = readl(ip_slice_base_addr);
    reg_write = (reg_read & (~CLKGEN_IP_SLICE_CTL_POST_DIV_NUM_MASK)) |
                CLKGEN_IP_SLICE_CTL_POST_DIV_NUM(post_div);
    writel(reg_write, ip_slice_base_addr);
    //wait post_upd_ack is 0
    ret = reg_poll_value(ip_slice_base_addr,
                         CLKGEN_IP_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
    return ret;
}
bool clkgen_ip_ctl_get(vaddr_t base, uint32_t slice_idx,
                       clkgen_ip_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;

    // Check the arguments.
    if (ctl) {
        slice_addr = base + CLKGEN_IP_SLICE_CTL_OFF(slice_idx);
        ctl->val = readl(slice_addr);
    }

    return ret;
}
bool clkgen_ip_ctl_set(vaddr_t base, uint32_t slice_idx,
                       const clkgen_ip_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;
    clkgen_ip_ctl v;

    if (ctl) {
        slice_addr = base + CLKGEN_IP_SLICE_CTL_OFF(slice_idx);
        //clear pre_en
        v.val = readl(slice_addr);

        if (v.cg_en != 0) {
            v.cg_en = 0;
            writel(v.val, slice_addr);
            ret |= reg_poll_clear(slice_addr, CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT,
                                  1, 1, 0, 100);
        }

        //select clk src
        v.src_sel = ctl->src_sel;
        writel(v.val, slice_addr);
        //set pre_en
        v.cg_en = ctl->cg_en;
        writel(v.val, slice_addr);
        ret |= reg_poll_clear(slice_addr, CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT,
                              1, 1, 0, 100);
        //set pre_div
        v.pre_div_num = ctl->pre_div_num;
        writel(v.val, slice_addr);
        //wait pre_upd_ack is 0
        ret = reg_poll_value(slice_addr,
                             CLKGEN_IP_SLICE_CTL_PRE_BUSY_SHIFT, 1, 0, 100);
        //set post_div
        v.post_div_num = ctl->post_div_num;
        writel(v.val, slice_addr);
        //wait post_upd_ack is 0
        ret = reg_poll_value(slice_addr,
                             CLKGEN_IP_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_bus_slice .
//!
//! \param base   clkgen peripheral base address
//! \bus_clk_cfg bus clock config
//!
//!This function is select bus clk
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_bus_slice_switch(vaddr_t base,
                             clkgen_bus_slice_drv_t *bus_clk_cfg)
{
    bool ret = false;
    vaddr_t bus_slice_base_addr;
    vaddr_t bus_gasket_slice_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    uint8_t a_b_sel = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "start..............\n");
    bus_slice_base_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(
                              bus_clk_cfg->bus_slice_idx);
    bus_gasket_slice_base_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(
                                     bus_clk_cfg->bus_slice_idx);
    //read pre_a_b_sel,if it is 0x0,current selected path is a
    reg_read = readl(bus_slice_base_addr);
    a_b_sel = ((reg_read & CLKGEN_BUS_SLICE_CTL_A_B_SEL_MASK) >>
               CLKGEN_BUS_SLICE_CTL_A_B_SEL_SHIFT);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "a_b_sel:%d\n", a_b_sel);

    if (/*(bus_clk_cfg->clk_a_b_switch == bus_clk_a_b_sel_b) &&*/
        (a_b_sel == bus_clk_a_b_sel_a)) {
        //set pre_en_b to 0x0,disable the clock
        reg_read = readl(bus_slice_base_addr);

        if ((reg_read & CLKGEN_BUS_SLICE_CTL_CG_EN_B_MASK) != 0) {
            reg_write = reg_read & (~CLKGEN_BUS_SLICE_CTL_CG_EN_B_MASK);
            writel(reg_write, bus_slice_base_addr);
            reg_poll_clear(bus_slice_base_addr,
                           CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1, 0, 100);
        }

        //set m/n/p/q div
        reg_read = readl(bus_gasket_slice_base_addr);
        reg_write = reg_read
                    & (~(CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_MASK));
        reg_write = reg_write
                    | CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM(bus_clk_cfg->gasket_cfg.m_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM(bus_clk_cfg->gasket_cfg.n_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM(bus_clk_cfg->gasket_cfg.p_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM(bus_clk_cfg->gasket_cfg.q_div_num);
        writel(reg_write, bus_gasket_slice_base_addr);
        ret = reg_poll_value(bus_gasket_slice_base_addr,
                             CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT, 4, 0, 100);
        //change the clock source by set pre_mux_sel_b,then set pre_en_b to 0x1
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B_MASK))
                    | CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B(bus_clk_cfg->clk_src_sel_b)
                    | CLKGEN_BUS_SLICE_CTL_CG_EN_B_MASK;
        writel(reg_write, bus_slice_base_addr);
        reg_poll_clear(bus_slice_base_addr,
                       CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1, 0, 100);
        //set pre_div_num_b,check pre_upd_ack_b bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B_MASK))
                    | CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B(bus_clk_cfg->pre_div_b);
        writel(reg_write, bus_slice_base_addr);
        ret = reg_poll_value(bus_slice_base_addr,
                             CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_SHIFT, 1, 0, 100);
        //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_MASK))
                    | CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM(bus_clk_cfg->post_div);
        writel(reg_write, bus_slice_base_addr);
        ret = reg_poll_value(bus_slice_base_addr,
                             CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
        //invert pre_a_b_sel
        reg_read = readl(bus_slice_base_addr);
        reg_write = reg_read | CLKGEN_BUS_SLICE_CTL_A_B_SEL_MASK;
        writel(reg_write, bus_slice_base_addr);
    }
    else {
        //default switch a
        //set pre_en_a to 0x0,disable the clock
        reg_read = readl(bus_slice_base_addr);

        if ((reg_read & CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK) != 0) {
            reg_write = reg_read & (~CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK);
            writel(reg_write, bus_slice_base_addr);
            reg_poll_clear(bus_slice_base_addr,
                           CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1, 0, 100);
        }

        //set m/n/p/q div
        reg_read = readl(bus_gasket_slice_base_addr);
        reg_write = reg_read
                    & (~(CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_MASK
                         | CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_MASK));
        reg_write = reg_write
                    | CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM(bus_clk_cfg->gasket_cfg.m_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM(bus_clk_cfg->gasket_cfg.n_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM(bus_clk_cfg->gasket_cfg.p_div_num)
                    | CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM(bus_clk_cfg->gasket_cfg.q_div_num);
        writel(reg_write, bus_gasket_slice_base_addr);
        ret = reg_poll_value(bus_gasket_slice_base_addr,
                             CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT, 4, 0, 100);
        //change the clock source by set pre_mux_sel_a,then set pre_en_a to 0x1
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A_MASK))
                    | CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A(bus_clk_cfg->clk_src_sel_a)
                    | CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK;
        writel(reg_write, bus_slice_base_addr);
        reg_poll_clear(bus_slice_base_addr,
                       CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1, 0, 100);
        //set pre_div_num_a,check pre_upd_ack_a bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A_MASK))
                    | CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A(bus_clk_cfg->pre_div_a);
        writel(reg_write, bus_slice_base_addr);
        ret = reg_poll_value(bus_slice_base_addr,
                             CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_SHIFT, 1, 0, 100);
        //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(bus_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_MASK))
                    | CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM(bus_clk_cfg->post_div);
        writel(reg_write, bus_slice_base_addr);
        ret = reg_poll_value(bus_slice_base_addr,
                             CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
        //invert pre_a_b_sel
        reg_read = readl(bus_slice_base_addr);
        reg_write = reg_read & (~CLKGEN_BUS_SLICE_CTL_A_B_SEL_MASK);
        writel(reg_write, bus_slice_base_addr);
    }

    return ret;
}
bool clkgen_bus_ctl_get(vaddr_t base, uint32_t slice_idx,
                        clkgen_bus_ctl *ctl,
                        clkgen_bus_gasket *gasket)
{
    bool ret = true;
    vaddr_t slice_addr;

// Check the arguments.
    if (ctl) {
        slice_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(slice_idx);
        ctl->val = readl(slice_addr);
    }

    if (gasket) {
        slice_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);
        gasket->val = readl(slice_addr);
    }

    return ret;
}
bool clkgen_bus_ctl_set(vaddr_t base, uint32_t slice_idx,
                        const clkgen_bus_ctl *ctl,
                        const clkgen_bus_gasket *gasket)
{
    bool ret = true;
    vaddr_t slice_addr;

    if (ctl) {
        slice_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(slice_idx);
        clkgen_bus_ctl v;
        //read pre_a_b_sel,if it is 0x0,current selected path is a
        v.val = readl(slice_addr);

        if (v.a_b_sel == 0) {
            //set pre_en_b to 0x0,disable the clock
            if (v.cg_en_b != 0) {
                v.cg_en_b = 0;
                writel(v.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1,
                               0, 100);
            }

            //set m/n/p/q div
            if (gasket) {
                vaddr_t g_addr;
                clkgen_bus_gasket g;
                g_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);
                g.val = readl(g_addr);
                g.m_div_num = gasket->m_div_num;
                g.n_div_num = gasket->n_div_num;
                g.p_div_num = gasket->p_div_num;
                g.q_div_num = gasket->q_div_num;
                writel(g.val, g_addr);
                ret = reg_poll_value(g_addr,
                                     CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT, 4, 0, 100);
            }

            //change the clock source by set pre_mux_sel_b,then set pre_en_b to 0x1
            v.src_sel_b = ctl->src_sel_a;
            v.cg_en_b = ctl->cg_en_a;

            if (v.cg_en_b != 0) {
                writel(v.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1,
                               0, 100);
            }

            //set pre_div_num_b,check pre_upd_ack_b bit 1 means update not finished yes,0 means update finished.
            v.pre_div_num_b = ctl->pre_div_num_a;
            writel(v.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_SHIFT, 1, 0, 100);
            //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
            v.post_div_num = ctl->post_div_num;
            writel(v.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
            //invert pre_a_b_sel
            v.a_b_sel = 1;
            writel(v.val, slice_addr);
        }
        else {
            //set pre_en_a to 0x0,disable the clock
            if (v.cg_en_a != 0) {
                v.cg_en_a = 0;
                writel(v.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1,
                               0, 100);
            }

            //set m/n/p/q div
            if (gasket) {
                vaddr_t g_addr;
                clkgen_bus_gasket g;
                g_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);
                g.val = readl(g_addr);
                g.m_div_num = gasket->m_div_num;
                g.n_div_num = gasket->n_div_num;
                g.p_div_num = gasket->p_div_num;
                g.q_div_num = gasket->q_div_num;
                writel(g.val, g_addr);
                ret = reg_poll_value(g_addr,
                                     CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT, 4, 0, 100);
            }

            //change the clock source by set pre_mux_sel_b,then set pre_en_b to 0x1
            v.src_sel_a = ctl->src_sel_a;
            v.cg_en_a = ctl->cg_en_a;

            if (v.cg_en_a != 0) {
                writel(v.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1,
                               0, 100);
            }

            //set pre_div_num_b,check pre_upd_ack_b bit 1 means update not finished yes,0 means update finished.
            v.pre_div_num_a = ctl->pre_div_num_a;
            writel(v.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_SHIFT, 1, 0, 100);
            //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
            v.post_div_num = ctl->post_div_num;
            writel(v.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
            //invert pre_a_b_sel
            v.a_b_sel = 0;
            writel(v.val, slice_addr);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_core_slice_switch .
//!
//! \param base   clkgen peripheral base address
//! \core_clk_cfg core clock config
//!
//!This function is select core clk
//!
//! \return Returns \b true if the rstgen is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool clkgen_core_slice_switch(vaddr_t base,
                              clkgen_core_slice_drv_t *core_clk_cfg)
{
    bool ret = false;
    vaddr_t core_slice_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    uint8_t a_b_sel = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "start..............\n");
    core_slice_base_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(
                               core_clk_cfg->core_slice_idx);
    //read pre_a_b_sel,if it is 0x0,current selected path is a
    reg_read = readl(core_slice_base_addr);
    a_b_sel = ((reg_read & CLKGEN_CORE_SLICE_CTL_A_B_SEL_MASK) >>
               CLKGEN_CORE_SLICE_CTL_A_B_SEL_SHIFT);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "a_b_sel:%d\n", a_b_sel);

    if (/*(core_clk_cfg->clk_a_b_switch == core_clk_a_b_sel_b) &&*/
        (a_b_sel == core_clk_a_b_sel_a)) {
        //set pre_en_b to 0x0,disable the clock
        reg_read = readl(core_slice_base_addr);

        if ((reg_read & CLKGEN_CORE_SLICE_CTL_CG_EN_B_MASK) != 0) {
            reg_write = reg_read & (~CLKGEN_CORE_SLICE_CTL_CG_EN_B_MASK);
            writel(reg_write, core_slice_base_addr);
            reg_poll_clear(core_slice_base_addr,
                           CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1, 0, 100);
        }

        //change the clock source by set pre_mux_sel_b,then set pre_en_b to 0x1
        reg_read = readl(core_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B_MASK))
                    | CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B(core_clk_cfg->clk_src_sel_b)
                    | CLKGEN_CORE_SLICE_CTL_CG_EN_B_MASK;
        writel(reg_write, core_slice_base_addr);
        reg_poll_clear(core_slice_base_addr,
                       CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1, 1, 0, 100);
        //invert pre_a_b_sel
        reg_read = readl(core_slice_base_addr);
        reg_write = reg_read | CLKGEN_CORE_SLICE_CTL_A_B_SEL_MASK;
        writel(reg_write, core_slice_base_addr);
        //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(core_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_MASK))
                    | CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM(core_clk_cfg->post_div);
        writel(reg_write, core_slice_base_addr);
        ret = reg_poll_value(core_slice_base_addr,
                             CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
    }
    else {
        //default switch a
        //set pre_en_a to 0x0,disable the clock
        reg_read = readl(core_slice_base_addr);

        if ((reg_read & CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK) != 0) {
            reg_write = reg_read & (~CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK);
            writel(reg_write, core_slice_base_addr);
            reg_poll_clear(core_slice_base_addr,
                           CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1, 0, 100);
        }

        //change the clock source by set pre_mux_sel_a,then set pre_en_a to 0x1
        reg_read = readl(core_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A_MASK))
                    | CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A(core_clk_cfg->clk_src_sel_a)
                    | CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK;
        writel(reg_write, core_slice_base_addr);
        reg_poll_clear(core_slice_base_addr,
                       CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1, 1, 0, 100);
        //invert pre_a_b_sel
        reg_read = readl(core_slice_base_addr);
        reg_write = reg_read & (~CLKGEN_CORE_SLICE_CTL_A_B_SEL_MASK);
        writel(reg_write, core_slice_base_addr);
        //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
        reg_read = readl(core_slice_base_addr);
        reg_write = (reg_read & (~CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_MASK))
                    | CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM(core_clk_cfg->post_div);
        writel(reg_write, core_slice_base_addr);
        ret = reg_poll_value(core_slice_base_addr,
                             CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
    }

    return ret;
}
bool clkgen_core_ctl_get(vaddr_t base, uint32_t slice_idx,
                         clkgen_core_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;

// Check the arguments.
    if (ctl) {
        slice_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(slice_idx);
        ctl->val = readl(slice_addr);
    }

    return ret;
}
bool clkgen_core_ctl_set(vaddr_t base, uint32_t slice_idx,
                         const clkgen_core_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;

    if (ctl) {
        slice_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(slice_idx);
        clkgen_core_ctl c;
        //read pre_a_b_sel,if it is 0x0,current selected path is a
        c.val = readl(slice_addr);

        if (c.a_b_sel == 0) {
            //set pre_en_b to 0x0,disable the clock
            if (c.cg_en_b != 0) {
                c.cg_en_b = 0;
                writel(c.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1,
                               1, 0, 100);
            }

            //change the clock source by set pre_mux_sel_b,then set pre_en_b to 0x1
            c.src_sel_b = ctl->src_sel_a;
            c.cg_en_b = ctl->cg_en_a;
            writel(c.val, slice_addr);

            if (c.cg_en_b)
                reg_poll_clear(slice_addr, CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT, 1,
                               1, 0, 100);

            //invert pre_a_b_sel
            c.a_b_sel = 1;
            writel(c.val, slice_addr);
            //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
            c.post_div_num = ctl->post_div_num;
            writel(c.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
        }
        else {
            //set pre_en_a to 0x0,disable the clock
            if (c.cg_en_a != 0) {
                c.cg_en_a = 0;
                writel(c.val, slice_addr);
                reg_poll_clear(slice_addr, CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1,
                               1, 0, 100);
            }

            //change the clock source by set pre_mux_sel_a,then set pre_en_a to 0x1
            c.src_sel_a = ctl->src_sel_a;
            c.cg_en_a = ctl->cg_en_a;
            writel(c.val, slice_addr);

            if (c.cg_en_a)
                reg_poll_clear(slice_addr, CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT, 1,
                               1, 0, 100);

            //invert pre_a_b_sel
            c.a_b_sel = 0;
            writel(c.val, slice_addr);
            //set post div num,check post upd ack bit 1 means update not finished yes,0 means update finished.
            c.post_div_num = ctl->post_div_num;
            writel(c.val, slice_addr);
            ret = reg_poll_value(slice_addr,
                                 CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT, 1, 0, 100);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_mon_ip_slice .
//!
//! \param base   clkgen peripheral base address
//! \core_clk_cfg core clock config
//!
//!This function is mointor ip slice clock
//!
//! \return ret is clk reg number: clk= ret*slow_clk*(ref_clk_div+1)
//! if it is not.
//
//*****************************************************************************
uint32_t clkgen_mon_ip_slice(vaddr_t base, uint16_t ip_slice_idx,
                             clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                             clkgen_slice_mon_ret_type ret_type)
{
    uint32_t ret = 0;
    vaddr_t mon_ctl_base_addr;
    vaddr_t ip_slice_mon_base_addr;
    vaddr_t mon_max_freq_base_addr;
    vaddr_t mon_avg_freq_base_addr;
    vaddr_t mon_min_freq_base_addr;
    vaddr_t mon_max_duty_base_addr;
    vaddr_t mon_min_duty_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    ip_slice_mon_base_addr = base + CLKGEN_IP_SLICE_MON_CTL_OFF;
    mon_max_freq_base_addr = base + CLKGEN_MON_MAX_FREQ_OFF;
    mon_avg_freq_base_addr = base + CLKGEN_MON_AVE_FREQ_OFF;
    mon_min_freq_base_addr = base + CLKGEN_MON_MIN_FREQ_OFF;
    mon_max_duty_base_addr = base + CLKGEN_MON_MAX_DUTY_OFF;
    mon_min_duty_base_addr = base + CLKGEN_MON_MIN_DUTY_OFF;
    // 1.set mon_clk_dis in clkgen_mon_ctl to 0x1.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 2.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x1
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   1, 1000);
    // 3.set CKGEN_IP/BUS/CORE_SLICE_MON_CTL to proper value.
    reg_read = readl(ip_slice_mon_base_addr);
    reg_write = reg_read & (~CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL(
                    ip_slice_idx);
    writel(reg_write, ip_slice_mon_base_addr);
    // 4.set mon_sel in CKGEN_MON_CTL to proper value to choose the slice to be monitored.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_SEL(
                    slice_mon_ip_clk); //ip slice
    writel(reg_write, mon_ctl_base_addr);

    if (slice_mon_ref_clk_24M == ref_clk_type) {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read & (~CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK);
        writel(reg_write, mon_ctl_base_addr);
    }
    else {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read | CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK;
        writel(reg_write, mon_ctl_base_addr);
    }

    //set clk clow monitor div number
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_DIV_NUM_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_DIV_NUM(ref_clk_div);
    writel(reg_write, mon_ctl_base_addr);
    // 5.set mon_clk_dis in CKGEN_MON_CTL to 0x0.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_CLK_DIS_MASK);
    writel(reg_write, mon_ctl_base_addr);
    // 6.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x0
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   0, 1000);
    // 7.wait until freq_rdy_sta  in CKGEN_MON_CTL is equal to 0x1.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_FREQ_RDY_STA_SHIFT, 1, 1,
                   1000);
    // 8.set freq_upd_en in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_VAL_UPD_EN_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 9.wait until freq_upd_en  in CKGEN_MON_CTL is equal to 0x0.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_VAL_UPD_EN_SHIFT, 1,
                   0, 1000);

    //10.read CKGEN_MON_AVE_FREQ.
    if (mon_max_freq == ret_type) {
        ret = readl(mon_max_freq_base_addr);
    }
    else if (mon_avg_freq == ret_type) {
        ret = readl(mon_avg_freq_base_addr);
    }
    else if (mon_min_freq == ret_type) {
        ret = readl(mon_min_freq_base_addr);
    }
    else if (mon_max_duty == ret_type) {
        ret = readl(mon_max_duty_base_addr);
    }
    else if (mon_min_duty == ret_type) {
        ret = readl(mon_min_duty_base_addr);
    }
    else {
        ret = 0;
    }

    // 11.set mon_clk_dis in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    return ret;
}

//*****************************************************************************
//
//! clkgen_mon_bus_slice .
//!
//! \param base   clkgen peripheral base address
//! \core_clk_cfg core clock config
//!
//!This function is mointor bus slice clock
//!
//! \return ret is clk reg number: clk= ret*slow_clk*(ref_clk_div+1)
//! if it is not.
//
//*****************************************************************************
uint32_t clkgen_mon_bus_slice(vaddr_t base, uint16_t bus_slice_idx,
                              clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                              clkgen_slice_mon_ret_type ret_type)
{
    uint32_t ret = 0;
    vaddr_t mon_ctl_base_addr;
    vaddr_t bus_slice_mon_base_addr;
    vaddr_t mon_max_freq_base_addr;
    vaddr_t mon_avg_freq_base_addr;
    vaddr_t mon_min_freq_base_addr;
    vaddr_t mon_max_duty_base_addr;
    vaddr_t mon_min_duty_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    bus_slice_mon_base_addr = base + CLKGEN_BUS_SLICE_MON_CTL_OFF;
    mon_max_freq_base_addr = base + CLKGEN_MON_MAX_FREQ_OFF;
    mon_avg_freq_base_addr = base + CLKGEN_MON_AVE_FREQ_OFF;
    mon_min_freq_base_addr = base + CLKGEN_MON_MIN_FREQ_OFF;
    mon_max_duty_base_addr = base + CLKGEN_MON_MAX_DUTY_OFF;
    mon_min_duty_base_addr = base + CLKGEN_MON_MIN_DUTY_OFF;
    // 1.set mon_clk_dis in clkgen_mon_ctl to 0x1.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 2.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x1
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   1, 100);
    // 3.set CKGEN_IP/BUS/CORE_SLICE_MON_CTL to proper value.
    reg_read = readl(bus_slice_mon_base_addr);
    reg_write = reg_read & (~CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL(
                    bus_slice_idx);
    writel(reg_write, bus_slice_mon_base_addr);
    // 4.set mon_sel in CKGEN_MON_CTL to proper value to choose the slice to be monitored.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_SEL(
                    slice_mon_bus_clk); //bus slice
    writel(reg_write, mon_ctl_base_addr);

    if (slice_mon_ref_clk_24M == ref_clk_type) {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read & (~CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK);
        writel(reg_write, mon_ctl_base_addr);
    }
    else {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read | CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK;
        writel(reg_write, mon_ctl_base_addr);
    }

    //set clk clow monitor div number
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_DIV_NUM_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_DIV_NUM(ref_clk_div);
    writel(reg_write, mon_ctl_base_addr);
    // 5.set mon_clk_dis in CKGEN_MON_CTL to 0x0.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_CLK_DIS_MASK);
    writel(reg_write, mon_ctl_base_addr);
    // 6.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x0
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   0, 1000);
    // 7.wait until freq_rdy_sta  in CKGEN_MON_CTL is equal to 0x1.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_FREQ_RDY_STA_SHIFT, 1, 1,
                   1000);
    // 8.set freq_upd_en in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_VAL_UPD_EN_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 9.wait until freq_upd_en  in CKGEN_MON_CTL is equal to 0x0.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_VAL_UPD_EN_SHIFT, 1,
                   0, 1000);

    //10.read CKGEN_MON_AVE_FREQ.
    if (mon_max_freq == ret_type) {
        ret = readl(mon_max_freq_base_addr);
    }
    else if (mon_avg_freq == ret_type) {
        ret = readl(mon_avg_freq_base_addr);
    }
    else if (mon_min_freq == ret_type) {
        ret = readl(mon_min_freq_base_addr);
    }
    else if (mon_max_duty == ret_type) {
        ret = readl(mon_max_duty_base_addr);
    }
    else if (mon_min_duty == ret_type) {
        ret = readl(mon_min_duty_base_addr);
    }
    else {
        ret = 0;
    }

    // 11.set mon_clk_dis in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    return ret;
}

//*****************************************************************************
//
//! clkgen_mon_core_slice .
//!
//! \param base   clkgen peripheral base address
//! \core_clk_cfg core clock config
//!
//!This function is mointor core slice clock
//!
//! \return ret is clk reg number: clk= ret*slow_clk*(ref_clk_div+1)
//! if it is not.
//
//*****************************************************************************
uint32_t clkgen_mon_core_slice(vaddr_t base, uint16_t core_slice_idx,
                               clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                               clkgen_slice_mon_ret_type ret_type)
{
    uint32_t ret = 0;
    vaddr_t mon_ctl_base_addr;
    vaddr_t core_slice_mon_base_addr;
    vaddr_t mon_max_freq_base_addr;
    vaddr_t mon_avg_freq_base_addr;
    vaddr_t mon_min_freq_base_addr;
    vaddr_t mon_max_duty_base_addr;
    vaddr_t mon_min_duty_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    core_slice_mon_base_addr = base + CLKGEN_CORE_SLICE_MON_CTL_OFF;
    mon_max_freq_base_addr = base + CLKGEN_MON_MAX_FREQ_OFF;
    mon_avg_freq_base_addr = base + CLKGEN_MON_AVE_FREQ_OFF;
    mon_min_freq_base_addr = base + CLKGEN_MON_MIN_FREQ_OFF;
    mon_max_duty_base_addr = base + CLKGEN_MON_MAX_DUTY_OFF;
    mon_min_duty_base_addr = base + CLKGEN_MON_MIN_DUTY_OFF;
    // 1.set mon_clk_dis in clkgen_mon_ctl to 0x1.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 2.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x1
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   1, 1000);
    // 3.set CKGEN_IP/BUS/CORE_SLICE_MON_CTL to proper value.
    reg_read = readl(core_slice_mon_base_addr);
    reg_write = reg_read &
                (~CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL(
                    core_slice_idx);
    writel(reg_write, core_slice_mon_base_addr);
    // 4.set mon_sel in CKGEN_MON_CTL to proper value to choose the slice to be monitored.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_SEL_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_SEL(
                    slice_mon_core_clk); //core slice
    writel(reg_write, mon_ctl_base_addr);

    if (slice_mon_ref_clk_24M == ref_clk_type) {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read & (~CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK);
        writel(reg_write, mon_ctl_base_addr);
    }
    else {
        //clk slow select
        reg_read = readl(mon_ctl_base_addr);
        reg_write = reg_read | CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK;
        writel(reg_write, mon_ctl_base_addr);
    }

    //set clk clow monitor div number
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_DIV_NUM_MASK);
    reg_write = reg_write | CLKGEN_MON_CTL_MON_DIV_NUM(ref_clk_div);
    writel(reg_write, mon_ctl_base_addr);
    // 5.set mon_clk_dis in CKGEN_MON_CTL to 0x0.
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read & (~CLKGEN_MON_CTL_MON_CLK_DIS_MASK);
    writel(reg_write, mon_ctl_base_addr);
    // 6.wait until mon_clk_dis_sta in CKGEN_MON_CTL is equal to 0x0
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT, 1,
                   0, 1000);
    // 7.wait until freq_rdy_sta  in CKGEN_MON_CTL is equal to 0x1.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_FREQ_RDY_STA_SHIFT, 1, 1,
                   1000);
    // 8.set freq_upd_en in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_VAL_UPD_EN_MASK;
    writel(reg_write, mon_ctl_base_addr);
    // 9.wait until freq_upd_en  in CKGEN_MON_CTL is equal to 0x0.
    reg_poll_value(mon_ctl_base_addr, CLKGEN_MON_CTL_MON_VAL_UPD_EN_SHIFT, 1,
                   0, 1000);

    //10.read CKGEN_MON_AVE_FREQ.
    if (mon_max_freq == ret_type) {
        ret = readl(mon_max_freq_base_addr);
    }
    else if (mon_avg_freq == ret_type) {
        ret = readl(mon_avg_freq_base_addr);
    }
    else if (mon_min_freq == ret_type) {
        ret = readl(mon_min_freq_base_addr);
    }
    else if (mon_max_duty == ret_type) {
        ret = readl(mon_max_duty_base_addr);
    }
    else if (mon_min_duty == ret_type) {
        ret = readl(mon_min_duty_base_addr);
    }
    else {
        ret = 0;
    }

    // 11.set mon_clk_dis in CKGEN_MON_CTL to 0x1
    reg_read = readl(mon_ctl_base_addr);
    reg_write = reg_read | CLKGEN_MON_CTL_MON_CLK_DIS_MASK;
    writel(reg_write, mon_ctl_base_addr);
    return ret;
}

//*****************************************************************************
//
//! clkgen_uuu_clock_wrapper .
//!
//! \param base   clkgen peripheral base address
//! \uuu_clock_wrapper_idx core idx
//! \gasket_div:m/n/q/p division
//! \low_power_mode:if it is low power mode,need close pll
//! \input_type:uuu input clock type(ckgen_soc,pll_x,ckgen_soc&pll_x)
//!This function is an application specific clock generator for specific IP blocks such as:CPU1/CPU2/GPU1/GPU2/VPU/AI/HIS
//! which need dedicated PLL aside and don't have wide frequency divider requirement
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_uuu_clock_wrapper(vaddr_t base, uint16_t uuu_clock_wrapper_idx,
                              clkgen_gasket_type_t *gasket_div, bool low_power_mode,
                              uint8_t input_clk_type)
{
    bool ret = false;
    vaddr_t uuu_wrapper_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;

    // Check the arguments.
    if (base != CKGEN_SOC_BASE) {
        LTRACEF("base paramenter error \n");
        return 0;
    }

    uuu_wrapper_base_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(
                                uuu_clock_wrapper_idx);

    if (low_power_mode) {
        // 8.If you want to turn-off the PLL or enter into low-power mode etc.
        //a) Program CKGEN_SOC corresponding slice to get proper clk_ckgen frequency
        //b)Set Div_m/n/p/q to the proper value.
        reg_read = readl(uuu_wrapper_base_addr);
        reg_write = reg_read & (~(CLKGEN_UUU_SLICE_M_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_N_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_P_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_Q_DIV_NUM_MASK));
        reg_write |=
            (CLKGEN_UUU_SLICE_M_DIV_NUM(gasket_div->m_div_num)
             | CLKGEN_UUU_SLICE_N_DIV_NUM(gasket_div->n_div_num)
             | CLKGEN_UUU_SLICE_P_DIV_NUM(gasket_div->p_div_num)
             | CLKGEN_UUU_SLICE_Q_DIV_NUM(gasket_div->q_div_num));
        writel(reg_write, uuu_wrapper_base_addr);
        //c)Change clk_sel[0] and clk_sel[1] both to 0x0.
        reg_read = readl(uuu_wrapper_base_addr);
        reg_write = reg_read & (~(CLKGEN_UUU_SLICE_UUU_SEL(0x3)));
        reg_write = reg_write | CLKGEN_UUU_SLICE_UUU_SEL(input_clk_type);
        writel(reg_write, uuu_wrapper_base_addr);
        //d)Turn-off PLL_x
        //wait#######################
    }
    else {
        // 1.Program CKGEN_SOC corresponding slice to get proper clk_ckgen frequency. hal process
        //wait#######################
        // 2.Release SS reset(some ss don't have separate SS reset). set reset to 1 release reset,hal process
        //wait#######################
        // 3.Set clk_sel[0] and clk_sel[1]both to 0x0(default of both of them are 0x0)
        reg_read = readl(uuu_wrapper_base_addr);
        reg_write = reg_read & (~(CLKGEN_UUU_SLICE_UUU_SEL(
                                      0x3)));
        reg_write = reg_write | CLKGEN_UUU_SLICE_UUU_SEL(input_clk_type << 0);
        writel(reg_write, uuu_wrapper_base_addr);
        // 4.Program PLL_x to enable PLL and get the proper frequency.(see PLL programming guide).
        // call pll func enable pll clock,
        //wait#######################
        // 5.Set Div_m/n/p/q to the proper value before change the clock source from ckgen to PLL.(Div_m/n/p/q can be changed on-the-fly.)
        reg_read = readl(uuu_wrapper_base_addr);
        reg_write = reg_read & (~(CLKGEN_UUU_SLICE_M_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_N_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_P_DIV_NUM_MASK
                                  | CLKGEN_UUU_SLICE_Q_DIV_NUM_MASK));
        reg_write |= CLKGEN_UUU_SLICE_M_DIV_NUM(gasket_div->m_div_num)
                     | CLKGEN_UUU_SLICE_N_DIV_NUM(gasket_div->n_div_num)
                     | CLKGEN_UUU_SLICE_P_DIV_NUM(gasket_div->p_div_num)
                     | CLKGEN_UUU_SLICE_Q_DIV_NUM(gasket_div->q_div_num);
        writel(reg_write, uuu_wrapper_base_addr);
        // 6.Set clk_sel[0] to 0x1 to set clk_out_1/2/3 source from PLL_x.
        // 7.Set clk_sel[1] to 0x1 to set clk_out_0 source from PLL_x.
#if 0
        reg_read = readl(uuu_wrapper_base_addr);
        reg_write = reg_read | CLKGEN_UUU_SLICE_UUU_SEL(0x3);
        writel(reg_write, uuu_wrapper_base_addr);
#endif
    }

    ret = true;
    return ret;
}
bool clkgen_uuu_ctl_get(vaddr_t base, uint32_t slice_idx,
                        clkgen_uuu_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;

// Check the arguments.
    if (ctl) {
        slice_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(slice_idx);
        ctl->val = readl(slice_addr);
    }

    return ret;
}
bool clkgen_uuu_ctl_set(vaddr_t base, uint32_t slice_idx,
                        const clkgen_uuu_ctl *ctl)
{
    bool ret = true;
    vaddr_t slice_addr;

    if (ctl) {
        clkgen_uuu_ctl c;
        slice_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(slice_idx);
        // 1.Program CKGEN_SOC corresponding slice to get proper clk_ckgen frequency. hal process
        //wait#######################
        // 2.Release SS reset(some ss don't have separate SS reset). set reset to 1 release reset,hal process
        //wait#######################
        // 3.Set clk_sel[0] and clk_sel[1]both to 0x0(default of both of them are 0x0)
        c.val = readl(slice_addr);
        c.uuu_sel0 = 0;
        c.uuu_sel1 = 0;
        writel(c.val, slice_addr);
        // 4.Program PLL_x to enable PLL and get the proper frequency.(see PLL programming guide).
        // call pll func enable pll clock,
        //wait#######################
        // 5.Set Div_m/n/p/q to the proper value before change the clock source from ckgen to PLL.(Div_m/n/p/q can be changed on-the-fly.)
        c.m_div = ctl->m_div;
        c.n_div = ctl->n_div;
        c.p_div = ctl->p_div;
        c.q_div = ctl->q_div;
        writel(c.val, slice_addr);
        // 6.Set clk_sel[0] to 0x1 to set clk_out_1/2/3 source from PLL_x.
        // 7.Set clk_sel[1] to 0x1 to set clk_out_0 source from PLL_x.
        c.uuu_sel0 = ctl->uuu_sel0;
        c.uuu_sel1 = ctl->uuu_sel1;
        writel(c.val, slice_addr);
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_ipslice_debug_enable .
//!
//! \param base   clkgen peripheral base address
//! \slice_idx ip/bus/coe slice idx
//! \dbg_div:clock division
//!This function is for clock pad debug output
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_ipslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                 uint8_t dbg_div)
{
    vaddr_t debug_crtl_base_addr;
    vaddr_t debug_ip_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    debug_ip_base_addr = base + CLKGEN_IP_SLICE_DBG_CTL_OFF;
    reg_read = readl(debug_ip_base_addr);
    reg_write = reg_read & (~(CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                                  slice_idx)));
    reg_write = reg_write | CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                    slice_idx);
    writel(reg_write, debug_ip_base_addr);
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_DIV_NUM(
                                  dbg_div) | CLKGEN_DBG_CTL_DBG_SEL(0) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    reg_write = reg_write | (CLKGEN_DBG_CTL_DBG_DIV_NUM(dbg_div) |
                             CLKGEN_DBG_CTL_DBG_SEL(0) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_ipslice_debug_disable .
//!
//! \param base   clkgen peripheral base address
//!This function is for clock pad debug output disable
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_ipslice_debug_disable(vaddr_t base)
{
    vaddr_t debug_crtl_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_busslice_debug_enable .
//!
//! \param base   clkgen peripheral base address
//! \slice_idx ip/bus/coe slice idx
//! \dbg_div:clock division
//!This function is for clock pad debug output
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_busslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div)
{
    vaddr_t debug_crtl_base_addr;
    vaddr_t debug_bus_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    debug_bus_base_addr = base + CLKGEN_BUS_SLICE_DBG_CTL_OFF;
    reg_read = readl(debug_bus_base_addr);
    reg_write = reg_read & (~(CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                                  slice_idx)));
    reg_write = reg_write | CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                    slice_idx);
    writel(reg_write, debug_bus_base_addr);
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_DIV_NUM(
                                  dbg_div) | CLKGEN_DBG_CTL_DBG_SEL(1) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    reg_write = reg_write | (CLKGEN_DBG_CTL_DBG_DIV_NUM(dbg_div) |
                             CLKGEN_DBG_CTL_DBG_SEL(1) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_busslice_debug_disable .
//!
//! \param base   clkgen peripheral base address
//!This function is for clock pad debug output disable
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_busslice_debug_disable(vaddr_t base)
{
    vaddr_t debug_crtl_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_coreslice_debug_enable .
//!
//! \param base   clkgen peripheral base address
//! \slice_idx ip/bus/coe slice idx
//! \dbg_div:clock division
//!This function is for clock pad debug output
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_coreslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                   uint8_t dbg_div)
{
    vaddr_t debug_crtl_base_addr;
    vaddr_t debug_core_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    debug_core_base_addr = base + CLKGEN_CORE_SLICE_DBG_CTL_OFF;
    reg_read = readl(debug_core_base_addr);
    reg_write = reg_read & (~(CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                                  slice_idx)));
    reg_write = reg_write | CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(
                    slice_idx);
    writel(reg_write, debug_core_base_addr);
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_DIV_NUM(
                                  dbg_div) | CLKGEN_DBG_CTL_DBG_SEL(2) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    reg_write = reg_write | (CLKGEN_DBG_CTL_DBG_DIV_NUM(dbg_div) |
                             CLKGEN_DBG_CTL_DBG_SEL(2) | CLKGEN_DBG_CTL_DBG_CLK_DIS(1));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_coreslice_debug_disable .
//!
//! \param base   clkgen peripheral base address
//!This function is for clock pad debug output disable
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_coreslice_debug_disable(vaddr_t base)
{
    vaddr_t debug_crtl_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    //select ip/bus/core debug mode
    reg_read = readl(debug_crtl_base_addr);
    reg_write = reg_read & (~(CLKGEN_DBG_CTL_DBG_CLK_DIS(1)));
    writel(reg_write, debug_crtl_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_uuuslice_debug_enable .
//!
//! \param base   clkgen peripheral base address
//! \slice_idx ip/bus/coe slice idx
//! \dbg_div:clock division
//!This function is for clock pad debug output
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_uuuslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div)
{
    vaddr_t debug_uuu_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_uuu_base_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(slice_idx);
    //enable uuu debug mode
    reg_read = readl(debug_uuu_base_addr);
    reg_write = reg_read & (~(CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN(1)));
    reg_write = reg_write | CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN(1);
    reg_write = reg_write & (~(CLKGEN_UUU_SLICE_DBG_DIV_NUM(0xF)));
    reg_write = reg_write | CLKGEN_UUU_SLICE_DBG_DIV_NUM(dbg_div);
    writel(reg_write, debug_uuu_base_addr);
    return true;
}

//*****************************************************************************
//
//! clkgen_uuuslice_debug_disable .
//!
//! \param base   clkgen peripheral base address
//! \slice_idx uuu slice index
//!This function is for clock pad debug output disable
//!
//! \return true/false
//! if it is not.
//
//*****************************************************************************
bool clkgen_uuuslice_debug_disable(vaddr_t base, uint16_t slice_idx)
{
    vaddr_t debug_uuu_base_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);
    debug_uuu_base_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(slice_idx);
    //enable uuu debug mode
    reg_read = readl(debug_uuu_base_addr);
    reg_write = reg_read & (~(CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN(1)));
    writel(reg_write, debug_uuu_base_addr);
    return true;
}


