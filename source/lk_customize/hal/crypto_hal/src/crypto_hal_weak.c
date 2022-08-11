//*****************************************************************************
//
// crypto_hal.c - the crypto hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <stdint.h>

#ifndef NULL
#define NULL    0
#endif

typedef struct {
    uint32_t    version;
} crypto_config_t;

typedef struct _crypto_instance {
    uint32_t vce_id;

} crypto_instance_t;

typedef enum crypto_hal_status_type {
    CRYPTO_HAL_OK       = 0x00U,
    CRYPTO_HAL_ERROR    = 0x01U,
    CRYPTO_HAL_BUSY     = 0x02U,
    CRYPTO_HAL_TIMEOUT  = 0x03U
} crypto_hal_status_t;

//*****************************************************************************
//
//! .hal_crypto_get_instance
//!
//! @param crypto_idx input resource_id define in chipcfg/domain_res.h
//!
//! This function get  instance hand.
//! @return hanle crypto_instance_t
//
//*****************************************************************************
static crypto_instance_t* hal_crypto_get_instance(uint32_t crypto_idx)
{
    return NULL;
}
//*****************************************************************************
//
//! hal_crypto_release_instance.
//!
//! @param cryptoinstance crypto_instance_t
//!
//! This function release instance handle.
//! @return void
//
//*****************************************************************************
static void hal_crypto_release_instance(crypto_instance_t* cryptoinstance)
{

}

//*****************************************************************************
//
//! hal_crypto_creat_handle.
//!
//! @param handle output crypto handle,
//! @param crypto_idx input resource_id define in chipcfg...domain_res.h
//!
//! This function get ce handle.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_creat_handle(void** handle, uint32_t crypto_idx)
{
    return CRYPTO_HAL_OK;
}

//*****************************************************************************
//
//! hal_crypto_delete_handle.
//!
//! @param handle input
//!
//! This function delete  instance hand.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_delete_handle(void* handle)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    return ret;
}

//*****************************************************************************
//
//! hal_crypto_init.
//!
//! @param handle input, ce handle
//! @param crypto_cfg input, ce config for init
//! This function is for init ce
//!
//! @return  status
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_init(void* handle, crypto_config_t* crypto_cfg)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    return ret;
}

//*****************************************************************************
//
//! hal_crypto_deinit.
//!
//! @param handle input
//!
//! This function is for  deinit.
//!
//! @return crypto_hal_status_t status
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_deinit(void* handle)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    return ret;
}