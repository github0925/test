//*****************************************************************************
//
// sd_cmac.c - cmac.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_cmac.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

//*****************************************************************************
//
//! cmac.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param key input, aes key
//! @param key_len input, key len
//! @param src input, msg for cmac
//! @param src_size input, msg size
//! @param dst output, buff used for cmac result
//! @param dst_size input, buff len
//!
//! This function is for cmac
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_cmac(void* handle, const uint8_t* key, uint32_t key_len, const void* src, size_t src_size, uint8_t* dst, uint32_t dst_size)
{
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;

    LTRACEF("enter key_len= %d\n", key_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {

        hal_aes_init(handle, SD_FCT_CMAC, OPERATION_ENC, addr_type, key, NULL, key_len, NULL, 0);
        ret = hal_aes_final(handle, (uint8_t*)src, src_size, 0, (uint8_t*)dst, dst_size);
    }

    return ret;
}
