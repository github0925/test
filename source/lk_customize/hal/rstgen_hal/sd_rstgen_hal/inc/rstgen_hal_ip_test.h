//*****************************************************************************
//
// rstgen_hal_ip_test.h - Prototypes for the Watchdog ip test hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __RSTGEN_IP_TEST_H__
#define __RSTGEN_IP_TEST_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include "rstgen_hal.h"
//*****************************************************************************
//
// This section defines the HAL log level
// WATCHDOG component
//
//*****************************************************************************
#define DEFAULT_HAL_RSTGEN_LOG_LEVEL	1

/*rstgen driver interface structure */
typedef struct _rstgen_test_drv_controller_interface
{
    void (*get_default_config)(uint32_t *global_rst_maks);
    bool (*dump_all_reg_for_test)(vaddr_t base,uint32_t core_idx,uint32_t module_idx,uint32_t iso_idx);
    bool (*core_readonlyreg_check_test)(vaddr_t base,uint32_t core_idx);
    bool (*module_readonlyreg_check_test)(vaddr_t base,uint32_t module_idx);
    bool (*global_rw_reg_check_test)(vaddr_t base);
    bool (*core_rw_reg_check_test)(vaddr_t base,uint32_t core_idx);
    bool (*module_rw_reg_check_test)(vaddr_t base,uint32_t module_idx);
    bool (*iso_rw_reg_check_test)(vaddr_t base,uint32_t iso_idx);
    bool (*general_rw_reg_check_test)(vaddr_t base,uint32_t general_idx);
    bool (*module_rst_test)(vaddr_t base,uint32_t module_idx);
    bool (*init)(vaddr_t base,const uint32_t global_rst_maks);
    bool (*global_rst_enable)(vaddr_t base,uint32_t mask);
    bool (*global_rst_disable)(vaddr_t base,uint32_t mask);
    bool (*sw_self_rst)(vaddr_t base,bool rst_release);
    bool (*sw_oth_rst)(vaddr_t base,bool rst_release);
    uint32_t (*get_rst_sta)(vaddr_t base);
    bool (*iso_enable)(vaddr_t base,uint32_t iso_idx);
    bool (*iso_disable)(vaddr_t base,uint32_t iso_idx);
    bool (*core_rst_enable)(vaddr_t base,uint32_t core_idx,uint32_t mask);
    bool (*core_rst_disable)(vaddr_t base,uint32_t core_idx,uint32_t mask);
    bool (*core_reset)(vaddr_t base,uint32_t core_idx);
    bool (*module_reset)(vaddr_t base,uint32_t module_idx,bool rst_release);
    bool (*clear_rst_sta)(vaddr_t base);
} rstgen_test_drv_controller_interface_t;

/*rstgen instance */
typedef struct _rstgen_test_instance
{
    uint32_t global_rst_maks;   /*!< rstgen global reset mask*/
    mutex_t rstgenMutex;   /*!< rstgen layer mutex*/
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
    bool rstgen_inited;
    paddr_t phy_addr;
    rstgen_res_config_t rstgen_res[2];
    const rstgen_test_drv_controller_interface_t *controllerTable;  /*!< rstgen driver interface*/
} rstgen_test_instance_t;

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
bool hal_rstgen_test_creat_handle(void **handle,uint32_t global_rst_res_idx);
bool hal_rstgen_test_release_handle(void *handle);
bool hal_rstgen_core_readonlyreg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_module_readonlyreg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_global_rw_reg_check_test(void *handle);
bool hal_rstgen_core_rw_reg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_module_rw_reg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_iso_rw_reg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_general_rw_reg_check_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_global_sw_self_rst_test(void *handle);
bool hal_rstgen_global_sem_rst_test(void *handle);
bool hal_rstgen_global_dbg_rst_test(void *handle);
bool hal_rstgen_global_wdg_rst_test(void *handle,uint8_t watchdog_really_num);
bool hal_rstgen_global_pre_rst_test(void *handle);
bool hal_rstgen_self_rst_test(void *handle);
bool hal_rstgen_other_rst_test(void *handle);
bool hal_rstgen_core_wdg_rst_test(void *handle,uint8_t watchdog_really_num,uint32_t res_glb_idx);
bool hal_rstgen_core_dbg_rst_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_core_rst_test(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_module_rst_test(void *handle,uint32_t res_glb_idx);
#ifdef __cplusplus
}
#endif
#endif // __RSTGEN_IP_TEST_H__

