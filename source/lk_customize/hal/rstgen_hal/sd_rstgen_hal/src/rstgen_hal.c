//*****************************************************************************
//
// rstgen_hal.c - Driver for the rstgen hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <trace.h>

#include "rstgen_drv.h"
#include "rstgen_hal.h"
#include "system_cfg.h"
#include "res.h"
#include "chip_res.h"

#define LOCAL_TRACE 0

#if defined (rstgen_res_capability_def)
static rstgen_capability_t g_rstgen_res_capability =
    rstgen_res_capability_def;
#else
static rstgen_capability_t g_rstgen_res_capability = {
    .version = 0x10000,
    .res_category = "rstgen",
    .res_max_num = 2,
    .res_cap[0].res_glb_idx = RES_GLOBAL_RST_SEC_RST_EN,
    .res_cap[0].glb_self_rst_en = false,
    .res_cap[0].glb_sem_rst_en = false,
    .res_cap[0].glb_dbg_rst_en = false,
    .res_cap[0].glb_wdg1_rst_en = false,
    .res_cap[0].glb_wdg2_rst_en = false,
    .res_cap[0].glb_wdg3_rst_en = false,
    .res_cap[0].glb_wdg4_rst_en = false,
    .res_cap[0].glb_wdg5_rst_en = false,
    .res_cap[0].glb_wdg6_rst_en = false,
    .res_cap[0].glb_wdg7_rst_en = false,
    .res_cap[0].glb_wdg8_rst_en = false,
    .res_cap[1].res_glb_idx = RES_GLOBAL_RST_SAF_RST_EN,
    .res_cap[1].glb_self_rst_en = true,
    .res_cap[1].glb_sem_rst_en = true,
    .res_cap[1].glb_dbg_rst_en = true,
    .res_cap[1].glb_wdg1_rst_en = false,
    .res_cap[1].glb_wdg2_rst_en = false,
    .res_cap[1].glb_wdg3_rst_en = false,
    .res_cap[1].glb_wdg4_rst_en = false,
    .res_cap[1].glb_wdg5_rst_en = false,
    .res_cap[1].glb_wdg6_rst_en = false,
    .res_cap[1].glb_wdg7_rst_en = false,
    .res_cap[1].glb_wdg8_rst_en = false,
};
#endif


/*rstgen global instance*/
static rstgen_instance_t g_RstgenInstance = {
    .occupied = 0,
};

spin_lock_t rstgen_spin_lock = SPIN_LOCK_INITIAL_VALUE;

/*rstgen driver interface*/
static const
rstgen_drv_controller_interface_t s_RstgenDrvInterface = {
    rstgen_get_default_config,
    rstgen_init,
    rstgen_global_rst_enable,
    rstgen_global_rst_disable,
    rstgen_sw_self_rst,
    rstgen_sw_oth_rst,
    rstgen_get_rst_sta,
    rstgen_iso_enable,
    rstgen_iso_disable,
    rstgen_iso_status,
    rstgen_core_reset,
    rstgen_core_ctl,
    rstgen_module_ctl,
    rstgen_module_status,
    rstgen_core_status,
};

//*****************************************************************************
//
//! hal_rstgen_get_controller_interface.
//!
//! \param controllerTable is rstgen interface ptr
//!
//! This function get rstgen driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_rstgen_get_controller_interface(const
        rstgen_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_RstgenDrvInterface;
}

//*****************************************************************************
//
//! hal_rstgen_get_instance.
//!
//! \void.
//!
//! This function get rstgen instance hand.
//!
//! \return rstgen hanle
//
//*****************************************************************************
static rstgen_instance_t *hal_rstgen_get_instance(void)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&rstgen_spin_lock, states);

    if (g_RstgenInstance.occupied != 1) {
        memset(&g_RstgenInstance, 0, sizeof(rstgen_instance_t));

        /* get rstgen driver API table */
        hal_rstgen_get_controller_interface(
            &g_RstgenInstance.controllerTable);

        if (g_RstgenInstance.controllerTable) {
            g_RstgenInstance.occupied = 1;
            g_RstgenInstance.rstgen_res[0].glb_self_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_self_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_sem_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_sem_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_dbg_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_dbg_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg1_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg1_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg2_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg2_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg3_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg3_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg4_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg4_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg5_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg5_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg6_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg6_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg7_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg7_rst_en;
            g_RstgenInstance.rstgen_res[0].glb_wdg8_rst_en =
                g_rstgen_res_capability.res_cap[0].glb_wdg8_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_self_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_self_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_sem_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_sem_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_dbg_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_dbg_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg1_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg1_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg2_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg2_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg3_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg3_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg4_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg4_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg5_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg5_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg6_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg6_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg7_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg7_rst_en;
            g_RstgenInstance.rstgen_res[1].glb_wdg8_rst_en =
                g_rstgen_res_capability.res_cap[1].glb_wdg8_rst_en;
        }
    }

    spin_unlock_irqrestore(&rstgen_spin_lock, states);
    return &g_RstgenInstance;
}

//*****************************************************************************
//
//! hal_rstgen_release_instance.
//!
//! \void.
//!
//! This function release rstgen instance hand.
//!
//! \return
//
//*****************************************************************************
void hal_rstgen_release_instance(rstgen_instance_t *instance)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&rstgen_spin_lock, states);
    instance->occupied = 0;
    spin_unlock_irqrestore(&rstgen_spin_lock, states);
}

//*****************************************************************************
//
//! hal_rstgen_creat_handle.
//!
//! \handle rstgen handle for rstgen func.
//! \global_rst_res_idx rstgen globale reset global resource index
//!
//! This function get hal handle.
//!
//! \return rstgen handle
//
//*****************************************************************************
bool hal_rstgen_creat_handle(void **handle,
                             uint32_t global_rst_res_idx)
{
    int32_t idx = -1;
    rstgen_instance_t  *instance = NULL;

    if (handle == NULL) {
        LTRACEF("hal_get_resource paramenter error handle:%p\n", handle);
        return false;
    }

    if (rstgen_spin_lock != SPIN_LOCK_INITIAL_VALUE) {
        spin_lock_init(&rstgen_spin_lock);
    }

    instance = hal_rstgen_get_instance();

    if (instance == NULL) {
        return false;
    }

    if (res_get_info_by_id(global_rst_res_idx,
                           &(instance->phy_addr), &idx)) {
        LTRACEF("hal_rstgen_creat_handle paramenter error global_rst_res_idx:%d\n",
                global_rst_res_idx);
        return false;
    }

    mutex_init(&instance->rstgenMutex);

    *handle = instance;
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_release_handle.
//!
//! \void.
//!
//! This function delete rstgen instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_rstgen_release_handle(void *handle)
{
    rstgen_instance_t *instance = NULL;

    if (handle == NULL) {
        LTRACEF("hal_rstgen_release_handle paramenter error handle:%p\n",
                handle);
        return false;
    }

    instance = (rstgen_instance_t *)handle;
    instance->occupied = 0;
    mutex_destroy(&instance->rstgenMutex);
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_init.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen used rstgen_app_cfg parameter init.
//!
//! \return bool status
//
//*****************************************************************************
#if ENABLE_SD_RSTGEN_INIT
bool hal_rstgen_init(void *handle)
{
    bool ret = false;
    uint32_t global_rst_mask = 0;
    vaddr_t base = 0x0;
    rstgen_instance_t *instance = NULL;

    instance = (rstgen_instance_t *)handle;

    if (APB_RSTGEN_SEC_BASE == instance->phy_addr) {
        global_rst_mask = RSTGEN_GLB_RST_SELF_RST_EN(
                              instance->rstgen_res[0].glb_self_rst_en) \
                          | RSTGEN_GLB_RST_SEM_RST_EN(
                              instance->rstgen_res[0].glb_sem_rst_en) \
                          | RSTGEN_GLB_RST_DBG_RST_EN(
                              instance->rstgen_res[0].glb_dbg_rst_en) \
                          | RSTGEN_GLB_RST_WDG1_RST_EN(
                              instance->rstgen_res[0].glb_wdg1_rst_en) \
                          | RSTGEN_GLB_RST_WDG2_RST_EN(
                              instance->rstgen_res[0].glb_wdg2_rst_en) \
                          | RSTGEN_GLB_RST_WDG3_RST_EN(
                              instance->rstgen_res[0].glb_wdg3_rst_en) \
                          | RSTGEN_GLB_RST_WDG4_RST_EN(
                              instance->rstgen_res[0].glb_wdg4_rst_en) \
                          | RSTGEN_GLB_RST_WDG5_RST_EN(
                              instance->rstgen_res[0].glb_wdg5_rst_en) \
                          | RSTGEN_GLB_RST_WDG6_RST_EN(
                              instance->rstgen_res[0].glb_wdg6_rst_en) \
                          | RSTGEN_GLB_RST_WDG7_RST_EN(
                              instance->rstgen_res[0].glb_wdg7_rst_en) \
                          | RSTGEN_GLB_RST_WDG8_RST_EN(
                              instance->rstgen_res[0].glb_wdg8_rst_en);
    }
    else if (APB_RSTGEN_SAF_BASE == instance->phy_addr) {
        global_rst_mask = RSTGEN_GLB_RST_SELF_RST_EN(
                              instance->rstgen_res[1].glb_self_rst_en) \
                          | RSTGEN_GLB_RST_SEM_RST_EN(
                              instance->rstgen_res[1].glb_sem_rst_en) \
                          | RSTGEN_GLB_RST_DBG_RST_EN(
                              instance->rstgen_res[1].glb_dbg_rst_en) \
                          | RSTGEN_GLB_RST_WDG1_RST_EN(
                              instance->rstgen_res[1].glb_wdg1_rst_en) \
                          | RSTGEN_GLB_RST_WDG2_RST_EN(
                              instance->rstgen_res[1].glb_wdg2_rst_en) \
                          | RSTGEN_GLB_RST_WDG3_RST_EN(
                              instance->rstgen_res[1].glb_wdg3_rst_en) \
                          | RSTGEN_GLB_RST_WDG4_RST_EN(
                              instance->rstgen_res[1].glb_wdg4_rst_en) \
                          | RSTGEN_GLB_RST_WDG5_RST_EN(
                              instance->rstgen_res[1].glb_wdg5_rst_en) \
                          | RSTGEN_GLB_RST_WDG6_RST_EN(
                              instance->rstgen_res[1].glb_wdg6_rst_en) \
                          | RSTGEN_GLB_RST_WDG7_RST_EN(
                              instance->rstgen_res[1].glb_wdg7_rst_en) \
                          | RSTGEN_GLB_RST_WDG8_RST_EN(
                              instance->rstgen_res[1].glb_wdg8_rst_en);
    }
    else {
        return ret;
    }

    base = (vaddr_t)_ioaddr(instance->phy_addr);
    instance->global_rst_maks = global_rst_mask;

    if (instance->controllerTable->init) {
        ret = instance->controllerTable->init(base,
                                              instance->global_rst_maks);
    }

    instance->rstgen_inited = true;

    return ret;
}
#else
bool hal_rstgen_init(void *handle)
{
    return true;
}
#endif

//*****************************************************************************
//
//! hal_rstgen_sw_self_rst.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen self reset.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_sw_self_rst(void *handle)
{
    rstgen_instance_t *instance = (rstgen_instance_t *)handle;
    vaddr_t base = (vaddr_t)_ioaddr(instance->phy_addr);
    bool ret = false;

    if (instance->controllerTable->sw_self_rst) {
        ret = instance->controllerTable->sw_self_rst(base, false);
        ret = instance->controllerTable->sw_self_rst(base, true);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_sw_oth_rst.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen reset other rstgen
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_sw_oth_rst(void *handle)
{
    rstgen_instance_t *instance = (rstgen_instance_t *)handle;
    vaddr_t base = (vaddr_t)_ioaddr(instance->phy_addr);
    bool ret = false;

    if (instance->controllerTable->sw_oth_rst) {
        ret = instance->controllerTable->sw_oth_rst(base, false);
        ret = instance->controllerTable->sw_oth_rst(base, true);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_get_rst_sta.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for get reset status mask
//!
//! \return rstgen reset status mask
//
//*****************************************************************************
uint32_t hal_rstgen_get_rst_sta(void *handle)
{
    rstgen_instance_t *instance = (rstgen_instance_t *)handle;
    vaddr_t base = (vaddr_t)_ioaddr(instance->phy_addr);
    uint32_t reset_mask = 0;

    if (instance->controllerTable->get_rst_sta) {
        reset_mask = instance->controllerTable->get_rst_sta(base);
    }

    return reset_mask;
}

//*****************************************************************************
//
//! hal_rstgen_iso_enable.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx iso globale resource index
//!
//! This function is for rstgen isolation enable
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_enable(void *handle, uint32_t res_glb_idx)
{
    bool ret = false;
    paddr_t phy_addr = 0;
    int32_t iso_idx = -1;
    vaddr_t base = 0x0;
    rstgen_instance_t *instance = NULL;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &iso_idx)) {
        LTRACEF("hal_rstgen_iso_enable paramenter error res_glb_idx:%d\n",
                res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if ((iso_idx > 0) && instance->controllerTable->iso_enable) {
        ret = instance->controllerTable->iso_enable(
                  (vaddr_t)base, (uint32_t)iso_idx);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_iso_disable.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx iso globale resource index
//!
//! This function is for rstgen isolation disable
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_disable(void *handle, uint32_t res_glb_idx)
{
    bool ret = false;
    paddr_t phy_addr = 0;
    int32_t iso_idx = -1;
    vaddr_t base = 0x0;
    rstgen_instance_t *instance = NULL;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &iso_idx)) {
        LTRACEF("hal_rstgen_iso_disable paramenter error res_glb_idx:%d\n",
                res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if ((iso_idx > 0) && instance->controllerTable->iso_disable) {
        ret = instance->controllerTable->iso_disable(
                  (vaddr_t)base, (uint32_t)iso_idx);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_iso_status.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx iso globale resource index
//!
//! This function is for rstgen isolation status indication
//!
//! \return status, 1 not isolated, 0 isolated, -1 invalid res idx.
//
//*****************************************************************************
uint32_t hal_rstgen_iso_status(void *handle, uint32_t res_glb_idx)
{
    uint32_t ret = -1;
    paddr_t phy_addr = 0;
    int32_t iso_idx = -1;
    vaddr_t base = 0x0;
    rstgen_instance_t *instance = NULL;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &iso_idx)) {
        LTRACEF("hal_rstgen_iso_disable paramenter error res_glb_idx:%d\n",
                res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if ((iso_idx > 0) && instance->controllerTable->iso_status) {
        ret = instance->controllerTable->iso_status(
                  (vaddr_t)base, (uint32_t)iso_idx);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_core_reset.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core globale resource index
//!
//! This function is for rstgen core reset
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_reset(void *handle, uint32_t res_glb_idx)
{
    rstgen_instance_t *instance;
    paddr_t phy_addr;
    int32_t core_idx;
    vaddr_t base;
    bool ret = false;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &core_idx)) {
        LTRACEF("hal_rstgen_core_reset paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if ((core_idx > 0) && instance->controllerTable->core_reset) {
        ret = instance->controllerTable->core_reset(
                  (vaddr_t)base, (uint32_t)core_idx);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_core_ctl.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core globale resource index
//! \release assecrt or release the reset
//!
//! This function is for rstgen core reset ctl
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_ctl(void *handle, uint32_t res_glb_idx, bool release)
{
    rstgen_instance_t *instance;
    paddr_t phy_addr;
    int32_t core_idx;
    vaddr_t base;
    bool ret = false;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &core_idx)) {
        LTRACEF("hal_rstgen_core_ctl paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if ((core_idx > 0) && instance->controllerTable->core_ctl) {
        ret = instance->controllerTable->core_ctl(
                  (vaddr_t)base, (uint32_t)core_idx, release);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_module_reset.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module globale resource index
//! \release assecrt or release the reset
//!
//! This function is for rstgen module reset ctl
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_reset(void *handle, uint32_t res_glb_idx)
{
    rstgen_instance_t *instance;
    paddr_t     phy_addr;
    int32_t     module_idx;
    vaddr_t     base;
    bool        ret = false;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &module_idx)) {
        LTRACEF("hal_rstgen_module_reset paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if (module_idx > 0 && instance->controllerTable->module_ctl) {
        ret = instance->controllerTable->module_ctl(
                  base, (uint32_t)module_idx, false);
        ret = instance->controllerTable->module_ctl(
                  base, (uint32_t)module_idx, true);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_module_ctl.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module globale resource index
//! \release assecrt or release the reset
//!
//! This function is for rstgen module reset ctl
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_ctl(void *handle, uint32_t res_glb_idx, bool release)
{
    rstgen_instance_t *instance;
    paddr_t     phy_addr;
    int32_t     module_idx;
    vaddr_t     base;
    bool        ret = false;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &module_idx)) {
        LTRACEF("hal_rstgen_module_ctl paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if (module_idx > 0 && instance->controllerTable->module_ctl) {
        ret = instance->controllerTable->module_ctl(
                  base, (uint32_t)module_idx, release);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_module_status.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module globale resource index
//!
//! This function is for querying rstgen module reset status
//!
//! \return 1, the module reset has been released.
//! \return 0, the module reset hasn't been released.
//! \return 0xffffffff, invaled module id.
//
//*****************************************************************************
uint32_t hal_rstgen_module_status(void *handle, uint32_t res_glb_idx)
{
    rstgen_instance_t *instance;
    paddr_t     phy_addr;
    int32_t     module_idx;
    vaddr_t     base;
    uint32_t    ret = -1;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &module_idx)) {
        LTRACEF("hal_rstgen_module_status paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if (module_idx > 0 && instance->controllerTable->module_reset_status) {
        ret = instance->controllerTable->module_reset_status(
                  base, (uint32_t)module_idx);
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rstgen_core_status.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module globale resource index
//!
//! This function is for querying rstgen core reset status
//!
//! \return 1, the core reset has been released.
//! \return 0, the core reset hasn't been released.
//! \return 0xffffffff, invaled module id.
//
//*****************************************************************************
uint32_t hal_rstgen_core_status(void *handle, uint32_t res_glb_idx)
{
    rstgen_instance_t *instance;
    paddr_t     phy_addr;
    int32_t     core_idx;
    vaddr_t     base;
    uint32_t    ret = -1;

    if (res_get_info_by_id(res_glb_idx, &phy_addr, &core_idx)) {
        LTRACEF("hal_rstgen_core_status paramenter error res_glb_idx:%d\n",
                        res_glb_idx);
        return ret;
    }

    instance = (rstgen_instance_t *)handle;
    base = (vaddr_t)_ioaddr(phy_addr);

    if (core_idx > 0 && instance->controllerTable->core_reset_status) {
        ret = instance->controllerTable->core_reset_status(
                  base, (uint32_t)core_idx);
    }

    return ret;
}
