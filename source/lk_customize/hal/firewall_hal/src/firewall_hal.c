//*****************************************************************************
//
// firewall_hal.c - the firewall hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <res.h>
#include <firewall_hal.h>
#include <trace.h>

#define LOCAL_TRACE 1 //close local trace 1->0

static firewall_instance_t g_firewallinstance = {0};

//*****************************************************************************
//
//! .hal_firewall_get_instance
//!
//! @param firewall_idx input resource_id only 1
//!
//! This function get  instance hand.
//! @return hanle firewall_instance_t
//
//*****************************************************************************
static firewall_instance_t* hal_firewall_get_instance(uint32_t firewall_idx)
{

    if (g_firewallinstance.is_created == 0) {

        g_firewallinstance.in_used = 1;
        g_firewallinstance.is_created = 1;
        return &g_firewallinstance;

    }
    else {

        if (g_firewallinstance.in_used == 0) {
            g_firewallinstance.in_used = 1;
            return &g_firewallinstance;
        }

    }

    LTRACEF("return NULL\n");
    return NULL;
}
//*****************************************************************************
//
//! hal_firewall_release_instance.
//!
//! @param firewallinstance firewall_instance_t
//!
//! This function release instance handle.
//! @return void
//
//*****************************************************************************
static void hal_firewall_release_instance(firewall_instance_t* firewallinstance)
{
    if (firewallinstance != NULL) {
        firewallinstance->in_used = 0;
    }
}

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
    firewall_instance_t*  firewallinstance = NULL;

    firewallinstance = hal_firewall_get_instance(firewall_idx);

    if (firewallinstance == NULL) {
        return HAL_ERROR;
    }

    *handle = firewallinstance;
    return HAL_OK;
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

    firewall_instance_t* l_firewallinstance = NULL;

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        l_firewallinstance = (firewall_instance_t*)handle;
        l_firewallinstance->in_used = 0;
    }

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
hal_status_t hal_firewall_init(void* handle, uint32_t cfg_count, firewall_cfg_t* firewall_cfg, bool cfg_debug)
{
    hal_status_t ret = HAL_OK;

    firewall_instance_t* l_firewallinstance = NULL;

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        l_firewallinstance = (firewall_instance_t*)handle;
        l_firewallinstance->is_init = 1;

        ret = firewall_init(cfg_count, firewall_cfg, cfg_debug);
    }

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

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        ret = share_resource(resource_id, domain_id, is_shared);
    }

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

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        firewall_enable(fw_enable);
    }

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
hal_status_t hal_rid_init(void* handle, uint32_t cfg_count, firewall_cfg_t* rid_cfg)
{
    hal_status_t ret = HAL_OK;

    firewall_instance_t* l_firewallinstance = NULL;

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        l_firewallinstance = (firewall_instance_t*)handle;
        l_firewallinstance->is_init = 1;

        ret = rid_init(cfg_count, rid_cfg);
    }

    return ret;
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
    hal_status_t ret = HAL_OK;

    if (handle == NULL) {
        ret = HAL_ERROR;
    }
    else {
        ret = permission_enable(res_id, enable);
    }

    return ret;
}
