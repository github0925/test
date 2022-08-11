//*****************************************************************************
//
// sd_hmac.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_HMAC_H__
#define __SD_HMAC_H__
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
#include <sd_hash.h>

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
crypto_status_t hal_hmac(void* handle, sd_hash_alg_t hash_alg, const void* key, size_t key_len,
                     const void* src, size_t src_size, uint8_t* dst, uint32_t dst_size);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_HMAC_H__

