//*****************************************************************************
//
// sd_sm2.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_SM2_H__
#define __SD_SM2_H__
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
#include <sd_ecc.h>
#include <sd_hash.h>

//*****************************************************************************
//
//! SM2_compute_id_digest.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param curve_nid input, curve id to chose curve,
//! @param in input, id
//! @param in_len input, id len
//! @param out output, z msg buff
//! @param out_len input, z msg len, should <= out buff len
//! @param eckey input, ecc key includ priv key and pub key
//! @param hash_fct input, hash type
//!
//! This function is for get sm2 z msg
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_sm2_compute_id_digest(void* handle, int curve_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                         unsigned out_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct);

//*****************************************************************************
//
//! hal_sm2_sign.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param curve_nid input, curve id to chose curve,
//! @param in input, in msg for sign
//! @param in_len input, in msg len
//! @param out output, sign msg buff
//! @param out_len input, sign msg len, should <= out buff len
//! @param eckey input, ecc key includ priv key and pub key
//! @param hash_fct input, hash type
//!
//! This function is for sm2 signature
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_sm2_sign(void* handle, int curve_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                         unsigned out_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct);

//*****************************************************************************
//
//! hal_sm2_verify.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param curve_nid input, curve id to chose curve,
//! @param msg input, in msg for sign
//! @param msg_len input, in msg len
//! @param sig input, sign msg for compare
//! @param sig_len input, sign msg len
//! @param eckey input, ecc key includ priv key and pub key
//! @param hash_fct input, hash type
//!
//! This function is for sm2 verify
//!
//! @return crypto_status_t CRYPTO_SUCCESS: verify pass
//
//*****************************************************************************
crypto_status_t hal_sm2_verify(void* handle, int curve_nid, const uint8_t* msg, size_t msg_len,
                           const uint8_t* sig, size_t sig_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct);

//*****************************************************************************
//
//! hal_sm2_key_exchange.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param curve_nid input, curve id to chose curve,
//! @param eckey input, ecc key includ priv key and pub key
//! @param remote_point input, receive remote point on curve
//! @param remote_point_len input,
//! @param key output, native point on curve
//! @param key_len input, buff len
//!
//! This function is for sm2 key exchange, not support !!!
//!
//! @return crypto_status_t CRYPTO_SUCCESS
//
//*****************************************************************************
crypto_status_t hal_sm2_key_exchange(void* handle, int curve_nid, const ec_key_t* eckey, const uint8_t* remote_point, size_t remote_point_len,
                                 const uint8_t* key, size_t key_len);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_SM2_H__

