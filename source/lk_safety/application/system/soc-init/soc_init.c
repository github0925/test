/*
 * Copyright (c) 2020 Semidrive Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <reg.h>
#include <platform.h>
#include <lk/init.h>

#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_hw.h"
#endif

#include <hal_port.h>
#include <hal_dio.h>
#include <uart_hal.h>
#include <target_res.h>
#include <target.h>
#include <trace.h>
#include <dcf.h>
#include <clkgen_hal.h>
#if WITH_HAL_MODULE_HELPER_HAL
#include <module_helper_hal.h>
#include <res/res_clk.h>
#endif

#include "pll_hal.h"
#include "rstgen_hal.h"
#include "res.h"
#include "scr_hal.h"
#include "pmu_hal.h"
//#include "hal/pmu_hal/sd_pmu_hal/inc/pmu_hal.h"

extern int scr_init(void);
extern void hpi_qos_init(void);

static const uint32_t refclk_ResIdx[] = {
    RES_SCR_L16_SEC_FSREFCLK_CPU1,
    RES_SCR_L16_SEC_FSREFCLK_CPU2,
    RES_SCR_L16_SEC_FSREFCLK_GPU1,
    RES_SCR_L16_SEC_FSREFCLK_GPU2,
    RES_SCR_L16_SEC_FSREFCLK_HIS,
    RES_SCR_L16_SEC_FSREFCLK_VPU,
    RES_SCR_L16_SEC_FSREFCLK_VSN,
    RES_SCR_L16_SEC_FSREFCLK_DISP,
};

//added by walter for power on eth0 in xianhong proj
int set_pmu_ctrl_ap_domain(int state)
{
    int ret = 0;
    void *handle;

    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);

    if (ret != 0) {
        printf("get handle fail\n");
        return 1;
    }

    ret = hal_pmu_init(handle);

    if (ret != 0) {
        printf("pmu init fail\n");
        return 1;
    }

	ret = hal_pmu_set_powerctrl_io_mode(handle, PWR_CTRL_2, 0);

	if (ret != 0) {
		printf("pmu set power ctrl PWR_CTRL_2 to out mode fail\n");
		return 1;
	}

	ret = hal_pmu_set_powerctrl_out_mode(handle, PWR_CTRL_2, 1);

	if (ret != 0) {
		printf("pmu set power ctrl PWR_CTRL_2 to soft mode fail\n");
		return 1;
	}

	ret = hal_pmu_set_powerctrl_out_ctrl(handle, PWR_CTRL_2, state);

	if (ret != 0) {
		printf("pmu set power ctrl PWR_CTRL_2 to low level fail\n");
		return 1;
	}

    ret = hal_pmu_exit(handle);

    if (ret != 0) {
        printf("pmu exit fail\n");
        return 1;
    }

    return hal_pmu_release_handle(handle);
}
//ended by walter
static void refclk_config(void)
{
    void *g_sec_handle;
    bool ret = true;
    ret = hal_clock_creat_handle(&g_sec_handle);

    if (!ret) {
        printf("clkgen creat handle failed\n");
        return;
    }

    uint8_t glb_res_idx_size = sizeof(refclk_ResIdx) / sizeof(
                                   refclk_ResIdx[0]);

    for (uint8_t i = 0; i < glb_res_idx_size; i++) {
        ret = hal_clock_osc_init(g_sec_handle, refclk_ResIdx[i], xtal_saf_24M,
                                 true);
    }

    hal_clock_release_handle(g_sec_handle);
}

static void setup_pll(uint32_t resid)
{
    pll_handle_t pll;
    pll =  hal_pll_create_handle(resid);

    if (pll == (pll_handle_t)0) {
        dprintf(INFO, "pll res 0x%x not belong this domain\n", resid);
        return;
    }

    hal_pll_config(pll, NULL);
    hal_pll_delete_handle(pll);
}

void module_iso_disable(uint32_t res_id)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_rstgen_creat_handle(&handle, res_id);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_creat_handle res_id 0x%x fail.\n", res_id);
        return;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_init res_id 0x%x fail.\n", res_id);
        goto fail;
    }

    ret = hal_rstgen_iso_disable(handle, res_id);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_iso_disable res_id 0x%x fail.\n", res_id);
    }

fail:
    hal_rstgen_release_handle(handle);
}

void module_rst(uint32_t res_id)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_rstgen_creat_handle(&handle, res_id);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_creat_handle res_id 0x%x fail.\n", res_id);
        return;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_init res_id 0x%x fail.\n", res_id);
        goto fail;
    }

    ret = hal_rstgen_module_reset(handle, res_id);

    if (!ret) {
        dprintf(CRITICAL, "hal_rstgen_module_reset res_id 0x%x fail.\n", res_id);
    }

fail:
    hal_rstgen_release_handle(handle);
}

void init_gpu1_ss(void)
{
    void *handle = NULL;
    bool ret = false;
    module_iso_disable(RES_ISO_EN_SEC_GPU1);
    module_rst(RES_MODULE_RST_SEC_GPU1_SS);
    module_rst(RES_MODULE_RST_SEC_GPU1_CORE);

    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_creat_handle fail\n");
        return;
    }

    /* soc gpu1--->24M */
    clkgen_app_core_cfg_t corecfg;
    /* default--24MHz */
    corecfg.clk_src_select_a_num = 0;
    /* default--24MHz */
    corecfg.clk_src_select_b_num = 0;
    corecfg.clk_a_b_select = 0;
    corecfg.post_div = 0;
    ret = hal_clock_coreclk_set(handle, RES_CORE_SLICE_SOC_GPU1, &corecfg);

    if (!ret) {
        dprintf(CRITICAL, "clock_coreclk resid 0x%x fail\n",
                RES_CORE_SLICE_SOC_GPU1);
        goto fail;
    }

    /* config uuu */
    clkgen_app_uuu_cfg_t uuu_clk = {0};
    uuu_clk.uuu_input_clk_sel = uuu_input_soc_clk;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 0;
    /* pclk of pll_gpu1 */
    uuu_clk.p_div = 0;
    uuu_clk.q_div = 0;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_GPU1, &uuu_clk);

    if (!ret) {
        dprintf(CRITICAL, "clock_uuuclk resid 0x%x fail\n", RES_UUU_WRAP_SOC_GPU1);
        goto fail;
    }

    /* enable clock */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_GPU1_2);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_GPU1_2);
        goto fail;
    }

    /* config pll_gpu1 */
    setup_pll(RES_PLL_PLL_GPU1);
fail:
    hal_clock_release_handle(handle);
}

void init_gpu2_ss(void)
{
    void *handle = NULL;
    bool ret = false;
    module_rst(RES_MODULE_RST_SEC_GPU2_SS);
    module_rst(RES_MODULE_RST_SEC_GPU2_CORE);
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_creat_handle fail\n");
        return;
    }

    /* soc gpu2--->24M */
    clkgen_app_core_cfg_t corecfg;
    /* default--24MHz */
    corecfg.clk_src_select_a_num = 0;
    /* default--24MHz */
    corecfg.clk_src_select_b_num = 0;
    corecfg.clk_a_b_select = 0;
    corecfg.post_div = 0;
    ret = hal_clock_coreclk_set(handle, RES_CORE_SLICE_SOC_GPU2, &corecfg);

    if (!ret) {
        dprintf(CRITICAL, "clock_coreclk resid 0x%x fail\n",
                RES_CORE_SLICE_SOC_GPU2);
        goto fail;
    }

    /* config uuu */
    clkgen_app_uuu_cfg_t uuu_clk = {0};
    uuu_clk.uuu_input_clk_sel = uuu_input_soc_clk;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 0;
    /* pclk of pll_gpu2 */
    uuu_clk.p_div = 0;
    uuu_clk.q_div = 0;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_GPU2, &uuu_clk);

    if (!ret) {
        dprintf(CRITICAL, "clock_uuuclk resid 0x%x fail\n", RES_UUU_WRAP_SOC_GPU2);
        goto fail;
    }

    /* enable clock */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_GPU2_2);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_GPU2_2);
        goto fail;
    }

    /* config pll_gpu2 */
    setup_pll(RES_PLL_PLL_GPU2);
fail:
    hal_clock_release_handle(handle);
}

#if !defined(PLATFORM_G9X) && !defined(PLATFORM_G9Q)
/* temp for bringup, improve later */
void init_display_ss(void)
{
#if WITH_HAL_MODULE_HELPER_HAL
#if MODULE_HELPER_PER_DISP
    module_set_state(PER_ID_DISP, DISP_INIT);
#endif
#endif
    /* init lvds_clk_wrappers, they are in IP inner */
    uint32_t rval;
    const uint32_t lvds_clock_offset = 0x10000;
    addr_t addr = APB_LVDS_COMMON_BASE + lvds_clock_offset + 0x4;

    for (int i = 0; i < 4; i++) {
        rval = readl(addr);
        writel((rval & (~(0x3F))) | 0x26, addr);
        addr += lvds_clock_offset;
    }
}

void init_vpu_ss(void)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_creat_handle fail\n");
        return;
    }

    /* config uuu for vpu_bus */
    clkgen_app_uuu_cfg_t uuu_clk = {0};
    uuu_clk.uuu_input_clk_sel = 3;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 3;
    uuu_clk.p_div = 0;
    uuu_clk.q_div = 0;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_VPU_BUS, &uuu_clk);

    if (!ret) {
        dprintf(CRITICAL, "clock_uuuclk resid 0x%x fail\n",
                RES_UUU_WRAP_SOC_VPU_BUS);
        goto fail;
    }

    /* config uuu for vpu1 */
    uuu_clk.uuu_input_clk_sel = 3;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 0;
    uuu_clk.p_div = 0;
    uuu_clk.q_div = 0;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_VPU1, &uuu_clk);
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_MJPEG, &uuu_clk);

    if (!ret) {
        dprintf(CRITICAL, "clock_uuuclk resid 0x%x fail\n", RES_UUU_WRAP_SOC_VPU1);
        goto fail;
    }

    /* enable pclk */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_1_VPU2_PCLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_1_VPU1_PCLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_1_MJPEG_PCLK);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_VPU_BUS_1_VPU1_PCLK);
        goto fail;
    }

    /* enable aclk */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_0_VPU2_ACLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_0_VPU1_ACLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_0_MJPEG_ACLK);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_VPU_BUS_0_VPU1_ACLK);
        goto fail;
    }

    /* vpu1.bclk */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU1);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_MJPEG);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_GPU1_2);
        goto fail;
    }

    /* pll_vpu.pclk */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VPU_BUS_1_PLL_VPU);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable resid 0x%x fail\n",
                RES_GATING_EN_SOC_GPU1_2);
        goto fail;
    }

fail:
    hal_clock_release_handle(handle);
}
#endif

#ifdef VDSP_ENABLE
void init_vsn_ss(void)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_creat_handle fail\n");
        return;
    }

    clkgen_app_bus_cfg_t bus_app_cfg;
    bus_app_cfg.clk_src_select_a_num = 0,
    bus_app_cfg.clk_src_select_b_num = 0,
    bus_app_cfg.clk_a_b_select = 0;
    bus_app_cfg.pre_div_a = 0;
    bus_app_cfg.pre_div_b = 0;
    bus_app_cfg.post_div = 0;
    bus_app_cfg.m_div = 0;
    bus_app_cfg.n_div = 0;
    bus_app_cfg.p_div = 0;
    bus_app_cfg.q_div = 0;
    ret = hal_clock_busclk_set(handle, RES_BUS_SLICE_SOC_VSN_BUS_CTL,
                               &bus_app_cfg);

    if (!ret) {
        dprintf(CRITICAL, "clock_coreclk resid 0x%x fail\n",
                RES_BUS_SLICE_SOC_VSN_BUS_CTL);
        goto fail;
    }

    /* config uuu for vsn_bus */
    clkgen_app_uuu_cfg_t uuu_clk = {0};
    uuu_clk.uuu_input_clk_sel = uuu_input_soc_clk;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 0;
    /* pclk of pll_vsn */
    uuu_clk.p_div = 0;
    uuu_clk.q_div = 0;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_VSN, &uuu_clk);

    if (!ret) {
        dprintf(CRITICAL, "clock_uuuclk resid 0x%x fail\n", RES_UUU_WRAP_SOC_VSN);
        goto fail;
    }

    /* enable pclk */
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_1_EIC_VSN);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_1_BIPC_VDSP_PCLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_0_BIPC_VDSP_ACLK);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_1_PLL_VSN);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_1_NOC_VSP);
    ret = hal_clock_enable(handle, RES_GATING_EN_SOC_VSN_BUS_0_VDSP_CLK);

    if (!ret) {
        dprintf(CRITICAL, "hal_clock_enable VDSP resid 0x%x fail\n",
                RES_GATING_EN_SOC_VSN_BUS_0_VDSP_CLK);
        goto fail;
    }

    /* config pll_vsn */
    setup_pll(RES_VDSP_PLL_VSN);
fail:
    hal_clock_release_handle(handle);
}
#endif

void init_usb_ss(void)
{
    module_iso_disable(RES_ISO_EN_SEC_USB);
}

void init_pcie_ss(void)
{
    module_iso_disable(RES_ISO_EN_SEC_PCIE);
}

void rstgen_sec_module_rst(unsigned int idx, unsigned int rst_b)
{
    unsigned int rval;
    uint32_t count = 100;
    const uint32_t module_rst_offset = 0x100 << 10;
    addr_t addr = APB_RSTGEN_SEC_BASE + module_rst_offset + ((
                      0x4 * idx) << 10);
    rval = readl(addr);
    writel(((rval & (~(0x1 << 1))) | (0x1 << 1)), addr);

    /* Wait module reset enable done. */
    while ((count--) && (!((readl(addr) & (0x1 << 1)) == (0x1 << 1))));

    rval = readl(addr);
    writel(((rval & (~(0x1 << 0))) | rst_b), addr);
    /* Wait module reset done. */
    count = 100;

    while ((count--) && ((readl(addr) & (0x1 << 30)) != rst_b << 30));
}

void release_rst_all(void)
{
    for (unsigned int i = 0; i < 104; i++) {
        /* 21 - cpu1_core0, 26 - cpu1_core5 */
        if (i >= 21 && i <= 26) {
            continue;
        }

        /* skip vdsp dreset */
        if (i == 38) {
            continue;
        }

        rstgen_sec_module_rst(i, 0x1);
    }
}

/* enable sec/disp/soc gating_en & release rst_modue in secure subsystem */
void ckgen_lp_gating_disable(unsigned int ckgen_base, unsigned int idx,
                             unsigned int value)
{
    const uint32_t gating_en_offset = 0x400 << 10;
    addr_t addr = ckgen_base + gating_en_offset + ((idx * 4) << 10);
    uint32_t rval = readl(addr);
    writel(((rval & (~(0x1 << 1))) | (value << 1)), addr);
}

void enable_gating_all(void)
{
    uint32_t i;
    const uint32_t gating_max_num = 128;

    for (i = 0; i < gating_max_num; i++) {
        ckgen_lp_gating_disable(APB_CKGEN_SOC_BASE, i, 0x0);
    }

    for (i = 0; i < gating_max_num; i++) {
        ckgen_lp_gating_disable(APB_CKGEN_SEC_BASE, i, 0x0);
    }

    for (i = 0; i < gating_max_num; i++) {
        ckgen_lp_gating_disable(APB_CKGEN_DISP_BASE, i, 0x0);
    }
}


/* init module config */
int module_config_init(void)
{
    //init pll
    setup_pll(RES_PLL_PLL6);
    setup_pll(RES_PLL_PLL7);
#if !DISABLE_VPU
    setup_pll(RES_PLL_PLL_VPU);
#endif
    setup_pll(RES_PLL_PLL_HIS);
    init_usb_ss();
    init_pcie_ss();
    //debug
#if !defined(PLATFORM_G9X) && !defined(PLATFORM_G9Q)
    /* init GPU */
#if !DISABLE_GPU
    init_gpu1_ss();
    init_gpu2_ss();
#endif
    /* init disp */
    init_display_ss();
    /* init vpu */
#if !DISABLE_VPU
    init_vpu_ss();
#endif

#endif

#ifdef VDSP_ENABLE
    init_vsn_ss();
#endif
    hpi_qos_init();
    hal_sec_clock_set_default();
    hal_soc_clock_set_default();
#if SUPPORT_DISP_SDDRV
    hal_disp_clock_set_default();
#endif
    /* enable all gating_en of sec/disp/soc */
    enable_gating_all();
    /* release all module_rst of sec */
    release_rst_all();
    return 0;
}

static void platform_port_init(void)
{
    void *port_init_handle = NULL;
    hal_port_creat_handle(&port_init_handle, RES_PAD_CONTROL_SAF_JTAG_TMS);

    if (port_init_handle) {
        hal_port_init(port_init_handle);
#if ENABLE_PIN_DELTA_CONFIG
        bool ret = false;
        dprintf(INFO, "delta patch\n");
        int type = get_part_id(PART_BOARD_TYPE);

        //get hwid
        if (type == BOARD_TYPE_MS) {
            int major = get_part_id(PART_BOARD_ID_MAJ);

            if (major != BOARDID_MAJOR_UNKNOWN) {
                ret = hal_port_init_delta(port_init_handle, 0, major);
                dprintf(CRITICAL, "hal_port_init_delta status: %d.\n", ret);
            }

            int minor = get_part_id(PART_BOARD_ID_MIN);

            if (minor != BOARDID_MINOR_UNKNOWN) {
                ret = hal_port_init_delta(port_init_handle, 1, minor);
                dprintf(CRITICAL, "hal_port_init_delta status: %d.\n", ret);
            }
        }

        dprintf(INFO, "delta patch ok\n");
#endif
    }
    else {
        dprintf(ALWAYS, "port get handle failed!\n");
    }

    hal_port_release_handle(port_init_handle);
}

void soc_init(uint level)
{
    refclk_config();
    scr_init();

    module_config_init();
    platform_port_init();
	set_pmu_ctrl_ap_domain(1);
}
LK_INIT_HOOK(soc_init, soc_init, LK_INIT_LEVEL_PLATFORM);
