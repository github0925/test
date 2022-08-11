//*****************************************************************************
//
// sd_aes.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_AES_H__
#define __SD_AES_H__
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

#include <crypto_hal.h>
#include <sram_conf.h>
#include <sx_dma.h>
typedef enum sd_aes_fct {
    SD_FCT_ECB  = 0x0,              /**< Electronic Codebook */
    SD_FCT_CBC  = 0x1,              /**< Cipher Block Chaining */
    SD_FCT_CTR  = 0x2,              /**< Counter Feedback */
    SD_FCT_CFB  = 0x3,              /**< Cipher Feedback */
    SD_FCT_OFB  = 0x4,              /**< Output Feedback */
    SD_FCT_CCM  = 0x5,              /**< Counter with CBC-MAC Mode */
    SD_FCT_GCM  = 0x6,              /**< Galois/Counter Mode */
    SD_FCT_XTS  = 0x7,              /**< XEX-based tweaked-codebook mode with ciphertext stealing */
    SD_FCT_CMAC  = 0x8,             /**< CMAC mode */
    SD_FCT_NUM_MAX  = 0x9,
} sd_aes_fct_t;

/**
* @brief Enumeration of possible key type for AES algorithm.
*/
typedef enum aes_key_type_t {
    KEY_TYPE_128 = 0,
    KEY_TYPE_256 = 1,
    KEY_TYPE_192 = 2
} aes_key_type_t;

typedef enum aes_operation {
    OPERATION_ENC = 0,            /**< Encrypt */
    OPERATION_DEC = 1,            /**< Decrypt but does not return any tag */
} aes_operation_t;

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
crypto_status_t hal_aes_init(void* handle, sd_aes_fct_t fct, aes_operation_t mode, buff_addr_type_t key_addr_type,
                         const uint8_t* key1, const uint8_t* key2, uint32_t key_len,
                         const uint8_t* iv, uint32_t iv_len);

//*****************************************************************************
//
//! hal_aes_final.
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
crypto_status_t hal_aes_final(void* handle, const uint8_t* src, uint32_t src_size, uint32_t head_len,
                          uint8_t* dst, uint32_t dst_size);

crypto_status_t hal_aes_final_test(void* handle, const uint8_t* src, uint32_t src_size, uint32_t head_len,
                               uint8_t* dst, uint32_t dst_size, buff_addr_type_t src_addr_type, buff_addr_type_t dst_addr_type);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_AES_H__

