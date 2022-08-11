//*****************************************************************************
//
// wdg_drv_test.h - Prototypes for the Watchdog Timer API
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DW_WDG_TEST_H__
#define __DW_WDG_TEST_H__

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
#include "wdg_drv.h"

#define SEM_BASE    (vaddr_t)_ioaddr(APB_SEM1_BASE)
#define SCR_SEC_BASE    (vaddr_t)_ioaddr(APB_SCR_SEC_BASE)
#define SCR_SAF_BASE    (vaddr_t)_ioaddr(APB_SCR_SAF_BASE)

void wdg_test_get_default_config(wdg_config_t *wdg_config);
bool wdg_readonlyreg_check_test(wdg_reg_type_t* base);
bool wdg_rw_reg_check_test(wdg_reg_type_t* base);
bool wdg_self_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config);
bool wdg_terminal_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_terminal_from_fuse_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_reset_control_restart_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_reset_control_donot_restart_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode2_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode3_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode2_window_reset_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode2_1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode3_2_1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_mode1_overflow_int_check_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool wdg_debug_mode_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout);
bool dump_all_reg_for_test(wdg_reg_type_t *base);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __DW_WDG_H__
