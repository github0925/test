//*****************************************************************************
//
// wdg_hal_ip_test.h - Prototypes for the Watchdog ip test hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __WDG_IP_TEST_H__
#define __WDG_IP_TEST_H__
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
#include "wdg_hal.h"
//*****************************************************************************
//
// This section defines the HAL log level
// WATCHDOG component
//
//*****************************************************************************
#ifdef LOCAL_TRACE
#undef LOCAL_TRACE
#endif
#define LOCAL_TRACE 1
#define DEFAULT_HAL_WDG_LOG_LEVEL	1

#if ENABLE_SD_WDG
/*wdg driver interface structure */
typedef struct _wdg_test_drv_controller_interface
{
    void (*get_default_config)(wdg_config_t *wdg_config);
    bool (*readonlyreg_check_test)(wdg_reg_type_t* base);
    bool (*rw_reg_check_test)(wdg_reg_type_t* base);
    bool (*self_test)(wdg_reg_type_t* base,const wdg_config_t *wdg_config);
    bool (*terminal_test)(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
    bool (*terminal_from_fuse_test)(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
    bool (*reset_control_restart_test)(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
    bool (*reset_control_donot_restart_test)(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode1_refresh_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode2_refresh_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode3_refresh_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode2_window_reset_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode2_1_refresh_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode3_2_1_refresh_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*mode1_overflow_int_check_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
    bool (*debug_mode_test)(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
} wdg_test_drv_controller_interface_t;
#endif
/*wdg instance */
typedef struct _wdg_test_instance
{
    #if ENABLE_SD_WDG
    wdg_config_t wdg_cfg;   /*!< watchdog config*/
    #endif
    mutex_t wdgMutex;   /*!< wdg layer mutex*/
    #if ENABLE_SD_WDG
    const wdg_test_drv_controller_interface_t *controllerTable;  /*!< wdg driver interface*/
    #endif
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
    wdg_res_config_t wdg_res;
    bool wdg_inited;
    bool wdg_enabled;
} wdg_test_instance_t;

bool hal_wdg_test_creat_handle(void **handle,uint32_t wdg_res_glb_idx);
bool hal_wdg_test_delete_handle(void *handle);
bool hal_wdg_read_only_reg_test(void *handle);
bool hal_wdg_rw_reg_test(void *handle);
bool hal_wdg_self_test(void *handle);
bool hal_wdg_terminal_test(void *handle,uint32_t timeout);
bool hal_wdg_terminal_from_fuse_test(void *handle,uint32_t timeout);
bool hal_wdg_reset_control_restart_test(void *handle,uint32_t timeout);
bool hal_wdg_reset_control_not_restart_test(void *handle,uint32_t timeout);
bool hal_wdg_mode1_refresh_test(void *handle,uint32_t timeout);
bool hal_wdg_mode2_refresh_test(void *handle,uint32_t timeout);
bool hal_wdg_mode3_refresh_test(void *handle,uint32_t timeout);
bool hal_wdg_mode2_window_reset_test(void *handle,uint32_t timeout);
bool hal_wdg_mode2_1_refresh_test(void *handle,uint32_t timeout);
bool hal_wdg_mode3_2_1_refresh_test(void *handle,uint32_t timeout);
bool hal_wdg_mode1_overflow_intcheck_test(void *handle,uint32_t timeout);
bool hal_wdg_debug_mode_test(void *handle,uint32_t timeout);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __WDG_IP_TEST_H__

