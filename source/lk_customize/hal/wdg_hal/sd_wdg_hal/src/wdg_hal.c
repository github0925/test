//*****************************************************************************
//
// wdg_hal.c - Driver for the Watchdog hal Module.
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
#include "wdg_drv.h"
#include "wdg_hal.h"
#include "res.h"
#include "chip_res.h"

/*watchdog global instance*/
static wdg_instance_t g_WdgInstance[DEFAULT_WDG_MAX_NUM] = {0};
spin_lock_t wdg_spin_lock = SPIN_LOCK_INITIAL_VALUE;
/*
//watchdog num  |reset module    |reset saf domain   |reset ap domain    |reset pmic
//wdt1:             |cr5_saf            |configurable         |configurable         |configurable
//wdt2:             |vdsp                |N                       |N                       |N
//wdt3:             |cr5_sec            |N                       |configurable         |configurable
//wdt4:             |cr5_mp             |N                       |configurable         |configurable
//wdt5:             |cpu1                |N                       |configurable         |configurable
//wdt6:             |cpu2                |N                       |configurable         |configurable
//wdt7:             |adsp                |N                       |N                        |N
//wdt8:             |security violation|N                       |N                        |N
*/
#if defined (watchdog_res_capability_def)
static wdg_capability_t g_watchdog_res_capability= watchdog_res_capability_def;
#else
static wdg_capability_t g_watchdog_res_capability= {.version = 0x10000, \
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

/*watchdog driver interface*/
static const wdg_drv_controller_interface_t s_WdgDrvInterface =
{
    wdg_get_default_config,
    wdg_set_timeout,
    wdg_set_window_limit,
    wdg_set_seq_delta,
    wdg_refesh_mechanism_select,
    wdg_get_refesh_mechanism,
    wdg_init,
    wdg_deInit,
    wdg_set_testmode_config,
    wdg_enable,
    wdg_disable,
    wdg_enable_Interrupts,
    wdg_disable_Interrupts,
    wdg_get_status_flag,
    wdg_clear_status_flag,
    wdg_refresh,
    wdg_get_reset_cnt,
    wdg_get_ext_reset_cnt,
    wdg_clear_reset_cnt,
    wdg_clear_ext_reset_cnt,
    wdg_Int_register,
    wdg_int_unregister,
    wdg_int_clear,
    wdg_halt_enable,
    wdg_halt_disable,
    wdg_get_cnt,
    wdg_set_reset,
};

//*****************************************************************************
//
//! hal_wdg_get_controller_interface.
//!
//! \param controllerTable is watchdog interface ptr
//!
//! This function get watchdog driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_wdg_get_controller_interface(const wdg_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_WdgDrvInterface;
}

//*****************************************************************************
//
//! hal_wdg_get_instance.
//!
//! \void.
//!
//! This function get watchdog instance hand.
//!
//! \return watchdog hanle
//
//*****************************************************************************
static wdg_instance_t *hal_wdg_get_instance(uint32_t wdg_res_glb_idx)
{
    uint8_t i = 0;
    int8_t ret = 0;
    paddr_t phy_addr = 0;
    int32_t wdg_really_num = 0;
    spin_lock_saved_state_t states;

    ret = res_get_info_by_id(wdg_res_glb_idx,&phy_addr,&wdg_really_num);

    if(ret == -1){
        //can not find res in current domain res
        return NULL;
    }

    spin_lock_irqsave(&wdg_spin_lock, states);

    for (i=0; i < wdg_really_num_max; i++)
    {
        if (g_WdgInstance[i].occupied != 1)
        {
            uint8_t *buffer = (uint8_t *)&g_WdgInstance[i];
            memset(buffer,0,sizeof(wdg_instance_t));

            /* get watchdog driver API table */
            hal_wdg_get_controller_interface(&(g_WdgInstance[i].controllerTable));
            if(g_WdgInstance[i].controllerTable){
                g_WdgInstance[i].occupied = 1;
                g_WdgInstance[i].controllerTable->get_default_config(&(g_WdgInstance[i].wdg_cfg));
                g_WdgInstance[i].wdg_res.wdg_really_num = wdg_really_num;
                g_WdgInstance[i].wdg_res.wdg_phy_addr = phy_addr;
                g_WdgInstance[i].wdg_cfg.wdg_reset_cfg.enableSysReset = g_watchdog_res_capability.res_cap[wdg_really_num-1].enableIntReset;
                g_WdgInstance[i].wdg_cfg.wdg_ext_reset_cfg.enableSysExtReset = g_watchdog_res_capability.res_cap[wdg_really_num-1].enableExtReset;
                spin_unlock_irqrestore(&wdg_spin_lock, states);
                return &g_WdgInstance[i];
            }
        }
    }
    spin_unlock_irqrestore(&wdg_spin_lock, states);
    return NULL;
}
//*****************************************************************************
//
//! hal_wdg_release_instance.
//!
//! \void.
//!
//! This function release watchdog instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_wdg_release_instance(wdg_instance_t *wdgInstance)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&wdg_spin_lock, states);
    wdgInstance->occupied = 0;
    spin_unlock_irqrestore(&wdg_spin_lock, states);
}

//*****************************************************************************
//
//! hal_wdg_creat_handle.
//!
//! \handle watchdog handle for wdg func.
//! \wdg_res_glb_idx watchdog globale index
//!
//! This function get hal handle.
//!
//! \return watchdog handle
//
//*****************************************************************************
bool hal_wdg_creat_handle(void **handle,uint32_t wdg_res_glb_idx)
{
    wdg_instance_t  *wdgInstance = NULL;

    if(wdg_spin_lock !=SPIN_LOCK_INITIAL_VALUE){
        spin_lock_init(&wdg_spin_lock);
    }

    wdgInstance = hal_wdg_get_instance(wdg_res_glb_idx);
    if(wdgInstance == NULL){
        return false;
    }
#if 0
    mutex_init(&wdgInstance->wdgMutex);
#endif

    *handle = wdgInstance;
    return true;
}

//*****************************************************************************
//
//! hal_wdg_release_handle.
//!
//! \void.
//!
//! This function delete watchdog instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_wdg_release_handle(void *handle)
{
    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;
    l_wdgInstance->occupied = 0;

#if 0
    mutex_destroy(&l_wdgInstance->wdgMutex);
#endif

    return true;
}

//*****************************************************************************
//
//! hal_wdg_init.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog used wdg_app_cfg parameter init.
//!
//! \return bool status
//
//*****************************************************************************
#if ENABLE_SD_WDG_INIT
bool hal_wdg_init(void *handle,wdg_app_config_t *wdg_app_cfg)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;
    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_init has inited\n");
        return false;
    }

    l_wdgInstance->wdg_cfg.wdg_refresh_config.wdgModeSelect = wdg_app_cfg->workMode;
    l_wdgInstance->wdg_cfg.wdg_timeout = wdg_app_cfg->timeoutValue;
    l_wdgInstance->wdg_cfg.refresh_wind_limit = wdg_app_cfg->windowLimitValue;
    l_wdgInstance->wdg_cfg.refresh_seq_delta = wdg_app_cfg->seqDeltaValue;
#if 0
    l_wdgInstance->wdg_cfg.wdg_ctrl_config.clockSource = wdg_app_cfg->clocksource;
    l_wdgInstance->wdg_cfg.wdg_ctrl_config.prescaler = wdg_app_cfg->divisor;
#endif

    if(l_wdgInstance->controllerTable->init){
        ret = l_wdgInstance->controllerTable->init((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg));
    }

    if(!ret){
        LTRACEF("hal_wdg_init failed\n");
        return false;
    }

    l_wdgInstance->wdg_inited = true;

    return true;
}
#else
bool hal_wdg_init(void *handle,wdg_app_config_t *wdg_app_cfg)
{
    return true;
}
#endif
//*****************************************************************************
//
//! hal_wdg_deinit.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog deinit.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_deinit(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_init has not inited\n");
        return false;
    }

    if(l_wdgInstance->controllerTable->deinit){
        ret = l_wdgInstance->controllerTable->deinit((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_deinit failed\n");
        return false;
    }

    //release wdg instance
    hal_wdg_release_instance(l_wdgInstance);

    l_wdgInstance->wdg_inited = false;

    return true;
}

//*****************************************************************************
//
//! hal_wdg_set_test_mode.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog used enable_selftest parameter set test mode enable.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_enable_selftest(void *handle,bool enable_selftest)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited || !l_wdgInstance->wdg_cfg.wdg_ctrl_config.enableSelftest){
        LTRACEF("hal_wdg_enable_selftest does not inited or enabled\n");
        return false;
    }

    l_wdgInstance->wdg_cfg.wdg_ctrl_config.enableSelftest = enable_selftest;

    if(l_wdgInstance->controllerTable->set_testmode_config){
        ret = l_wdgInstance->controllerTable->set_testmode_config((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg));
    }

    if(!ret){
        LTRACEF("hal_wdg_init failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_enable.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog enbale.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_enable(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if((l_wdgInstance->wdg_inited == false) || (l_wdgInstance->wdg_enabled == true)){
        LTRACEF("hal_wdg_init has not inited or enable wdg_inited:%d  wdg_enabled:%d\n",l_wdgInstance->wdg_inited,l_wdgInstance->wdg_enabled);
        return false;
    }

    if(l_wdgInstance->controllerTable->enable){
        ret = l_wdgInstance->controllerTable->enable((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_enable failed\n");
        return false;
    }

    l_wdgInstance->wdg_enabled = true;

    return true;
}

//*****************************************************************************
//
//! hal_wdg_disable.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog disabale.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_disable(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if((l_wdgInstance->wdg_inited == false) || (l_wdgInstance->wdg_enabled == false)){
        LTRACEF("hal_wdg_init has not inited or disabled wdg_inited:%d  wdg_enabled:%d\n",l_wdgInstance->wdg_inited,l_wdgInstance->wdg_enabled);
        return false;
    }

    if(l_wdgInstance->controllerTable->disable){
        ret = l_wdgInstance->controllerTable->disable((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_enable failed\n");
        return false;
    }

    l_wdgInstance->wdg_enabled = false;

    return true;
}

//*****************************************************************************
//
//! hal_wdg_enable_interrupts.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog enable interrupts.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_enable_interrupts(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_enable_interrupts has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->enable_interrupts){
        ret = l_wdgInstance->controllerTable->enable_interrupts((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_enable_interrupts failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_disable_interrupts.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog disable interrupts.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_disable_interrupts(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_disable_interrupts has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->disable_interrupts){
        ret = l_wdgInstance->controllerTable->disable_interrupts((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_disable_interrupts failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_get_status_flags.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for get watchdog enable and interrupt status.
//!
//! \return watchdog enable and interrupt status flags
//
//*****************************************************************************
uint32_t hal_wdg_get_status_flags(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_get_status_flags has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return 0;
    }

    if(l_wdgInstance->controllerTable->get_status_flag){
        ret = l_wdgInstance->controllerTable->get_status_flag((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_get_status_flags failed\n");
        return 0;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_wdg_clear_status_flags.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for gclear watchdog enable and interrupt status.
//!
//! \return watchdog enable and interrupt status flags
//
//*****************************************************************************
bool hal_wdg_clear_status_flags(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_get_status_flags has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->clear_status_flag){
        ret = l_wdgInstance->controllerTable->clear_status_flag((wdg_reg_type_t *)wdg_base_addr,0xffffffff);
    }

    if(!ret){
        LTRACEF("hal_wdg_get_status_flags failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_set_timeout.
//!
//! \handle watchdog handle for wdg func.
//! \timeout_ms watchdog timeout ms.
//!
//! This function is for set watchdog timeout value.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_set_timeout(void *handle,uint32_t timeout_ms)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_set_timeout does not inited\n");
        return false;
    }

    if(l_wdgInstance->controllerTable->set_timeout){
        ret = l_wdgInstance->controllerTable->set_timeout((wdg_reg_type_t *)wdg_base_addr,timeout_ms);
    }

    if(!ret){
        LTRACEF("hal_wdg_set_timeout failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_set_windowvalue.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for set watchdog window limit value.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_set_windowvalue(void *handle,uint32_t window_timeout_ms)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_set_windowvalue does not inited\n");
        return false;
    }

    if(l_wdgInstance->controllerTable->set_window_limit){
        ret = l_wdgInstance->controllerTable->set_window_limit((wdg_reg_type_t *)wdg_base_addr,window_timeout_ms);
    }

    if(!ret){
        LTRACEF("hal_wdg_set_windowvalue failed\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! hal_wdg_set_seqdelta.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for set watchdog window limit value.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_set_seqdelta(void *handle,uint32_t seq_delta_timeout_ms)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_set_seqdelta does not inited\n");
        return false;
    }

    if(l_wdgInstance->controllerTable->set_seq_delta){
        ret = l_wdgInstance->controllerTable->set_seq_delta((wdg_reg_type_t *)wdg_base_addr,seq_delta_timeout_ms);
    }

    if(!ret){
        LTRACEF("hal_wdg_set_seqdelta failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_refresh.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for set watchdog refresh.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_refresh(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!(l_wdgInstance->wdg_inited) || !(l_wdgInstance->wdg_enabled)){
        LTRACEF("hal_wdg_refresh does not inited or not enable wdg_inited:%d  wdg_enabled:%d\n",l_wdgInstance->wdg_inited,l_wdgInstance->wdg_enabled);
        return false;
    }

    if(l_wdgInstance->controllerTable->refresh){
        ret = l_wdgInstance->controllerTable->refresh((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_refresh failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_get_reset_count.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for get watchdog reset count.
//!
//! \return watchdog reset count
//
//*****************************************************************************
uint32_t hal_wdg_get_reset_count(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_get_reset_count has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return 0;
    }

    if(l_wdgInstance->controllerTable->get_reset_cnt){
        ret = l_wdgInstance->controllerTable->get_reset_cnt((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_get_reset_count failed\n");
        return 0;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_wdg_get_ext_reset_count.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for get watchdog external reset count.
//!
//! \return watchdog reset count
//
//*****************************************************************************
uint32_t hal_wdg_get_ext_reset_count(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_get_reset_count has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return 0;
    }

    if(l_wdgInstance->controllerTable->get_ext_reset_cnt){
        ret = l_wdgInstance->controllerTable->get_ext_reset_cnt((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_get_reset_count failed\n");
        return 0;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_wdg_clear_reset_count.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for clear watchdog reset count.
//!
//! \return watchdog reset count
//
//*****************************************************************************
bool hal_wdg_clear_reset_count(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_clear_reset_count has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->clear_reset_cnt){
        ret = l_wdgInstance->controllerTable->clear_reset_cnt((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_clear_reset_count failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_clear_ext_reset_count.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for clear watchdog external reset count.
//!
//! \return watchdog reset count
//
//*****************************************************************************
bool hal_wdg_clear_ext_reset_count(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_clear_ext_reset_count has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->clear_ext_reset_cnt){
        ret = l_wdgInstance->controllerTable->clear_ext_reset_cnt((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_clear_ext_reset_count failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_int_register.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for register int callback func.
//!
//! \return watchdog reset count
//
//*****************************************************************************
bool hal_wdg_int_register(void *handle,int_handler call_func,bool overflow_int)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_int_register has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->int_register){
        ret = l_wdgInstance->controllerTable->int_register(handle,(wdg_reg_type_t *)wdg_base_addr,call_func,overflow_int);
    }

    if(!ret){
        LTRACEF("hal_wdg_int_register failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_int_unregister.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for unregister int callback func.
//!
//! \return watchdog reset count
//
//*****************************************************************************
bool hal_wdg_int_unregister(void *handle,bool overflow_int)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_int_unregister has not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->int_unregister){
        ret = l_wdgInstance->controllerTable->int_unregister((wdg_reg_type_t *)wdg_base_addr,overflow_int);
    }

    if(!ret){
        LTRACEF("hal_wdg_int_unregister failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_int_clear.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for clear watchdog int.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_int_clear(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_int_clear does not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->int_clear){
        ret = l_wdgInstance->controllerTable->int_clear((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_int_clear failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_halt_enable.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog halt enable.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_halt_enable(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_halt_enable does not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->halt_enable){
        ret = l_wdgInstance->controllerTable->halt_enable((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_halt_enable failed\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! hal_wdg_halt_disable.
//!
//! \handle watchdog handle for wdg func.
//!
//! This function is for watchdog halt disable.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_wdg_halt_disable(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(!l_wdgInstance->wdg_inited){
        LTRACEF("hal_wdg_halt_disable does not inited wdg_inited:%d\n",l_wdgInstance->wdg_inited);
        return false;
    }

    if(l_wdgInstance->controllerTable->halt_disable){
        ret = l_wdgInstance->controllerTable->halt_disable((wdg_reg_type_t *)wdg_base_addr);
    }

    if(!ret){
        LTRACEF("hal_wdg_halt_disable failed\n");
        return false;
    }

    return true;
}


uint32_t hal_wdg_get_cnt(void *handle)
{
    uint32_t ret = false;
    vaddr_t wdg_base_addr = 0x0;

    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->controllerTable->get_cnt){
        ret = l_wdgInstance->controllerTable->get_cnt((wdg_reg_type_t *)wdg_base_addr);
    }
    return ret;
}

bool hal_wdg_set_reset(void *handle)
{
    bool ret = false;
    vaddr_t wdg_base_addr = 0x0;
    wdg_instance_t *l_wdgInstance = NULL;

    HAL_WDG_ASSERT_PARAMETER(handle);

    l_wdgInstance = (wdg_instance_t *)handle;

    wdg_base_addr = (vaddr_t)_ioaddr(l_wdgInstance->wdg_res.wdg_phy_addr);

    if(l_wdgInstance->wdg_inited == false){
        LTRACEF("hal_wdg_init has not inited\n");
        return false;
    }

    if(l_wdgInstance->controllerTable->set_reset){
        ret = l_wdgInstance->controllerTable->set_reset((wdg_reg_type_t *)wdg_base_addr,&(l_wdgInstance->wdg_cfg));
    }

    if(!ret){
        LTRACEF("hal_wdg_set_reset failed\n");
        return false;
    }

    return true;
}