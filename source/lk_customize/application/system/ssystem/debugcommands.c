/*
 * ssystem.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>

#include "chip_res.h"
#include "clkgen_hal.h"

#include "pll_hal.h"
#include "res.h"
#include "pll.h"
#include <string.h>

#include "debugcommands.h"

static void setup_gpu2_pll(int argc, const cmd_args* argv)
{
    uint32_t div = 2;
    static size_t len;

    if (argc < 2 && len == 0) {
        return;
    }

    if (strcmp(argv[0].str, "set_pll_div") == 0) {
        div = argv[1].u;
    }
    else {
        return;
    }

    pll_config_t gpu2_pll_cfg = {    // For GPU2
        .pll = PLL_GPU2,
        .integer = true,
        .spread_spectrum = false,
        .refdiv = 3,
        .postdiv = { 2, 1 },
        .fbdiv = 182,
        .frac = 0,
        .out_div = { 4, 6, 0, 0 },
    };

    if (div <= 7) {
        gpu2_pll_cfg.postdiv[0] = div;
        gpu2_pll_cfg.postdiv[1] = 1;
        gpu2_pll_cfg.fbdiv = 182;
    }
    else {
        switch(div) {
            case 8:
                gpu2_pll_cfg.postdiv[0] = 4;
                gpu2_pll_cfg.postdiv[1] = 2;
                gpu2_pll_cfg.fbdiv = 182;
                break;
            case 16:
                gpu2_pll_cfg.postdiv[0] = 4;
                gpu2_pll_cfg.postdiv[1] = 4;
                gpu2_pll_cfg.fbdiv = 182;
                break;
            case 32:
                gpu2_pll_cfg.postdiv[0] = 7;
                gpu2_pll_cfg.postdiv[1] = 4;
                gpu2_pll_cfg.fbdiv = 159;
                break;
            case 64:
                gpu2_pll_cfg.postdiv[0] = 7;
                gpu2_pll_cfg.postdiv[1] = 7;
                gpu2_pll_cfg.fbdiv = 139;
                break;
            default:
                dprintf(CRITICAL, "div value is invalid!!!!!!!\n");
            return;
        }
    }

    pll_config(APB_PLL_GPU2_BASE, &gpu2_pll_cfg);

    dprintf(ALWAYS, "set gpu2 pll clock to %d MHz, div: %d, regist value: 0x%x\n", 1456 / div, div, readl(0xf1490004));
}

static void list_gpu2_div_freq(void)
{
    dprintf(ALWAYS, "the following is relationship between div and frequence:\n");
    dprintf(ALWAYS, "div: 2  ---- 728MHz\n");
    dprintf(ALWAYS, "div: 4  ---- 364MHz\n");
    dprintf(ALWAYS, "div: 8  ---- 182MHz\n");
    dprintf(ALWAYS, "div: 16  ---- 91MHz\n");
    dprintf(ALWAYS, "div: 32  ---- 45MHz\n");
    dprintf(ALWAYS, "div: 64  ---- 22MHz\n");
}

void clean_or_invalid_cache(int argc, const cmd_args* argv)
{
    static size_t len;
    addr_t start;
    size_t size;

    if (argc < 3 && len == 0) {
        return;
    }

    dprintf(CRITICAL, "len is %d\n", len);

    start = argv[1].u;
    size = argv[2].u;

    if (strcmp(argv[0].str, "cc") == 0) {
        arch_clean_cache_range(start, size);
    }
    else if  (strcmp(argv[0].str, "ic") == 0) {
        arch_invalidate_cache_range(start, size);
    }
    else {
        return;
    }
}

struct clk_mapping clk_map[] = {
    {"MJPEG",           IP_SLICE,  RES_UUU_WRAP_SOC_MJPEG},//RES_IP_SLICE_SOC_MJPEG
    {"VPU1",            IP_SLICE,  RES_UUU_WRAP_SOC_VPU1},//RES_IP_SLICE_SOC_VPU1
    {"AUD4",            IP_SLICE,  RES_IP_SLICE_DISP_EXT_AUD4},
    {"AUD3",            IP_SLICE,  RES_IP_SLICE_DISP_EXT_AUD3},
    {"AUD2",            IP_SLICE,  RES_IP_SLICE_DISP_EXT_AUD2},
    {"AUD1",            IP_SLICE,  RES_IP_SLICE_DISP_EXT_AUD1},
    {"SPARE2",          IP_SLICE,  RES_IP_SLICE_DISP_SPARE2},
    {"SPARE1",          IP_SLICE,  RES_IP_SLICE_DISP_SPARE1},
    {"DC4",             IP_SLICE,  RES_IP_SLICE_DISP_DC4},
    {"DC3",             IP_SLICE,  RES_IP_SLICE_DISP_DC3},
    {"DC2",             IP_SLICE,  RES_IP_SLICE_DISP_DC2},
    {"DC1",             IP_SLICE,  RES_IP_SLICE_DISP_DC1},
    {"DC5",             IP_SLICE,  RES_IP_SLICE_DISP_DC5},
    {"DP3",             IP_SLICE,  RES_IP_SLICE_DISP_DP3},
    {"DP2",             IP_SLICE,  RES_IP_SLICE_DISP_DP2},
    {"DP1",             IP_SLICE,  RES_IP_SLICE_DISP_DP1},
    {"CSI3",            IP_SLICE,  RES_IP_SLICE_DISP_MIPI_CSI3_PIX},
    {"CSI2",            IP_SLICE,  RES_IP_SLICE_DISP_MIPI_CSI2_PIX},
    {"CSI1",            IP_SLICE,  RES_IP_SLICE_DISP_MIPI_CSI1_PIX},
    {"HPI_CLK800",      IP_SLICE,  RES_IP_SLICE_SEC_HPI_CLK800},
    {"HPI_CLK600",      IP_SLICE,  RES_IP_SLICE_SEC_HPI_CLK600},
    {"MSHC_TIMER",      IP_SLICE,  RES_IP_SLICE_SEC_MSHC_TIMER},
    {"SYS_CNT",         IP_SLICE,  RES_IP_SLICE_SEC_SYS_CNT},
    {"TRACE",           IP_SLICE,  RES_IP_SLICE_SEC_TRACE},
    {"CAN20",           IP_SLICE,  RES_IP_SLICE_SEC_CAN5_CAN20},
    {"GIC5",            IP_SLICE,  RES_IP_SLICE_SEC_GIC4_GIC5},
    {"CSI_MCLK2",       IP_SLICE,  RES_IP_SLICE_SEC_CSI_MCLK2},
    {"CSI_MCLK1",       IP_SLICE,  RES_IP_SLICE_SEC_CSI_MCLK1},
    {"SC8",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC8},
    {"SC7",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC7},
    {"SC6",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC6},
    {"SC5",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC5},
    {"SC4",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC4},
    {"SC3",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_SC3},
    {"MC2",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_MC2},
    {"MC1",             IP_SLICE,  RES_IP_SLICE_SEC_I2S_MC1},
    {"I2S_MCLK3",       IP_SLICE,  RES_IP_SLICE_SEC_I2S_MCLK3},
    {"I2S_MCLK2",       IP_SLICE,  RES_IP_SLICE_SEC_I2S_MCLK2},
    {"PWM8",            IP_SLICE,  RES_IP_SLICE_SEC_PWM8},
    {"PWM7",            IP_SLICE,  RES_IP_SLICE_SEC_PWM7},
    {"PWM6",            IP_SLICE,  RES_IP_SLICE_SEC_PWM6},
    {"PWM5",            IP_SLICE,  RES_IP_SLICE_SEC_PWM5},
    {"PWM4",            IP_SLICE,  RES_IP_SLICE_SEC_PWM4},
    {"PWM3",            IP_SLICE,  RES_IP_SLICE_SEC_PWM3},
    {"TIMER8",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER8},
    {"TIMER7",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER7},
    {"TIMER6",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER6},
    {"TIMER5",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER5},
    {"TIMER4",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER4},
    {"TIMER3",          IP_SLICE,  RES_IP_SLICE_SEC_TIMER3},
    {"OSPI2 ",          IP_SLICE,  RES_IP_SLICE_SEC_OSPI2},
    {"SPDIF4",          IP_SLICE,  RES_IP_SLICE_SEC_SPDIF4},
    {"SPDIF3",          IP_SLICE,  RES_IP_SLICE_SEC_SPDIF3},
    {"SPDIF2",          IP_SLICE,  RES_IP_SLICE_SEC_SPDIF2},
    {"SPDIF1",          IP_SLICE,  RES_IP_SLICE_SEC_SPDIF1},
    {"ENET2_TIMER_SEC", IP_SLICE,  RES_IP_SLICE_SEC_ENET2_TIMER_SEC},
    {"ENET2_PHY_REF",   IP_SLICE,  RES_IP_SLICE_SEC_ENET2_PHY_REF},
    {"ENET2_RMII",      IP_SLICE,  RES_IP_SLICE_SEC_ENET2_RMII},
    {"ENET2_TX",        IP_SLICE,  RES_IP_SLICE_SEC_ENET2_TX},
    {"EMMC4",           IP_SLICE,  RES_IP_SLICE_SEC_EMMC4},
    {"EMMC3",           IP_SLICE,  RES_IP_SLICE_SEC_EMMC3},
    {"EMMC2",           IP_SLICE,  RES_IP_SLICE_SEC_EMMC2},
    {"EMMC1",           IP_SLICE,  RES_IP_SLICE_SEC_EMMC1},
    {"UART_SEC1",       IP_SLICE,  RES_IP_SLICE_SEC_UART_SEC1},
    {"UART_SEC0",       IP_SLICE,  RES_IP_SLICE_SEC_UART_SEC0},
    {"SPI_SEC1",        IP_SLICE,  RES_IP_SLICE_SEC_SPI_SEC1},
    {"SPI_SEC0",        IP_SLICE,  RES_IP_SLICE_SEC_SPI_SEC0},
    {"I2C_SEC1",        IP_SLICE,  RES_IP_SLICE_SEC_I2C_SEC1},
    {"CE2",             IP_SLICE,  RES_IP_SLICE_SEC_CE2},
    {"HIS_BUS",          BUS_SLICE, RES_UUU_WRAP_SOC_HIS_BUS},//RES_BUS_SLICE_SOC_HIS_BUS_CTL
    {"NOC_BUS_CLOCK_GASKET", BUS_SLICE, RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_GASKET},
    {"NOC_BUS_CLOCK_CTL",    BUS_SLICE, RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL},
    {"VSN_BUS",          BUS_SLICE, RES_UUU_WRAP_SOC_VSN},//RES_BUS_SLICE_SOC_VSN_BUS_CTL
    {"VPU_BUS",          BUS_SLICE, RES_UUU_WRAP_SOC_VPU_BUS},//RES_BUS_SLICE_SOC_VPU_BUS_CTL
    {"DISP_BUS_GASKET",      BUS_SLICE, RES_BUS_SLICE_DISP_DISP_BUS_GASKET},
    {"DISP_BUS_CTL",         BUS_SLICE, RES_BUS_SLICE_DISP_DISP_BUS_CTL},
    {"SEC_PLAT_GASKET",      BUS_SLICE, RES_BUS_SLICE_SEC_SEC_PLAT_GASKET},
    {"SEC_PLAT_CTL",         BUS_SLICE, RES_BUS_SLICE_SEC_SEC_PLAT_CTL},
    {"DDR",            CORE_SLICE, RES_UUU_WRAP_SOC_DDR},//RES_CORE_SLICE_SOC_DDR
    {"GPU2",           CORE_SLICE, RES_UUU_WRAP_SOC_GPU2},//RES_CORE_SLICE_SOC_GPU2
    {"GPU1",           CORE_SLICE, RES_UUU_WRAP_SOC_GPU1},//RES_CORE_SLICE_SOC_GPU1
    {"CPU2",           CORE_SLICE, RES_UUU_WRAP_SOC_CPU2},//RES_CORE_SLICE_SOC_CPU2
    {"CPU1B",          CORE_SLICE, RES_UUU_WRAP_SOC_CPU1B},//RES_CORE_SLICE_SOC_CPU1B
    {"CPU1A",          CORE_SLICE, RES_UUU_WRAP_SOC_CPU1A},//RES_CORE_SLICE_SOC_CPU1A
    {"DISP_BUS",       CORE_SLICE, RES_CORE_SLICE_DISP_DISP_BUS},
    {"MP_PLAT",        CORE_SLICE, RES_CORE_SLICE_SEC_MP_PLAT}
};

void enable_uuu_debug(void)
{
    void *handle;

    bool ret = hal_clock_creat_handle(&handle);
    if (!ret) {
        dprintf(CRITICAL, "do_clkgen_ip_clock_test creat handle failed\n");
        return;
    }

    for (uint32_t i = RES_UUU_WRAP_SOC_CPU1A; i <= RES_UUU_WRAP_SOC_HIS_BUS; i++) {
        hal_clock_uuuslice_debug_enable(handle, i, 3);
    }

    hal_clock_release_handle(handle);
}

bool get_res_id(const char * ip_name, uint32_t *res_id, enum clk_type *type)
{
    bool ret = false;

    if (!ip_name) {
        return false;
    }

    int clk_count = sizeof(clk_map) / sizeof(clk_map[0]);

    for (int i  = 0; i < clk_count; i++) {
        if (0 == strcmp(ip_name, (const char*)(clk_map[i].name))) {
            *res_id = clk_map[i].res_id;
            *type = clk_map[i].type;
            ret = true;
            break;
        }
    }

    dprintf(ALWAYS, "ip name : %s， res_id : 0x%x\n", ip_name, *res_id);

    return ret;
}

void acquire_clk(uint32_t res_id)
{
    void * handle;
    uint32_t clk_val;

    bool ret = hal_clock_creat_handle(&handle);
    if (!ret) {
        dprintf(CRITICAL, "do_clkgen_ip_clock_test creat handle failed\n");
        return;
    }

    uint32_t ref_clk = mon_ref_clk_24M;

    if (RES_UUU_WRAP_SOC_CPU1A <= res_id && res_id <= RES_UUU_WRAP_SOC_HIS_BUS) {
            uint32_t pre_div = 1;

            if (mon_ref_clk_32K == ref_clk) {
                pre_div = 0xf;
            }

            clk_val = hal_clock_uuuclk_get(handle, res_id, ref_clk, pre_div);
            dprintf(ALWAYS, "UUU clock value : %ud, res_id : 0x%x\n", clk_val, res_id);
        }
    else if (res_id <= RES_IP_SLICE_SOC_MJPEG
        || (res_id > RES_DBG_SOC_CKGEN_MON_MIN_DUTY && res_id <= RES_IP_SLICE_DISP_EXT_AUD4)
        || (res_id > RES_DBG_DISP_CKGEN_MON_MIN_DUTY && res_id <= RES_IP_SLICE_SEC_HPI_CLK800)) {
        clk_val = hal_clock_ipclk_get(handle, res_id,
                                    ref_clk, 0);
        dprintf(ALWAYS, "IP clock value : %ud, res_id : 0x%x\n", clk_val, res_id);
    }
    else if (res_id <= RES_BUS_SLICE_SOC_HIS_BUS_GASKET
        || (res_id > RES_IP_SLICE_DISP_EXT_AUD4 && res_id <= RES_BUS_SLICE_DISP_DISP_BUS_GASKET)
        || (res_id > RES_IP_SLICE_SEC_HPI_CLK800 && res_id <= RES_BUS_SLICE_SEC_SEC_PLAT_GASKET)) {
        clk_val = hal_clock_busclk_get(handle, res_id, ref_clk, 0);

        dprintf(ALWAYS, "BUS clock value : %ud, res_id : 0x%x\n", clk_val, res_id);
    }
    else if (res_id <= RES_CORE_SLICE_SOC_DDR
        || (res_id > RES_BUS_SLICE_DISP_DISP_BUS_GASKET && res_id <= RES_CORE_SLICE_DISP_DISP_BUS)
        || (res_id > RES_BUS_SLICE_SEC_SEC_PLAT_GASKET && res_id <= RES_CORE_SLICE_SEC_MP_PLAT)) {
        clk_val = hal_clock_coreclk_get(handle, res_id, ref_clk, 0);

        dprintf(ALWAYS, "CORE clock value : %ud, res_id : 0x%x\n", clk_val, res_id);
    }
    else {
        dprintf(CRITICAL, "res_id 0x%x is invalid\n", res_id);
    }

    hal_clock_release_handle(handle);
}

static void dump_freq(void)
{
    int clk_count = sizeof(clk_map) / sizeof(clk_map[0]);
    char * type_name;

    for (int i  = 0; i < clk_count; i++) {
        if (IP_SLICE == clk_map[i].type) {
            type_name = (char*)"IP";
        }
        else if (BUS_SLICE == clk_map[i].type) {
            type_name = (char*)"BUS";
        }
        else {
            type_name = (char*)"CORE";
        }

        dprintf(ALWAYS, "%s name: %s , clock info as following:\n", type_name, clk_map[i].name);

        acquire_clk(clk_map[i].res_id);
    }
}

static void check_freq(int argc, const cmd_args* argv)
{
    uint32_t res_id;
    char * ip_name;
    enum clk_type type;
    static size_t len;
    int ret = 0;

    if (argc < 2 && len == 0) {
        return;
    }

    if (strcmp(argv[0].str, "check_freq") == 0) {
        ip_name = (char*)(argv[1].str);
    }
    else {
        return;
    }

    ret = get_res_id((const char*)ip_name, &res_id, &type);
    if (!ret) {
        dprintf(CRITICAL, "can't find resources by ip name.\n");
        return;
    }

    acquire_clk(res_id);
}

void list_ip_resid(void)
{
    dprintf(ALWAYS, "ip name               resid\n");
    dprintf(ALWAYS, "MJPEG           :     RES_IP_SLICE_SOC_MJPEG\n");
    dprintf(ALWAYS, "VPU1            :     RES_IP_SLICE_SOC_VPU1\n");
    dprintf(ALWAYS, "AUD4            :     RES_IP_SLICE_DISP_EXT_AUD4\n");
    dprintf(ALWAYS, "AUD3            :     RES_IP_SLICE_DISP_EXT_AUD3\n");
    dprintf(ALWAYS, "AUD2            :     RES_IP_SLICE_DISP_EXT_AUD2\n");
    dprintf(ALWAYS, "AUD1            :     RES_IP_SLICE_DISP_EXT_AUD1\n");
    dprintf(ALWAYS, "SPARE2          :     RES_IP_SLICE_DISP_SPARE2\n");
    dprintf(ALWAYS, "SPARE1          :     RES_IP_SLICE_DISP_SPARE1\n");
    dprintf(ALWAYS, "DC4             :     RES_IP_SLICE_DISP_DC4\n");
    dprintf(ALWAYS, "DC3             :     RES_IP_SLICE_DISP_DC3\n");
    dprintf(ALWAYS, "DC2             :     RES_IP_SLICE_DISP_DC2\n");
    dprintf(ALWAYS, "DC1             :     RES_IP_SLICE_DISP_DC1\n");
    dprintf(ALWAYS, "DC5             :     RES_IP_SLICE_DISP_DC5\n");
    dprintf(ALWAYS, "DP3             :     RES_IP_SLICE_DISP_DP3\n");
    dprintf(ALWAYS, "DP2             :     RES_IP_SLICE_DISP_DP2\n");
    dprintf(ALWAYS, "DP1             :     RES_IP_SLICE_DISP_DP1\n");
    dprintf(ALWAYS, "CSI3            :     RES_IP_SLICE_DISP_MIPI_CSI3_PIX\n");
    dprintf(ALWAYS, "CSI2            :     RES_IP_SLICE_DISP_MIPI_CSI2_PIX\n");
    dprintf(ALWAYS, "CSI1            :     RES_IP_SLICE_DISP_MIPI_CSI1_PIX\n");
    dprintf(ALWAYS, "HPI_CLK800      :     RES_IP_SLICE_SEC_HPI_CLK800\n");
    dprintf(ALWAYS, "HPI_CLK600      :     RES_IP_SLICE_SEC_HPI_CLK600\n");
    dprintf(ALWAYS, "MSHC_TIMER      :     RES_IP_SLICE_SEC_MSHC_TIMER\n");
    dprintf(ALWAYS, "SYS_CNT         :     RES_IP_SLICE_SEC_SYS_CNT\n");
    dprintf(ALWAYS, "TRACE           :     RES_IP_SLICE_SEC_TRACE\n");
    dprintf(ALWAYS, "CAN20           :     RES_IP_SLICE_SEC_CAN5_CAN20\n");
    dprintf(ALWAYS, "GIC5            :     RES_IP_SLICE_SEC_GIC4_GIC5\n");
    dprintf(ALWAYS, "CSI_MCLK2       :     RES_IP_SLICE_SEC_CSI_MCLK2\n");
    dprintf(ALWAYS, "CSI_MCLK1       :     RES_IP_SLICE_SEC_CSI_MCLK1\n");
    dprintf(ALWAYS, "SC8             :     RES_IP_SLICE_SEC_I2S_SC8\n");
    dprintf(ALWAYS, "SC7             :     RES_IP_SLICE_SEC_I2S_SC7\n");
    dprintf(ALWAYS, "SC6             :     RES_IP_SLICE_SEC_I2S_SC6\n");
    dprintf(ALWAYS, "SC5             :     RES_IP_SLICE_SEC_I2S_SC5\n");
    dprintf(ALWAYS, "SC4             :     RES_IP_SLICE_SEC_I2S_SC4\n");
    dprintf(ALWAYS, "SC3             :     RES_IP_SLICE_SEC_I2S_SC3\n");
    dprintf(ALWAYS, "MC2             :     RES_IP_SLICE_SEC_I2S_MC2\n");
    dprintf(ALWAYS, "MC1             :     RES_IP_SLICE_SEC_I2S_MC1\n");
    dprintf(ALWAYS, "I2S_MCLK3       :     RES_IP_SLICE_SEC_I2S_MCLK3\n");
    dprintf(ALWAYS, "I2S_MCLK2       :     RES_IP_SLICE_SEC_I2S_MCLK2\n");
    dprintf(ALWAYS, "PWM8            :     RES_IP_SLICE_SEC_PWM8\n");
    dprintf(ALWAYS, "PWM7            :     RES_IP_SLICE_SEC_PWM7\n");
    dprintf(ALWAYS, "PWM6            :     RES_IP_SLICE_SEC_PWM6\n");
    dprintf(ALWAYS, "PWM5            :     RES_IP_SLICE_SEC_PWM5\n");
    dprintf(ALWAYS, "PWM4            :     RES_IP_SLICE_SEC_PWM4\n");
    dprintf(ALWAYS, "PWM3            :     RES_IP_SLICE_SEC_PWM3\n");
    dprintf(ALWAYS, "TIMER8          :     RES_IP_SLICE_SEC_TIMER8\n");
    dprintf(ALWAYS, "TIMER7          :     RES_IP_SLICE_SEC_TIMER7\n");
    dprintf(ALWAYS, "TIMER6          :     RES_IP_SLICE_SEC_TIMER6\n");
    dprintf(ALWAYS, "TIMER5          :     RES_IP_SLICE_SEC_TIMER5\n");
    dprintf(ALWAYS, "TIMER4          :     RES_IP_SLICE_SEC_TIMER4\n");
    dprintf(ALWAYS, "TIMER3          :     RES_IP_SLICE_SEC_TIMER3\n");
    dprintf(ALWAYS, "OSPI2           :     RES_IP_SLICE_SEC_OSPI2\n");
    dprintf(ALWAYS, "SPDIF4          :     RES_IP_SLICE_SEC_SPDIF4\n");
    dprintf(ALWAYS, "SPDIF3          :     RES_IP_SLICE_SEC_SPDIF3\n");
    dprintf(ALWAYS, "SPDIF2          :     RES_IP_SLICE_SEC_SPDIF2\n");
    dprintf(ALWAYS, "SPDIF1          :     RES_IP_SLICE_SEC_SPDIF1\n");
    dprintf(ALWAYS, "ENET2_TIMER_SEC :     RES_IP_SLICE_SEC_ENET2_TIMER_SEC\n");
    dprintf(ALWAYS, "ENET2_PHY_REF   :     RES_IP_SLICE_SEC_ENET2_PHY_REF\n");
    dprintf(ALWAYS, "ENET2_RMII      :     RES_IP_SLICE_SEC_ENET2_RMII\n");
    dprintf(ALWAYS, "ENET2_TX        :     RES_IP_SLICE_SEC_ENET2_TX\n");
    dprintf(ALWAYS, "EMMC4           :     RES_IP_SLICE_SEC_EMMC4\n");
    dprintf(ALWAYS, "EMMC3           :     RES_IP_SLICE_SEC_EMMC3\n");
    dprintf(ALWAYS, "EMMC2           :     RES_IP_SLICE_SEC_EMMC2\n");
    dprintf(ALWAYS, "EMMC1           :     RES_IP_SLICE_SEC_EMMC1\n");
    dprintf(ALWAYS, "UART_SEC1       :     RES_IP_SLICE_SEC_UART_SEC1\n");
    dprintf(ALWAYS, "UART_SEC0       :     RES_IP_SLICE_SEC_UART_SEC0\n");
    dprintf(ALWAYS, "SPI_SEC1        :     RES_IP_SLICE_SEC_SPI_SEC1\n");
    dprintf(ALWAYS, "SPI_SEC0        :     RES_IP_SLICE_SEC_SPI_SEC0\n");
    dprintf(ALWAYS, "I2C_SEC1        :     RES_IP_SLICE_SEC_I2C_SEC1\n");
    dprintf(ALWAYS, "I2C_SEC0        :     RES_IP_SLICE_SEC_I2C_SEC0\n");
    dprintf(ALWAYS, "AUD_DSP1        :     RES_IP_SLICE_SEC_AUD_DSP1\n");
    dprintf(ALWAYS, "CE2             :     RES_IP_SLICE_SEC_CE2\n");
}

void list_bus_resid(void)
{
    dprintf(ALWAYS, "bus name               resid\n");
    dprintf(ALWAYS, "HIS_BUS_GASKET  :     RES_BUS_SLICE_SOC_HIS_BUS_GASKET\n");
    dprintf(ALWAYS, "HIS_BUS_CTL     :     RES_BUS_SLICE_SOC_HIS_BUS_CTL\n");
    dprintf(ALWAYS, "NOC_BUS_CLOCK_GASKET:     RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_GASKET\n");
    dprintf(ALWAYS, "NOC_BUS_CLOCK_CTL:     RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL\n");
    dprintf(ALWAYS, "VSN_BUS_GASKET  :     RES_BUS_SLICE_SOC_VSN_BUS_GASKET\n");
    dprintf(ALWAYS, "VSN_BUS_CTL     :     RES_BUS_SLICE_SOC_VSN_BUS_CTL\n");
    dprintf(ALWAYS, "VPU_BUS_GASKET  :     RES_BUS_SLICE_SOC_VPU_BUS_GASKET\n");
    dprintf(ALWAYS, "VPU_BUS_CTL     :     RES_BUS_SLICE_SOC_VPU_BUS_CTL\n");
    dprintf(ALWAYS, "DISP_BUS_GASKET :     RES_BUS_SLICE_DISP_DISP_BUS_GASKET\n");
    dprintf(ALWAYS, "DISP_BUS_CTL    :     RES_BUS_SLICE_DISP_DISP_BUS_CTL\n");
    dprintf(ALWAYS, "SEC_PLAT_GASKET :     RES_BUS_SLICE_SEC_SEC_PLAT_GASKET\n");
    dprintf(ALWAYS, "SEC_PLAT_CTL    :     RES_BUS_SLICE_SEC_SEC_PLAT_CTL\n");
}

void list_core_resid(void)
{
    dprintf(ALWAYS, "core name               resid\n");
    dprintf(ALWAYS, "DDR             :     RES_CORE_SLICE_SOC_DDR\n");
    dprintf(ALWAYS, "GPU2            :     RES_CORE_SLICE_SOC_GPU2 \n");
    dprintf(ALWAYS, "GPU1            :     RES_CORE_SLICE_SOC_GPU1 \n");
    dprintf(ALWAYS, "CPU2            :     RES_CORE_SLICE_SOC_CPU2 \n");
    dprintf(ALWAYS, "CPU1B           :     RES_CORE_SLICE_SOC_CPU1B\n");
    dprintf(ALWAYS, "CPU1A           :     RES_CORE_SLICE_SOC_CPU1A\n");
    dprintf(ALWAYS, "DISP_BUS        :     RES_CORE_SLICE_DISP_DISP_BUS\n");
    dprintf(ALWAYS, "MP_PLAT         :     RES_CORE_SLICE_SEC_MP_PLAT\n");
}

void overclock_cpu1(int argc, const cmd_args* argv)
{
    float cpu1_clk;
    float cpu1b_clk;
    static size_t len;

    if (strcmp(argv[0].str, "overclock_cpu1") != 0) {
        return;
    }

    if (argc < 2 && len == 0) {
        dprintf(ALWAYS, "please input expect clock based on GHz.\n");
        return;
    }

    pll_config_t cpu1a_pll_cfg = {    // For CPU1
		.pll = PLL_CPU1A,
		.integer = true,
		.spread_spectrum = false,
		.refdiv = 3,
		.postdiv = { 1, 1 },
		.fbdiv = 207,
		.frac = 0,
		.out_div = { 2, 4, 0, 0 },
	};

	pll_config_t cpu1b_pll_cfg = {   //For CPU1 bus
		.pll = PLL_CPU1B,
		.integer = true,
		.spread_spectrum = false,
		.refdiv = 3,
		.postdiv = { 1, 1 },
		.fbdiv = 166,
		.frac = 0,
		.out_div = { 2, 4, 0, 0 },
	};

    if (strcmp(argv[1].str, "1.8") == 0) {
        cpu1a_pll_cfg.fbdiv = 225;
        cpu1b_pll_cfg.fbdiv = 180;
        cpu1b_clk = 1.44;
        cpu1_clk = 1.8;
    }
    else if (strcmp(argv[1].str, "1.9") == 0) {
        cpu1a_pll_cfg.fbdiv = 237;
        cpu1b_pll_cfg.fbdiv = 190;
        cpu1b_clk = 1.52;
        cpu1_clk = 1.9;
    }
    else if (strcmp(argv[1].str, "2.0") == 0) {
        cpu1a_pll_cfg.fbdiv = 250;
        cpu1b_pll_cfg.fbdiv = 200;
        cpu1b_clk = 1.6;
        cpu1_clk = 2.0;
    }
    else if (strcmp(argv[1].str, "2.1") == 0) {
        cpu1a_pll_cfg.fbdiv = 263;
        cpu1b_pll_cfg.fbdiv = 210;
        cpu1b_clk = 1.68;
        cpu1_clk = 2.1;
    }
    else if (strcmp(argv[1].str, "2.2") == 0) {
        cpu1a_pll_cfg.fbdiv = 275;
        cpu1b_pll_cfg.fbdiv = 220;
        cpu1b_clk = 1.76;
        cpu1_clk = 2.2;
    }
    else {
        dprintf(ALWAYS, "can't support this refrenquence!!!!!\n");
        return;
    }

    pll_config(APB_PLL_CPU1A_BASE, &cpu1a_pll_cfg);
    pll_config(APB_PLL_CPU1B_BASE, &cpu1b_pll_cfg);

    dprintf(ALWAYS, "set cpu1 clock to %f GHz, set cpu1 bus clock to %f GHz\n", cpu1_clk, cpu1b_clk);

}

// LK console cmd
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("list_gpu2_div_freq", "list gpu2 coresponding div for some frequence", (console_cmd)&list_gpu2_div_freq)
STATIC_COMMAND("set_pll_div", "set gpu2 frequence by pll", (console_cmd)&setup_gpu2_pll)
STATIC_COMMAND("cc", "clean cache", (console_cmd)&clean_or_invalid_cache)
STATIC_COMMAND("ic", "clean cache", (console_cmd)&clean_or_invalid_cache)
STATIC_COMMAND("check_freq", "check frequence", (console_cmd)&check_freq)
STATIC_COMMAND("list_ip_res", "list ip resources for check frequence", (console_cmd)&list_ip_resid)
STATIC_COMMAND("list_bus_res", "list bus resources for check frequence", (console_cmd)&list_bus_resid)
STATIC_COMMAND("list_core_res", "list core resources for check frequence", (console_cmd)&list_core_resid)
STATIC_COMMAND("dump_freq", "dump all resources clk", (console_cmd)&dump_freq)
STATIC_COMMAND("enable_uuu_debug", "enable uuu slice debug", (console_cmd)&enable_uuu_debug)
STATIC_COMMAND("overclock_cpu1", "cpu1 overclock", (console_cmd)&overclock_cpu1)
STATIC_COMMAND_END(ss_debugcommands);

#endif

APP_START(ss_debugcommands)
.flags = 0,
APP_END
