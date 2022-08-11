//*****************************************************************************
//
// clkgen_hal.c - Driver for the clkgen hal Module.
//
// Copyright (c) 2019 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include "clkgen_drv.h"
#include "clkgen_hal.h"
#include "res.h"
#include "ckgen_cfg.h"
#include <chip_res.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

#define LOCAL_TRACE 0
/*clkgen global instance*/
static clkgen_instance_t g_ClkgenInstance = {
    .occupied = 0,
};

spin_lock_t clkgen_spin_lock = SPIN_LOCK_INITIAL_VALUE;
spin_lock_saved_state_t spin_lock_states;


/*clkgen driver interface*/
static const clkgen_drv_controller_interface_t s_ClkgenDrvInterface = {
    clkgen_get_default_config,
    clkgen_fsrefclk_sel,
    clkgen_gating_enable,
    clkgen_ip_slice_set,
    clkgen_ip_ctl_get,
    clkgen_ip_ctl_set,
    clkgen_bus_slice_switch,
    clkgen_bus_ctl_get,
    clkgen_bus_ctl_set,
    clkgen_core_slice_switch,
    clkgen_core_ctl_get,
    clkgen_core_ctl_set,
    clkgen_mon_ip_slice,
    clkgen_mon_bus_slice,
    clkgen_mon_core_slice,
    clkgen_uuu_clock_wrapper,
    clkgen_uuu_ctl_get,
    clkgen_uuu_ctl_set,
    clkgen_ipslice_debug_enable,
    clkgen_ipslice_debug_disable,
    clkgen_busslice_debug_enable,
    clkgen_busslice_debug_disable,
    clkgen_coreslice_debug_enable,
    clkgen_coreslice_debug_disable,
    clkgen_uuuslice_debug_enable,
    clkgen_uuuslice_debug_disable,
};

//*****************************************************************************
//
//! hal_clock_get_controller_interface.
//!
//! \param controllerTable is clkgen interface ptr
//!
//! This function get clkgen driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_clock_get_controller_interface(const
        clkgen_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_ClkgenDrvInterface;
}
//*****************************************************************************
//
//! hal_clock_get_instance.
//!
//! \void.
//!
//! This function get clkgen instance.
//!
//! \return clkgen hanle
//
//*****************************************************************************
static clkgen_instance_t *hal_clock_get_instance(void)
{
    //spin_lock_saved_state_t states;
    spin_lock_irqsave(&clkgen_spin_lock, spin_lock_states);

    if (g_ClkgenInstance.occupied != 1) {
        memset(&g_ClkgenInstance, 0, sizeof(clkgen_instance_t));
        /* get clkgen driver API table */
        hal_clock_get_controller_interface(&(g_ClkgenInstance.controllerTable));

        if (g_ClkgenInstance.controllerTable) {
            g_ClkgenInstance.occupied = 1;
            //sec clkgen
            g_ClkgenInstance.controllerTable->get_default_config(&
                    (g_ClkgenInstance.def_cfg));
        }

        //spin_unlock_irqrestore(&clkgen_spin_lock, states);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL, "hal_clock_get_instance is ok \n");
        return &g_ClkgenInstance;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_get_instance is failed \n");
    spin_unlock_irqrestore(&clkgen_spin_lock, spin_lock_states);
    return NULL;
}
//*****************************************************************************
//
//! hal_clock_release_instance.
//!
//! \clkgenInstance: clkgen instance
//!
//! This function release clkgen instance.
//!
//! \return
//
//*****************************************************************************
static void hal_clock_release_instance(clkgen_instance_t *clkgenInstance)
{
    ASSERT((clkgenInstance != NULL));
    //spin_lock_saved_state_t states;
    //spin_lock_irqsave(&clkgen_spin_lock, states);
    clkgenInstance->occupied = 0;
    spin_unlock_irqrestore(&clkgen_spin_lock, spin_lock_states);
}
//*****************************************************************************
//
//! hal_clock_creat_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function creat clkgen handle.
//!
//! \return clkgen handle
//
//*****************************************************************************
bool hal_clock_creat_handle(void **handle)
{
    clkgen_instance_t  *clkgenInstance = NULL;
    ASSERT((handle != NULL));
    //if (clkgen_spin_lock != SPIN_LOCK_INITIAL_VALUE) {
    //    spin_lock_init(&clkgen_spin_lock);
    //}
    clkgenInstance = hal_clock_get_instance();

    if (clkgenInstance == NULL) {
        LTRACEF("hal_clock_creat_handle instance is null\n");
        return false;
    }

    //mutex_init(&clkgenInstance->clkgenMutex);
    *handle = clkgenInstance;
    return true;
}

//*****************************************************************************
//
//! hal_clock_release_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function release clkgen instance handle.
//!
//! \return
//
//*****************************************************************************
bool hal_clock_release_handle(void *handle)
{
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)handle;
    hal_clock_release_instance(l_clkgenInstance);
    // mutex_destroy(&l_clkgenInstance->clkgenMutex);
    return true;
}

bool hal_saf_clock_set_default(void)
{
    bool ret = false;
    void *handle;
    LTRACEF("init saf clock\n");
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        printf("hal_saf_clock_set_default clkgen creat handle failed\n");
        return ret;
    }

    /*init start*/
#ifdef saf_ip_slice
    LTRACEF("init saf clock:ip\n");
    static const clkgen_ip_slice_t ip[] = saf_ip_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(ip); i++) {
#if OSPI_DIRECT_ACCESS

        if (i == saf_ip_slice_ospi1) continue;

#endif
        ret = hal_clock_ip_init(handle, CFG_CKGEN_SAF_BASE, &ip[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef saf_bus_slice
    LTRACEF("init saf clock:bus\n");
    static const clkgen_bus_slice_t bus[] = saf_bus_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(bus); i++) {
        ret = hal_clock_bus_init(handle, CFG_CKGEN_SAF_BASE, &bus[i]);

        if (!ret) goto fail;
    }

#endif
fail:
    hal_clock_release_handle(handle);
    ASSERT(ret);
    return ret;
}


bool hal_sec_clock_set_default(void)
{
    bool ret = false;
    void *handle;
    LTRACEF("init sec clock\n");
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        printf("hal_sec_clock_set_default clkgen creat handle failed\n");
        return ret;
    }

    /*init start*/
#ifdef sec_ip_slice
    LTRACEF("init sec clock:ip\n");
    static const clkgen_ip_slice_t ip[] = sec_ip_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(ip); i++) {
        ret = hal_clock_ip_init(handle, CFG_CKGEN_SEC_BASE, &ip[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef sec_bus_slice
    LTRACEF("init sec clock:bus\n");
    static const clkgen_bus_slice_t bus[] = sec_bus_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(bus); i++) {
        ret = hal_clock_bus_init(handle, CFG_CKGEN_SEC_BASE, &bus[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef sec_core_slice
    LTRACEF("init sec clock:core\n");
    static const clkgen_core_slice_t core[] = sec_core_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(core); i++) {
        ret = hal_clock_core_init(handle, CFG_CKGEN_SEC_BASE, &core[i]);

        if (!ret) goto fail;
    }

#endif
fail:
    hal_clock_release_handle(handle);
    ASSERT(ret);
    return ret;
}

bool hal_soc_clock_set_default(void)
{
    bool ret = false;
    void *handle;
    LTRACEF("init soc clock\n");
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        printf("hal_soc_clock_set_default clkgen creat handle failed\n");
        return ret;
    }

    /*init start*/
#ifdef uuu_wrapper
    LTRACEF("init soc clock:uuu\n");
    static const clkgen_uuu_cfg_t uuu[] = uuu_wrapper;

    for (uint8_t i = 0; i < ARRAYSIZE(uuu); i++) {
        if (uuu[i].slice_index == uuu_clock_wrapper_ddr) continue;

        ret = hal_clock_uuu_init(handle, CFG_CKGEN_SOC_BASE, &uuu[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef soc_ip_slice
    LTRACEF("init soc clock:ip\n");
    static const clkgen_ip_slice_t ip[] = soc_ip_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(ip); i++) {
        ret = hal_clock_ip_init(handle, CFG_CKGEN_SOC_BASE, &ip[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef soc_bus_slice
    LTRACEF("init soc clock:bus\n");
    static const clkgen_bus_slice_t bus[] = soc_bus_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(bus); i++) {
        ret = hal_clock_bus_init(handle, CFG_CKGEN_SOC_BASE, &bus[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef soc_core_slice
    LTRACEF("init soc clock:core\n");
    static const clkgen_core_slice_t core[] = soc_core_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(core); i++) {
        if (core[i].slice_index == soc_core_slice_ddr) continue;

        ret = hal_clock_core_init(handle, CFG_CKGEN_SOC_BASE, &core[i]);

        if (!ret) goto fail;
    }

#endif
fail:
    hal_clock_release_handle(handle);
    ASSERT(ret);
    return ret;
}

bool hal_disp_clock_set_default(void)
{
    bool ret = false;
    void *handle;
    LTRACEF("init disp clock\n");
    ret = hal_clock_creat_handle(&handle);

    if (!ret) {
        printf("hal_disp_clock_set_default clkgen creat handle failed\n");
        return ret;
    }

    /*init start*/
#ifdef disp_ip_slice
    LTRACEF("init disp clock:ip\n");
    static const clkgen_ip_slice_t ip[] = disp_ip_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(ip); i++) {
        ret = hal_clock_ip_init(handle, CFG_CKGEN_DISP_BASE, &ip[i]);

        if (!ret) goto fail;
    }

#endif
#ifdef disp_bus_slice
    LTRACEF("init disp clock:bus\n");
    static const clkgen_bus_slice_t bus[] = disp_bus_slice;

    for (uint8_t i = 0; i < ARRAYSIZE(bus); i++) {
        ret = hal_clock_bus_init(handle, CFG_CKGEN_DISP_BASE, &bus[i]);

        if (!ret) goto fail;
    }

#endif
fail:
    hal_clock_release_handle(handle);
    ASSERT(ret);
    return ret;
}


//*****************************************************************************
//
//! hal_clock_osc_init.
//!
//! \g_handle clock handle.
//!
//! This function is for system doamin fsrefclk init. application use all fsrefclk global index init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_osc_init(void *g_handle, uint32_t res_glb_idx,
                        clkgen_app_fsrefclk_sel_type src_sel_mask, bool en_safety_mode)
{
    int ret = -1;
    vaddr_t clkgen_base_addr = 0x0;
    paddr_t phy_addr;
    int32_t scr_idx = -1;
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &scr_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_osc_init res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_osc_init res_glb_idx:0x%x,src_sel_mask:0x%x,en_safety_mode:%d\n",
                  res_glb_idx, src_sel_mask, en_safety_mode);

    if ((scr_idx >= DEFAULT_FSREFCLK_IDX_START)
            && l_clkgenInstance->controllerTable->fsrefclk_sel) {
        /*init fsrefclk*/
        if (l_clkgenInstance->controllerTable->fsrefclk_sel(clkgen_base_addr,
                (scr_idx - DEFAULT_FSREFCLK_IDX_START), src_sel_mask, en_safety_mode)) {
            return true;
        }
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_osc_init failed scr_idx:%d,res_glb_idx:0x%x \n", scr_idx,
                  res_glb_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_ip_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ip_init(void *g_handle, paddr_t phy_addr,
                       const clkgen_ip_slice_t *ip_slice_default)
{
    vaddr_t clkgen_base_addr = 0x0;
    //paddr_t phy_addr;
    clkgen_instance_t *l_clkgenInstance = NULL;
    uint8_t ip_slice_idx = ip_slice_default->slice_index;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
#if 0
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if ((ret == -1) || (ip_slice_idx < 0)
            || (ip_slice_idx >= (ip_slice_idx_max + DEFAULT_IPSLICE_IDX_START))) {
        LTRACEF("hal_clock_ip_init res_glb_idx:0x%x is not find ip_slice_idx:%d\n",
                res_glb_idx, ip_slice_idx);
        return false;
    }

#endif
    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->ip_slice_set) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->ip_slice_set(clkgen_base_addr,
                ip_slice_idx, ip_slice_default->clk_src_select_num,
                ip_slice_default->pre_div,
                ip_slice_default->post_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_ip_init success slice id %d\n", ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_ip_init fail %d\n", ip_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_bus_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_bus_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_bus_slice_t *bus_slice_default)
{
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_bus_slice_drv_t bus_clock_drv_cfg;
    uint8_t bus_slice_idx = bus_slice_default->slice_index;
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
#if 0
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if ((ret == -1) || (bus_slice_idx < 0)
            || (bus_slice_idx >= (bus_slice_idx_max * 2 +
                                  DEFAULT_BUSSLICE_IDX_START))) {
        LTRACEF("hal_clock_bus_init res_glb_idx:0x%x is not find bus_slice_idx:%d\n",
                res_glb_idx, bus_slice_idx);
        return false;
    }

#endif
    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->bus_slice_switch) {
        //set clk src and div
        //user config
        bus_clock_drv_cfg.bus_slice_idx = bus_slice_idx;
        bus_clock_drv_cfg.clk_a_b_switch =
            bus_slice_default->clk_a_b_select;
        bus_clock_drv_cfg.clk_src_sel_a =
            bus_slice_default->clk_src_select_a_num;
        bus_clock_drv_cfg.clk_src_sel_b =
            bus_slice_default->clk_src_select_b_num;
        bus_clock_drv_cfg.pre_div_a = bus_slice_default->pre_div_a;
        bus_clock_drv_cfg.pre_div_b = bus_slice_default->pre_div_b;
        bus_clock_drv_cfg.post_div = bus_slice_default->post_div;
        bus_clock_drv_cfg.gasket_cfg.m_div_num =
            bus_slice_default->m_div;
        bus_clock_drv_cfg.gasket_cfg.n_div_num =
            bus_slice_default->n_div;
        bus_clock_drv_cfg.gasket_cfg.p_div_num =
            bus_slice_default->p_div;
        bus_clock_drv_cfg.gasket_cfg.q_div_num =
            bus_slice_default->q_div;

        if (l_clkgenInstance->controllerTable->bus_slice_switch(clkgen_base_addr,
                &bus_clock_drv_cfg)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_bus_init success slice_idx:%d\n", bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_bus_init slice_idx:%d fail\n", bus_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_core_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_core_init(void *g_handle, paddr_t phy_addr,
                         const clkgen_core_slice_t *core_slice_default)
{
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_core_slice_drv_t core_clock_drv_cfg;
    clkgen_instance_t *l_clkgenInstance = NULL;
    uint8_t core_slice_idx = core_slice_default->slice_index;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
#if 0
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if ((ret == -1) || (core_slice_idx < 0)
            || (core_slice_idx >= (core_slice_idx_max +
                                   DEFAULT_CORESLICE_IDX_START))) {
        LTRACEF("hal_clock_core_init res_glb_idx:0x%x is not find core_slice_idx:%d\n",
                res_glb_idx, core_slice_idx);
        return false;
    }

#endif
    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->core_slice_switch) {
        //set clk src and div
        //user config
        core_clock_drv_cfg.core_slice_idx = core_slice_idx;
        core_clock_drv_cfg.clk_a_b_switch =
            core_slice_default->clk_a_b_select;
        core_clock_drv_cfg.clk_src_sel_a =
            core_slice_default->clk_src_select_a_num;
        core_clock_drv_cfg.clk_src_sel_b =
            core_slice_default->clk_src_select_b_num;
        core_clock_drv_cfg.post_div = core_slice_default->post_div;

        if (l_clkgenInstance->controllerTable->core_slice_switch(clkgen_base_addr,
                &core_clock_drv_cfg)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_core_init success slice_idx:%d\n", core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_core_init not find slice_idx:%d  \n", core_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_uuu_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuu_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_uuu_cfg_t *uuu_default)
{
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_gasket_type_t l_gasket_div;
    uint8_t uuu_slice_idx = uuu_default->slice_index;
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
#if 0
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &uuu_slice_idx);

    if ((ret == -1) || (uuu_slice_idx < 0)
            || (uuu_slice_idx >= (uuu_slice_idx_max + DEFAULT_UUUSLICE_IDX_START))) {
        LTRACEF("hal_clock_uuu_init res_glb_idx:0x%x is not find uuu_slice_idx:%d\n",
                res_glb_idx, uuu_slice_idx);
        return false;
    }

#endif
    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->uuu_clock_wrapper) {
        //set clk src and div
        //user config
        l_gasket_div.m_div_num = uuu_default->m_div;
        l_gasket_div.n_div_num = uuu_default->n_div;
        l_gasket_div.p_div_num = uuu_default->p_div;
        l_gasket_div.q_div_num = uuu_default->q_div;

        if (l_clkgenInstance->controllerTable->uuu_clock_wrapper(clkgen_base_addr,
                uuu_slice_idx, &l_gasket_div, false,
                uuu_default->uuu_input_clk_sel)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_uuu_init success slice_idx:%d\n", uuu_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_uuu_init not find slice_idx:%d  \n", uuu_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \slice_idx_name module name please reference clkgen_hal slice idx name
//! \module_type is ip/bus/core module
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_enable(void *g_handle, uint32_t res_glb_idx)
{
    int ret = -1;
    int32_t gating_idx = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &gating_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_enable res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    //enable clk gating
    if ((gating_idx >= DEFAULT_LPGATING_IDX_START) && (gating_idx != -1)
            && l_clkgenInstance->controllerTable->gating_enable) {
        if (l_clkgenInstance->controllerTable->gating_enable(clkgen_base_addr,
                (gating_idx - DEFAULT_LPGATING_IDX_START), true)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_enable res_glb_idx:0x%x success\n", res_glb_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_enable res_glb_idx:0x%x enable failed\n", res_glb_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \slice_idx_name module name please reference clkgen_hal slice idx name
//! \module_type is ip/bus/core module
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_disable(void *g_handle, uint32_t res_glb_idx)
{
    int ret = -1;
    int32_t gating_idx = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &gating_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_disable res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    //enable clk gating
    if ((gating_idx != -1) && (gating_idx >= DEFAULT_LPGATING_IDX_START)
            && l_clkgenInstance->controllerTable->gating_enable) {
        if (l_clkgenInstance->controllerTable->gating_enable(clkgen_base_addr,
                (gating_idx - DEFAULT_LPGATING_IDX_START), false)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_disable res_glb_idx:0x%x success\n", res_glb_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_disable res_glb_idx:0x%x enable failed\n", res_glb_idx);
    return false;
}
//*****************************************************************************
//
//! hal_clock_ipclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \ip_slice_idx_name module name please reference clkgen_hal slice idx name
//! \ip_cfg
//! \clk_src_sel please reference clkgen_hal_cfg.h
//! \pre_div clock pre division
//! \post_div  clock post division
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipclk_set(void *g_handle, uint32_t res_glb_idx,
                         clkgen_app_ip_cfg_t *ip_cfg)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t ip_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_ipclk_set res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ip_slice_set) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->ip_slice_set(clkgen_base_addr,
                (ip_slice_idx - DEFAULT_IPSLICE_IDX_START), ip_cfg->clk_src_select_num,
                ip_cfg->pre_div, ip_cfg->post_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_ipclk_set success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_ipclk_set not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, ip_slice_idx);
    return false;
}
//*****************************************************************************
//
//! hal_clock_ipclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \ip_slice_idx_name module name please reference clkgen_hal slice idx name
//! This function is for get module clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type)*(ref_clk_div + 1)
//
//*****************************************************************************
uint32_t hal_clock_ipclk_get(void *g_handle, uint32_t res_glb_idx,
                             clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    clkgen_slice_mon_ret_type ret_type = mon_avg_freq;
    int32_t ip_slice_idx = -1;
    uint32_t clk_reg_vaule = 0;
    uint32_t clk_vaule = 0;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_ipclk_get res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->mon_ip_slice) {
        //get clk
        clk_reg_vaule = l_clkgenInstance->controllerTable->mon_ip_slice(
                            clkgen_base_addr, ip_slice_idx - DEFAULT_IPSLICE_IDX_START, ref_clk_type,
                            ref_clk_div, ret_type);
    }

    if (mon_ref_clk_24M == ref_clk_type) {
        clk_vaule = (clk_reg_vaule * 24 * 1000 * 1000) * (ref_clk_div + 1);
    }
    else {
        clk_vaule = (clk_reg_vaule * 32 * 1000) * (ref_clk_div + 1);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_ipclk_get clk_reg_vaule:%d,ref_clk_type:%d, ret:%d\n",
                  clk_reg_vaule, ref_clk_type, clk_vaule);
    return clk_vaule;
}
bool hal_clock_ipctl_get(void *g_handle, uint32_t res_glb_idx,
                         clkgen_ip_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ip_ctl_get) {
        slice_idx -= DEFAULT_IPSLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->ip_ctl_get(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

bool hal_clock_ipctl_set(void *g_handle, uint32_t res_glb_idx,
                         const clkgen_ip_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ip_ctl_set) {
        slice_idx -= DEFAULT_IPSLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->ip_ctl_set(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_clock_busclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \bus_slice_idx_name module name please reference clkgen_hal slice idx name
//! \bus_cfg
//! \clk_src_select_a_num;//0:a,1:b
//! \clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
//! \clk_a_b_select;//0:a,1:b
//! \pre_div_a;
//! \pre_div_b;
//! \post_div;
//! \m_div;
//! \n_div;
//! \p_div;
//! \q_div;
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_bus_cfg_t *bus_app_cfg)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t bus_slice_idx = -1;
    clkgen_bus_slice_drv_t bus_clock_drv_cfg;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_busclk_set res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    //user config
    bus_clock_drv_cfg.bus_slice_idx = (bus_slice_idx -
                                       DEFAULT_BUSSLICE_IDX_START) / 2;
    bus_clock_drv_cfg.clk_a_b_switch = bus_app_cfg->clk_a_b_select;
    bus_clock_drv_cfg.clk_src_sel_a = bus_app_cfg->clk_src_select_a_num;
    bus_clock_drv_cfg.clk_src_sel_b = bus_app_cfg->clk_src_select_b_num;
    bus_clock_drv_cfg.pre_div_a = bus_app_cfg->pre_div_a;
    bus_clock_drv_cfg.pre_div_b = bus_app_cfg->pre_div_b;
    bus_clock_drv_cfg.post_div = bus_app_cfg->post_div;
    bus_clock_drv_cfg.gasket_cfg.m_div_num = bus_app_cfg->m_div;
    bus_clock_drv_cfg.gasket_cfg.n_div_num = bus_app_cfg->n_div;
    bus_clock_drv_cfg.gasket_cfg.p_div_num = bus_app_cfg->p_div;
    bus_clock_drv_cfg.gasket_cfg.q_div_num = bus_app_cfg->q_div;

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->bus_slice_switch) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->bus_slice_switch(clkgen_base_addr,
                &bus_clock_drv_cfg)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_busclk_set success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_busclk_set not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, bus_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_ipclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \bus_slice_idx_name bus name please reference clkgen_hal bus slice idx name
//! This function is for get module clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type)*(ref_clk_div + 1)
//
//*****************************************************************************
uint32_t hal_clock_busclk_get(void *g_handle, uint32_t res_glb_idx,
                              clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    clkgen_slice_mon_ret_type ret_type = mon_avg_freq;
    int32_t bus_slice_idx = -1;
    uint32_t clk_reg_vaule = 0;
    uint32_t clk_vaule = 0;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_busclk_get res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->mon_bus_slice) {
        //get clk
        clk_reg_vaule = l_clkgenInstance->controllerTable->mon_bus_slice(
                            clkgen_base_addr, (bus_slice_idx - DEFAULT_BUSSLICE_IDX_START) / 2,
                            ref_clk_type, ref_clk_div, ret_type);
    }

    if (mon_ref_clk_24M == ref_clk_type) {
        clk_vaule = (clk_reg_vaule * 24 * 1000 * 1000) * (ref_clk_div + 1);
    }
    else {
        clk_vaule = (clk_reg_vaule * 32 * 1000) * (ref_clk_div + 1);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_busclk_get clk_reg_vaule:%d,ref_clk_type:%d, ret:%d\n",
                  clk_reg_vaule, ref_clk_type, clk_vaule);
    return clk_vaule;
}

bool hal_clock_busctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_bus_ctl *ctl, clkgen_bus_gasket *gasket)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->bus_ctl_get) {
        slice_idx = (slice_idx - DEFAULT_BUSSLICE_IDX_START) / 2;
        ret = l_clkgenInstance->controllerTable->bus_ctl_get(
                  clkgen_base_addr, slice_idx, ctl, gasket);
    }

    return ret;
}

bool hal_clock_busctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_bus_ctl *ctl, const clkgen_bus_gasket *gasket)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->bus_ctl_set) {
        slice_idx = (slice_idx - DEFAULT_BUSSLICE_IDX_START) / 2;
        ret = l_clkgenInstance->controllerTable->bus_ctl_set(
                  clkgen_base_addr, slice_idx, ctl, gasket);
    }

    return ret;
}


//*****************************************************************************
//
//! hal_clock_busclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \core_slice_idx_name core name please reference clkgen_hal slice idx name
//! \core_app_cfg
//! \clk_src_select_a_num;//0:a,1:b
//! \clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
//! \clk_a_b_select;//0:a,1:b
//! \post_div;

//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreclk_set(void *g_handle, uint32_t res_glb_idx,
                           clkgen_app_core_cfg_t *core_app_cfg)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t core_slice_idx = -1;
    clkgen_core_slice_drv_t core_clock_drv_cfg;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_coreclk_set res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    //user config
    core_slice_idx -= DEFAULT_CORESLICE_IDX_START;
    core_clock_drv_cfg.core_slice_idx = core_slice_idx;
    core_clock_drv_cfg.clk_a_b_switch = core_app_cfg->clk_a_b_select;
    core_clock_drv_cfg.clk_src_sel_a = core_app_cfg->clk_src_select_a_num;
    core_clock_drv_cfg.clk_src_sel_b = core_app_cfg->clk_src_select_b_num;
    core_clock_drv_cfg.post_div = core_app_cfg->post_div;

    if ((core_slice_idx != -1) && (core_slice_idx >= 0)
            && l_clkgenInstance->controllerTable->core_slice_switch) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->core_slice_switch(clkgen_base_addr,
                &core_clock_drv_cfg)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_coreclk_set success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_coreclk_set not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, core_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_coreclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \core_slice_idx_name core name please reference clkgen_hal bus slice idx name
//! This function is for get core clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type) * (ref_clk_div + 1)
//
//*****************************************************************************
uint32_t hal_clock_coreclk_get(void *g_handle, uint32_t res_glb_idx,
                               clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    clkgen_slice_mon_ret_type ret_type = mon_avg_freq;
    int32_t core_slice_idx = -1;
    uint32_t clk_reg_vaule = 0;
    uint32_t clk_vaule = 0;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_coreclk_get res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((core_slice_idx != -1)
            && (core_slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->mon_core_slice) {
        //get clk
        clk_reg_vaule = l_clkgenInstance->controllerTable->mon_core_slice(
                            clkgen_base_addr, core_slice_idx - DEFAULT_CORESLICE_IDX_START,
                            ref_clk_type, ref_clk_div, ret_type);
    }

    if (mon_ref_clk_24M == ref_clk_type) {
        clk_vaule = (clk_reg_vaule * 24 * 1000 * 1000) * (ref_clk_div + 1);
    }
    else {
        clk_vaule = (clk_reg_vaule * 32 * 1000) * (ref_clk_div + 1);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_coreclk_get clk_reg_vaule:%d,ref_clk_type:%d, ret:%d\n",
                  clk_reg_vaule, ref_clk_type, clk_vaule);
    return clk_vaule;
}
bool hal_clock_corectl_get(void *g_handle, uint32_t res_glb_idx,
                           clkgen_core_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->core_ctl_get) {
        slice_idx -= DEFAULT_CORESLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->core_ctl_get(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

bool hal_clock_corectl_set(void *g_handle, uint32_t res_glb_idx,
                           const clkgen_core_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->core_ctl_set) {
        slice_idx -= DEFAULT_CORESLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->core_ctl_set(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_clock_uuuclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \uuu_wrapper_idx_name uuu name please reference clkgen_hal slice idx name
//! \clkgen_app_uuu_cfg_t
//! \clkgen_uuu_input_type uuu_input_clk_sel;
//! \low_power_mode_en;
//! \m_div;
//! \n_div;
//! \p_div;
//! \q_div;
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuuclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_uuu_cfg_t *uuu_app_cfg)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t uuu_slice_idx = -1;
    clkgen_gasket_type_t l_gasket_div;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &uuu_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_uuuclk_set res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    //user config
    l_gasket_div.m_div_num = uuu_app_cfg->m_div;
    l_gasket_div.n_div_num = uuu_app_cfg->n_div;
    l_gasket_div.p_div_num = uuu_app_cfg->p_div;
    l_gasket_div.q_div_num = uuu_app_cfg->q_div;

    if ((uuu_slice_idx != -1) && (uuu_slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuu_clock_wrapper) {
        //set clk src and div
        uuu_slice_idx -= DEFAULT_UUUSLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->uuu_clock_wrapper(clkgen_base_addr,
                uuu_slice_idx, &l_gasket_div, uuu_app_cfg->low_power_mode_en,
                uuu_app_cfg->uuu_input_clk_sel)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_uuuclk_set success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, uuu_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_uuuclk_set not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, uuu_slice_idx);
    return false;
}
bool hal_clock_uuuctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_uuu_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuu_ctl_get) {
        slice_idx -= DEFAULT_UUUSLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->uuu_ctl_get(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

bool hal_clock_uuuctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_uuu_ctl *ctl)
{
    int ret = 0;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &slice_idx);

    if (ret == -1) {
        LTRACEF("res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    ret = false;

    if ((slice_idx != -1) && (slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuu_ctl_set) {
        slice_idx -= DEFAULT_UUUSLICE_IDX_START;
        ret = l_clkgenInstance->controllerTable->uuu_ctl_set(
                  clkgen_base_addr, slice_idx, ctl);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_clock_ipslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                    uint8_t dbg_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t ip_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_ipslice_debug_enable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ipslice_debug_enable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->ipslice_debug_enable(
                    clkgen_dbg_base_addr, (ip_slice_idx - DEFAULT_IPSLICE_IDX_START),
                    dbg_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_ipslice_debug_enable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_ipslice_debug_enable not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, ip_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_ipslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipslice_debug_disable(void *g_handle, uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t ip_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_debug_disable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ipslice_debug_disable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->ipslice_debug_disable(
                    clkgen_dbg_base_addr)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_ipslice_debug_disable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_ipslice_debug_disable not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, ip_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_busslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set bus clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                     uint8_t dbg_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t bus_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_busslice_debug_enable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->busslice_debug_enable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->busslice_debug_enable(
                    clkgen_dbg_base_addr, (bus_slice_idx - DEFAULT_BUSSLICE_IDX_START),
                    dbg_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_busslice_debug_enable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_busslice_debug_enable not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, bus_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_busslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set bus clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busslice_debug_disable(void *g_handle, uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t bus_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_busslice_debug_disable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->busslice_debug_disable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->busslice_debug_disable(
                    clkgen_dbg_base_addr)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_busslice_debug_disable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_busslice_debug_disable not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, bus_slice_idx);
    return false;
}


//*****************************************************************************
//
//! hal_clock_coreslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                      uint8_t dbg_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t core_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_coreslice_debug_enable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((core_slice_idx != -1)
            && (core_slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->coreslice_debug_enable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->coreslice_debug_enable(
                    clkgen_dbg_base_addr, (core_slice_idx - DEFAULT_CORESLICE_IDX_START),
                    dbg_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_coreslice_debug_enable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_coreslice_debug_enable not find res_glb_idx:0x%x slice_idx:%d\n",
            res_glb_idx, core_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_coreslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreslice_debug_disable(void *g_handle,
                                       uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t core_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_coreslice_debug_disable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((core_slice_idx != -1)
            && (core_slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->coreslice_debug_disable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->coreslice_debug_disable(
                    clkgen_dbg_base_addr)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_coreslice_debug_disable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_coreslice_debug_disable not find res_glb_idx:0x%x slice_idx:%d\n",
            res_glb_idx, core_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_coreslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuuslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                     uint8_t dbg_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t uuu_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &uuu_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_uuuslice_debug_enable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((uuu_slice_idx != -1)
            && (uuu_slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuuslice_debug_enable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->uuuslice_debug_enable(
                    clkgen_dbg_base_addr, (uuu_slice_idx - DEFAULT_UUUSLICE_IDX_START),
                    dbg_div)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_uuuslice_debug_enable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, uuu_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_uuuslice_debug_enable not find res_glb_idx:0x%x slice_idx:%d\n",
            res_glb_idx, uuu_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_coreslice_debug_disabe.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuuslice_debug_disable(void *g_handle, uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_dbg_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    int32_t uuu_slice_idx = -1;
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &uuu_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_uuuslice_debug_enable res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return false;
    }

    clkgen_dbg_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((uuu_slice_idx != -1)
            && (uuu_slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuuslice_debug_enable) {
        //set clk src and div
        if (l_clkgenInstance->controllerTable->uuuslice_debug_disable(
                    clkgen_dbg_base_addr, (uuu_slice_idx - DEFAULT_UUUSLICE_IDX_START))) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clock_uuuslice_debug_disable success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, uuu_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clock_uuuslice_debug_disable not find res_glb_idx:0x%x slice_idx:%d\n",
            res_glb_idx, uuu_slice_idx);
    return false;
}

//*****************************************************************************
//
//! hal_clock_uuuclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \ip_slice_idx_name module name please reference clkgen_hal slice idx name
//! This function is for get module clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type) * ref_clk_div
//
//*****************************************************************************
uint32_t hal_clock_uuuclk_get(void *g_handle, uint32_t res_glb_idx,
                              clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_instance_t *l_clkgenInstance = NULL;
    clkgen_slice_mon_ret_type ret_type = mon_avg_freq;
    int32_t ip_slice_idx = -1;
    uint32_t clk_reg_vaule = 0;
    uint32_t clk_vaule = 0;
    int uuu_map_count = 0;
    clkgen_uuu_monitor_t uuu_map[] = {
        {RES_UUU_WRAP_SOC_CPU1A, 3}, {RES_UUU_WRAP_SOC_CPU1B, 4}, {RES_UUU_WRAP_SOC_CPU2, 5}, {RES_UUU_WRAP_SOC_GPU1, 6},
        {RES_UUU_WRAP_SOC_GPU2, 7}, {RES_UUU_WRAP_SOC_VPU1, 8}, {RES_UUU_WRAP_SOC_MJPEG, 9}, {RES_UUU_WRAP_SOC_VPU_BUS, 10},
        {RES_UUU_WRAP_SOC_VSN, 11}, {RES_UUU_WRAP_SOC_DDR, 12}, {RES_UUU_WRAP_SOC_HIS_BUS, 13}
    };
    ASSERT((g_handle != NULL));
    l_clkgenInstance = (clkgen_instance_t *)g_handle;
    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clock_ipclk_get res_glb_idx:0x%x is not find\n", res_glb_idx);
        return false;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);
    uuu_map_count = sizeof(uuu_map) / sizeof(uuu_map[0]);
    ip_slice_idx = -1;

    for (int i = 0; i < uuu_map_count; i++) {
        if (res_glb_idx == uuu_map[i].res_id) {
            ip_slice_idx = uuu_map[i].ip_idx;
            break;
        }
    }

    //enable uuu monitor
    hal_clock_uuuslice_debug_enable(g_handle, res_glb_idx, ref_clk_div);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->mon_ip_slice) {
        //get clk
        clk_reg_vaule = l_clkgenInstance->controllerTable->mon_ip_slice(
                            clkgen_base_addr, ip_slice_idx - DEFAULT_IPSLICE_IDX_START, ref_clk_type,
                            0, ret_type);
    }

    if (mon_ref_clk_24M == ref_clk_type) {
        clk_vaule = (clk_reg_vaule * 24 * 1000 * 1000) * (ref_clk_div + 1);
    }
    else {
        clk_vaule = (clk_reg_vaule * 32 * 1000) * (ref_clk_div + 1);
    }

    hal_clock_uuuslice_debug_disable(g_handle, res_glb_idx);
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clock_uuuclk_get clk_reg_vaule:%d,ref_clk_type:%d, ret:%d\n",
                  clk_reg_vaule, ref_clk_type, clk_vaule);
    return clk_vaule;
}
