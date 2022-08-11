//*****************************************************************************
//
// pvt_hal.c - the pvt hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <stdint.h>
#include <pvt_hal.h>

#ifndef NULL
#define NULL    0
#endif

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param pvt_device_type input, pvt device type
//! @param out_data output, pvt value
//! This function is for get pvt value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_pvt(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, pvt_out_data_t * out_data){
    int ret = 0;

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param pvt_device_type input, pvt device type
//! @param out_data output, p value
//! This function is for get p value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_p(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, uint32_t * out_data){
    int ret = 0;

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param out_data output, v value
//! This function is for get v value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_v(pvt_res_id_t res_id, float * out_data){
    int ret = 0;

    return ret;
}
//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param out_data output, t value
//! This function is for get t value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_t(pvt_res_id_t res_id, float * out_data){
    int ret = 0;

    return ret;
}