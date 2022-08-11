//*****************************************************************************
//
// sd_hmac.c - hmac hal.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_hmac.h>
#include <trace.h>
#include <string.h>
#include <sx_dma.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define SRAM_KEY_OFFSET    (5 * 64)//(2 * 64)

//*****************************************************************************
//
//! hmac.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_alg input, hash algorithm
//! @param key input, aes key
//! @param key_len input, key len
//! @param src input, msg for hmac
//! @param src_size input, msg size
//! @param dst output, buff used for hmac result
//! @param dst_size input, buff len
//!
//! This function is for hmac
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_hmac(void* handle, sd_hash_alg_t hash_alg, const void* key, size_t key_len, const void* src, size_t src_size, uint8_t* dst,
                     uint32_t dst_size)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    buff_addr_type_t key_type = HAL_SRAM_PUB;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    //addr_t addr_key;
    //addr_t sram_base;

    LTRACEF("enter key_len= %d\n", (uint32_t)key_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;
/*
        sram_base = sram_addr(key_type, vce_id);

        addr_key = sram_base + SRAM_KEY_OFFSET;

        memcpy((void*)addr_key, key, key_len);
*/
        if(key_len > CE2_SRAM_PUB_HASH_HMAC_KEY_SIZE){
            LTRACEF("hmac key size is error\n");
            return CRYPTO_ERROR;
        }

        ret = memcpy_blk(vce_id,
            block_t_convert((void*)CE2_SRAM_PUB_HASH_HMAC_KEY_ADDR_OFFSET, key_len, key_type),
            block_t_convert(key, key_len, EXT_MEM), key_len);

        if (ret) {
            LTRACEF("memcpy_blk fail= %d\n", ret);
            return ret;
        }

        if (l_cryptoinstance->hash_method->hmac) {
            ret = l_cryptoinstance->hash_method->hmac(hash_alg, vce_id,
                    block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t*)(CE2_SRAM_PUB_HASH_HMAC_KEY_ADDR_OFFSET), key_len, key_type),
                    block_t_convert(src, src_size, addr_type),
                    block_t_convert(dst, dst_size, addr_type));
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}
