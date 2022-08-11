//*****************************************************************************
//
// rstgen_program.c - Driver for the rstgen Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup rstgen_program api
//! @{
//
//*****************************************************************************
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <reg.h>
#include <trace.h>

#include "__regs_base.h"
#include "target_res.h"
#include "rstgen_drv.h"

#define LOCAL_TRACE 0

//*****************************************************************************
//
//! rstgen_get_default_config .
//! \global_rst_maks is Pointer to rstgen config structure
//! This function initializes the rstgen configure structure to default value.
//!
//! \return
//
//*****************************************************************************
void rstgen_get_default_config(uint32_t *global_rst_maks)
{
    if (!global_rst_maks) {
        LTRACEF("config paramenter error !!\n");
        return;
    }

    *global_rst_maks = RSTGEN_GLB_RST_SELF_RST_EN(1)
                       | RSTGEN_GLB_RST_SEM_RST_EN(1)
                       | RSTGEN_GLB_RST_DBG_RST_EN(1);

    LTRACEF("rstgen_get_default_config global_rst_maks:0x%x\n", *global_rst_maks);
}

//*****************************************************************************
//
//! Initializes the rstgen .
//!
//! \param base   rstgen peripheral base address
//! \global_rst_maks The configuration of rstgen
//!
//!This function initializes the rstgen. When called, the rstgen global reset
// according to the configuration.
//!
//! \return Returns \b true if the wdg timer is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool rstgen_init(vaddr_t base, const uint32_t global_rst_maks)
{
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    uint32_t rstgen_global_cfg = RSTGEN_GLB_RST_SELF_RST_EN(global_rst_maks)
                                 | RSTGEN_GLB_RST_SEM_RST_EN(global_rst_maks)
                                 | RSTGEN_GLB_RST_DBG_RST_EN(global_rst_maks)
                                 | RSTGEN_GLB_RST_WDG1_RST_EN(global_rst_maks);

    //clear all global register
    reg_write = 0x00000000;
    writel(reg_write, base);

    //unlock
    reg_read = readl(base);
    reg_write = reg_read & (~RSTGEN_GLB_RST_LOCK_MASK);
    writel(reg_write, base);

    reg_read = readl(base);
    reg_write = reg_read | rstgen_global_cfg;
    writel(reg_write, base);

    return true;
}

//*****************************************************************************
// Enable global reset inputs.
//
// mask is enable type:
//  bit0: self software reset enable
//  bit1: sem reset enable
//  bit2: dbg reset enable
//  bit[10:3]: watchdog reset enable
//      safe_ss:    [3] for wdt1
//      sec_ss:     [8:3] for wdt 7~2
//*****************************************************************************
bool rstgen_global_rst_enable(vaddr_t base, uint32_t mask)
{
    vaddr_t global_rst_en = base + GLB_RST_EN;

    LTRACEF("rstgen_global_rst_enable mask:0x%x\n", mask);

    /* Unlock global reset control. */
    RMWREG32(global_rst_en, RSTGEN_GLB_RST_LOCK_SHIFT, 1, 0);

    /* Update GLB_RST_EN bits. */
    uint32_t val = readl(global_rst_en);
    val |= mask;
    writel(val, global_rst_en);

    return true;
}

//*****************************************************************************
// Disable global reset inputs.
//
// mask is enable type:
//  bit0: self software reset enable
//  bit1: sem reset enable
//  bit2: dbg reset enable
//  bit[10:3]: watchdog reset enable
//      safe_ss:    [3] for wdt1
//      sec_ss:     [8:3] for wdt 7~2
//*****************************************************************************
bool rstgen_global_rst_disable(vaddr_t base, uint32_t mask)
{
    vaddr_t global_rst_en = base + GLB_RST_EN;

    /* Unlock global reset control. */
    RMWREG32(global_rst_en, RSTGEN_GLB_RST_LOCK_SHIFT, 1, 0);

    /* Update GLB_RST_EN bits. */
    uint32_t val = readl(global_rst_en);
    val &= ~mask;
    writel(val, global_rst_en);

    return true;
}

//*****************************************************************************
//
// Reset current domain.
//
// RSTGEN_SAF: saf.sef_rst_trig reset safety and AP domain
// RSTGEN_RTC: rtc.self_rst_trig reset the RTC domain
// RSTGEN_SEC: ap.self_rst_trig reset the AP domain (sec & ap)
//
//*****************************************************************************
bool rstgen_sw_self_rst(vaddr_t base, bool release)
{
    vaddr_t sw_self_rst = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_SELF_RST_OFF);

    /* Unlock self reset register. */
    RMWREG32(sw_self_rst, RSTGEN_SELF_RST_SW_GLB_RST_LOCK_SHIFT, 1, 0);

    /* Configure rst bit. */
    RMWREG32(sw_self_rst, RSTGEN_SELF_RST_SW_GLB_RST_SHIFT, 1, (uint32_t)release);

    return true;
}

//*****************************************************************************
//
// Reset other domains.
//
// RSTGEN_SAF: saf.oth_rst_b reset the AP domain (sec & ap)
// RSTGEN_RTC and RSTGEN_SEC: oth reset has no effect
//
//*****************************************************************************
bool rstgen_sw_oth_rst(vaddr_t base, bool release)
{
    vaddr_t sw_oth_rst = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_OTH_RST_OFF);

    /* Unlock the register. */
    RMWREG32(sw_oth_rst, RSTGEN_OTH_RST_SW_GLB_RST_LOCK_SHIFT, 1, 0);

    /* Configure rst bit. */
    RMWREG32(sw_oth_rst, RSTGEN_OTH_RST_SW_GLB_RST_SHIFT, 1, (uint32_t)release);

    return true;
}

//*****************************************************************************
//
//! rstgen_get_rst_sta .
//!\param base   rstgen peripheral base address
//! This function is used to get rstgen reset status mask
//!
//! \return reset status
//
//*****************************************************************************
uint32_t rstgen_get_rst_sta(vaddr_t base)
{
    vaddr_t rst_sta_addr;
    uint32_t rstgen_rst_sta = 0;

    rst_sta_addr = base + SOC_RSTGEN_REG_MAP(RSTGEN_RST_STA_OFF);

    rstgen_rst_sta = readl(rst_sta_addr);
    LTRACEF("rstgen_get_rst_sta rst_sta:0x%x\n", rstgen_rst_sta);
    return rstgen_rst_sta;
}

//*****************************************************************************
//
//! rstgen_iso_enable .
//!\param base   rstgen peripheral base address
//!\iso_idx isolate index
//! This function is used for isolation enable
//!
//! \return true is success else return false
//
//*****************************************************************************
bool rstgen_iso_enable(vaddr_t base, uint32_t iso_idx)
{
    uint32_t iso_slice_idx = 0;
    vaddr_t iso_addr;

    iso_slice_idx = iso_idx - (ISO_EN / 4);

    iso_addr = base + SOC_RSTGEN_REG_MAP(ISO_EN + 0x04 * iso_slice_idx);
    LTRACEF("rstgen_iso_enable is ok iso_slice_idx:%d\n", iso_slice_idx);
    writel(~RSTGEN_ISO_EN_B_MASK, iso_addr);

    return true;
}

//*****************************************************************************
//
//! rstgen_iso_status .
//!\param base   rstgen peripheral base address
//!\iso_idx isolate index
//! This function is used for isolation status indication
//!
//! \0 isloation, 1 not isloation.
//
//*****************************************************************************
uint32_t rstgen_iso_status(vaddr_t base, uint32_t iso_idx)
{
    uint32_t iso_slice_idx = 0;
    vaddr_t iso_addr;

    iso_slice_idx = iso_idx - (ISO_EN / 4);

    iso_addr = base + SOC_RSTGEN_REG_MAP(ISO_EN + 0x04 * iso_slice_idx);
    LTRACEF("rstgen_iso_status is ok iso_slice_idx:%d\n", iso_slice_idx);
    return readl(iso_addr) & RSTGEN_ISO_EN_B_MASK;
}

//*****************************************************************************
//
//! rstgen_iso_disable .
//!\param base   rstgen peripheral base address
//!\iso_idx isolate index
//! This function is used for isolation disable
//!
//! \return true is success else return false
//
//*****************************************************************************
bool rstgen_iso_disable(vaddr_t base, uint32_t iso_idx)
{
    uint32_t iso_slice_idx = 0;
    vaddr_t iso_addr;

    iso_slice_idx = iso_idx - (ISO_EN / 4);

    iso_addr = base + SOC_RSTGEN_REG_MAP(ISO_EN + 0x04 * iso_slice_idx);
    LTRACEF("rstgen_iso_disable is ok iso_slice_idx:%d\n", iso_slice_idx);
    writel(RSTGEN_ISO_EN_B_MASK, iso_addr);

    return true;
}

//*****************************************************************************
//
//! rstgen_core_rst_enable .
//!\param base   rstgen peripheral base address
//!\core_idx reset core index
//!\mask core reset enable mask for core reset test
//! This function is used for core reset enable
//!
//! \return true is success else return false
//
//*****************************************************************************
bool rstgen_core_rst_enable(vaddr_t base, uint32_t core_idx,
                            uint32_t mask)
{
    uint32_t core_slice = 0;
    vaddr_t core_rst_en;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;

    core_slice = (core_idx / 2) - (CORE_RST_EN / 4);
    LTRACEF("rstgen_core_rst_enable is ok core_slice:%d\n", core_slice);
    core_rst_en = base + SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(
            core_slice));
    reg_read = readl(core_rst_en);
    reg_write = reg_read | mask;
    writel(reg_write, core_rst_en);

    return true;
}

//*****************************************************************************
//
//! rstgen_iso_disable .
//!\param base   rstgen peripheral base address
//!\core_idx reset core index
//!\mask core reset enable mask for core reset test
//! This function is used for core reset disable
//!
//! \return true is success else return false
//
//*****************************************************************************
bool rstgen_core_rst_disable(vaddr_t base, uint32_t core_idx,
                             uint32_t mask)
{
    vaddr_t core_en_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    uint32_t core_slice;

    core_slice = (core_idx / 2) - (CORE_RST_EN / 4);
    LTRACEF("rstgen_core_rst_disable is ok core_slice:%d\n", core_slice);
    core_en_rst_addr = base + SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(
                           core_idx));

    reg_read = readl(core_en_rst_addr);
    reg_write = reg_read & (~mask);
    writel(reg_write, core_en_rst_addr);
    return true;
}


//*****************************************************************************
//
// Reset core processor.
//
// auto clear reset: SW write 1 to AUTO_CLR bit to reset the core.
// RSTGEN assert and release the reset signal automatically.
//
// This is for reseting a core after running normally scenario
//
// NOTE: HW doesn't check bus status before resetting the core. SW must make
// sure there's no active bus transaction on the core.
//
//*****************************************************************************
bool rstgen_core_reset(vaddr_t base, uint32_t core_idx)
{
    uint32_t core_slice = (core_idx / 2) - (CORE_RST_EN / 4);
    vaddr_t core_rst_en = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_RST_EN_OFF(core_slice));
    vaddr_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_SW_RST_OFF(core_slice));

    /* Check the reset lock bit */
    if ((readl(core_rst_en) & RSTGEN_CORE_RST_RST_LOCK_MASK))
    {
        LTRACEF("core_slice:%d has been locked\n", core_slice);
        return false;
    }

    /* Release STATIC_RST_B. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_STATIC_RST_SHIFT, 1, 1);
    while (!(readl(core_sw_rst) &
                 RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_MASK));

    /* Enable SW core reset. */
    RMWREG32(core_rst_en, RSTGEN_CORE_RST_SW_RST_EN_SHIFT, 1, 1);

    while (!(readl(core_rst_en) & RSTGEN_CORE_RST_SW_RST_EN_STA_MASK));
    /* Trigger auto-clear reset. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_AUTO_CLR_SHIFT, 1, 1);

    /* Polling reset status */
    while (!(readl(core_sw_rst) & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK));

    return true;
}
//*****************************************************************************
//
// Reset core processor.
//
// static reset: SW controls STATIC_RST bit manually. SW write 0
// to STATIC_RST to assert reset, or write 1 to STATIC_RST to release
// the core from reset.
//
// NOTE: HW doesn't check bus status before resetting the core. SW must make
// sure there's no active bus transaction on the core.
//
//*****************************************************************************
bool rstgen_core_ctl(vaddr_t base, uint32_t core_idx,
                               bool release)
{
    uint32_t core_slice = (core_idx / 2) - (CORE_RST_EN / 4);
    vaddr_t core_rst_en = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_RST_EN_OFF(core_slice));
    vaddr_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_SW_RST_OFF(core_slice));

    /* Check the reset lock bit */
    if ((readl(core_rst_en) & RSTGEN_CORE_RST_RST_LOCK_MASK))
    {
        LTRACEF("core_slice:%d has been locked\n", core_slice);
        return false;
    }
    /* Check core SW reset status */
    if (release && (readl(core_sw_rst) & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK))
    {
        LTRACEF("core_slice:%d already in release status\n", core_slice);
        return true;
    }

    if (!release && !(readl(core_sw_rst) & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK))
    {
        LTRACEF("core_slice:%d already in assert status\n", core_slice);
        return true;
    }

    uint32_t sw_rst = release?1:0;
    /* Enable SW core reset. */
    RMWREG32(core_rst_en, RSTGEN_CORE_RST_SW_RST_EN_SHIFT, 1, 1);

    while (!(readl(core_rst_en) & RSTGEN_CORE_RST_SW_RST_EN_STA_MASK));

    /* STATIC_RST_B. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_STATIC_RST_SHIFT, 1, sw_rst);

    /* Wait reset done. */
    while (sw_rst != (readl(core_sw_rst)
                            & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK)>>RSTGEN_CORE_SW_RST_CORE_RST_STA_SHIFT);
    return true;
}

//*****************************************************************************
// Reset IP module.
//
// SW write 0 to MODULE_RST_B_idx[1:1] to assert the reset or withe 1 to
// MODULE_RST_B_idx[1:1] to release the reset.
//
// RSTGEN doesn't support auto-clear IP reset, so SW must assert the reset
// signal first (release = 0), then release te reset signal (release = 1).
// No delay is required, since this function polls reset signal.
//*****************************************************************************
bool rstgen_module_ctl(vaddr_t base, uint32_t module_idx,
                         bool release)
{
    uint32_t module_slice = module_idx - (MODULE_RST / 4);;
    vaddr_t module_rst = base + SOC_RSTGEN_REG_MAP(
                             RSTGEN_MODULE_RST_OFF(module_slice));

    LTRACEF("rstgen_module_reset base:0x%llx module_idx:%d release:%d\n",
            (uint64_t)base, module_slice, release);

    if (readl(module_rst) & RSTGEN_MODULE_RST_LOCK_MASK)
    {
        LTRACEF("module_slice:%d has been locked.\n", module_slice);
        return false;
    }

    if (release && (readl(module_rst) & RSTGEN_MODULE_RST_STA_MASK))
    {
        LTRACEF("module_slice:%d already in release status\n", module_slice);
        return true;
    }
    if (!release && !(readl(module_rst) & RSTGEN_MODULE_RST_STA_MASK))
    {
        LTRACEF("module_slice:%d already in assert status\n", module_slice);
        return true;
    }

    /* Enable module reset. */
    RMWREG32(module_rst, RSTGEN_MODULE_RST_EN_SHIFT, 1, 1);

    while (!(readl(module_rst) & RSTGEN_MODULE_RST_EN_MASK));

    /* Write RST_N bit. 0 = reset, 1 = release. */
    uint32_t rst_n = release?1:0;
    RMWREG32(module_rst, RSTGEN_MODULE_RST_N_SHIFT, 1, rst_n);

    /* Wait module reset done. */
    while (rst_n != (readl(module_rst) & RSTGEN_MODULE_RST_STA_MASK)>>RSTGEN_MODULE_RST_STA_SHIFT);

    return true;
}

//*****************************************************************************
// Query IP module reset status.
//
// MODULE_RST_B_idx [30:30], the reset staus for this module reset.
// return 1, the module reset has be released.
// return 0, the module reset hasn't be released.
//*****************************************************************************
uint32_t rstgen_module_status(vaddr_t base, uint32_t module_idx)
{
    uint32_t module_slice = module_idx - (MODULE_RST / 4);;
    vaddr_t module_rst = base + SOC_RSTGEN_REG_MAP(
                             RSTGEN_MODULE_RST_OFF(module_slice));

    LTRACEF("rstgen_module_reset base:0x%llx module_idx:%d \n",
            (uint64_t)base, module_slice);

    /* RSTGEN_MODULE_RST_STA, 0 = reset, 1 = release. */
    return !!(readl(module_rst) & RSTGEN_MODULE_RST_STA_MASK);
}


//*****************************************************************************
// Query core reset status.
//
// CORE_SW_RST_idx [30:30], the reset staus for this core reset.
// return 1, the core reset has be released.
// return 0, the core reset hasn't be released.
//*****************************************************************************
uint32_t rstgen_core_status(vaddr_t base, uint32_t core_idx)
{
    uint32_t core_slice = (core_idx / 2) - (CORE_RST_EN / 4);
    vaddr_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                RSTGEN_CORE_SW_RST_OFF(core_slice));
    return !!(readl(core_sw_rst) & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK);
}
