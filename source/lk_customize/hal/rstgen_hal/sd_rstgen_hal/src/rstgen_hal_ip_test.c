//*****************************************************************************
//
// rstgen_ip_test_hal.c - Driver for the rstgen hal Module.
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
#include "rstgen_drv.h"
#include "rstgen_drv_test.h"
#include "rstgen_hal_ip_test.h"
#include "rstgen_hal.h"
#if ENABLE_SD_WDG
#include "wdg_hal.h"
#endif
#include "system_cfg.h"
#include "res.h"
#include "chip_res.h"

#if ENABLE_SD_WDG
static wdg_app_config_t wdg_app_cfg;
static void *watchdog_handle;
#endif

#define WDG_REALLY_NUM_MAX 0x8U

#if defined (rstgen_res_capability_def)
static rstgen_capability_t g_rstgen_test_res_capability= rstgen_res_capability_def;
#else
static rstgen_capability_t g_rstgen_test_res_capability= {.version = 0x10000,
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
static rstgen_test_instance_t g_RstgenTestInstance = {
    .occupied = 0,
};

spin_lock_t rstgen_test_spin_lock = SPIN_LOCK_INITIAL_VALUE;

/*rstgen driver interface*/
static const rstgen_test_drv_controller_interface_t s_RstgenTestDrvInterface = \
{
    rstgen_get_default_config,
    rstgen_dump_all_reg_for_test,
    rstgen_core_readonlyreg_check_test,
    rstgen_module_readonlyreg_check_test,
    rstgen_global_rw_reg_check_test,
    rstgen_core_rw_reg_check_test,
    rstgen_module_rw_reg_check_test,
    rstgen_iso_rw_reg_check_test,
    rstgen_general_rw_reg_check_test,
    rstgen_module_rst_test,
    rstgen_init,
    rstgen_global_rst_enable,
    rstgen_global_rst_disable,
    rstgen_sw_self_rst,
    rstgen_sw_oth_rst,
    rstgen_get_rst_sta,
    rstgen_iso_enable,
    rstgen_iso_disable,
    rstgen_core_rst_enable,
    rstgen_core_rst_disable,
    rstgen_core_reset,
    rstgen_module_reset,
    rstgen_clear_rst_sta,
};

const wdg_glb_idx_to_id g_rstgen_test_wdg_glb_idx_to_id[DEFAULT_WDG_MAX_NUM] = {
    {RES_WATCHDOG_WDT1, wdg_really_num1},
    {RES_WATCHDOG_WDT2, wdg_really_num2},
    {RES_WATCHDOG_WDT3, wdg_really_num3},
    {RES_WATCHDOG_WDT4, wdg_really_num4},
    {RES_WATCHDOG_WDT5, wdg_really_num5},
    {RES_WATCHDOG_WDT6, wdg_really_num6},
    {RES_WATCHDOG_WDT7, wdg_really_num7},
    {RES_WATCHDOG_WDT8, wdg_really_num8},
};

//*****************************************************************************
//
//! hal_rstgen_test_get_controller_interface.
//!
//! \param controllerTable is rstgen interface ptr
//!
//! This function get rstgen driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_rstgen_test_get_controller_interface(const rstgen_test_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_RstgenTestDrvInterface;
}
//*****************************************************************************
//
//! hal_rstgen_test_get_instance.
//!
//! \void.
//!
//! This function get rstgen instance hand.
//!
//! \return rstgen hanle
//
//*****************************************************************************
static rstgen_test_instance_t *hal_rstgen_test_get_instance(void)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&rstgen_test_spin_lock, states);
    if (g_RstgenTestInstance.occupied != 1)
    {
        memset(&g_RstgenTestInstance,0,sizeof(rstgen_instance_t));

        /* get rstgen driver API table */
        hal_rstgen_test_get_controller_interface(&(g_RstgenTestInstance.controllerTable));
        if(g_RstgenTestInstance.controllerTable){
            g_RstgenTestInstance.occupied = 1;
            g_RstgenTestInstance.rstgen_res[0].glb_self_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_self_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_sem_rst_en =  g_rstgen_test_res_capability.res_cap[0].glb_sem_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_dbg_rst_en =  g_rstgen_test_res_capability.res_cap[0].glb_dbg_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg1_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg1_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg2_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg2_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg3_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg3_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg4_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg4_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg5_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg5_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg6_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg6_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg7_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg7_rst_en;
            g_RstgenTestInstance.rstgen_res[0].glb_wdg8_rst_en = g_rstgen_test_res_capability.res_cap[0].glb_wdg8_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_self_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_self_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_sem_rst_en =  g_rstgen_test_res_capability.res_cap[1].glb_sem_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_dbg_rst_en =  g_rstgen_test_res_capability.res_cap[1].glb_dbg_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg1_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg1_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg2_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg2_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg3_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg3_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg4_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg4_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg5_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg5_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg6_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg6_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg7_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg7_rst_en;
            g_RstgenTestInstance.rstgen_res[1].glb_wdg8_rst_en = g_rstgen_test_res_capability.res_cap[1].glb_wdg8_rst_en;
        }
    }
    spin_unlock_irqrestore(&rstgen_test_spin_lock, states);
    return &g_RstgenTestInstance;
}


//*****************************************************************************
//
//! hal_rstgen_test_creat_handle.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function get hal handle.
//!
//! \return rstgen handle
//
//*****************************************************************************
bool hal_rstgen_test_creat_handle(void **handle,uint32_t global_rst_res_idx)
{
    int32_t idx = -1;
    rstgen_test_instance_t  *rstgenInstance = NULL;

    if(handle == NULL){
        LTRACEF("hal_rstgen_test_creat_handle paramenter error handle:%p\n",handle);
        return false;
    }

    if(rstgen_test_spin_lock !=SPIN_LOCK_INITIAL_VALUE){
        spin_lock_init(&rstgen_test_spin_lock);
    }

    rstgenInstance = hal_rstgen_test_get_instance();
    if(rstgenInstance == NULL){
        return false;
    }

    if(res_get_info_by_id(global_rst_res_idx,&(rstgenInstance->phy_addr),&idx)){
        LTRACEF("hal_rstgen_test_creat_handle paramenter error global_rst_res_idx:%d\n",global_rst_res_idx);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_test_creat_handle phy_addr:0x%lx\n",rstgenInstance->phy_addr);

    mutex_init(&rstgenInstance->rstgenMutex);

    *handle = rstgenInstance;
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_test_release_handle.
//!
//! \void.
//!
//! This function delete rstgen instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_rstgen_test_release_handle(void *handle)
{
    rstgen_test_instance_t *l_rstgenInstance = NULL;

    if(handle == NULL){
        LTRACEF("hal_rstgen_test_release_handle paramenter error handle:%p\n",handle);
        return false;
    }

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    l_rstgenInstance->occupied = 0;
    mutex_destroy(&l_rstgenInstance->rstgenMutex);
    return true;
}
//*****************************************************************************
//case1.1 test1
//! hal_rstgen_core_readonlyreg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_readonlyreg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t core_idx = -1;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&core_idx)){
        LTRACEF("hal_rstgen_core_readonlyreg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_readonlyreg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((core_idx != -1) && (l_rstgenInstance->controllerTable->core_readonlyreg_check_test)){
        ret = l_rstgenInstance->controllerTable->core_readonlyreg_check_test(rstgen_base_addr,(uint32_t)core_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_readonlyreg_check_test success core_idx:%d,ret:%d\n",core_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_readonlyreg_check_test failed core_idx:%d,ret:%d\n",core_idx,ret);
    }

    return ret;
}

//*****************************************************************************
//case1.1   test2
//! hal_rstgen_module_readonlyreg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_readonlyreg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t module_idx = 0;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&module_idx)){
        LTRACEF("hal_rstgen_module_readonlyreg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_readonlyreg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((module_idx != -1) && (l_rstgenInstance->controllerTable->module_readonlyreg_check_test)){
        ret = l_rstgenInstance->controllerTable->module_readonlyreg_check_test(rstgen_base_addr,(uint32_t)module_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_readonlyreg_check_test success module_idx:%d,ret:%d\n",module_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_readonlyreg_check_test failed module_idx:%d,ret:%d\n",module_idx,ret);
    }

    return ret;
}
//*****************************************************************************
//case1.2 case1.3   test3
//! hal_rstgen_global_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_rw_reg_check_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_rw_reg_check_test success phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->global_rw_reg_check_test){
        ret = l_rstgenInstance->controllerTable->global_rw_reg_check_test(rstgen_base_addr);
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_rw_reg_check_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case1.2 case1.3   test4
//! hal_rstgen_core_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t core_idx = 0;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&core_idx)){
        LTRACEF("hal_rstgen_core_rw_reg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rw_reg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((core_idx != -1) && (l_rstgenInstance->controllerTable->core_rw_reg_check_test)){
        ret = l_rstgenInstance->controllerTable->core_rw_reg_check_test(rstgen_base_addr,(uint32_t)core_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rw_reg_check_test success core_idx:%d,ret:%d\n",core_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rw_reg_check_test failed core_idx:%d,ret:%d\n",core_idx,ret);
    }

    return ret;
}

//*****************************************************************************
//case1.2 case1.3   test5
//! hal_rstgen_module_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx module global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t module_idx = -1;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;
    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&module_idx)){
        LTRACEF("hal_rstgen_module_rw_reg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rw_reg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((module_idx != -1) && (l_rstgenInstance->controllerTable->module_rw_reg_check_test)){
        ret = l_rstgenInstance->controllerTable->module_rw_reg_check_test(rstgen_base_addr,module_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rw_reg_check_test success module_idx:%d,ret:%d\n",module_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rw_reg_check_test failed module_idx:%d,ret:%d\n",module_idx,ret);
    }

    return ret;
}

//*****************************************************************************
//case1.2 case1.3   test6
//! hal_rstgen_iso_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx iso global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t iso_idx = -1;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&iso_idx)){
        LTRACEF("hal_rstgen_iso_rw_reg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_iso_rw_reg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((iso_idx != -1) && (l_rstgenInstance->controllerTable->iso_rw_reg_check_test)){
        ret = l_rstgenInstance->controllerTable->iso_rw_reg_check_test(rstgen_base_addr,(uint32_t)iso_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_iso_rw_reg_check_test success iso_idx:%d,ret:%d\n",iso_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_iso_rw_reg_check_test failed iso_idx:%d,ret:%d\n",iso_idx,ret);
    }

    return ret;
}
//*****************************************************************************
//case1.4 case1.3   test7
//! hal_rstgen_general_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx general global resource index
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_general_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t general_idx = 0;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&general_idx)){
        LTRACEF("hal_rstgen_general_rw_reg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_general_rw_reg_check_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((general_idx != -1) && l_rstgenInstance->controllerTable->general_rw_reg_check_test){
        ret = l_rstgenInstance->controllerTable->general_rw_reg_check_test(rstgen_base_addr,(uint32_t)general_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_general_rw_reg_check_test success general_idx:%d,ret:%d\n",general_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_general_rw_reg_check_test failed general_idx:%d,ret:%d\n",general_idx,ret);
    }

    return ret;
}

//*****************************************************************************
//case1.5   test8
//! hal_rstgen_global_sw_self_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global software reset test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_sw_self_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sw_self_rst_test start\n");

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sw_self_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_self_sw_sta) == rstgen_glb_rst_self_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sw_self_rst_test success\n");
        ret = true;
    }else{
        if(l_rstgenInstance->controllerTable->global_rst_enable){
            ret = l_rstgenInstance->controllerTable->global_rst_enable(rstgen_base_addr,rstgen_glb_rst_self_rst_en);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "global_rst_enable ret:%d\n",ret);
        }

        if(l_rstgenInstance->controllerTable->sw_self_rst){
            ret = l_rstgenInstance->controllerTable->sw_self_rst(rstgen_base_addr,false);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "sw_self_rst ret:%d\n",ret);
            spin(10);
            ret = l_rstgenInstance->controllerTable->sw_self_rst(rstgen_base_addr,true);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "sw_self_rst ret:%d\n",ret);
            spin(10);
            ret = false;
        }
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sw_self_rst_test end ret:%d\n",ret);
    return ret;
}
//*****************************************************************************
//case1.6   test9
//! hal_rstgen_global_sem_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global software reset test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_sem_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sem_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_self_sw_sta) == rstgen_glb_rst_sem_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sem_rst_test success\n");
        ret = true;
    }else{
        if(l_rstgenInstance->controllerTable->global_rst_enable){
            ret = l_rstgenInstance->controllerTable->global_rst_enable(rstgen_base_addr,rstgen_glb_rst_sem_rst_en);
        }
        /*##################################*/
        //enable sem interrupt
        /*##################################*/

        spin(10);
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sem_rst_test end ret:%d\n",ret);
    return ret;
}
//*****************************************************************************
//case1.7   test10
//! hal_rstgen_global_dbg_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global debug reset test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_dbg_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_dbg_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_self_sw_sta) == rstgen_glb_rst_dbg_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_dbg_rst_test success\n");
        ret = false;
    }else{
        if(l_rstgenInstance->controllerTable->global_rst_enable){
            ret = l_rstgenInstance->controllerTable->global_rst_enable(rstgen_base_addr,rstgen_glb_rst_dbg_rst_en);
        }
        /*##################################*/
        //enable hardware interrupt
        /*##################################*/
        spin(10);
        ret= false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_sem_rst_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case1.8   test11
//! hal_rstgen_global_wdg_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global watchdog reset test. watchdog_really_num:1,2,3,4,5,6,7
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_wdg_rst_test(void *handle,uint8_t watchdog_really_num)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;
    uint32_t wdg_mask[WDG_REALLY_NUM_MAX] = {rstgen_glb_rst_saf_wdg1_rst_en,
                                                                    rstgen_glb_rst_sec_wdg2_rst_en,
                                                                    rstgen_glb_rst_sec_wdg3_rst_en,
                                                                    rstgen_glb_rst_sec_wdg4_rst_en,
                                                                    rstgen_glb_rst_sec_wdg5_rst_en,
                                                                    rstgen_glb_rst_sec_wdg6_rst_en,
                                                                    rstgen_glb_rst_sec_wdg7_rst_en};

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    if(handle == NULL){
        LTRACEF("hal_rstgen_global_wdg_rst_test error handle:%p\n",handle);
        return false;
    }

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test start watchdog_really_num:%d phy_addr:0x%lx\n",watchdog_really_num,l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
        l_rstgenInstance->controllerTable->dump_all_reg_for_test(rstgen_base_addr,0,0,0);
    }else{
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test rst_sta:0x%x\n",rst_sta);
    if((rst_sta & wdg_mask[watchdog_really_num-1]) == wdg_mask[watchdog_really_num-1]){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test success\n");
        ret = true;
    }else{
        if(l_rstgenInstance->controllerTable->global_rst_enable){
            ret = l_rstgenInstance->controllerTable->global_rst_enable(rstgen_base_addr,wdg_mask[watchdog_really_num-1]);
        }
#if ENABLE_SD_WDG
        //watchdog enable and wait timeout
        if(ret){
            bool wdg_ret = false;
            wdg_app_cfg.workMode = wdg_mode1;
            wdg_app_cfg.seqDeltaValue = 100; //ms
            wdg_app_cfg.timeoutValue = 1; //ms
            wdg_app_cfg.windowLimitValue = 1000;//ms
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test hal_wdg_creat_handle\n");
            wdg_ret = hal_wdg_creat_handle(&watchdog_handle,g_rstgen_test_wdg_glb_idx_to_id[watchdog_really_num-1].res_glb_idx);
            if(wdg_ret == true){
                LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test hal_wdg_init\n");
                wdg_ret = hal_wdg_init(watchdog_handle,&wdg_app_cfg);
            }

            if(wdg_ret == true){
                LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test hal_wdg_set_timeout\n");
                wdg_ret = hal_wdg_set_timeout(watchdog_handle,wdg_app_cfg.timeoutValue);//timeout 1ms
            }

            if(wdg_ret == true){
                LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test hal_wdg_enable\n");
                wdg_ret = hal_wdg_enable(watchdog_handle);
            }

            if(wdg_ret == true){
                vaddr_t wdg_base_addr = 0x0;
                wdg_base_addr = (vaddr_t)_ioaddr(((wdg_instance_t *)watchdog_handle)->wdg_res.wdg_phy_addr);
                wdg_ret = wdg_delay_timeout((wdg_reg_type_t*)wdg_base_addr,2*wdg_app_cfg.timeoutValue);
            }
            hal_wdg_disable(watchdog_handle);
            hal_wdg_release_handle(watchdog_handle);
            ret = false;
        }
#else
        ret = false;
#endif
    }
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case1.9   test12
//! hal_rstgen_global_pre_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global pre reset test,only run sec system
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_global_pre_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_pre_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta && (rstgen_base_addr == (vaddr_t)_ioaddr(APB_RSTGEN_SEC_BASE))){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
        l_rstgenInstance->controllerTable->dump_all_reg_for_test(rstgen_base_addr,0,0,0);
    }else{
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_pre_rst_test rst_sta:0x%x\n",rst_sta);
    if((rst_sta & rstgen_glb_rst_pre_sw_sta) == rstgen_glb_rst_pre_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_pre_rst_test success\n");
        ret = false;
    }else{
        vaddr_t rstgen_tmp_base_addr = 0x0;

        if(l_rstgenInstance->controllerTable->global_rst_enable){
            rstgen_tmp_base_addr = (vaddr_t)_ioaddr(APB_RSTGEN_SAF_BASE);
            ret = l_rstgenInstance->controllerTable->global_rst_enable(rstgen_tmp_base_addr,rstgen_glb_rst_self_rst_en);
            l_rstgenInstance->controllerTable->dump_all_reg_for_test(rstgen_tmp_base_addr,0,0,0);
        }
        //Only other reset can produce pre reset status.if you enable safety self global reset,sec rst status is only power reset,so register is zero
        if(ret && (l_rstgenInstance->controllerTable->sw_oth_rst)){
            ret = l_rstgenInstance->controllerTable->sw_oth_rst(rstgen_tmp_base_addr,false);
            spin(10);
            ret = l_rstgenInstance->controllerTable->sw_oth_rst(rstgen_tmp_base_addr,true);
        }
        spin(10);
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case2.0   test13
//! hal_rstgen_self_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen self reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_self_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_self_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta && (rstgen_base_addr == (vaddr_t)_ioaddr(APB_RSTGEN_SEC_BASE))){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_self_sw_sta) == rstgen_glb_rst_self_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_self_rst_test success\n");
        ret = false;
    }else{
        if(l_rstgenInstance->controllerTable->sw_self_rst){
            ret = l_rstgenInstance->controllerTable->sw_self_rst(rstgen_base_addr,false);
            spin(10);
            ret = l_rstgenInstance->controllerTable->sw_self_rst(rstgen_base_addr,true);
        }
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_self_rst_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case2.1   test14
//! hal_rstgen_othr_rst_test.
//!
//! \handle rstgen handle for rstgen func. only running sec system
//!
//! This function is for rstgen other reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_other_rst_test(void *handle)
{
    bool ret = false;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_other_rst_test phy_addr:0x%lx\n",l_rstgenInstance->phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(l_rstgenInstance->phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta && (rstgen_base_addr == (vaddr_t)_ioaddr(APB_RSTGEN_SEC_BASE))){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_pre_sw_sta) == rstgen_glb_rst_pre_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_other_rst_test success\n");
        ret = false;
    }else{
        if(l_rstgenInstance->controllerTable->sw_oth_rst){
            vaddr_t rstgen_saf_base_addr = 0x0;
            rstgen_saf_base_addr = (vaddr_t)_ioaddr(APB_RSTGEN_SAF_BASE);
            ret = l_rstgenInstance->controllerTable->sw_oth_rst(rstgen_saf_base_addr,false);
            spin(10);
            ret = l_rstgenInstance->controllerTable->sw_oth_rst(rstgen_saf_base_addr,true);
        }
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_other_rst_test end ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case2.2   test15
//! hal_rstgen_core_wdg_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core global resource index
//!
//! This function is for rstgen global watchdog reset test. watchdog_really_num:1,2,3,4,5,6,7
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_wdg_rst_test(void *handle,uint8_t watchdog_really_num,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t core_idx = -1;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;
    uint32_t wdg_mask[WDG_REALLY_NUM_MAX] = {rstgen_glb_rst_saf_wdg1_rst_en,
                                                                    rstgen_glb_rst_sec_wdg2_rst_en,
                                                                    rstgen_glb_rst_sec_wdg3_rst_en,
                                                                    rstgen_glb_rst_sec_wdg4_rst_en,
                                                                    rstgen_glb_rst_sec_wdg5_rst_en,
                                                                    rstgen_glb_rst_sec_wdg6_rst_en,
                                                                    rstgen_glb_rst_sec_wdg7_rst_en};

    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&core_idx)){
        LTRACEF("hal_rstgen_core_wdg_rst_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_wdg_rst_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    if((rst_sta & rstgen_glb_rst_saf_wdg1_sta) == rstgen_glb_rst_saf_wdg1_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test success\n");
        ret = false;
    }else{
        if(l_rstgenInstance->controllerTable->global_rst_disable){
            ret = l_rstgenInstance->controllerTable->global_rst_disable(rstgen_base_addr,wdg_mask[watchdog_really_num-1]);
        }

        if((core_idx != -1) && (l_rstgenInstance->controllerTable->core_rst_enable)){
            ret = l_rstgenInstance->controllerTable->core_rst_enable(rstgen_base_addr,(uint32_t)core_idx,(uint32_t)rstgen_core_wdg_rst_en);
        }

#if ENABLE_SD_WDG
        //watchdog enable and wait timeout
        if(ret){
            bool wdg_ret = false;
            wdg_app_cfg.workMode = wdg_mode1;
            wdg_app_cfg.seqDeltaValue = 100; //ms
            wdg_app_cfg.timeoutValue = 1; //ms
            wdg_app_cfg.windowLimitValue = 1000;//ms
            wdg_ret = hal_wdg_creat_handle(&watchdog_handle,g_rstgen_test_wdg_glb_idx_to_id[watchdog_really_num-1].res_glb_idx);
            if(wdg_ret == true){
                wdg_ret = hal_wdg_init(watchdog_handle,&wdg_app_cfg);
            }

            if(wdg_ret == true){
                wdg_ret = hal_wdg_set_timeout(watchdog_handle,wdg_app_cfg.timeoutValue);//timeout 1ms
            }

            if(wdg_ret == true){
                wdg_ret = hal_wdg_enable(watchdog_handle);
            }

            if(wdg_ret == true){
                vaddr_t wdg_base_addr = 0x0;
                wdg_base_addr = (vaddr_t)_ioaddr(((wdg_instance_t *)watchdog_handle)->wdg_res.wdg_phy_addr);
                wdg_ret = wdg_delay_timeout((wdg_reg_type_t*)wdg_base_addr,2*wdg_app_cfg.timeoutValue);
            }
            hal_wdg_disable(watchdog_handle);
            hal_wdg_release_handle(watchdog_handle);
            ret = false;
        }
#else
        ret = false;
#endif
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_global_wdg_rst_test end ret:%d\n",ret);
    return ret;
}
//*****************************************************************************
//case2.3   test16
//! hal_rstgen_core_dbg_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global dbg reset test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_dbg_rst_test(void *handle,uint32_t res_glb_idx)
{
    return true;
}
//*****************************************************************************
//case2.4   test17
//! hal_rstgen_core_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx core global resource index
//!
//! This function is for rstgen core reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_rst_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    paddr_t phy_addr = 0;
    int32_t core_idx = -1;
    vaddr_t rstgen_base_addr = 0x0;
    uint32_t rst_sta = 0;
    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&core_idx)){
        LTRACEF("hal_rstgen_general_rw_reg_check_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rst_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if(l_rstgenInstance->controllerTable->get_rst_sta){
        rst_sta = l_rstgenInstance->controllerTable->get_rst_sta(rstgen_base_addr);
    }else{
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ..............\n");
    l_rstgenInstance->controllerTable->dump_all_reg_for_test(rstgen_base_addr,0,0,0);
    if((rst_sta & rstgen_glb_rst_self_sw_sta) == rstgen_glb_rst_self_sw_sta){
        if(l_rstgenInstance->controllerTable->clear_rst_sta){
            ret = l_rstgenInstance->controllerTable->clear_rst_sta(rstgen_base_addr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "clear_rst_sta ret:%d\n",ret);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rst_test success\n");
        ret = false;
    }else{
        if((core_idx != -1) && (l_rstgenInstance->controllerTable->core_reset)){
            ret = l_rstgenInstance->controllerTable->core_reset(rstgen_base_addr,(uint32_t)core_idx);
        }
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_core_rst_test end core_idx:%d,ret:%d\n",core_idx,ret);
    return ret;
}

//*****************************************************************************
//case2.5   test18
//! hal_rstgen_module_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//! \res_glb_idx modulel resource index
//!
//! This function is for rstgen module reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_rst_test(void *handle,uint32_t res_glb_idx)
{
    bool ret = false;
    int32_t module_idx = -1;
    paddr_t phy_addr = 0;
    vaddr_t rstgen_base_addr = 0x0;
    rstgen_test_instance_t *l_rstgenInstance = NULL;

    ASSERT((handle != NULL));

    l_rstgenInstance = (rstgen_test_instance_t *)handle;

    if(res_get_info_by_id(res_glb_idx,&phy_addr,&module_idx)){
        LTRACEF("hal_rstgen_module_rst_test res_glb_idx:0x%x not find\n",res_glb_idx);
        return true;//return ret;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rst_test res_glb_idx:0x%x,phy_addr:0x%lx\n",res_glb_idx,phy_addr);

    rstgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if((module_idx != -1) && (l_rstgenInstance->controllerTable->module_rst_test)){
        ret = l_rstgenInstance->controllerTable->module_rst_test(rstgen_base_addr,module_idx);
    }

    if(ret){
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rst_test success module_idx:%d,ret:%d\n",module_idx,ret);
    }else{
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "hal_rstgen_module_rst_test failed module_idx:%d,ret:%d\n",module_idx,ret);
    }

    return ret;
}

