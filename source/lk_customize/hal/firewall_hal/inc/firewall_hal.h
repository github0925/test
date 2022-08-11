//*****************************************************************************
//
// firewall_hal.h - Prototypes for the firewall hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __FIREWALL_HAL_H__
#define __FIREWALL_HAL_H__
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
#include <chip_res.h>
#include <firewall.h>

typedef enum _hal_status_type {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} hal_status_t;

typedef struct _firewall_instance {
    uint8_t is_created;
    uint8_t in_used;
    uint8_t is_init;
    uint8_t is_enable;
} firewall_instance_t;

//*****************************************************************************
//
//! hal_firewall_creat_handle.
//!
//! @param handle output firewall handle,
//! @param firewall_idx input resource_id define in chipcfg...domain_res.h
//!
//! This function get firewall handle.
//!
//! @return hal_status_t
//
//*****************************************************************************
hal_status_t hal_firewall_creat_handle(void** handle, uint32_t firewall_idx);

//*****************************************************************************
//
//! hal_firewall_delete_handle.
//!
//! @param handle input
//!
//! This function delete  instance hand.
//!
//! @return hal_status_t
//
//*****************************************************************************
hal_status_t hal_firewall_delete_handle(void* handle);

//*****************************************************************************
//
//! hal_firewall_init.
//!
//! @param handle input, firewall handle
//! @param cfg_count input, firewall config count
//! @param firewall_cfg input, firewall config for init
//! @param cfg_debug input, firewall debug
//! This function is for init firewall
//!
//! @return  status
//
//*****************************************************************************
hal_status_t hal_firewall_init(void* handle, uint32_t cfg_count, firewall_cfg_t* firewall_cfg, bool cfg_debug);

//*****************************************************************************
//
//! hal_firewall_share_res.
//!
//! @param handle input
//! @param resource_id input, the resource which will be changed share info
//! @param domain_id input, share or not share the resource to domain
//! @param is_shared input, share or not share
//!
//! This function is for share res.
//!
//! @return hal_status_t status
//
//*****************************************************************************
hal_status_t hal_firewall_share_res(void* handle, uint32_t resource_id, uint32_t domain_id, uint32_t is_shared);

//*****************************************************************************
//
//! hal_firewall_enable.
//!
//! @param handle input
//! @param fw_enable input, share or not share
//!
//! This function is for enable/disable firewall.
//!
//! @return hal_status_t status
//
//*****************************************************************************
hal_status_t hal_firewall_enable(void* handle, bool fw_enable);

//*****************************************************************************
//
//! hal_rid_config.
//!
//! @param handle input, firewall handle
//! @param cfg_count input, route_id config count
//! @param rid_cfg input, route_id config for init
//! This function is for init rid config
//!
//! @return  status
//
//*****************************************************************************
hal_status_t hal_rid_init(void* handle, uint32_t cfg_count, firewall_cfg_t* rid_cfg);

//*****************************************************************************
//
//! hal_permission_enable.
//!
//! @param handle input, firewall handle
//! @param res_id input, resources id
//! @param bool input, disable or enable
//! This function is for disabling/enabling permission of some resource
//!
//! @return  status
//
//*****************************************************************************
hal_status_t hal_permission_enable(void* handle, uint32_t res_id, bool enable);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __FIREWALL_HAL_H__

