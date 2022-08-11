/*
 * boot_ss.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <debug.h>

#include "boot_ss.h"
#include "chip_res.h"
#include "clkgen_hal.h"
#include "cpu_hal.h"
#include "pll_hal.h"
#include "res.h"
#include "rstgen_hal.h"
#include "scr_hal.h"

#define RSTGEN_GENERAL_REG(n) ((0x50 + (n)*4) << 10)
#define RSTGEN_SEC_REMAP_STATUS_REG (APB_RSTGEN_SEC_BASE + RSTGEN_GENERAL_REG(1))
#define RSTGEN_SAF_REMAP_STATUS_REG (APB_RSTGEN_SAF_BASE + RSTGEN_GENERAL_REG(1))
#define REMAP_DONE (0x52454d50) /* 'REMP' */
#define SEC_LOCKSTEP_SCR_ADDR (0xf82ad000)
#define SEC_LOCKSTEP_SCR_BIT   0

#define SAF_LOCKSTEP_SCR_ADDR (0xfc297000)
#define SAF_LOCKSTEP_SCR_BIT   0

static bool boot_ss(struct ss_boot_cfg *cfg);
static bool boot_ss_by_id(sd_cpu_id cpu_id, uint64_t addr);
static bool boot_sec(uint32_t addr);
static bool boot_saf(uint32_t addr);

bool hal_cpu_create_handle(void **handle_p)
{
    return true;
}

bool hal_cpu_release_handle(void *handle)
{
    return true;
}

bool hal_cpu_boot(void *handle, sd_cpu_id cpu_id, uint64_t entry)
{
    if (cpu_id >= CPU_ID_MAX || cpu_id <= CPU_ID_MIN) {
        dprintf(CRITICAL, "%s cpu id:%d error\n", __func__, cpu_id);
        return false;
    }

    if (cpu_id >= CPU_ID_AP1 && cpu_id <= CPU_ID_MP) {
        return boot_ss_by_id(cpu_id, entry);
    }
    else if (cpu_id == CPU_ID_SEC) {
        return boot_sec(entry);
    }
    else if (cpu_id == CPU_ID_SAF) {
        return boot_saf(entry);
    }
    else {
        return false;
    }
}

static void sec_core_reset(void)
{
    /* Reset R5 core. */
    void *handle;
    bool ret;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);

    writel(readl(SEC_LOCKSTEP_SCR_ADDR) | (0x3 << SEC_LOCKSTEP_SCR_BIT),
           SEC_LOCKSTEP_SCR_ADDR);
    hal_rstgen_core_reset(handle, RES_CORE_RST_SEC_CR5_SEC_SW);
    hal_rstgen_release_handle(handle);
}

static bool boot_sec(uint32_t img_base)
{
    /* base address must be 4 KB aligned */
    ASSERT((img_base & 0xFFF) == 0);
    writel(REMAP_DONE, _ioaddr(RSTGEN_SEC_REMAP_STATUS_REG));
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_handle_t handle;
    handle = hal_scr_create_handle(
                        SCR_SEC__L31__remap_cr5_sec_ar_addr_offset_19_0);
    ASSERT(handle);
    hal_scr_set(handle, img_base >> 12);
    hal_scr_delete_handle(handle);
    handle = hal_scr_create_handle(
                        SCR_SEC__L31__remap_cr5_sec_ar_remap_ovrd_en);
    ASSERT(handle);
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);
    sec_core_reset();
    return true;
}

static void saf_core_reset(void)
{
    /* Reset R5 core. */
    int ret;
    int32_t idx = 0;
    addr_t phy_addr = 0;

    ret = res_get_info_by_id(RES_CORE_RST_SAF_CR5_SAF_SW, &phy_addr, &idx);
    ASSERT(!ret);
    arch_disable_ints();
    arch_disable_cache(UCACHE);
    writel(readl(SAF_LOCKSTEP_SCR_ADDR) | (0x3 << SAF_LOCKSTEP_SCR_BIT),
           SAF_LOCKSTEP_SCR_ADDR);
    /* Need to pass 2 as idx to find correct register addr */
    rstgen_core_reset(phy_addr, idx);
}

static bool boot_saf(uint32_t img_base)
{
    /* base address must be 4 KB aligned */
    ASSERT((img_base & 0xFFF) == 0);
    writel(REMAP_DONE, _ioaddr(RSTGEN_SAF_REMAP_STATUS_REG));
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_handle_t handle;
    ASSERT(handle = hal_scr_create_handle(
                        SCR_SAFETY__L31__remap_cr5_saf_ar_addr_offset_19_0));
    hal_scr_set(handle, img_base >> 12);
    hal_scr_delete_handle(handle);
    ASSERT(handle = hal_scr_create_handle(
                        SCR_SAFETY__L31__remap_cr5_saf_ar_remap_ovrd_en));
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);
    saf_core_reset();
    return true;
}

static bool __boot_ss(sd_cpu_id cpu_id, uint32_t h, uint32_t l)
{
    struct ss_boot_cfg *cfg;

    cfg = get_ss_cfg(cpu_id);

    if (!cfg) {
        dprintf(CRITICAL, "%s failed to get cpu id:%d cfg\n", __func__, cpu_id);
        return false;
    }

    cfg->ss_entry_l = l;
    cfg->ss_entry_h = h;
    return boot_ss(cfg);

}

static bool boot_ss_by_id(sd_cpu_id cpu_id, uint64_t addr)
{
    uint32_t h = (uint32_t)(addr >> 30);
    uint32_t l = ((uint32_t)addr & BIT_MASK(30)) >> 2;

    if (cpu_id == CPU_ID_MP) {
        h = 0x1;
        l = ((uint32_t)addr) >> 12;
    }

    return __boot_ss(cpu_id, h, l);
}

static void setup_pll(uint32_t resid)
{
    pll_handle_t pll;

    pll =  hal_pll_create_handle(resid);
    ASSERT(pll != (pll_handle_t)0);

    hal_pll_config(pll, NULL);
    hal_pll_delete_handle(pll);
}

static bool boot_ss(struct ss_boot_cfg *cfg)
{
    bool ret = true;
    uint32_t i = 0;
    void *clk_handle = NULL;
    void *rst_handle = NULL;
    scr_handle_t entry_high;
    scr_handle_t entry_low;

    ASSERT(cfg != NULL);

    ret = hal_rstgen_creat_handle(&rst_handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);

    ret = hal_clock_creat_handle(&clk_handle);
    ASSERT(ret);

    hal_rstgen_init(rst_handle);

    /* disable iso */
    for (i = 0; i < cfg->ss_iso_rst.num; i++) {
        dprintf(INFO, "%s %d  iso:0x%0x\n", __func__, __LINE__,
                cfg->ss_iso_rst.res_id[i]);
        ret = hal_rstgen_iso_disable(rst_handle, cfg->ss_iso_rst.res_id[i]);
        ASSERT(ret);
    }

    /* ss module reset */
    for (i = 0; i < cfg->ss_pre_module_rst.num; i++) {
        dprintf(INFO, "%s %d  pre rst:0x%0x\n", __func__, __LINE__,
                cfg->ss_pre_module_rst.res_id[i]);
        ret = hal_rstgen_module_reset(rst_handle, cfg->ss_pre_module_rst.res_id[i]);
        ASSERT(ret);
    }

    /* some subsystem needs core clock pre set  */
    for (i = 0; i < cfg->ss_pre_core_clk.num; i++) {
        dprintf(INFO, "%s %d  pre core clock:0x%0x\n", __func__, __LINE__,
                cfg->ss_pre_core_clk.clk[i].res_id);
        ret = hal_clock_coreclk_set(clk_handle, cfg->ss_pre_core_clk.clk[i].res_id,
                                    &(cfg->ss_pre_core_clk.clk[i].clk));
        ASSERT(ret);
    }

    /* some subsystem needs uuu pre set  */
    for (i = 0; i < cfg->ss_pre_uuu_clk.num; i++) {
        dprintf(INFO, "%s %d  pre uuu :0x%0x\n", __func__, __LINE__,
                cfg->ss_pre_uuu_clk.clk[i].res_id);
        ret = hal_clock_uuuclk_set(clk_handle, cfg->ss_pre_uuu_clk.clk[i].res_id,
                                   &(cfg->ss_pre_uuu_clk.clk[i].clk));
        ASSERT(ret);
    }

    /* clock gating disable */
    for (i = 0; i < cfg->ss_pre_gating.num; i++) {
        dprintf(INFO, "%s %d  clock enable :0x%0x\n", __func__, __LINE__,
                cfg->ss_pre_gating.res_id[i]);
        ret = hal_clock_enable(clk_handle, cfg->ss_pre_gating.res_id[i]);
        ASSERT(ret);
    }

    /* setup pll config */
    for (i = 0; i < cfg->ss_pll.num; i++) {
        dprintf(INFO, "%s %d  pll:0x%0x\n", __func__, __LINE__,
                cfg->ss_pll.res_id[i]);
        setup_pll(cfg->ss_pll.res_id[i]);
    }

    /* set uuu clock for higher freq
     * which may be from pll
     * */
    for (i = 0; i < cfg->ss_post_uuu_clk.num; i++) {
        dprintf(INFO, "%s %d  uuu:0x%0x\n", __func__, __LINE__,
                cfg->ss_post_uuu_clk.clk[i].res_id);
        ret = hal_clock_uuuclk_set(clk_handle, cfg->ss_post_uuu_clk.clk[i].res_id,
                                   &(cfg->ss_post_uuu_clk.clk[i].clk));
        ASSERT(ret);
    }

    /* disable each core clock gate for smp */
    for (i = 0; i < cfg->ss_post_core_clk_gating.num; i++) {
        dprintf(INFO, "%s %d  core clock enable:0x%0x\n", __func__, __LINE__,
                cfg->ss_post_core_clk_gating.res_id[i]);
        ret = hal_clock_enable(clk_handle, cfg->ss_post_core_clk_gating.res_id[i]);
        ASSERT(ret);
    }

    entry_low  = hal_scr_create_handle(cfg->ss_entry_signal_low);
    entry_high = hal_scr_create_handle(cfg->ss_entry_signal_high);

    if (entry_low != (scr_handle_t)0) {
        hal_scr_get(entry_low);
        ret = hal_scr_set(entry_low, cfg->ss_entry_l);
        ASSERT(ret);
        hal_scr_delete_handle(entry_low);
    }

    if (entry_high != (scr_handle_t)0) {
        hal_scr_get(entry_high);
        ret = hal_scr_set(entry_high, cfg->ss_entry_h);
        ASSERT(ret);
        hal_scr_delete_handle(entry_high);
    }

    for (i = 0; i < cfg->ss_post_module_rst.num; i++) {
        dprintf(INFO, "%s %d  rst:0x%0x\n", __func__, __LINE__,
                cfg->ss_post_module_rst.res_id[i]);
        ret = hal_rstgen_module_reset(rst_handle, cfg->ss_post_module_rst.res_id[i]);
        ASSERT(ret);
    }

    for (i = 0; i < cfg->ss_post_core_rst.num; i++) {
        dprintf(INFO, "%s %d  core rst:0x%0x\n", __func__, __LINE__,
                cfg->ss_post_core_rst.res_id[i]);
        ret = hal_rstgen_core_reset(rst_handle, cfg->ss_post_core_rst.res_id[i]);
        ASSERT(ret);
    }

    hal_clock_release_handle(clk_handle);
    hal_rstgen_release_handle(rst_handle);

    dprintf(INFO, "%s boot ss done\n", __func__);
    return true;
}

