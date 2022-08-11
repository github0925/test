//*****************************************************************************
//
// wdg_hal.h - Prototypes for the Watchdog hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __WDG_HAL_H__
#define __WDG_HAL_H__
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
#include "__regs_base.h"
#if ENABLE_SD_WDG
#include "wdg_drv.h"
#endif
#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include "chip_res.h"
#include "system_cfg.h"

#define SDV_WDG_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define DEFAULT_WDG_MAX_NUM  8

// Check the arguments.
#define HAL_WDG_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n",handle);    \
    return false;   \
}   \

/*This section defines the fresh mode select*/
typedef enum _wdg_mode
{
    wdg_mode1 = 0x1U, /*!< normal refresh mode */
    wdg_mode2 = 0x2U, /*!< window limit refresh mode */
    wdg_mode3 = 0x3U, /*!< seq delta refresh mode */
    wdg_mode_max = 0x3U, /*!< seq delta refresh mode */
} wdg_mode_t;

/*watchdog global index to really number */
typedef struct _wdg_glb_idx_to_id
{
    uint32_t res_glb_idx;
    uint32_t wdg_really_num;
} wdg_glb_idx_to_id;

#if ENABLE_SD_WDG
/*wdg driver interface structure */
typedef struct _wdg_drv_controller_interface
{
    void (*get_default_config)(wdg_config_t *wdg_config);
    bool (*set_timeout)(wdg_reg_type_t *base,uint32_t timeout_ms);
    bool (*set_window_limit)(wdg_reg_type_t *base,uint32_t window_timeout_ms);
    bool (*set_seq_delta)(wdg_reg_type_t *base,uint32_t seq_delta_timeout_ms);
    bool (*refesh_mechanism_select)(wdg_reg_type_t *base,const wdg_config_t *wdg_config);
    uint32_t (*get_refesh_mechanism)(wdg_reg_type_t *base);
    bool (*init)(wdg_reg_type_t *base,const wdg_config_t *wdg_config);
    bool (*deinit)(wdg_reg_type_t *base);
    bool (*set_testmode_config)(wdg_reg_type_t *base, const wdg_config_t *wdg_config);
    bool (*enable)(wdg_reg_type_t *base);
    bool (*disable)(wdg_reg_type_t *base);
    bool (*enable_interrupts)(wdg_reg_type_t *base);
    bool (*disable_interrupts)(wdg_reg_type_t *base);
    uint32_t (*get_status_flag)(wdg_reg_type_t *base);
    bool (*clear_status_flag)(wdg_reg_type_t *base,uint32_t mask);
    bool (*refresh)(wdg_reg_type_t *base);
    uint32_t (*get_reset_cnt)(wdg_reg_type_t *base);
    uint32_t (*get_ext_reset_cnt)(wdg_reg_type_t *base);
    bool (*clear_reset_cnt)(wdg_reg_type_t *base);
    bool (*clear_ext_reset_cnt)(wdg_reg_type_t *base);
    bool (*int_register)(void *handle,wdg_reg_type_t *base,int_handler call_func,bool overflow_int);
    bool (*int_unregister)(wdg_reg_type_t *base,bool overflow_int);
    bool (*int_clear)(wdg_reg_type_t *base);
    bool (*halt_enable)(wdg_reg_type_t *base);
    bool (*halt_disable)(wdg_reg_type_t *base);
    uint32_t (*get_cnt)(wdg_reg_type_t *base);
    bool (*set_reset)(wdg_reg_type_t *base,const wdg_config_t *wdg_config);
} wdg_drv_controller_interface_t;
#endif

/*resource excel version*/
typedef struct
{
    #if ENABLE_SD_WDG
    wdg_really_num_t wdg_really_num;
    #endif
    paddr_t wdg_phy_addr;//WDG1~WDG8  address
    bool enableIntReset;             /*!< Enables or disables WDG interrupt reset*/
    bool enableExtReset;             /*!< Enables or disables WDG extrnal reset for pmic etc */
} wdg_res_config_t;

/*Describes wdg configuration structure for app. */
typedef struct _wdg_app_config
{
    wdg_mode_t workMode;        /*!< Configures WDG work mode in debug stop and wait mode */
    uint32_t timeoutValue;            /*!< Timeout value */
    uint32_t windowLimitValue;             /*!< Window value */
    uint32_t seqDeltaValue;             /*!< seq mode delta value */
    uint32_t clocksource;
    uint32_t divisor;
} wdg_app_config_t;

/*wdg instance */
typedef struct _wdg_instance
{
    #if ENABLE_SD_WDG
    wdg_config_t wdg_cfg;   /*!< watchdog config*/
    #endif
    mutex_t wdgMutex;   /*!< wdg layer mutex*/
    #if ENABLE_SD_WDG
    const wdg_drv_controller_interface_t *controllerTable;  /*!< wdg driver interface*/
    #endif
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
    wdg_res_config_t wdg_res;
    bool wdg_inited;
    bool wdg_enabled;
} wdg_instance_t;

typedef struct
{
    uint32_t res_glb_idx;
    bool enableIntReset;
    bool enableExtReset;
} wdg_capability_cfg_t;

typedef struct
{
    uint32_t version;
    char res_category[20];
    uint8_t res_max_num;
    wdg_capability_cfg_t res_cap[DEFAULT_WDG_MAX_NUM];
} wdg_capability_t;

bool hal_wdg_creat_handle(void **handle,uint32_t wdg_res_glb_idx);
bool hal_wdg_release_handle(void *handle);
bool hal_wdg_init(void *handle,wdg_app_config_t *wdg_app_cfg);
bool hal_wdg_deinit(void *handle);
bool hal_wdg_enable_selftest(void *handle,bool enable_selftest);
bool hal_wdg_enable(void *handle);
bool hal_wdg_disable(void *handle);
bool hal_wdg_enable_interrupts(void *handle);
bool hal_wdg_disable_interrupts(void *handle);
bool hal_wdg_clear_status_flags(void *handle);
bool hal_wdg_set_timeout(void *handle,uint32_t timeout_ms);
bool hal_wdg_set_windowvalue(void *handle,uint32_t window_timeout_ms);
bool hal_wdg_set_seqdelta(void *handle,uint32_t seq_delta_timeout_ms);
bool hal_wdg_refresh(void *handle);
bool hal_wdg_clear_reset_count(void *handle);
bool hal_wdg_clear_ext_reset_count(void *handle);
bool hal_wdg_int_register(void *handle,int_handler call_func,bool overflow_int);
bool hal_wdg_int_unregister(void *handle,bool overflow_int);
bool hal_wdg_int_clear(void *handle);
bool hal_wdg_halt_enable(void *handle);
bool hal_wdg_halt_disable(void *handle);
uint32_t hal_wdg_get_status_flags(void *handle);
uint32_t hal_wdg_get_reset_count(void *handle);
uint32_t hal_wdg_get_ext_reset_count(void *handle);
uint32_t hal_wdg_get_cnt(void* handle);
bool hal_wdg_set_reset(void *handle);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __WDG_IP_TEST_H__

