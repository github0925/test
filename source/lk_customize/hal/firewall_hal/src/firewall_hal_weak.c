//*****************************************************************************
//
// firewall_hal_weak.c - the firewall hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL    0
#endif

typedef enum _hal_status_type {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} hal_status_t;

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
hal_status_t hal_firewall_creat_handle(void** handle, uint32_t firewall_idx)
{
    hal_status_t ret = HAL_OK;
    return ret;
}

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
hal_status_t hal_firewall_delete_handle(void* handle)
{
    hal_status_t ret = HAL_OK;
    return ret;
}

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
hal_status_t hal_firewall_init(void* handle, uint32_t cfg_count, void* firewall_cfg, uint32_t cfg_debug)
{
    hal_status_t ret = HAL_OK;

    return ret;
}

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
hal_status_t hal_firewall_share_res(void* handle, uint32_t resource_id, uint32_t domain_id, uint32_t is_shared)
{
    hal_status_t ret = HAL_OK;

    return ret;
}

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
hal_status_t hal_firewall_enable(void* handle, bool fw_enable)
{
    hal_status_t ret = HAL_OK;

    return ret;
}

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
hal_status_t hal_rid_init(void* handle, uint32_t cfg_count, void* rid_cfg)
{
    return HAL_OK;
}

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
hal_status_t hal_permission_enable(void* handle, uint32_t res_id, bool enable)
{
    return HAL_OK;
}

