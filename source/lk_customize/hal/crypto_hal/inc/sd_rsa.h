//*****************************************************************************
//
// sd_rsa.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_RSA_H__
#define __SD_RSA_H__
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
#include <sd_hash.h>

typedef enum sd_rsa_pad_types {
    SD_RSA_PADDING_NONE      = 0x0,/**< No padding */
    SD_RSA_PADDING_OAEP      = 0x1,/**< Optimal Asymmetric Encryption Padding */
    SD_RSA_PADDING_EME_PKCS  = 0x2,/**< EME-PKCS padding */
    SD_RSA_PADDING_EMSA_PKCS = 0x3,/**< EMSA-PKCS padding */
    SD_RSA_PADDING_PSS       = 0x4 /**< EMSA-PSS padding */
} sd_rsa_pad_types_t;

//*****************************************************************************
//
//! hal_rsa_encrypt.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param pub_key input, rsa pubkey includ n,e
//! @param src input, msg for encrypt
//! @param src_size input, in msg len
//! @param dst output, buff for encrypted msg
//! @param dst_size input, buff len
//! @param padding input, padding type(not used)
//!
//! This function is for rsa encrypt
//!
//! @return crypto_status_t CRYPTO_SUCCESS
//
//*****************************************************************************

crypto_status_t hal_rsa_encrypt(void* handle, const rsa_pubkey_t* pub_key,
                            const uint8_t* src, uint32_t src_size, uint8_t* dst, uint32_t dst_size,
                            int padding);

//*****************************************************************************
//
//! hal_rsa_decrypt.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param pri_key input, rsa pri key includ d info
//! @param out_len input, decrypt out len
//! @param out output, buff for decrypt
//! @param max_out input, buff len
//! @param in input, msg for decrypt
//! @param in_len input, msg len
//! @param padding input, padding type(not used)
//!
//! This function is for rsa decrypt
//!
//! @return crypto_status_t CRYPTO_SUCCESS
//
//*****************************************************************************
crypto_status_t hal_rsa_decrypt(void* handle, const rsa_keypair_t* pri_key, size_t out_len, uint8_t* out, size_t max_out,
                            const uint8_t* in, size_t in_len, int padding);

//*****************************************************************************
//
//! hal_rsa_sign.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param hash_nid input, hash type (not used )
//! @param in input, msg for sign
//! @param in_len input, msg len
//! @param out output
//! @param out_len output len
//! @param pri_key input, priv key
//! @param padding input, padding type
//! @param salt_len input, salt length
//!
//! This function is for rsa signature
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_rsa_sign(void* handle, int hash_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                         unsigned out_len, const rsa_keypair_t* pri_key, int padding, uint32_t salt_len);

//*****************************************************************************
//
//! hal_rsa_verify.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param hash_nid input, hash type (not used )
//! @param msg input, msg for verify
//! @param msg_len input, msg len
//! @param sig input, sig msg for compare
//! @param sig_len input, sig msg len
//! @param pub_key input, pub key
//! @param padding input, padding type
//! @param salt_len input, salt length
//!
//! This function is for rsa Verification
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_verify(void* handle, int hash_nid, const uint8_t* msg, size_t msg_len,
                           const uint8_t* sig, size_t sig_len, const rsa_pubkey_t* pub_key,
                           int padding, uint32_t salt_len);

//*****************************************************************************
//
//! hal_rsa_keygen.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param key_e input, pub key
//! @param key_e_len input
//! @param key_len input,support RSA_1024_LEN 128 RSA_2048_LEN 256 RSA_3072_LEN 384 RSA_4096_LEN 512
//! @param key_n output, pub key
//! @param key_d output, priv key
//!
//! This function is for rsa key generation
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_keygen(void* handle, uint8_t* key_e, uint32_t key_e_len, int32_t key_len, uint8_t* key_n, uint8_t* key_d);

//*****************************************************************************
//
//! hal_rsa_keygen_ex.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param key_len input,support RSA_1024_LEN 128 RSA_2048_LEN 256 RSA_3072_LEN 384 RSA_4096_LEN 512
//! @param key_e input, pub key
//! @param key output, key combination, modulus+lambda+private key+exponent
//!
//! This function is for rsa key generation
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_keygen_ex(void* handle, int32_t key_len, uint8_t* key_e, uint8_t* key);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_RSA_H__

