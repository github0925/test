/*
 * boot_ss_cfg.c
 *
 * Copyright (c) 2021 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include "boot_ss.h"
#include "chip_res.h"
#include "scr_hal.h"

/* ap1 config data start */
static uint32_t ap1_iso_rst[]        = {RES_ISO_EN_SEC_CPU1};
static uint32_t ap1_pre_module_rst[] = {RES_MODULE_RST_SEC_CPU1_SS};
static uint32_t ap1_pre_gating[]     = {RES_GATING_EN_SOC_CPU1A_2_PLL_CPU1A_PLL_CPU1B_PCLK};
static struct uuu_clk_cfg ap1_pre_uuu_clk[] = {
    {
        .res_id = RES_UUU_WRAP_SOC_CPU1A,
        .clk = {
            .uuu_input_clk_sel = uuu_input_soc_clk,
            .low_power_mode_en = 0,
            .m_div = 0,
            .n_div = 0,
            .p_div = 0,
            .q_div = 0
        }
    }
};

static struct core_clk_cfg ap1_pre_core_clk[] = {
    {
        .res_id = RES_CORE_SLICE_SOC_CPU1A,
        .clk = {
            .clk_src_select_a_num = 0,
            .clk_src_select_b_num = 0,
            .clk_a_b_select = 0,
            .post_div = 0
        }
    }
};

static uint32_t ap1_pll[] = {RES_PLL_PLL_CPU1A, RES_PLL_PLL_CPU1B};
static uint32_t ap1_post_core_clk_gating[] = {
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5
};

static struct uuu_clk_cfg ap1_post_uuu_clk[] = {
    {
        .res_id = RES_UUU_WRAP_SOC_CPU1A,
        .clk =  {
            .uuu_input_clk_sel = uuu_input_pll_clk,
            .low_power_mode_en = 0,
            .m_div = 0,
            .n_div = 1,
            .p_div = 3,
            .q_div = 7
        }
    },
    {
        .res_id = RES_UUU_WRAP_SOC_CPU1B,
        .clk =  {
            .uuu_input_clk_sel = uuu_input_pll_clk,
            .low_power_mode_en = 0,
            .m_div = 0,
            .n_div = 1,
            .p_div = 3,
            .q_div = 7
        }
    }
};

static uint32_t ap1_post_module_rst[] = {RES_MODULE_RST_SEC_CPU1_CORE0_WARM, RES_MODULE_RST_SEC_CPU1_SCU_WARM};
static uint32_t ap1_post_core_rst[]   = {RES_CORE_RST_SEC_CPU1_CORE_ALL_SW};

static struct ss_boot_cfg ap1_cfg = {
    .ss_iso_rst              = {ap1_iso_rst, ARRAY_SIZE(ap1_iso_rst)},
    .ss_pre_module_rst       = {ap1_pre_module_rst, ARRAY_SIZE(ap1_pre_module_rst)},
    .ss_pre_gating           = {ap1_pre_gating, ARRAY_SIZE(ap1_pre_gating)},
    .ss_pre_uuu_clk          = {ap1_pre_uuu_clk, ARRAY_SIZE(ap1_pre_uuu_clk)},
    .ss_pre_core_clk         = {ap1_pre_core_clk, ARRAY_SIZE(ap1_pre_core_clk)},
    .ss_pll                  = {ap1_pll, ARRAY_SIZE(ap1_pll)},
    .ss_post_core_clk_gating = {ap1_post_core_clk_gating, ARRAY_SIZE(ap1_post_core_clk_gating)},
    .ss_post_uuu_clk         = {ap1_post_uuu_clk, ARRAY_SIZE(ap1_post_uuu_clk)},
    .ss_post_module_rst      = {ap1_post_module_rst, ARRAY_SIZE(ap1_post_module_rst)},
    .ss_post_core_rst        = {ap1_post_core_rst, ARRAY_SIZE(ap1_post_core_rst)},
    .ss_entry_signal_low     = SCR_SEC__L31__cpu1_rvbaraddr0_29_2,
    .ss_entry_signal_high    = SCR_SEC__L31__cpu1_rvbaraddr0_39_30,
};
/* ap1 config data end */

/* ap2 config data start */
static uint32_t ap2_pre_module_rst[] = {RES_MODULE_RST_SEC_CPU2_SS};
static uint32_t ap2_pre_gating[]     = {RES_GATING_EN_SOC_CPU2_PLL_CPU2_PCLK};

static struct uuu_clk_cfg ap2_pre_uuu_clk[] = {
    {
        .res_id = RES_UUU_WRAP_SOC_CPU2,
        .clk = {
            .uuu_input_clk_sel = uuu_input_soc_clk,
            .low_power_mode_en = 0,
            .m_div = 0,
            .n_div = 0,
            .p_div = 0,
            .q_div = 0
        }
    }
};

static struct core_clk_cfg ap2_pre_core_clk[] = {
    {
        .res_id = RES_CORE_SLICE_SOC_CPU2,
        .clk = {
            .clk_src_select_a_num = 0,
            .clk_src_select_b_num = 0,
            .clk_a_b_select = 0,
            .post_div = 0
        }
    }
};

static uint32_t ap2_pll[] = {RES_PLL_PLL_CPU2};
static uint32_t ap2_post_core_clk_gating[] = {
    RES_GATING_EN_SOC_CPU2_0,
    RES_GATING_EN_SOC_CPU2_PCLK_ATCLK_GICCLK,
};

static struct uuu_clk_cfg ap2_post_uuu_clk[] = {
    {
        .res_id = RES_UUU_WRAP_SOC_CPU2,
        .clk =  {
            .uuu_input_clk_sel = uuu_input_pll_clk,
            .low_power_mode_en = 0,
            .m_div = 0,
            .n_div = 1,
            .p_div = 3,
            .q_div = 7
        }
    }
};

static uint32_t ap2_post_core_rst[] = {RES_CORE_RST_SEC_CPU2_CORE_SW};

static struct ss_boot_cfg ap2_cfg = {
    .ss_iso_rst              = {NULL, 0},
    .ss_pre_module_rst       = {ap2_pre_module_rst, ARRAY_SIZE(ap2_pre_module_rst)},
    .ss_pre_gating           = {ap2_pre_gating, ARRAY_SIZE(ap2_pre_gating)},
    .ss_pre_uuu_clk          = {ap2_pre_uuu_clk, ARRAY_SIZE(ap2_pre_uuu_clk)},
    .ss_pre_core_clk         = {ap2_pre_core_clk, ARRAY_SIZE(ap2_pre_core_clk)},
    .ss_pll                  = {ap2_pll, ARRAY_SIZE(ap2_pll)},
    .ss_post_core_clk_gating = {ap2_post_core_clk_gating, ARRAY_SIZE(ap2_post_core_clk_gating)},
    .ss_post_uuu_clk         = {ap2_post_uuu_clk, ARRAY_SIZE(ap2_post_uuu_clk)},
    .ss_post_module_rst      = {NULL, 0},
    .ss_post_core_rst        = {ap2_post_core_rst, ARRAY_SIZE(ap2_post_core_rst)},
    .ss_entry_signal_low     = SCR_SEC__L31__cpu2_rvbaraddr0_29_2,
    .ss_entry_signal_high    = SCR_SEC__L31__cpu2_rvbaraddr0_39_30,
};
/* ap2 config data end */

/* mp config data start */
static struct core_clk_cfg mp_pre_core_clk[] = {
    {
        .res_id = RES_CORE_SLICE_SEC_MP_PLAT,
        .clk = {
            .clk_src_select_a_num = 5,
            .clk_src_select_b_num = 5,
            .clk_a_b_select = 0,
            .post_div = 0
        }
    }
};

static uint32_t mp_post_core_rst[] = {RES_CORE_RST_SEC_CR5_MP_SW};

static struct ss_boot_cfg mp_cfg = {
    .ss_iso_rst              = {NULL, 0},
    .ss_pre_module_rst       = {NULL, 0},
    .ss_pre_gating           = {NULL, 0},
    .ss_pre_uuu_clk          = {NULL, 0},
    .ss_pre_core_clk         = {mp_pre_core_clk, ARRAY_SIZE(mp_pre_core_clk)},
    .ss_pll                  = {NULL, 0},
    .ss_post_core_clk_gating = {NULL, 0},
    .ss_post_uuu_clk         = {NULL, 0},
    .ss_post_module_rst      = {NULL, 0},
    .ss_post_core_rst        = {mp_post_core_rst, ARRAY_SIZE(mp_post_core_rst)},
    .ss_remap_signal         = SCR_SEC__L31__remap_cr5_mp_ar_addr_offset_19_0,
    .ss_remap_en_signal      = SCR_SEC__L31__remap_cr5_mp_ar_remap_en,
};
/* mp config data end */

struct ss_boot_cfg *get_ss_cfg(sd_cpu_id cpu_id)
{
    struct ss_boot_cfg *cfg[CPU_ID_MAX] = {
        NULL,
        &ap1_cfg,
        &ap2_cfg,
        &mp_cfg,
        NULL,
        NULL,
    };

    if (cpu_id >= CPU_ID_MAX || cpu_id <= CPU_ID_MIN) {
        dprintf(CRITICAL, "%s cpu id:%d error\n", __func__, cpu_id);
        return NULL;
    }

    return cfg[cpu_id];
}
