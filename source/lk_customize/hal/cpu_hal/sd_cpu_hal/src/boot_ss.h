/*
 * ssystem.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __BOOT_SS__
#define __BOOT_SS__

#include "clkgen_hal.h"
#include "cpu_hal.h"

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))
#define SS_BOOT_CFG(SS,entry_h, entry_l) \
            uint32_t __ss_iso_rst[]                 = SS##_SS_ISO;\
            uint32_t __ss_pre_module_rst[]          = SS##_SS_PRE_MODULE_RST;\
            uint32_t __ss_pre_gating[]              = SS##_SS_PRE_GATING;\
            struct uuu_clk_cfg __ss_pre_uuu_clk[]   = SS##_SS_PRE_UUU;\
            struct core_clk_cfg __ss_pre_core_clk[] = SS##_SS_PRE_CORE_CLK;\
            uint32_t __pll[]                        = SS##_SS_PLL;\
            uint32_t __ss_post_core_clk_gating[]    = SS##_SS_POST_CLK_GATING;\
            struct uuu_clk_cfg __ss_post_uuu_clk[]  = SS##_SS_POST_UUU;\
            uint64_t __ss_entry_signal_low          = SS##_SS_ENTRY_LOW;\
            uint64_t __ss_entry_signal_high         = SS##_SS_ENTRY_HIGH;\
            uint32_t __ss_post_module_rst[]         = SS##_SS_POST_MOUDLE_RST;\
            uint32_t __ss_post_core_rst[]           = SS##_SS_POST_CORE_RST;\
            struct ss_boot_cfg SS                   = {0};\
            SS.ss_iso_rst                           = __ss_iso_rst;\
            SS.ss_iso_rst_num                       = ARRAY_SIZE(__ss_iso_rst);\
            SS.ss_pre_module_rst                    = __ss_pre_module_rst;\
            SS.ss_pre_module_rst_num                = ARRAY_SIZE(__ss_pre_module_rst);\
            SS.ss_pre_gating                        = __ss_pre_gating;\
            SS.ss_pre_gating_num                    = ARRAY_SIZE(__ss_pre_gating);\
            SS.ss_pre_core_clk                      = __ss_pre_core_clk;\
            SS.ss_pre_core_clk_num                  = ARRAY_SIZE(__ss_pre_core_clk);\
            SS.ss_pre_uuu_clk                       = __ss_pre_uuu_clk;\
            SS.ss_pre_uuu_clk_num                   = ARRAY_SIZE(__ss_pre_uuu_clk);\
            SS.ss_pll                               = __pll;\
            SS.ss_pll_num                           = ARRAY_SIZE(__pll);\
            SS.ss_post_core_clk_gating              = __ss_post_core_clk_gating;\
            SS.ss_post_core_clk_gating_num          = ARRAY_SIZE(__ss_post_core_clk_gating);\
            SS.ss_post_uuu_clk                      = __ss_post_uuu_clk;\
            SS.ss_post_uuu_clk_num                  = ARRAY_SIZE(__ss_post_uuu_clk);\
            SS.ss_post_core_rst                     = __ss_post_core_rst;\
            SS.ss_post_core_rst_num                 = ARRAY_SIZE(__ss_post_core_rst);\
            SS.ss_post_module_rst                   = __ss_post_module_rst;\
            SS.ss_post_module_rst_num               = ARRAY_SIZE(__ss_post_module_rst);\
            SS.ss_entry_signal_low                  = __ss_entry_signal_low;\
            SS.ss_entry_signal_high                 = __ss_entry_signal_high;\
            SS.ss_entry_l                           = entry_l;\
            SS.ss_entry_h                           = entry_h;\

struct core_clk_cfg {
    uint32_t res_id;
    clkgen_app_core_cfg_t clk;
};

struct uuu_clk_cfg {
    uint32_t res_id;
    clkgen_app_uuu_cfg_t  clk;
};

struct ss_config_item {
    uint32_t *res_id;
    uint32_t  num;
};

struct ss_uuu_config_item {
    struct uuu_clk_cfg *clk;
    uint32_t  num;
};

struct ss_core_clk_config_item {
    struct core_clk_cfg *clk;
    uint32_t  num;
};

struct ss_boot_cfg {
    struct ss_config_item ss_iso_rst;
    struct ss_config_item ss_pre_module_rst;
    struct ss_config_item ss_pre_gating;
    struct ss_uuu_config_item ss_pre_uuu_clk;
    struct ss_core_clk_config_item ss_pre_core_clk;
    struct ss_config_item ss_pll;
    struct ss_config_item ss_post_core_clk_gating;
    struct ss_uuu_config_item ss_post_uuu_clk;
    struct ss_config_item ss_post_module_rst;
    struct ss_config_item ss_post_core_rst;

    union {
        uint64_t ss_entry_signal_low;
        uint64_t ss_remap_signal;
    };

    union {
        uint64_t ss_entry_signal_high;
        uint64_t ss_remap_en_signal;
    };
    uint32_t ss_entry_h;
    uint32_t ss_entry_l;
};

/* AP1 boot cfg begin
 * The macros format are subsystem_xxx
 * If one of the macros is no need, pls don't remove it and keep it empty.
 * */
#define AP1_SS_ISO               { RES_ISO_EN_SEC_CPU1 }
#define AP1_SS_PRE_MODULE_RST    { RES_MODULE_RST_SEC_CPU1_SS }

#define AP1_SS_PRE_GATING        { RES_GATING_EN_SOC_CPU1A_2_PLL_CPU1A_PLL_CPU1B_PCLK }
#define AP1_SS_PRE_CORE_CLK      {                                          \
                                    {                                      \
                                        .res_id = RES_CORE_SLICE_SOC_CPU1A,\
                                        .clk ={                            \
                                                .clk_src_select_a_num = 0, \
                                                .clk_src_select_b_num = 0, \
                                                .clk_a_b_select = 0,       \
                                                .post_div = 0              \
                                              }                            \
                                    }                                      \
                                }                                          \

#define AP1_SS_PRE_UUU           {                                                           \
                                    {                                                       \
                                        .res_id = RES_UUU_WRAP_SOC_CPU1A,                   \
                                        .clk = {                                            \
                                                    .uuu_input_clk_sel = uuu_input_soc_clk, \
                                                    .low_power_mode_en = 0,                 \
                                                    .m_div = 0,                             \
                                                    .n_div = 0,                             \
                                                    .p_div = 0,                             \
                                                    .q_div = 0                              \
                                                }                                           \
                                    }                                                       \
                                }                                                           \

#define AP1_SS_PLL               { RES_PLL_PLL_CPU1A, RES_PLL_PLL_CPU1B }
#define AP1_SS_POST_UUU          {                                                           \
                                    {                                                       \
                                        .res_id = RES_UUU_WRAP_SOC_CPU1A,                   \
                                        .clk =  {                                           \
                                                    .uuu_input_clk_sel = uuu_input_pll_clk, \
                                                    .low_power_mode_en = 0,                 \
                                                    .m_div = 0,                             \
                                                    .n_div = 1,                             \
                                                    .p_div = 3,                             \
                                                    .q_div = 7                              \
                                                }                                           \
                                    },                                                      \
                                    {                                                       \
                                        .res_id = RES_UUU_WRAP_SOC_CPU1B,                   \
                                        .clk =  {                                           \
                                                    .uuu_input_clk_sel = uuu_input_pll_clk, \
                                                    .low_power_mode_en = 0,                 \
                                                    .m_div = 0,                             \
                                                    .n_div = 1,                             \
                                                    .p_div = 3,                             \
                                                    .q_div = 7                              \
                                                }                                           \
                                    }                                                       \
                                }                                                           \

#define AP1_SS_ENTRY_LOW         SCR_SEC__L31__cpu1_rvbaraddr0_29_2
#define AP1_SS_ENTRY_HIGH        SCR_SEC__L31__cpu1_rvbaraddr0_39_30

#define AP1_SS_POST_CLK_GATING   {                                        \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0, \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1, \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2, \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3, \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4, \
                                    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5  \
                                }

#define AP1_SS_POST_CORE_RST     { RES_CORE_RST_SEC_CPU1_CORE_ALL_SW }
#define AP1_SS_POST_MOUDLE_RST   { RES_MODULE_RST_SEC_CPU1_CORE0_WARM, RES_MODULE_RST_SEC_CPU1_SCU_WARM }
/* AP1 boot cfg end */

/* AP2 boot cfg begin
 * The macros format are subsystem_xxx
 * If one of the macros is no need, pls don't remove it and keep it empty.
 * */
#define AP2_SS_ISO               {}
#define AP2_SS_PRE_MODULE_RST    { RES_MODULE_RST_SEC_CPU2_SS }

#define AP2_SS_PRE_GATING        { RES_GATING_EN_SOC_CPU2_PLL_CPU2_PCLK }

#define AP2_SS_PRE_CORE_CLK      {                                          \
                                    {                                      \
                                        .res_id = RES_CORE_SLICE_SOC_CPU2,\
                                        .clk ={                            \
                                                .clk_src_select_a_num = 0, \
                                                .clk_src_select_b_num = 0, \
                                                .clk_a_b_select = 0,       \
                                                .post_div = 0              \
                                              }                            \
                                    }                                      \
                                }                                          \

#define AP2_SS_PRE_UUU   {                                                           \
                            {                                                       \
                                .res_id = RES_UUU_WRAP_SOC_CPU2,                   \
                                .clk = {                                            \
                                            .uuu_input_clk_sel = uuu_input_soc_clk, \
                                            .low_power_mode_en = 0,                 \
                                            .m_div = 0,                             \
                                            .n_div = 0,                             \
                                            .p_div = 0,                             \
                                            .q_div = 0                              \
                                        }                                           \
                            }                                                       \
                        }                                                           \

#define AP2_SS_PLL           { RES_PLL_PLL_CPU2 }

#define AP2_SS_POST_UUU      {                                                           \
                                {                                                       \
                                    .res_id = RES_UUU_WRAP_SOC_CPU2,                    \
                                    .clk =  {                                           \
                                                .uuu_input_clk_sel = uuu_input_pll_clk, \
                                                .low_power_mode_en = 0,                 \
                                                .m_div = 0,                             \
                                                .n_div = 1,                             \
                                                .p_div = 3,                             \
                                                .q_div = 7                              \
                                            }                                           \
                                }                                                       \
                            }                                                           \

#define AP2_SS_ENTRY_LOW     SCR_SEC__L31__cpu2_rvbaraddr0_29_2
#define AP2_SS_ENTRY_HIGH    SCR_SEC__L31__cpu2_rvbaraddr0_39_30

#define AP2_SS_POST_CLK_GATING   {                                             \
                                    RES_GATING_EN_SOC_CPU2_0,                 \
                                    RES_GATING_EN_SOC_CPU2_PCLK_ATCLK_GICCLK, \
                                }

#define AP2_SS_POST_CORE_RST    { RES_CORE_RST_SEC_CPU2_CORE_SW }
#define AP2_SS_POST_MOUDLE_RST  {}
/* AP2 boot cfg end */

/* MP boot cfg begin
 * The macros format are subsystem_xxx
 * If one of the macros is no need, pls don't remove it and keep it empty.
 * */
#define MP_SS_ISO               {}
#define MP_SS_PRE_MODULE_RST    {}
#define MP_SS_PRE_GATING        {}
#define MP_SS_PRE_CORE_CLK      {                                          \
                                    {                                      \
                                        .res_id = RES_CORE_SLICE_SEC_MP_PLAT,\
                                        .clk ={                            \
                                                .clk_src_select_a_num = 5, \
                                                .clk_src_select_b_num = 5, \
                                                .clk_a_b_select = 0,       \
                                                .post_div = 0              \
                                              }                            \
                                    }                                      \
                                }                                          \

#define MP_SS_PRE_UUU           {}
#define MP_SS_PLL               {}
#define MP_SS_POST_UUU          {}

#define MP_SS_ENTRY_LOW         SCR_SEC__L31__remap_cr5_mp_ar_addr_offset_19_0
#define MP_SS_ENTRY_HIGH        SCR_SEC__L31__remap_cr5_mp_ar_remap_en

#define MP_SS_POST_CLK_GATING    {}
#define MP_SS_POST_CORE_RST      { RES_CORE_RST_SEC_CR5_MP_SW }
#define MP_SS_POST_MOUDLE_RST    {}
/* MP boot cfg end */

struct ss_boot_cfg *get_ss_cfg(sd_cpu_id cpu_id);
#endif
