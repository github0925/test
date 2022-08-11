//*****************************************************************************
//
// sd_hash.h -  for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_HASH_H__
#define __SD_HASH_H__
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

/* Size of MD5 data block in bytes */
#define MD5_BLOCKSIZE      64
/* Size of MD5 initialization value in bytes */
#define MD5_INITSIZE       16
/* Size of MD5 digest in bytes */
#define MD5_DIGESTSIZE     16
/* Size of SHA1 data block in bytes */
#define SHA1_BLOCKSIZE     64
/* Size of SHA1 initialization value in bytes */
#define SHA1_INITSIZE      20
/* Size of SHA1 digest in bytes */
#define SHA1_DIGESTSIZE    20
/* Size of SHA224 data block in bytes */
#define SHA224_BLOCKSIZE   64
/* Size of SHA224 initialization value in bytes */
#define SHA224_INITSIZE    32
/* Size of SHA224 digest in bytes */
#define SHA224_DIGESTSIZE  28
/* Size of SHA256 data block in bytes */
#define SHA256_BLOCKSIZE   64
/* Size of SHA256 initialization value in bytes */
#define SHA256_INITSIZE    32
/* Size of SHA256 digest in bytes */
#define SHA256_DIGESTSIZE  32
/* Size of SHA384 data block in bytes */
#define SHA384_BLOCKSIZE   128
/* Size of SHA384 initialization value in bytes */
#define SHA384_INITSIZE    64
/* Size of SHA384 digest in bytes */
#define SHA384_DIGESTSIZE  48
/* Size of SHA512 data block in bytes */
#define SHA512_BLOCKSIZE   128
/* Size of SHA512 initialization value in bytes */
#define SHA512_INITSIZE    64
/* Size of SHA512 digest in bytes */
#define SHA512_DIGESTSIZE  64
/* Size of SHA512 data block in bytes */
#define SM3_BLOCKSIZE   64
/* Size of SHA512 initialization value in bytes */
#define SM3_INITSIZE    32
/* Size of SHA512 digest in bytes */
#define SM3_DIGESTSIZE  32
/* Maximum block size to be supported */
#define MAX_BLOCKSIZE   SHA512_BLOCKSIZE
/* Maximum digest size to be supported */
#define MAX_DIGESTSIZE  SHA512_DIGESTSIZE

typedef enum sd_hash_alg {
    SD_ALG_MD5     = 0x0,
    SD_ALG_SHA1    = 0x1,
    SD_ALG_SHA224  = 0x2,
    SD_ALG_SHA256  = 0x3,
    SD_ALG_SHA384  = 0x4,
    SD_ALG_SHA512  = 0x5,
    SD_ALG_SM3     = 0x6
} sd_hash_alg_t;

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
                     uint32_t dst_size);

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
crypto_status_t hal_hash_update(void* handle, sd_hash_alg_t hash_alg, bool first_part,
                            const void* src, size_t src_size);

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
crypto_status_t hal_hash_finish(void* handle, sd_hash_alg_t hash_alg, const void* src,
                            size_t src_size, void* dst, size_t dst_size, uint32_t total_len);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_HASH_H__

