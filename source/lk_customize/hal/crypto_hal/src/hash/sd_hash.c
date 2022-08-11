//*****************************************************************************
//
// sd_hash.c .
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_hash.h>
#include <trace.h>
#include <string.h>

#define LOCAL_TRACE 0 //close local trace 1->0

//*****************************************************************************
//
//! hal_hash.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_alg input, hash algorithm
//! @param src input, msg for hash
//! @param src_size input, msg size
//! @param dst output, buff used for hash result
//! @param dst_size input, buff len
//!
//! This function is for hash
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_hash(void* handle, sd_hash_alg_t hash_alg, const void* src, size_t src_size, uint8_t* dst,
                     uint32_t dst_size)
{
    uint32_t vce_id;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    LTRACEF("enter src_size= %d\n", (uint32_t)src_size);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->hash_method->hash) {
            ret = l_cryptoinstance->hash_method->hash(hash_alg, vce_id,
                    block_t_convert(NULL, 0, 0),
                    block_t_convert(src, src_size, addr_type),
                    block_t_convert(dst, dst_size, addr_type));
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_hash_update.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_alg input, hash algorithm
//! @param first_part input, mult part msg for hash, is first part
//! @param src input, msg for hash
//! @param src_size input, msg len
//!
//! This function is for hash update
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_hash_update(void* handle, sd_hash_alg_t hash_alg, bool first_part, const void* src, size_t src_size)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_SUCCESS;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_instance_t* l_cryptoinstance = NULL;

    LTRACEF("enter src_size= %d\n", (uint32_t)src_size);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->hash_method->update) {
            ret = l_cryptoinstance->hash_method->update(hash_alg, vce_id, first_part,
                    block_t_convert((uint8_t*)src, src_size, addr_type));
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_hash_finish.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_alg input, hash algorithm
//! @param first_part input, mult part msg for hash, is first part
//! @param src input, msg for hash
//! @param src_size input, msg len
//! @param dst output, buff for hash result
//! @param dst_size input, buff len
//! @param total_len input, all part msg len
//!
//! This function is for hash finish
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_hash_finish(void* handle, sd_hash_alg_t hash_alg, const void* src, size_t src_size, void* dst, size_t dst_size, uint32_t total_len)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_SUCCESS;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_instance_t* l_cryptoinstance = NULL;

    LTRACEF("enter src=%p src_size= %d\n", src, (uint32_t)src_size);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->hash_method->finish) {
            ret = l_cryptoinstance->hash_method->finish(hash_alg, vce_id, block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t*)src, src_size, addr_type),
                    block_t_convert((uint8_t*)dst, dst_size, addr_type), total_len);
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}