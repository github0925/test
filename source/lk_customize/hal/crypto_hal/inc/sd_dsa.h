//*****************************************************************************
//
// sd_dsa.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_DSA_H__
#define __SD_DSA_H__
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

//*****************************************************************************
//
//! hal_dsa_init.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param priv_key_bits input, priv key bit value, to init dsa p q g value
//!
//! This function is for dsa init
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_dsa_init(void* handle, int priv_key_bits);

//*****************************************************************************
//
//! hal_dsa_sign.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_nid input, hash alg type, sd_hash_alg_t
//! @param in input, msg for sign
//! @param in_len input, msg len
//! @param out output, buff for sign output
//! @param out_len input, buff len
//! @param dsa_key input, dsa key
//!
//! This function is for dsa signature
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_dsa_sign(void* handle, int hash_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                         unsigned out_len, const dsa_key_t* dsa_key);

//*****************************************************************************
//
//! hal_dsa_verify.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param hash_nid input, hash alg type, sd_hash_alg_t
//! @param msg input, msg for verify
//! @param msg_len input, msg len
//! @param sig output, buff for verify output
//! @param sig_len input, buff len
//! @param dsa_key input, dsa key
//!
//! This function is for dsa verify
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_dsa_verify(void* handle, int hash_nid, const uint8_t* msg, size_t msg_len,
                           const uint8_t* sig, size_t sig_len, const dsa_key_t* dsa_key);

//*****************************************************************************
//
//! hal_dsa_keygen.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param dsa_key output, dsa key
//!
//! This function is for dsa keypair gen
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_dsa_keygen(void* handle, dsa_key_t* dsakey);

//*****************************************************************************
//
//! hal_dsa_keygen.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param dsa_key output, dsa key
//!
//! This function is for dsa pub key gen
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_dsa_generate_pub_key(void* handle, dsa_key_t* dsakey);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_DSA_H__

