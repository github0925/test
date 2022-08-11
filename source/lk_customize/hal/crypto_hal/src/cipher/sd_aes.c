//*****************************************************************************
//
// sd_aes.c - aes hal.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_aes.h>
#include <trace.h>
#include <string.h>
#include <kernel/thread.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define AES_KEY_LEN_TYPE_MAX 4
#define AES_KEY_LEN_MAX 32

#define CACHE_CLEAN_RANGE_LEN 32

#define AES_SRC_HEAD_LEN_MAX 96
#define AES_SRC_LEN_MAX (AES_KEY_LEN_MAX+AES_SRC_HEAD_LEN_MAX)

#define SRAM_IV_OFFSET     (0)
#define SRAM_KEY_OFFSET    (5 * 64)

uint8_t aes_cbc_iv[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
uint8_t aes_ctr_iv[16] = "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF";
uint8_t aes_cfb_iv[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
uint8_t aes_ofb_iv[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
uint8_t aes_ccm_iv[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
uint8_t aes_gcm_iv[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
uint8_t aes_xts_iv[16] = "\x4f\xae\xf7\x11\x7c\xda\x59\xc6\x6e\x4b\x92\x01\x3e\x76\x8a\xd5";


//    iv_table index follow:
//    SD_FCT_ECB  = 0x0, SD_FCT_CBC  = 0x1, SD_FCT_CTR  = 0x2,SD_FCT_CFB  = 0x3,SD_FCT_OFB  = 0x4,
//    SD_FCT_CCM  = 0x5,SD_FCT_GCM  = 0x6,SD_FCT_XTS  = 0x7,SD_FCT_CMAC  = 0x8,
uint8_t* iv_table[SD_FCT_NUM_MAX] = {NULL, aes_cbc_iv, aes_ctr_iv, aes_cfb_iv, aes_ofb_iv, NULL, aes_gcm_iv, aes_xts_iv, NULL};

//key store in efuse offset value for test, key len 16 bytes store at first, key len 24 store at 2, keylen 32 store at 3
uint8_t key_int_addr_table[AES_KEY_LEN_TYPE_MAX][2] = {
    {0x0, 0x60},
    {0x20, 0x80},
    {0x40, 0xa0},
    {0x0, 0x60},
};

//*****************************************************************************
//
//! hal_aes_init.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param fct input, aes mode use sd_aes_fct_t value
//! @param operation_type input , aes operation value, use aes_operation_t
//! @param key_addr_type input, key store in buff addr type, use buff_addr_type_t
//! @param key1 input, aes key
//! @param key2 input, aes XTS mode key
//! @param key_len input, key len
//! @param iv input, initialization vector, if not set will use default
//! @param iv_len input, iv len
//!
//! This function is for aes init
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_aes_init(void* handle, sd_aes_fct_t fct, aes_operation_t operation_type, buff_addr_type_t key_addr_type,
                         const uint8_t* key1, const uint8_t* key2, uint32_t key_len,
                         const uint8_t* iv, uint32_t iv_len)
{
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    uint32_t key_len_temp = 0;
    addr_t addr_key, addr_xkey = 0;
    //uint32_t i=0;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        l_cryptoinstance->fct = fct;
        l_cryptoinstance->operation_type = operation_type;
        l_cryptoinstance->key_addr_type = key_addr_type == HAL_SRAM_PUB ? HAL_SRAM_PUB : HAL_SRAM_SEC;

        if (iv == NULL) {
            if (fct < SD_FCT_NUM_MAX) {
                if (iv_table[fct] == NULL) {
                    l_cryptoinstance->aes_iv_len = 0;
                }
                else {

                    ret = memcpy_blk(l_cryptoinstance->vce_id,
                        block_t_convert((void*)CE2_SRAM_SEC_AES_IV_ADDR_OFFSET, AES_IV_LEN_MAX, SRAM_SEC),
                        block_t_convert(iv_table[fct], AES_IV_LEN_MAX, EXT_MEM), AES_IV_LEN_MAX);

                    if (ret) {
                        return ret;
                    }

                    l_cryptoinstance->aes_iv_len = AES_IV_LEN_MAX;
                }

            }
        }
        else {
            ret = memcpy_blk(l_cryptoinstance->vce_id,
                                block_t_convert((void*)CE2_SRAM_SEC_AES_IV_ADDR_OFFSET, iv_len, SRAM_SEC),
                                block_t_convert(iv, iv_len, EXT_MEM), iv_len);

            if (ret) {
                return ret;
            }

            l_cryptoinstance->aes_iv_len = iv_len;
        }

        key_len_temp = key_len > AES_KEY_SIZE_MAX ? AES_KEY_SIZE_MAX : key_len;

        if (HAL_KEY_INT != key_addr_type) { //key just in HAL_SRAM_PUB/sram_sec/key_interface

            addr_key = CE2_SRAM_SEC_AES_KEY_ADDR_OFFSET;

            ret = memcpy_blk(l_cryptoinstance->vce_id,
                                block_t_convert((void*)addr_key, key_len_temp, l_cryptoinstance->key_addr_type), 
                                block_t_convert(key1, key_len_temp, EXT_MEM), key_len_temp);

            if (ret) {
                return ret;
            }

            if (SD_FCT_XTS == l_cryptoinstance->fct) {

                addr_xkey = CE2_SRAM_SEC_AES_XKEY_ADDR_OFFSET;//addr_key + AES_KEY_SIZE_MAX;
                ret = memcpy_blk(l_cryptoinstance->vce_id,
                                    block_t_convert((void*)addr_xkey, key_len_temp, l_cryptoinstance->key_addr_type), 
                                    block_t_convert(key2, key_len_temp, EXT_MEM), key_len_temp);

                if (ret) {
                    return ret;
                }
            }
        }
        else {
            addr_key = key_int_addr_table[(l_cryptoinstance->aes_key_len - 16) >> 3][0];

            if (SD_FCT_XTS == l_cryptoinstance->fct) {
                addr_xkey = key_int_addr_table[(l_cryptoinstance->aes_key_len - 16) >> 3][1];
            }
        }

        l_cryptoinstance->aes_key = addr_key;
        l_cryptoinstance->aes_key_x = addr_xkey;
        l_cryptoinstance->aes_key_len = key_len_temp;

        l_cryptoinstance->aes_iv = CE2_SRAM_SEC_AES_IV_ADDR_OFFSET;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_aes_final.
//!
//! @param handle input,opration handle for uplayer,creat by ce hal
//! @param src input, msg for aes encrypt/decrypt
//! @param src_size input, msg size(with out head and tag for ccm/gcm mode)
//! @param head_len input, used for CCM/XCM mode, the Additional authenticated data len
//! @param dst output, output buff
//! @param dst_size input, output buff size
//!
//! This function is for aes encrypt/decrypt
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_aes_final(void* handle, const uint8_t* src, uint32_t src_size, uint32_t head_len,
                          uint8_t* dst, uint32_t dst_size)
{
    uint32_t vce_id;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    bool head_save = false;

    LTRACEF("enter head_len= %d\n", head_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        //head_save = SD_FCT_GCM == l_cryptoinstance->fct ? true : false;

        if (l_cryptoinstance->aes_method->cipher) {
            ret = l_cryptoinstance->aes_method->cipher(l_cryptoinstance->fct, vce_id, l_cryptoinstance->operation_type,
                    CTX_WHOLE, head_len, head_save, false,
                    block_t_convert((uint8_t*)l_cryptoinstance->aes_key, l_cryptoinstance->aes_key_len, l_cryptoinstance->key_addr_type),
                    SD_FCT_XTS == l_cryptoinstance->fct ? block_t_convert((uint8_t*)(l_cryptoinstance->aes_key_x), l_cryptoinstance->aes_key_len, l_cryptoinstance->key_addr_type) : block_t_convert(NULL, 0, 0),
                    (0 != l_cryptoinstance->aes_iv_len) ? block_t_convert((uint8_t*)(l_cryptoinstance->aes_iv), 16, HAL_SRAM_SEC) : block_t_convert(NULL, 0, 0),
                    block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t*)src, src_size, addr_type),
                    block_t_convert((uint8_t*)dst, dst_size, addr_type));
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_aes_final. test
//!
//! @param handle input,opration handle for uplayer,creat by ce hal
//! @param src input, msg for aes encrypt/decrypt
//! @param src_size input, msg size
//! @param head_len input, used for CCM/XCM mode, the Additional authenticated data len
//! @param dst output, output buff
//! @param dst_size input, output buff size
//!
//! This function is for aes encrypt/decrypt
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_aes_final_test(void* handle, const uint8_t* src, uint32_t src_size, uint32_t head_len,
                               uint8_t* dst, uint32_t dst_size, buff_addr_type_t src_addr_type, buff_addr_type_t dst_addr_type)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    bool head_save = false;

    LTRACEF("enter head_len= %d\n", head_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        head_save = SD_FCT_GCM == l_cryptoinstance->fct ? true : false;

        if (l_cryptoinstance->aes_method->cipher) {

            if (LOCAL_TRACE) {
                dprintf(CRITICAL, "vce_id=%d, src = %p, dst: %p,src_size=%d,dst_size=%d,key_len=%d\n", vce_id,src, dst, src_size, dst_size, l_cryptoinstance->aes_key_len);
                dprintf(CRITICAL, "aes_key=%d,key_addr_type=%d\n", (int)l_cryptoinstance->aes_key, l_cryptoinstance->key_addr_type);
            }

            ret = l_cryptoinstance->aes_method->cipher(l_cryptoinstance->fct, vce_id, l_cryptoinstance->operation_type,
                    CTX_WHOLE, head_len, head_save, false,
                    block_t_convert((uint8_t*)l_cryptoinstance->aes_key, l_cryptoinstance->aes_key_len, l_cryptoinstance->key_addr_type),
                    SD_FCT_XTS == l_cryptoinstance->fct ? block_t_convert((uint8_t*)(l_cryptoinstance->aes_key_x), l_cryptoinstance->aes_key_len, l_cryptoinstance->key_addr_type) : block_t_convert(NULL, 0, 0),
                    (0 != l_cryptoinstance->aes_iv_len) ? block_t_convert((uint8_t*)(l_cryptoinstance->aes_iv), 16, HAL_SRAM_SEC) : block_t_convert(NULL, 0, 0),
                    block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t*)src, src_size, src_addr_type),
                    block_t_convert((uint8_t*)dst, dst_size, dst_addr_type));
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}
