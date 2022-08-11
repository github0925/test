//*****************************************************************************
//
// clkgen_drv_test.c - Driver for the clkgen drv test Module.
//
// Copyright (c) 2019 SemiDrive Incorporated.  All rights reserved.
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
#include "clkgen_drv.h"
#include "clkgen_drv_test.h"

#define LOCAL_TRACE 0
//*****************************************************************************
//
//! clkgen_clkgen_dump_all_reg_for_test
//!
//! \param ulBase is the base address of the clkgen module.
//!
//! This function is dump all register value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_dump_reg_for_test(vaddr_t base,
                              clkgen_debug_module_type module_type, uint32_t slice_idx,
                              uint16_t gating_idx)
{
    bool ret = false;
    uint32_t reg_value = 0;
    vaddr_t slice_base_addr = 0;
    vaddr_t gating_base_addr = 0;
    vaddr_t gasket_slice_base_addr = 0;
    vaddr_t mon_ctl_base_addr = 0;
    vaddr_t ip_slice_mon_base_addr = 0;
    vaddr_t bus_slice_mon_base_addr = 0;
    vaddr_t core_slice_mon_base_addr = 0;
    vaddr_t mon_max_freq_base_addr = 0;
    vaddr_t mon_avg_freq_base_addr = 0;
    vaddr_t mon_min_freq_base_addr = 0;
    vaddr_t mon_max_duty_base_addr = 0;
    vaddr_t mon_min_duty_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "base:0x%llx;module_type:%d;slice_idx:%d\n", (uint64_t)base, module_type,
                  slice_idx);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "#######################################\n");
    gating_base_addr = base + CLKGEN_LP_GATING_EN_OFF(gating_idx);
    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    ip_slice_mon_base_addr = base + CLKGEN_IP_SLICE_MON_CTL_OFF;
    bus_slice_mon_base_addr = base + CLKGEN_BUS_SLICE_MON_CTL_OFF;
    core_slice_mon_base_addr = base + CLKGEN_CORE_SLICE_MON_CTL_OFF;
    mon_max_freq_base_addr = base + CLKGEN_MON_MAX_FREQ_OFF;
    mon_avg_freq_base_addr = base + CLKGEN_MON_AVE_FREQ_OFF;
    mon_min_freq_base_addr = base + CLKGEN_MON_MIN_FREQ_OFF;
    mon_max_duty_base_addr = base + CLKGEN_MON_MAX_DUTY_OFF;
    mon_min_duty_base_addr = base + CLKGEN_MON_MIN_DUTY_OFF;

    if (module_type == debug_module_ip) {
        slice_base_addr = base + CLKGEN_IP_SLICE_CTL_OFF(slice_idx);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "ip slice dump_all_reg:\n");
    }
    else if (module_type == debug_module_bus) {
        slice_base_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(slice_idx);
        gasket_slice_base_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "module slice dump_all_reg:\n");
    }
    else if (module_type == debug_module_core) {
        slice_base_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(slice_idx);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "core slice dump_all_reg:\n");
    }
    else if (module_type == debug_module_uuu) {
        slice_base_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(slice_idx);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "uuu slice dump_all_reg:\n");
    }
    else {
        LTRACEF("base paramenter error \n");
        return ret;
    }

    if (slice_base_addr != 0) {
        reg_value = readl(slice_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "reg_value:0x%x\n", reg_value);
    }

    if (gasket_slice_base_addr != 0) {
        reg_value = readl(gasket_slice_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "gasket reg_value:0x%x\n",
                      reg_value);
    }

    if (gating_base_addr != 0) {
        reg_value = readl(gating_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "gating reg_value:0x%x\n",
                      reg_value);
    }

    if (mon_ctl_base_addr != 0) {
        reg_value = readl(mon_ctl_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "mon_ctl_base_addr reg_value:0x%x\n", reg_value);
    }

    if (ip_slice_mon_base_addr != 0) {
        reg_value = readl(ip_slice_mon_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "ip_slice_mon_base_addr reg_value:0x%x\n", reg_value);
    }

    if (bus_slice_mon_base_addr != 0) {
        reg_value = readl(bus_slice_mon_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "bus_slice_mon_base_addr reg_value:0x%x\n", reg_value);
    }

    if (core_slice_mon_base_addr != 0) {
        reg_value = readl(core_slice_mon_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "core_slice_mon_base_addr reg_value:0x%x\n", reg_value);
    }

    if (mon_max_freq_base_addr != 0) {
        reg_value = readl(mon_max_freq_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "mon_max_freq_base_addr reg_value:0x%x\n", reg_value);
    }

    if (mon_avg_freq_base_addr != 0) {
        reg_value = readl(mon_avg_freq_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "mon_avg_freq_base_addr reg_value:0x%x\n", reg_value);
    }

    if (mon_min_freq_base_addr != 0) {
        reg_value = readl(mon_min_freq_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "mon_min_freq_base_addr reg_value:0x%x\n", reg_value);
    }

    if (mon_max_duty_base_addr != 0) {
        reg_value = readl(mon_max_duty_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "mon_max_duty_base_addr:0x%x\n",
                      reg_value);
    }

    if (mon_min_duty_base_addr != 0) {
        reg_value = readl(mon_min_duty_base_addr);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "mon_min_duty_base_addr:0x%x\n",
                      reg_value);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "#######################################\n");
    return true;
}

//*****************************************************************************
//
//! clkgen_ipslice_readonlyreg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_ipslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx)
{
    bool ret = true;
    uint32_t reg_read = 0;
    vaddr_t ip_slice_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_ipslice_readonlyreg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    ip_slice_base_addr = base + CLKGEN_IP_SLICE_CTL_OFF(slice_idx);
    reg_read = readl(ip_slice_base_addr);

    if ((reg_read & (CLKGEN_IP_SLICE_CTL_PRE_BUSY_MASK |
                     CLKGEN_IP_SLICE_CTL_POST_BUSY_MASK)) != 0x0) {
        LTRACEF("clkgen ip slice ctrl readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_busslice_readonlyreg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx bus slice index
//! This function is test clkgen read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_busslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx)
{
    bool ret = true;
    uint32_t reg_read = 0;
    vaddr_t bus_slice_base_addr = 0;
    vaddr_t bus_gasket_slice_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_busslice_readonlyreg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    bus_slice_base_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(slice_idx);
    bus_gasket_slice_base_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);

    reg_read = readl(bus_slice_base_addr);

    if ((reg_read & (CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_MASK |
                     CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_MASK |
                     CLKGEN_BUS_SLICE_CTL_POST_BUSY_MASK)) != 0x0) {
        LTRACEF("clkgen bus slice ctrl readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(bus_gasket_slice_base_addr);

    if ((reg_read & (CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_MASK |
                     CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY_MASK |
                     CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY_MASK |
                     CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_MASK)) != 0x0) {
        LTRACEF("clkgen bus gasket slice ctrl readonly value is error:0x%x\n",
                reg_read);
        ret = false;
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_coreslice_readonlyreg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx core slice index
//! This function is test clkgen read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_coreslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx)
{
    bool ret = true;
    uint32_t reg_read = 0;
    vaddr_t core_slice_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_coreslice_readonlyreg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    core_slice_base_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(slice_idx);

    reg_read = readl(core_slice_base_addr);

    if ((reg_read & CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_MASK) != 0x0) {
        LTRACEF("clkgen core slice ctrl readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    return ret;
}

//*****************************************************************************
//
//! clkgen_other_readonlyreg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! This function is test clkgen read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_other_readonlyreg_check_test(vaddr_t base)
{
    bool ret = true;
    uint32_t reg_read = 0;
    vaddr_t mon_ctl_base_addr = 0;
    vaddr_t mon_max_freq_base_addr = 0;
    vaddr_t mon_avg_freq_base_addr = 0;
    vaddr_t mon_min_freq_base_addr = 0;
    vaddr_t mon_max_duty_base_addr = 0;
    vaddr_t mon_min_duty_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_other_readonlyreg_check_test base:0x%llx\n", (uint64_t)base);

    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    mon_max_freq_base_addr = base + CLKGEN_MON_MAX_FREQ_OFF;
    mon_avg_freq_base_addr = base + CLKGEN_MON_AVE_FREQ_OFF;
    mon_min_freq_base_addr = base + CLKGEN_MON_MIN_FREQ_OFF;
    mon_max_duty_base_addr = base + CLKGEN_MON_MAX_DUTY_OFF;
    mon_min_duty_base_addr = base + CLKGEN_MON_MIN_DUTY_OFF;

    reg_read = readl(mon_ctl_base_addr);

    /*bit CLKGEN_MON_CTL_MON_CLK_DIS_STA will be reset to 0 in reset mode, after released the value will sync to CLKGEN_MON_CTL_MON_CLK,
     so we don't check it at here*/
    if ((reg_read & (CLKGEN_MON_CTL_MON_DIV_BUSY_MASK |
                     CLKGEN_MON_CTL_FREQ_RDY_STA_MASK)) != 0x0) {
        LTRACEF("mon_ctl_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(mon_max_freq_base_addr);

    if ((reg_read & 0xffff) != 0x0) {
        LTRACEF("mon_max_freq_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(mon_avg_freq_base_addr);

    if ((reg_read & 0xffff) != 0x0) {
        LTRACEF("mon_avg_freq_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(mon_min_freq_base_addr);

    if ((reg_read & 0xffff) != 0x0) {
        LTRACEF("mon_min_freq_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(mon_max_duty_base_addr);

    if ((reg_read & 0xffffffff) != 0x0) {
        LTRACEF("mon_max_duty_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    reg_read = readl(mon_min_duty_base_addr);

    if ((reg_read & 0xffffffff) != 0xffffffff) {
        LTRACEF("mon_min_duty_base_addr readonly value is error:0x%x\n", reg_read);
        ret = false;
    }

    return ret;
}

//*****************************************************************************
//! clkgen_ipslice_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_ipslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx)
{
    bool ret = false;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t ip_slice_base_addr = 0;
    //uint32_t timeout =100;
    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_ipslice_rw_reg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    ip_slice_base_addr = base + CLKGEN_IP_SLICE_CTL_OFF(slice_idx);
    reg_read_old = readl(ip_slice_base_addr);

    if ((reg_read_old & CLKGEN_IP_SLICE_CTL_CG_EN_MASK) !=
            CLKGEN_IP_SLICE_CTL_CG_EN_MASK) {
        LTRACEF("clkgen ip slice ctrl rw value is error:0x%x\n", reg_read);
        return false;
    }

    reg_write = 0xfffffffe;
    writel(reg_write, ip_slice_base_addr);
#if 0

    while (((reg_read = readl(ip_slice_base_addr)) &
            (CLKGEN_IP_SLICE_CTL_PRE_BUSY_MASK | CLKGEN_IP_SLICE_CTL_POST_BUSY_MASK))
            != 0
            && --timeout) spin(1);

    if (timeout == 0) {
        LTRACEF("timeout\n");
        clkgen_dump_reg_for_test(base, debug_module_ip, slice_idx, 0);
        goto fail;
    }

#else
    reg_read = readl(ip_slice_base_addr);
#endif

    if ((reg_read & 0x1fffffff) != 0x1000fc7e) {
        LTRACEF("ipslice test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, slice_idx, 0);
        goto fail;
    }

    ret = true;
fail:
    writel(reg_read_old, ip_slice_base_addr);

    return ret;
}
//*****************************************************************************
//! clkgen_busslice_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_busslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx)
{
    bool ret = false;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t bus_slice_base_addr = 0;
    vaddr_t bus_gasket_slice_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_busslice_rw_reg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    bus_slice_base_addr = base + CLKGEN_BUS_SLICE_CTL_OFF(slice_idx);
    bus_gasket_slice_base_addr = base + CLKGEN_BUS_SLICE_GASKET_OFF(slice_idx);

    reg_read_old = readl(bus_slice_base_addr);

    if ((reg_read_old & CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK) !=
            CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK) {
        LTRACEF("clkgen bus slice ctrl rw value is error:0x%x\n", reg_read);
        //return false;
    }

    reg_write = 0xfffffffe;
    writel(reg_write, bus_slice_base_addr);
    reg_read = readl(bus_slice_base_addr);
    spin(1);

    if ((reg_read & 0x1fffffff) != 0x187ffe7e) {
        LTRACEF("busslice test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_bus, slice_idx, 0);
        goto fail;
    }

    ret = true;
fail:
    writel(reg_read_old, bus_slice_base_addr);

    if (!ret) return ret;

    ret = false;
    reg_read_old = readl(bus_gasket_slice_base_addr);
#if 0

    if ((reg_read_old & 0xfff) != 0xfff) {
        LTRACEF("clkgen bus gasket slice ctrl rw value is error:0x%x\n",
                reg_read_old);
        return false;
    }

#endif
    reg_write = 0xfffff000;
    writel(reg_write, bus_gasket_slice_base_addr);
    reg_read = readl(bus_gasket_slice_base_addr);
    spin(1);

    if ((reg_read & 0xffffff) != 0x000000) {
        LTRACEF("busslice test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_bus, slice_idx, 0);
        goto fail_gask;
    }

    ret = true;
fail_gask:
    writel(reg_read_old, bus_gasket_slice_base_addr);

    return ret;
}
//*****************************************************************************
//! clkgen_coreslice_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_coreslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx)
{
    bool ret = true;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t core_slice_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_coreslice_rw_reg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, slice_idx);

    core_slice_base_addr = base + CLKGEN_CORE_SLICE_CTL_OFF(slice_idx);
    reg_read_old = readl(core_slice_base_addr);

    if ((reg_read_old & CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK) !=
            CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK) {
        LTRACEF("clkgen core slice ctrl rw value is error:0x%x\n", reg_read);
        return false;
    }

    reg_write = 0xfffffffe;
    writel(reg_write, core_slice_base_addr);
    reg_read = readl(core_slice_base_addr);
    spin(1);

    if ((reg_read & 0x7ffffffe) != 0x180ffe0e) {
        LTRACEF("core slice test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_core, slice_idx, 0);
        ret = false;
    }

    writel(reg_read_old, core_slice_base_addr);

    return ret;
}
//*****************************************************************************
//! clkgen_lpgating_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_lpgating_rw_reg_check_test(vaddr_t base, uint32_t gating_idx)
{
    bool ret = true;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t gating_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_lpgating_rw_reg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, gating_idx);

    gating_base_addr = base + CLKGEN_LP_GATING_EN_OFF(gating_idx);
    reg_read_old = readl(gating_base_addr);
#if 0

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen lp gating ctrl rw value is error:0x%x\n", reg_read_old);
        return false;
    }

#endif

    reg_write = 0x7fffffff;
    writel(reg_write, gating_base_addr);
    reg_read = readl(gating_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0x00000003) {
        LTRACEF("lp gating test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, gating_idx);
        ret = false;
    }

    writel(reg_read_old, gating_base_addr);

    return ret;
}
//*****************************************************************************
//! clkgen_uuuslice_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_uuuslice_rw_reg_check_test(vaddr_t base,
                                       uint16_t uuu_clock_wrapper_idx)
{
    bool ret = true;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t uuu_wrapper_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_lpgating_rw_reg_check_test base:0x%llx,slice_idx:%d\n",
                  (uint64_t)base, uuu_clock_wrapper_idx);

    uuu_wrapper_base_addr = base + CLKGEN_UUU_SLICE_WRAPPER_OFF(
                                uuu_clock_wrapper_idx);
    reg_read_old = readl(uuu_wrapper_base_addr);
#if 0

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen uuuslice ctrl rw value is error:0x%x\n", reg_read_old);
        return false;
    }

#endif
    reg_write = 0xffffffff;
    writel(reg_write, uuu_wrapper_base_addr);
    reg_read = readl(uuu_wrapper_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xf81fffff) {
        LTRACEF("uuuslice test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_uuu, uuu_clock_wrapper_idx, 0);
        ret = false;
    }

    writel(reg_read_old, uuu_wrapper_base_addr);

    return ret;
}

//*****************************************************************************
//! clkgen_other_rw_reg_check_test
//!
//! \param ulBase is the base address of the clkgen module.
//! \slice_idx ip slice index
//! This function is test clkgen rw register is ok
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool clkgen_other_rw_reg_check_test(vaddr_t base)
{
    bool ret = true;
    uint32_t reg_read_old = 0;
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;
    vaddr_t debug_crtl_base_addr = 0;
    vaddr_t debug_ip_base_addr = 0;
    vaddr_t debug_bus_base_addr = 0;
    vaddr_t debug_core_base_addr = 0;
    vaddr_t mon_ctl_base_addr = 0;
    vaddr_t ip_slice_mon_base_addr = 0;
    vaddr_t bus_slice_mon_base_addr = 0;

    // Check the arguments.
    CLKGEN_ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "clkgen_other_rw_reg_check_test base:0x%llx\n", (uint64_t)base);

    debug_crtl_base_addr = base + CLKGEN_DBG_CTL_OFF;
    debug_ip_base_addr = base + CLKGEN_IP_SLICE_DBG_CTL_OFF;
    debug_bus_base_addr = base + CLKGEN_BUS_SLICE_DBG_CTL_OFF;
    debug_core_base_addr = base + CLKGEN_CORE_SLICE_DBG_CTL_OFF;
    mon_ctl_base_addr = base + CLKGEN_MON_CTL_OFF;
    ip_slice_mon_base_addr = base + CLKGEN_IP_SLICE_MON_CTL_OFF;
    bus_slice_mon_base_addr = base + CLKGEN_BUS_SLICE_MON_CTL_OFF;

    /*<<begin debug_crtl_base_addr*/
    reg_read_old = readl(debug_crtl_base_addr);
#if 0

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen debug_crtl_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

#endif
    reg_write = 0xffffffff;
    writel(reg_write, debug_crtl_base_addr);
    reg_read = readl(debug_crtl_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xe000ffff) {
        LTRACEF("debug_crtl_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, debug_crtl_base_addr);

    if (!ret) return false;

    /*end debug_crtl_base_addr>>*/

    /*<<begin debug_ip_base_addr*/
    reg_read_old = readl(debug_ip_base_addr);

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen debug_ip_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0xffffffff;
    writel(reg_write, debug_ip_base_addr);
    reg_read = readl(debug_ip_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xffff) {
        LTRACEF("debug_ip_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, debug_ip_base_addr);

    if (!ret) return false;

    /*end debug_ip_base_addr>>*/

    /*<<begin debug_bus_base_addr*/
    reg_read_old = readl(debug_bus_base_addr);

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen debug_bus_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0xffffffff;
    writel(reg_write, debug_bus_base_addr);
    reg_read = readl(debug_bus_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xffff) {
        LTRACEF("debug_bus_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, debug_bus_base_addr);

    if (!ret) return false;

    /*end debug_bus_base_addr>>*/

    /*<<begin debug_core_base_addr*/
    reg_read_old = readl(debug_core_base_addr);

    if (reg_read_old != 0x00) {
        LTRACEF("clkgen debug_core_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0xffffffff;
    writel(reg_write, debug_core_base_addr);
    reg_read = readl(debug_core_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xffff) {
        LTRACEF("debug_core_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, debug_core_base_addr);

    if (!ret) return false;

    /*end debug_bus_base_addr>>*/

    /*<<begin mon_ctl_base_addr*/
    reg_read_old = readl(mon_ctl_base_addr);

    if ((reg_read_old & CLKGEN_MON_CTL_MON_CLK_DIS_MASK) !=
            CLKGEN_MON_CTL_MON_CLK_DIS_MASK) {
        LTRACEF("clkgen mon_ctl_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0x7fffffff;
    writel(reg_write, mon_ctl_base_addr);
    reg_read = readl(mon_ctl_base_addr);
    spin(1);

    if ((reg_read & 0xffff) != 0xffff) {
        LTRACEF("mon_ctl_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, mon_ctl_base_addr);

    if (!ret) return false;

    /*end mon_ctl_base_addr>>*/

    /*<<begin ip_slice_mon_base_addr*/
    reg_read_old = readl(ip_slice_mon_base_addr);

    if (reg_read_old  != 0x00) {
        LTRACEF("clkgen ip_slice_mon_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0xffffffff;
    writel(reg_write, ip_slice_mon_base_addr);
    reg_read = readl(ip_slice_mon_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xffff) {
        LTRACEF("ip_slice_mon_base_addr test reg write is error:0x%x\n", reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, ip_slice_mon_base_addr);

    if (!ret) return false;

    /*end mon_ctl_base_addr>>*/

    /*<<begin bus_slice_mon_base_addr*/
    reg_read_old = readl(bus_slice_mon_base_addr);

    if (reg_read_old  != 0x00) {
        LTRACEF("clkgen bus_slice_mon_base_addr ctrl rw value is error:0x%x\n",
                reg_read_old);
        ret = false;
    }

    reg_write = 0xffffffff;
    writel(reg_write, bus_slice_mon_base_addr);
    reg_read = readl(bus_slice_mon_base_addr);
    spin(1);

    if ((reg_read & 0xffffffff) != 0xffff) {
        LTRACEF("bus_slice_mon_base_addr test reg write is error:0x%x\n",
                reg_read);
        clkgen_dump_reg_for_test(base, debug_module_ip, 0, 0);
        ret = false;
    }

    writel(reg_read_old, bus_slice_mon_base_addr);
    /*end mon_ctl_base_addr>>*/


    return ret;
}
