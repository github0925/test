//*****************************************************************************
//
// pvt_hal.h - Prototypes for the pvt hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __PVT_HAL_H__
#define __PVT_HAL_H__
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

#include <string.h>
#include <platform/interrupts.h>
#include <kernel/spinlock.h>

typedef struct pvt_out_data
{
    uint32_t process_data;
    float temp_data;
    float voltage_data;
}pvt_out_data_t;

typedef enum pvt_res_id {
    PVT_RES_ID_SAF = 0,
    PVT_RES_ID_SEC,
} pvt_res_id_t;

typedef enum pvt_device_type {
    PVT_DEVICE_TYPE_ULVT = 1,
    PVT_DEVICE_TYPE_LVT,
    PVT_DEVICE_TYPE_SVT,
} pvt_device_type_t;

typedef enum pvt_int_type {
    PVT_INT_TYPE_HYST_HIGH = 0,
    PVT_INT_TYPE_HYST_LOW,
    PVT_INT_TYPE_HYST_R,
    PVT_INT_TYPE_HYST_F,
} pvt_int_type_t;

typedef struct pvt_instance {
    uint32_t res_id;
    paddr_t phy_addr;
    spin_lock_t lock;
    uint8_t is_init;
    uint8_t occupied;
    void *arg;
} pvt_instance_t;
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
int hal_pvt_get_pvt(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, pvt_out_data_t * out_data);

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
int hal_pvt_get_p(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, uint32_t * out_data);

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
int hal_pvt_get_v(pvt_res_id_t res_id, float * out_data);

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
int hal_pvt_get_t(pvt_res_id_t res_id, float * out_data);

//*****************************************************************************
//
//! hal_pvt_set_hyst_h.
//!
//! @param res_id input, pvt res id
//! @param hyst_h_thresh_h input, h temp, when the v/t is larger than h, alarm will be generated
//! @param hyst_h_thresh_l input, l temp, alarm will not been cleaned till v/t is smaller than l
//! @param index input, alarm index must be 0 or 1
//! This function is for h temp alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_set_hyst_h(pvt_res_id_t res_id, float hyst_h_thresh_h, float hyst_h_thresh_l, uint32_t index);

//*****************************************************************************
//
//! hal_pvt_set_hyst_l.
//!
//! @param res_id input, pvt res id
//! @param hyst_l_thresh_l input, h temp, when the v/t is small than l, alarm will be generated
//! @param hyst_l_thresh_h input, l temp, alarm will not been cleaned till v/t is larger than h
//! @param index input, alarm index must be 0 or 1
//! This function is for l temp alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_set_hyst_l(pvt_res_id_t res_id, float hyst_l_thresh_l, float hyst_l_thresh_h, uint32_t index);

//*****************************************************************************
//
//! hal_pvt_int_en.
//!
//! @param res_id input, pvt res id
//! @param int_type input, alarm type
//! @param int_en input, 0 disable int, 1 enable int
//! @param index input, alarm index must be 0 or 1
//! This function is for enable or disable alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_int_en(pvt_res_id_t res_id, pvt_int_type_t int_type, uint32_t int_en, uint32_t index);

//*****************************************************************************
//
//! hal_pvt_get_int_status.
//!
//! @param res_id input, pvt res id
//! @param index input, alarm index must be 0 or 1
//! @param int_status output, int status, bit 0: hyst_high, bit 1: hyst_low, bit 2:hyst_r ,bit 3:hyst_f
//! This function is for get int status,
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_int_status(pvt_res_id_t res_id, uint32_t index, uint32_t * int_status);

//*****************************************************************************
//
//! hal_pvt_clear_int.
//!
//! @param res_id input, pvt res id
//! @param int_type input, alarm type
//! @param index input, alarm index must be 0 or 1
//! This function is for clear alarm, h alarm, temp must small than h_l, l alarm, temp must larger than l_h
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_clear_int(pvt_res_id_t res_id, pvt_int_type_t int_type, uint32_t index);

//*****************************************************************************
//
//! hal_pvt_int_register.
//!
//! @param res_id input, pvt res id
//! @param reg_unreg input, 1 register int, 0 unregister int
//! @param call_func input, int call back function
//! @param index input, alarm index must be 0 or 1
//! This function is for register int, or mask interrupt
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_int_register(pvt_res_id_t res_id, uint32_t reg_unreg, int_handler call_func, uint32_t index, void *arg);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __PVT_HAL_H__

