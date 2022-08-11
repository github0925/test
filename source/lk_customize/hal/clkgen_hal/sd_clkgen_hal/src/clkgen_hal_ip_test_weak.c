//*****************************************************************************
//
// clkgen_hal_ip_test.c - Driver for the clkgen hal Module.
//
// Copyright (c) 2019 SemiDrive Incorporated.  All rights reserved.
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
#include "clkgen_hal_ip_test.h"

//*****************************************************************************
//
//! hal_clkgen_test_creat_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function get hal handle.
//!
//! \return clkgen handle
//
//*****************************************************************************
bool hal_clkgen_test_creat_handle(void **handle)
{
    return true;
}

//*****************************************************************************
//
//! hal_clkgen_test_release_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function delete clkgen instance handle.
//!
//! \return
//
//*****************************************************************************
bool hal_clkgen_test_release_handle(void *handle)
{
    return true;
}
//*****************************************************************************
//case1.1 test1
//! hal_clkgen_ipslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip global resoure index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ipslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.1   test2
//! hal_clkgen_busslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_busslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.1 test3
//! hal_clkgen_coreslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx core slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_coreslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.1.test4
//! hal_clkgen_other_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \phy_addr saf sec soc display phy address
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_other_readonlyreg_check_test(void *handle,
        paddr_t phy_addr)
{
    return true;
}

//*****************************************************************************
//case1.2 test4
//! hal_clkgen_ipslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ipslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.2   test5
//! hal_clkgen_busslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx bus slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_busslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}


//*****************************************************************************
//case1.2 test6
//! hal_clkgen_coreslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_coreslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.2 test6
//! hal_clkgen_lpgating_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx gating global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_lpgating_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.2 test7
//! hal_clkgen_uuuslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_uuuslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//case1.2 test7
//! hal_clkgen_other_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \phy_addr saf sec soc display phy address
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_other_rw_reg_check_test(void *handle, paddr_t phy_addr)
{
    return true;
}

//*****************************************************************************
//case1.3 test8
//! hal_clkgen_ip_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip slice global resource index
//! \ip_slice_default ip slice default value
//!
//! This function is for clkgen ip clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ip_clock_test(void *handle, uint32_t res_glb_idx,
                              const clkgen_ip_slice_t *ip_slice_default)
{
    return true;
}

//*****************************************************************************
//case1.4 test9
//! hal_clkgen_ip_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//! \ip_slice_default ip slice default value
//!
//! This function is for clkgen ip clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ip_clock_div_test(void *handle, uint32_t res_glb_idx,
                                  const clkgen_ip_slice_t *ip_slice_default)
{
    return true;
}
//*****************************************************************************
//case1.5 test10
//! hal_clkgen_bus_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx bus slice start index
//! \bus_slice_default bus slice default frequence
//!
//! This function is for clkgen bus clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_bus_clock_test(void *handle, uint32_t res_glb_idx,
                               const clkgen_bus_slice_t *bus_slice_default)
{
    return true;
}

//*****************************************************************************
//case1.6 test11
//! hal_clkgen_bus_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx bus slice start index
//! \bus_slice_default bus slice default frequence
//!
//! This function is for clkgen bus clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_bus_clock_div_test(void *handle, uint32_t res_glb_idx,
                                   const clkgen_bus_slice_t *bus_slice_default)
{
    return true;
}

//*****************************************************************************
//case1.7 test12
//! hal_clkgen_core_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx core slice start index
//! \core_slice_default core slice default frequence
//!
//! This function is for clkgen bus clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_core_clock_test(void *handle, uint32_t res_glb_idx,
                                const clkgen_core_slice_t *core_slice_default)
{
    return true;
}
//*****************************************************************************
//case1.8 test13
//! hal_clkgen_core_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx core slice start index
//!
//! This function is for clkgen core clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_core_clock_div_test(void *handle, uint32_t res_glb_idx,
                                    const clkgen_core_slice_t *core_slice_default)
{
    return true;
}


