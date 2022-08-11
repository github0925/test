//*****************************************************************************
//
// csi_hal_weak.c - Driver for the csi hal Module.
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
#include "csi_hal.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif


//*****************************************************************************
//
//! hal_csi_get_instance.
//!
//! \void.
//!
//! This function get csi instance hand.
//!
//! \return csi hanle
//
//*****************************************************************************
static csi_instance_t *hal_csi_get_instance(uint32_t csi_res_glb_idx)
{
    return NULL;
}

//*****************************************************************************
//
//! hal_csi_release_instance.
//!
//! \void.
//!
//! This function release csi instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_csi_release_instance(csi_instance_t *csiInstance)
{
    return ;
}


//*****************************************************************************
//
//! hal_csi_creat_handle.
//!
//! \handle csi handle for csi func.
//!
//! This function get hal handle.
//!
//! \return csi handle
//
//*****************************************************************************
bool hal_csi_creat_handle(void **handle, uint32_t csi_res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_csi_release_handle.
//!
//! \void.
//!
//! This function delete csi instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_csi_release_handle(void *handle)
{
    return true;
}

