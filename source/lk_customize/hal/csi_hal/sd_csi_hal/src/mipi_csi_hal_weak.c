//*****************************************************************************
//
// mipi_csi_hal.c - Driver for the mipi csi hal Module.
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

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "mipi_csi_hal.h"
#include "sd_mipi_csi2.h"
#include "res.h"
#include "chip_res.h"


#define MIPICSI_LOG 0



#define MIPI_CSI_RES_NUM 2
/* csi global instance */
static mipi_csi_instance_t g_mipicsiInstance[MIPI_CSI_RES_NUM] = {0};


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
static mipi_csi_instance_t *hal_mipi_csi_get_instance(
    uint32_t mipi_csi_res_glb_idx)
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
static void hal_mipi_csi_release_instance(mipi_csi_instance_t *instance)
{
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
bool hal_mipi_csi_creat_handle(void **handle,
                               uint32_t mipi_csi_res_glb_idx)
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
bool hal_mipi_csi_release_handle(void *handle)
{
    return true;
}



//*****************************************************************************
//
//! hal_mipi_csi_init.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for mipi csi bus init.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_init(void *handle)
{

    return true;
}

