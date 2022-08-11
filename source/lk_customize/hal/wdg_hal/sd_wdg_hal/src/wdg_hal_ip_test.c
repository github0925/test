//*****************************************************************************
//
// wdg_ip_test_hal.c - Driver for the Watchdog ip test hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
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
#include "wdg_drv_test.h"
#include "wdg_hal_ip_test.h"
#include "res.h"
#include "chip_res.h"

/*watchdog test global instance*/
static wdg_test_instance_t g_WdgTestInstance[wdg_really_num_max] = {0};
spin_lock_t wdg_test_spin_lock = SPIN_LOCK_INITIAL_VALUE;

#if defined (watchdog_res_capability_def)
static wdg_capability_t g_watchdog_test_res_capability= watchdog_res_capability_def;
#else
static wdg_capability_t g_watchdog_test_res_capability= {.version = 0x10000, \
                                                                                .res_category = "watchdog",
                                                                                .res_max_num = 8,
                                                                                .res_cap[0].res_glb_idx = RES_WATCHDOG_WDT1,
                                                                                .res_cap[0].enableIntReset = true,
                                                                                .res_cap[0].enableExtReset = true,
                                                                                .res_cap[1].res_glb_idx = RES_WATCHDOG_WDT2,
                                                                                .res_cap[1].enableIntReset = true,
                                                                                .res_cap[1].enableExtReset = false,
                                                                                .res_cap[2].res_glb_idx = RES_WATCHDOG_WDT3,
                                                                                .res_cap[2].enableIntReset = true,
                                                                                .res_cap[2].enableExtReset = true,
                                                                                .res_cap[3].res_glb_idx = RES_WATCHDOG_WDT4,
                                                                                .res_cap[3].enableIntReset = true,
                                                                                .res_cap[3].enableExtReset = true,
                                                                                .res_cap[4].res_glb_idx = RES_WATCHDOG_WDT5,
                                                                                .res_cap[4].enableIntReset = true,
                                                                                .res_cap[4].enableExtReset = true,
                                                                                .res_cap[5].res_glb_idx = RES_WATCHDOG_WDT6,
                                                                                .res_cap[5].enableIntReset = true,
                                                                                .res_cap[5].enableExtReset = true,
                                                                                .res_cap[6].res_glb_idx = RES_WATCHDOG_WDT7,
                                                                                .res_cap[6].enableIntReset = true,
                                                                                .res_cap[6].enableExtReset = false,
                                                                                .res_cap[7].res_glb_idx = RES_WATCHDOG_WDT8,
                                                                                .res_cap[7].enableIntReset = false,
                                                                                .res_cap[7].enableExtReset = false,
                                                                                };
#endif

/*watchdog ip test driver interface*/
static const wdg_test_drv_controller_interface_t s_WdgTestDrvInterface =
{
    wdg_test_get_default_config,
    wdg_readonlyreg_check_test,
    wdg_rw_reg_check_test,
    wdg_self_test,
    wdg_terminal_test,
    wdg_terminal_from_fuse_test,
    wdg_reset_control_restart_test,
    wdg_reset_control_donot_restart_test,
    wdg_mode1_refresh_test,
    wdg_mode2_refresh_test,
    wdg_mode3_refresh_test,
    wdg_mode2_window_reset_test,
    wdg_mode2_1_refresh_test,
    wdg_mode3_2_1_refresh_test,
    wdg_mode1_overflow_int_check_test,
    wdg_debug_mode_test,
};

//*****************************************************************************
//
//! hal_wdg_test_get_controller_interface.
//!
//! \param controllerTable is watchdog test interface ptr
//!
//! This function get watchdog driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_wdg_test_get_controller_interface(const wdg_test_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_WdgTestDrvInterface;
}
//*****************************************************************************
//
//! hal_wdg_test_get_instance.
//!
//! \wdg_res_glb_idx watchdog globale index
//!
//! This function get watchdog test instance hand.
//!
//! \return watchdog hanle
//
//*****************************************************************************
static wdg_test_instance_t *hal_wdg_test_get_instance(uint32_t wdg_res_glb_idx)
{
    uint8_t i = 0;
    int8_t ret = 0;
    paddr_t phy_addr = 0;
    int32_t wdg_really_num = 0;
    spin_lock_saved_state_t states;

    ret = res_get_info_by_id(wdg_res_glb_idx,&phy_addr,&wdg_really_num);
    LTRACEF("hal_wdg_test_get_instance paramenter ret:%d,wdg_res_glb_idx:0x%x,wdg_really_num:%d\n",ret,wdg_res_glb_idx,wdg_really_num);
    spin_lock_irqsave(&wdg_test_spin_lock, states);

    for (i=0; i < wdg_really_num_max; i++)
    {
        LTRACEF("hal_wdg_test_get_instance paramenter ret:%d,g_WdgTestInstance[%d].occupied:%d\n",ret,i,g_WdgTestInstance[i].occupied);
        if ((ret == 0) && (g_WdgTestInstance[i].occupied != 1))
        {
            uint8_t *buffer = (uint8_t *)&g_WdgTestInstance[i];
            memset(buffer,0,sizeof(wdg_instance_t));
            LTRACEF("hal_wdg_test_get_instance find ok ret:%d,g_WdgTestInstance[%d].occupied:%d\n",ret,i,g_WdgTestInstance[i].occupied);
            /* get watchdog driver API table */
            hal_wdg_test_get_controller_interface(&(g_WdgTestInstance[i].controllerTable));
            if(g_WdgTestInstance[i].controllerTable){
                g_WdgTestInstance[i].occupied = 1;
                g_WdgTestInstance[i].controllerTable->get_default_config(&(g_WdgTestInstance[i].wdg_cfg));
                g_WdgTestInstance[i].wdg_res.wdg_really_num = wdg_really_num;
                g_WdgTestInstance[i].wdg_res.wdg_phy_addr = phy_addr;
                g_WdgTestInstance[i].wdg_cfg.wdg_reset_cfg.enableSysReset = g_watchdog_test_res_capability.res_cap[wdg_really_num-1].enableIntReset;
                g_WdgTestInstance[i].wdg_cfg.wdg_ext_reset_cfg.enableSysExtReset = g_watchdog_test_res_capability.res_cap[wdg_really_num-1].enableExtReset;
                spin_unlock_irqrestore(&wdg_test_spin_lock, states);
                return &g_WdgTestInstance[i];
            }
        }
    }
    spin_unlock_irqrestore(&wdg_test_spin_lock, states);
    return NULL;
}

//*****************************************************************************
//
//! hal_wdg_test_release_instance.
//!
//! \void.
//!
//! This function release watchdog test instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_wdg_test_release_instance(wdg_test_instance_t *wdgInstance)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&wdg_test_spin_lock, states);
    wdgInstance->occupied = 0;
    spin_unlock_irqrestore(&wdg_test_spin_lock, states);
}

//*****************************************************************************
//
//! hal_wdg_test_creat_handle.
//!
//! \handle watchdog handle for wdg func.
//! \wdg_res_glb_idx watchdog globale index
//!
//! This function get watchdog test hal handle.
//!
//! \return watchdog handle
//
//*****************************************************************************
bool hal_wdg_test_creat_handle(void **handle,uint32_t wdg_res_glb_idx)
{
    wdg_test_instance_t  *wdgInstance = NULL;

    if(wdg_test_spin_lock != SPIN_LOCK_INITIAL_VALUE){
        spin_lock_init(&wdg_test_spin_lock);
    }

    wdgInstance = hal_wdg_test_get_instance(wdg_res_glb_idx);
    if(wdgInstance == NULL){
        LTRACEF("hal_wdg_test_creat_handle paramenter error handle:%p\n",*handle);
        return false;
    }

    mutex_init(&wdgInstance->wdgMutex);

    *handle = wdgInstance;
    LTRACEF("hal_wdg_test_creat_handle paramenter is ok:%p\n",handle);

    return true;
}
//*****************************************************************************
//
//! hal_wdg_test_delete_handle.
//!
//! \void.
//!
//! This function delete watchdog instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_wdg_test_delete_handle(void *handle)
{
    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;
    l_wdgInstance->occupied = 0;
    mutex_destroy(&l_wdgInstance->wdgMutex);
    return true;
}

//*****************************************************************************
//
//! hal_wdg_read_only_reg_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_read_only_reg_test(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_read_only_reg_testparamenter wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->readonlyreg_check_test){
        ret = l_wdgInstance->controllerTable->readonlyreg_check_test((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_read_only_reg_test failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_rw_reg_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog rw register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_rw_reg_test(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_rw_reg_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->rw_reg_check_test){
        ret = l_wdgInstance->controllerTable->rw_reg_check_test((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_rw_reg_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_self_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog self test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_self_test(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_self_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->self_test){
        ret = l_wdgInstance->controllerTable->self_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg));
    }

    if(!ret){
        LTRACEF("hal_wdg_self_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_terminal_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog terninal test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_terminal_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_terminal_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->terminal_test){
        ret = l_wdgInstance->controllerTable->terminal_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_terminal_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_terminal_from_fuse_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog terninal test from fuse.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_terminal_from_fuse_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_terminal_from_fuse_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->terminal_from_fuse_test){
        ret = l_wdgInstance->controllerTable->terminal_from_fuse_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_terminal_from_fuse_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_reset_control_restart_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog reset control restart test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_reset_control_restart_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;


    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_reset_control_restart_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->reset_control_restart_test){
        ret = l_wdgInstance->controllerTable->reset_control_restart_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_reset_control_restart_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_reset_control_not_restart_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog reset control restart test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_reset_control_not_restart_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_reset_control_not_restart_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->reset_control_donot_restart_test){
        ret = l_wdgInstance->controllerTable->reset_control_donot_restart_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_reset_control_not_restart_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode1_refresh_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog refresh mode1 test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode1_refresh_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    LTRACEF("hal_wdg_mode1_refresh_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode1_refresh_test){
        ret = l_wdgInstance->controllerTable->mode1_refresh_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode1_refresh_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode2_refresh_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog refresh mode2 test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode2_refresh_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_mode2_refresh_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode2_refresh_test){
        ret = l_wdgInstance->controllerTable->mode2_refresh_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode2_refresh_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode3_refresh_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog refresh mode3 test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode3_refresh_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode3_refresh_test){
        ret = l_wdgInstance->controllerTable->mode3_refresh_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode3_refresh_test failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_mode2_window_reset_test.
//!
//! \handle watchdog handle for wdg func.
//! \timeout watchdog timeout timer ms.
//!
//! This function is for watchdog mode2 windows reset refresh test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode2_window_reset_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_mode2_window_reset_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode2_window_reset_test){
        ret = l_wdgInstance->controllerTable->mode2_window_reset_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode2_window_reset_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode2_1_refresh_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog mode2 and mode1 refresh test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode2_1_refresh_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_mode2_1_refresh_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode2_1_refresh_test){
        ret = l_wdgInstance->controllerTable->mode2_1_refresh_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode2_1_refresh_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode3_2_1_refresh_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog mode3 mode2 and mode1 refresh test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode3_2_1_refresh_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_mode3_2_1_refresh_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode3_2_1_refresh_test){
        ret = l_wdgInstance->controllerTable->mode3_2_1_refresh_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode3_2_1_refresh_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_mode1_overflow_intcheck_test.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog mode1 overflow int check test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_mode1_overflow_intcheck_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_mode1_overflow_intcheck_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->mode1_overflow_int_check_test){
        ret = l_wdgInstance->controllerTable->mode1_overflow_int_check_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_mode1_overflow_intcheck_test failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_debug_mode_test.
//!
//! \handle watchdog handle for wdg func.
//! \timeout watchdog timeout timer ms.
//!
//! This function is for watchdog halt test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_debug_mode_test(void *handle,uint32_t timeout)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_test_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_test_instance_t *)handle;
    TRACEF("hal_wdg_debug_mode_test wdg_phy_addr:0x%lx\n",l_wdgInstance->wdg_res.wdg_phy_addr);

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->debug_mode_test){
        ret = l_wdgInstance->controllerTable->debug_mode_test((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg),timeout);
    }

    if(!ret){
        LTRACEF("hal_wdg_debug_mode_test failed\n");
        return false;
    }

    return true;
}
