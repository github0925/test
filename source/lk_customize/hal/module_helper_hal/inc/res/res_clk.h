/*
 * res_clk.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER CLK.
 *
 * Revision History:
 * -----------------
 */
#ifndef _RES_CLK_H
#define _RES_CLK_H

#include <lk/list.h>
#include <assert.h>
#include <module_helper_hal.h>
#include <kernel/spinlock.h>
#include <pll_hal.h>

#define RES_CLK_DEBUG 1

#define MAX_PARENTS_NUM 8
#define REFCNT_NUM ((MAX_PER_ID-1)/32 +1)
enum clk_id {
    RES_CLK_START = RES_INVALID + 1,
    CLK_ID_FIXRATE_FIRST = RES_CLK_START,
    CLK_ID_RC_24M = CLK_ID_FIXRATE_FIRST,
    CLK_ID_RC_RTC,
    CLK_ID_XTAL_24M,
    CLK_ID_XTAL_RTC,
    /*
    CLK_ID_EXT_AUD1,
    CLK_ID_EXT_AUD2,
    CLK_ID_EXT_AUD3,
    CLK_ID_EXT_AUD4,
    */
    CLK_ID_FIXRATE_LAST = CLK_ID_XTAL_RTC,
    CLK_ID_PLL_FIRST,
    CLK_ID_PLL1 = CLK_ID_PLL_FIRST,
    CLK_ID_PLL2,
    CLK_ID_PLL3,
    CLK_ID_PLL4,
    CLK_ID_PLL5,
    CLK_ID_PLL6,
    CLK_ID_PLL7,
    CLK_ID_PLL_DISP,
    CLK_ID_PLL_LVDS1,
    CLK_ID_PLL_LVDS2,
    CLK_ID_PLL_LVDS3,
    CLK_ID_PLL_LVDS4,
    CLK_ID_PLL_CPU1A,
    CLK_ID_PLL_CPU1B,
    CLK_ID_PLL_CPU2,
    CLK_ID_PLL_GPU1,
    CLK_ID_PLL_GPU2,
    CLK_ID_PLL_VPU,
    CLK_ID_PLL_VSN,
    CLK_ID_PLL_HPI,
    CLK_ID_PLL_HIS,
    CLK_ID_PLL_DDR,
    CLK_ID_PLL1_ROOT,
    CLK_ID_PLL1_DIVA,
    CLK_ID_PLL1_DIVB,
    CLK_ID_PLL1_DIVC,
    CLK_ID_PLL1_DIVD,
    CLK_ID_PLL2_ROOT,
    CLK_ID_PLL2_DIVA,
    CLK_ID_PLL2_DIVB,
    CLK_ID_PLL2_DIVC,
    CLK_ID_PLL2_DIVD,
    CLK_ID_PLL3_ROOT,
    CLK_ID_PLL3_DIVA,
    CLK_ID_PLL3_DIVB,
    CLK_ID_PLL3_DIVC,
    CLK_ID_PLL3_DIVD,
    CLK_ID_PLL4_ROOT,
    CLK_ID_PLL4_DIVA,
    CLK_ID_PLL4_DIVB,
    CLK_ID_PLL4_DIVC,
    CLK_ID_PLL4_DIVD,
    CLK_ID_PLL5_ROOT,
    CLK_ID_PLL5_DIVA,
    CLK_ID_PLL5_DIVB,
    CLK_ID_PLL5_DIVC,
    CLK_ID_PLL5_DIVD,
    CLK_ID_PLL6_ROOT,
    CLK_ID_PLL6_DIVA,
    CLK_ID_PLL6_DIVB,
    CLK_ID_PLL6_DIVC,
    CLK_ID_PLL6_DIVD,
    CLK_ID_PLL7_ROOT,
    CLK_ID_PLL7_DIVA,
    CLK_ID_PLL7_DIVB,
    CLK_ID_PLL7_DIVC,
    CLK_ID_PLL7_DIVD,
    CLK_ID_PLL_DISP_ROOT,
    CLK_ID_PLL_DISP_DIVA,
    CLK_ID_PLL_DISP_DIVB,
    CLK_ID_PLL_LVDS1_ROOT,
    CLK_ID_PLL_LVDS1_DIVA,
    CLK_ID_PLL_LVDS1_DIVB,
    CLK_ID_PLL_LVDS2_ROOT,
    CLK_ID_PLL_LVDS2_DIVA,
    CLK_ID_PLL_LVDS2_DIVB,
    CLK_ID_PLL_LVDS3_ROOT,
    CLK_ID_PLL_LVDS3_DIVA,
    CLK_ID_PLL_LVDS3_DIVB,
    CLK_ID_PLL_LVDS4_ROOT,
    CLK_ID_PLL_LVDS4_DIVA,
    CLK_ID_PLL_LVDS4_DIVB,
    CLK_ID_PLL_CPU1A_ROOT,
    CLK_ID_PLL_CPU1A_DIVA,
    CLK_ID_PLL_CPU1A_DIVB,
    CLK_ID_PLL_CPU1B_ROOT,
    CLK_ID_PLL_CPU1B_DIVA,
    CLK_ID_PLL_CPU1B_DIVB,
    CLK_ID_PLL_CPU2_ROOT,
    CLK_ID_PLL_CPU2_DIVA,
    CLK_ID_PLL_CPU2_DIVB,
    CLK_ID_PLL_GPU1_ROOT,
    CLK_ID_PLL_GPU1_DIVA,
    CLK_ID_PLL_GPU1_DIVB,
    CLK_ID_PLL_GPU2_ROOT,
    CLK_ID_PLL_GPU2_DIVA,
    CLK_ID_PLL_GPU2_DIVB,
    CLK_ID_PLL_VPU_ROOT,
    CLK_ID_PLL_VPU_DIVA,
    CLK_ID_PLL_VPU_DIVB,
    CLK_ID_PLL_VSN_ROOT,
    CLK_ID_PLL_VSN_DIVA,
    CLK_ID_PLL_VSN_DIVB,
    CLK_ID_PLL_HPI_ROOT,
    CLK_ID_PLL_HPI_DIVA,
    CLK_ID_PLL_HPI_DIVB,
    CLK_ID_PLL_HIS_ROOT,
    CLK_ID_PLL_HIS_DIVA,
    CLK_ID_PLL_HIS_DIVB,
    CLK_ID_PLL_DDR_ROOT,
    CLK_ID_PLL_DDR_DIVA,
    CLK_ID_PLL_DDR_DIVB,

    CLK_ID_PLL_LAST = CLK_ID_PLL_DDR_DIVB,
    /*CKGEN*/
    CLK_ID_CKGEN_FIRST,
    /* BUS*/
    CLK_ID_CKGEN_BUS_FIRST = CLK_ID_CKGEN_FIRST,
    CLK_ID_SEC_PLAT = CLK_ID_CKGEN_BUS_FIRST,
    CLK_ID_DISP_BUS,
    CLK_ID_SAF_PLAT,
    CLK_ID_VPU_BUS,
    CLK_ID_VSN_BUS,
    CLK_ID_NOC_BUS,
    CLK_ID_HIS_BUS,
    CLK_ID_CKGEN_BUS_LAST = CLK_ID_HIS_BUS,
    /* CORE*/
    CLK_ID_CKGEN_CORE_FIRST,
    CLK_ID_MP_PLAT = CLK_ID_CKGEN_CORE_FIRST,
    CLK_ID_CPU1A,
    CLK_ID_CPU1B,
    CLK_ID_CPU2,
    CLK_ID_GPU1,
    CLK_ID_GPU2,
    CLK_ID_DDR,
    CLK_ID_CKGEN_CORE_LAST = CLK_ID_DDR,
    /* IP*/
    CLK_ID_CKGEN_IP_FIRST,
    CLK_ID_EXT_AUD1 = CLK_ID_CKGEN_IP_FIRST,
    CLK_ID_EXT_AUD2,
    CLK_ID_EXT_AUD3,
    CLK_ID_EXT_AUD4,
    CLK_ID_CE2,
    CLK_ID_I2C_SEC0,
    CLK_ID_I2C_SEC1,
    CLK_ID_SPI_SEC0,
    CLK_ID_SPI_SEC1,
    CLK_ID_UART_SEC0,
    CLK_ID_UART_SEC1,
    CLK_ID_EMMC1,
    CLK_ID_EMMC2,
    CLK_ID_EMMC3,
    CLK_ID_EMMC4,
    CLK_ID_ENET2_TX,
    CLK_ID_ENET2_RMII,
    CLK_ID_ENET2_PHY_REF,
    CLK_ID_ENET2_TIMER_SEC,
    CLK_ID_SPDIF1,
    CLK_ID_SPDIF2,
    CLK_ID_SPDIF3,
    CLK_ID_SPDIF4,
    CLK_ID_OSPI2,
    CLK_ID_TIMER3,
    CLK_ID_TIMER4,
    CLK_ID_TIMER5,
    CLK_ID_TIMER6,
    CLK_ID_TIMER7,
    CLK_ID_TIMER8,
    CLK_ID_PWM3,
    CLK_ID_PWM4,
    CLK_ID_PWM5,
    CLK_ID_PWM6,
    CLK_ID_PWM7,
    CLK_ID_PWM8,
    CLK_ID_I2S_MCLK2,
    CLK_ID_I2S_MC1,
    CLK_ID_I2S_SC3,
    CLK_ID_I2S_MCLK3,
    CLK_ID_I2S_MC2,
    CLK_ID_I2S_SC5,
    CLK_ID_I2S_SC4,
    CLK_ID_I2S_SC7,
    CLK_ID_I2S_SC6,
    CLK_ID_I2S_SC8,
    CLK_ID_CSI_MCLK1,
    CLK_ID_CSI_MCLK2,
    CLK_ID_GIC4_GIC5,
    CLK_ID_CAN5_TO_20,
    CLK_ID_TRACE,
    CLK_ID_SYS_CNT,
    CLK_ID_MSHC_TIMER,
    CLK_ID_HPI_CLK600,
    CLK_ID_HPI_CLK800,
    CLK_ID_MIPI_CSI1_PIX,
    CLK_ID_MIPI_CSI2_PIX,
    CLK_ID_MIPI_CSI3_PIX,
    CLK_ID_DP1,
    CLK_ID_DP2,
    CLK_ID_DP3,
    CLK_ID_DC1,
    CLK_ID_DC2,
    CLK_ID_DC3,
    CLK_ID_DC4,
    CLK_ID_DC5,
    CLK_ID_CE1,
    CLK_ID_I2C_SAF,
    CLK_ID_UART_SAF,
    CLK_ID_SPI_SAF,
    CLK_ID_I2S_MCLK1,
    CLK_ID_I2S_SC1,
    CLK_ID_I2S_SC2,
    CLK_ID_ENET1_TX,
    CLK_ID_ENET1_RMII,
    CLK_ID_ENET1_PHY_REF,
    CLK_ID_ENET1_TIMER_SEC,
    CLK_ID_OSPI1,
    CLK_ID_TIMER1,
    CLK_ID_TIMER2,
    CLK_ID_PWM1,
    CLK_ID_PWM2,
    CLK_ID_CAN1_TO_4,
    CLK_ID_VPU1,
    CLK_ID_MJPEG,
    CLK_ID_CKGEN_IP_LAST = CLK_ID_MJPEG,
    /* UUU*/
    CLK_ID_CKGEN_UUU_FIRST,
    CLK_ID_CPU1A_SEL0 = CLK_ID_CKGEN_UUU_FIRST,
    CLK_ID_CPU1A_M,
    CLK_ID_CPU1A_0,
    CLK_ID_CPU1A_1,
    CLK_ID_CPU1A_2,
    CLK_ID_CPU1A_3,
    CLK_ID_CPU1B_SEL0,
    CLK_ID_CPU1B_M,
    CLK_ID_CPU1B_0,
    CLK_ID_CPU1B_1,
    CLK_ID_CPU1B_2,
    CLK_ID_CPU1B_3,
    CLK_ID_CPU2_SEL0,
    CLK_ID_CPU2_M,
    CLK_ID_CPU2_0,
    CLK_ID_CPU2_1,
    CLK_ID_CPU2_2,
    CLK_ID_CPU2_3,
    CLK_ID_GPU1_SEL0,
    CLK_ID_GPU1_M,
    CLK_ID_GPU1_0,
    CLK_ID_GPU1_1,
    CLK_ID_GPU1_2,
    CLK_ID_GPU1_3,
    CLK_ID_GPU2_SEL0,
    CLK_ID_GPU2_M,
    CLK_ID_GPU2_0,
    CLK_ID_GPU2_1,
    CLK_ID_GPU2_2,
    CLK_ID_GPU2_3,
    CLK_ID_VPU1_SEL0,
    CLK_ID_VPU1_M,
    CLK_ID_VPU1_0,
    CLK_ID_VPU1_1,
    CLK_ID_VPU1_2,
    CLK_ID_VPU1_3,
    CLK_ID_MJPEG_SEL0,
    CLK_ID_MJPEG_M,
    CLK_ID_MJPEG_0,
    CLK_ID_MJPEG_1,
    CLK_ID_MJPEG_2,
    CLK_ID_MJPEG_3,
    CLK_ID_VPU_BUS_SEL0,
    CLK_ID_VPU_BUS_M,
    CLK_ID_VPU_BUS_0,
    CLK_ID_VPU_BUS_1,
    CLK_ID_VPU_BUS_2,
    CLK_ID_VPU_BUS_3,
    CLK_ID_VSN_BUS_SEL0,
    CLK_ID_VSN_BUS_M,
    CLK_ID_VSN_BUS_0,
    CLK_ID_VSN_BUS_1,
    CLK_ID_VSN_BUS_2,
    CLK_ID_VSN_BUS_3,
    CLK_ID_DDR_SEL0,
    CLK_ID_DDR_M,
    CLK_ID_DDR_0,
    CLK_ID_DDR_1,
    CLK_ID_DDR_2,
    CLK_ID_DDR_3,
    CLK_ID_HIS_BUS_SEL0,
    CLK_ID_HIS_BUS_M,
    CLK_ID_HIS_BUS_0,
    CLK_ID_HIS_BUS_1,
    CLK_ID_HIS_BUS_2,
    CLK_ID_HIS_BUS_3,
    CLK_ID_CKGEN_UUU_LAST = CLK_ID_HIS_BUS_3,
    CLK_ID_CKGEN_LAST = CLK_ID_CKGEN_UUU_LAST,
    RES_CLK_END = CLK_ID_CKGEN_LAST,
};

struct pll_spec {
    //only pll need this
    int ratetable_size;
    pll_config_t **ratetable;
    int plldiv; /*indicate pll type, 0, root; 1~4:a,b,c,d; 5 dummy root*/
    struct clk *dependson;
    bool moreprecise;
    pll_config_t config;
};
struct uuu_spec {
    //only uuu need this
    int uuu_type;   /*indicate uuu type, 0~3:M,N,P,Q */
};

struct clk {
    enum clk_id clkid;
    uint32_t resid;//global res id
#ifdef RES_CLK_DEBUG
    char name[32];
#endif
    spin_lock_t lock;
    spin_lock_saved_state_t lockstate;

    uint32_t refcount[REFCNT_NUM];  //care about rate
    uint32_t refcount_en[REFCNT_NUM]; //only care about en/dis status

    int cur_parent_index;
    int parent_nums;

    struct clk *parents[MAX_PARENTS_NUM];   //const
    struct list_node node;  //dynmic, current node
    struct list_node child; //dynmic, head of child list
    unsigned long rate;//dummy rate for dummy node;
    //can be optimized
    int ptable_size;
    int *ptable;
    uint32_t gate_resid;//global res id

    //unsigned long minfreq;//0, means no limit;
    //unsigned long maxfreq;//UINT32_MAX, means no limit;
    int mindiv; //1, means no limit;
    int maxdiv; //INT16_MAX, means no limit;
    union {
        struct pll_spec pll;
        struct uuu_spec uuu;
    };

    //debug
    int enable_cnt;
    int disable_cnt;

    int (*is_enable)(struct clk *clk);
    int (*enable)(struct clk *clk);
    int (*disable)(struct clk *clk);
    int (*set_rate)(struct clk *clk, unsigned long prate, unsigned long freq);
    unsigned long (*get_rate)(struct clk *clk, unsigned long prate,
                              bool bymonitor);
    unsigned long (*round_rate)(struct clk *clk, int pindex,
                                unsigned long *prate, unsigned long freq);
    int (*set_parent)(struct clk *clk, int parent_index);
};
void register_res_clks(void);
int res_clk_request(enum module_per_id per_id, enum clk_id clkid,
                    unsigned long rate);
unsigned long res_clk_get_rate_by_id(enum clk_id clkid, bool bymonitor);

//for debug
void dump_clktree(struct clk *clk, bool bymonitor);

#endif /* _RES_CLK_H */


