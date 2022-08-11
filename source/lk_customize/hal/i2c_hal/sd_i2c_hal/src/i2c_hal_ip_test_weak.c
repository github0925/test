//*****************************************************************************
//
// i2c_hal_ip_test_weak.c - Driver for the i2c hal Module.
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
//#include "dw_i2c_test.h"
#include "i2c_hal_ip_test.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "lib/reg.h"


//*****************************************************************************
//
//! hal_i2c_get_instance.
//!
//! \void.
//!
//! This function get i2c instance hand.
//!
//! \return i2c hanle
//
//*****************************************************************************
static i2c_test_instance_t *hal_i2c_test_get_instance(
    uint32_t i2c_res_glb_idx)
{
    return NULL;
}

//*****************************************************************************
//
//! hal_i2c_release_instance.
//!
//! \void.
//!
//! This function release i2c instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_i2c_test_release_instance(i2c_test_instance_t *i2cInstance)
{
    return ;
}


//*****************************************************************************
//
//! hal_i2c_creat_handle.
//!
//! \handle i2c handle for i2c func.
//!
//! This function get hal handle.
//!
//! \return i2c handle
//
//*****************************************************************************
bool hal_i2c_test_creat_handle(void **handle, uint32_t i2c_res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_i2c_release_handle.
//!
//! \void.
//!
//! This function delete i2c instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_i2c_test_release_handle(void *handle)
{
    return true;
}

bool hal_i2c_read_only_reg_test(void *handle)
{
    return true;
}
