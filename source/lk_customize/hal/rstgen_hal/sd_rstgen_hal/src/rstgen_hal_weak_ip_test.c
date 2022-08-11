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
#include "rstgen_hal.h"
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
    return true;
}
//*****************************************************************************
//case1.1 test1
//! hal_rstgen_core_readonlyreg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_readonlyreg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.1   test2
//! hal_rstgen_module_readonlyreg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_readonlyreg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;
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
    return true;
}

//*****************************************************************************
//case1.2 case1.3   test4
//! hal_rstgen_core_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.2 case1.3   test5
//! hal_rstgen_module_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;

}

//*****************************************************************************
//case1.2 case1.3   test6
//! hal_rstgen_iso_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;

}
//*****************************************************************************
//case1.4 case1.3   test7
//! hal_rstgen_general_rw_reg_check_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_general_rw_reg_check_test(void *handle,uint32_t res_glb_idx)
{
    return true;

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
    return true;

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
    return true;

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
    return true;

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
    return true;

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
    return true;
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
    return true;

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
    return true;

}

//*****************************************************************************
//case2.2   test15
//! hal_rstgen_core_wdg_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen global watchdog reset test. watchdog_really_num:1,2,3,4,5,6,7
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_wdg_rst_test(void *handle,uint8_t watchdog_really_num,uint32_t res_glb_idx)
{
    return true;

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
//!
//! This function is for rstgen core reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_rst_test(void *handle,uint32_t res_glb_idx)
{
    return true;

}

//*****************************************************************************
//case2.5   test18
//! hal_rstgen_module_rst_test.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen module reset test
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_rst_test(void *handle,uint32_t res_glb_idx)
{
    return true;

}

