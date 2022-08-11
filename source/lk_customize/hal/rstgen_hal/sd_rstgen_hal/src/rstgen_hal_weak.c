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
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include "rstgen_hal.h"
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
void hal_rstgen_release_instance(rstgen_instance_t *rstgenInstance)
{
    return;
}
//*****************************************************************************
//
//! hal_rstgen_creat_handle.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function get hal handle.
//!
//! \return rstgen handle
//
//*****************************************************************************
bool hal_rstgen_creat_handle(void **handle,uint32_t global_rst_res_idx)
{
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
bool hal_rstgen_init(void *handle)
{
    return true;
}
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
    return true;
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
    return true;
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
    return 0;
}

//*****************************************************************************
//
//! hal_rstgen_iso_enable.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen isolation enable
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_enable(void *handle,uint32_t res_glb_idx)
{
    return true;
}
//*****************************************************************************
//
//! hal_rstgen_iso_disable.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen isolation disable
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_iso_disable(void *handle,uint32_t res_glb_idx)
{
    return true;
}
//*****************************************************************************
////
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
    return 0;
}
//*****************************************************************************
//
//! hal_rstgen_core_reset.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen core reset
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_reset(void *handle,uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_module_reset.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen module reset
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_reset(void *handle,uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_module_status.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for querying rstgen module reset status
//!
//! \return status
//
//*****************************************************************************
uint32_t hal_rstgen_module_status(void *handle,uint32_t res_glb_idx)
{
    return 0;
}

//*****************************************************************************
//
//! hal_rstgen_core_status.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for querying rstgen core reset status
//!
//! \return status
//
//*****************************************************************************
uint32_t hal_rstgen_core_status(void *handle,uint32_t res_glb_idx)
{
    return 0;
}

//*****************************************************************************
//
//! hal_rstgen_module_ctl.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen module reset control
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_module_ctl(void *handle,uint32_t res_glb_idx, bool release)
{
    return true;
}

//*****************************************************************************
//
//! hal_rstgen_core_ctl.
//!
//! \handle rstgen handle for rstgen func.
//!
//! This function is for rstgen core reset control
//!
//! \return bool status
//
//*****************************************************************************
bool hal_rstgen_core_ctl(void *handle,uint32_t res_glb_idx, bool release)
{
    return true;
}
