//*****************************************************************************
//
// crypto_hal.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __CRYPTO_HAL_H__
#define __CRYPTO_HAL_H__
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
#include <chip_res.h>
#include <target_res.h>

#include <ce.h>
#include <sx_rsa.h>
#include <sx_cipher.h>
#include <sx_trng.h>
#include <sx_ecc.h>
#include <sx_ecc_keygen.h>
#include <sx_sm2.h>
#include <sx_dsa.h>
#include <sx_ecdsa.h>

#define VCE_RES_DESCRIPTION_STR_LEN_MAX 100
#define VCE_RES_MAGIC_STR_LEN_MAX 20
#define AES_KEY_SIZE_MAX 32
#define AES_IV_LEN_MAX 16

#define RSA_SUPPORT_PKA_ID_MASK 0x04

typedef enum crypto_status {
    CRYPTO_SUCCESS = 0,     /* Success */
    CRYPTO_ERROR   = 0xffff0000,    /* Generic Error */
    CRYPTO_NOSUPPORT,           /* Scheme not support */
    CRYPTO_INVALID_KEY,     /* Invalid Key in asymmetric scheme: RSA/DSA/ECCP/DH etc */
    CRYPTO_INVALID_TYPE,        /* Invalid aes_type/des_type/authenc_type/hash_type/cbcmac_type/cmac_type */
    CRYPTO_INVALID_CONTEXT,     /* Invalid context in multi-thread cipher/authenc/mac/hash etc */
    CRYPTO_INVALID_PADDING,     /* Invalid sym_padding/rsassa_padding/rsaes_padding */
    CRYPTO_INVALID_AUTHENTICATION,  /* Invalid authentication in AuthEnc(AES-CCM/AES-GCM)/asymmetric verify(RSA/DSA/ECCP DSA) */
    CRYPTO_INVALID_ARG,     /* Invalid arguments */
    CRYPTO_INVALID_PACKET,      /* Invalid packet in asymmetric enc/dec(RSA) */
    CRYPTO_LENGTH_ERR,          /* Invalid Length in arguments */
    CRYPTO_OUTOFMEM,            /* Memory alloc NULL */
    CRYPTO_SHORT_BUFFER,        /* Output buffer is too short to store result */
    CRYPTO_DATA_TOO_LARGE_FOR_MODULUS,
    CRYPTO_NULL,            /* NULL pointer in arguments */
    CRYPTO_ERR_STATE,           /* Bad state in mulit-thread cipher/authenc/mac/hash etc */
    CRYPTO_ERR_RESULT,          /* Bad result of test crypto */
    CRYPTO_INVALID_CE,   /* invalid ce, ce not support Algorithm */
} crypto_status_t;

typedef enum crypto_hal_status_type {
    CRYPTO_HAL_OK       = 0x00U,
    CRYPTO_HAL_ERROR    = 0x01U,
    CRYPTO_HAL_BUSY     = 0x02U,
    CRYPTO_HAL_TIMEOUT  = 0x03U
} crypto_hal_status_t;

typedef enum buff_addr_type {
    HAL_SRAM_PUB = 0,
    HAL_SRAM_SEC = 1,
    HAL_KEY_INT = 2,
    HAL_EXT_MEM = 3,
    HAL_PKE_INTERNAL = 4
} buff_addr_type_t;

typedef struct {
    uint8_t* n;       /* Modulus */
    uint8_t* e;       /* Public exponent */
    uint8_t* d;       /* Private exponent */
    uint32_t  n_len;
    uint32_t  e_len;
    uint32_t  d_len;
} rsa_keypair_t;

typedef struct {
    uint8_t* n;       /* Modulus */
    uint8_t* e;       /* Public exponent */
    uint32_t  n_len;
    uint32_t  e_len;
} rsa_pubkey_t;

typedef struct ec_key {
    block_t pub_key;
    block_t priv_key;
} ec_key_t;

typedef struct dsa_key {
    block_t pub_key;
    block_t priv_key;
} dsa_key_t;
/* openssl_method_common_st contains the common part of all method structures.
 * This must be the first member of all method structures. */
struct openssl_method_common_st {
    int references;  /* dummy – not used. */
    char is_static;
};

/* drive api define same with this */
struct rsa_meth_st {
    struct openssl_method_common_st common;

    void* app_data;

    uint32_t (*sign)(uint32_t vce_id, hash_alg_t sha_type, rsa_pad_types_t padding_type,
                     block_t message, block_t result, block_t n, block_t private_key,
                     uint32_t salt_length);

    /* Ignored. Set this to NULL. */
    uint32_t (*verify)(uint32_t vce_id, hash_alg_t sha_type, rsa_pad_types_t padding_type,
                       block_t message, block_t n, block_t public_expo, block_t signature,
                       uint32_t salt_length);

    /* These functions mirror the |RSA_*| functions of the same name. */
    uint32_t (*encrypt)(uint32_t vce_id, rsa_pad_types_t padding_type, block_t message,
                        block_t n, block_t public_expo, block_t result, hash_alg_t hashType);

    uint32_t (*decrypt)(uint32_t vce_id, rsa_pad_types_t padding_type, block_t cipher,
                        block_t n, block_t private_key, block_t result, uint32_t crt,
                        uint32_t* msg_len, hash_alg_t hashType);

    uint32_t (*keygen)(uint32_t vce_id, block_t p, block_t q, block_t public_expo,
                       block_t n, block_t private_key, uint32_t size, uint32_t lambda);

    uint32_t (*crt_key_keygen)(uint32_t vce_id, block_t p, block_t q, block_t d,
                               block_t dp, block_t dq, block_t inv, uint32_t size);

    uint32_t (*keygen_ex)(uint32_t vce_id, uint32_t nbr_of_bytes, block_t exponent,
                        block_t key, bool public, bool lambda);

};

/* drive api define same with this */
struct aes_meth_st {

    uint32_t (*cipher)(aes_fct_t aes_fct, uint32_t ce_id, aes_mode_t mode, aes_ctx_t ctx,
                       uint32_t header_len, bool header_save, bool pre_cal_key,
                       block_t key, block_t xtskey, block_t iv, block_t ctx_ptr, block_t data_in, block_t data_out);
};

/* drive api define same with this */
struct hash_meth_st {

    uint32_t (*update)(hash_alg_t hash_alg, uint32_t ce_id, bool first_part, block_t data_in);

    uint32_t (*finish)(hash_alg_t hash_alg, uint32_t ce_id, block_t state, block_t data_in, block_t data_out, uint32_t total_len);

    uint32_t (*hash)(hash_alg_t hash_alg, uint32_t ce_id, block_t iv, block_t data_in, block_t data_out);

    uint32_t (*hmac)(hash_alg_t hash_alg, uint32_t ce_id, block_t iv, block_t key, block_t data_in, block_t data_out);
};

/* drive api define same with this */
struct rng_meth_st {

    uint32_t (*trng)(uint32_t vce_id, uint8_t* dst, uint32_t size);

    uint32_t (*prng)(uint32_t vce_id, uint8_t* dst, uint32_t size);
};

/* drive api define same with this */
struct ec_meth_st {

    uint32_t (*keypair_gen)(uint32_t vce_id, block_t group, block_t pub_key, block_t n, block_t pri_key, uint32_t size, uint32_t curve_flags);

    uint32_t (*prikey_gen)(uint32_t vce_id, block_t n, block_t priv);

    uint32_t (*pubkey_gen)(uint32_t vce_id, block_t group, block_t pub, block_t priv, uint32_t size, uint32_t curve_flags);

    uint32_t (*sm2_sign)(uint32_t vce_id, const void* curve_info, block_t message, block_t key, block_t signature, hash_alg_t hash_fct);

    uint32_t (*sm2_verify)(uint32_t vce_id, const void* curve_info, block_t message, block_t key, block_t signature, hash_alg_t hash_fct);

    uint32_t (*sm2_key_exchange)(uint32_t vce_id, const void* curve_info, block_t prv_key, block_t pub_key, block_t pointb, block_t cofactor, block_t pointa, block_t two_w, block_t ex_pub_key);

    uint32_t (*sm2_generate_digest)(uint32_t vce_id, const sx_ecc_curve_t* curve, block_t message, hash_alg_t hash_fct, block_t* digest_blk);

    uint32_t (*ecdsa_sign)(uint32_t vce_id, const void* curve_info, block_t message, block_t key, block_t signature, hash_alg_t hash_fct);

    uint32_t (*ecdsa_verify)(uint32_t vce_id, const void* curve_info, block_t message, block_t key, block_t signature, hash_alg_t hash_fct);
};

/* drive api define same with this */
struct dsa_meth_st {

    uint32_t (*prikey_gen)(uint32_t vce_id, block_t q, block_t priv);

    uint32_t (*pubkey_gen)(uint32_t vce_id, block_t p, block_t generator, block_t priv_key, block_t pub_key);

    uint32_t (*sign)(uint32_t vce_id, hash_alg_t hash_alg, block_t p, block_t q, block_t generator, block_t priv, block_t message, block_t signature);

    uint32_t (*verify)(uint32_t vce_id, hash_alg_t hash_alg, block_t p, block_t q, block_t generator, block_t pub, block_t message, block_t signature);
};

typedef struct rsa_meth_st RSA_METHOD;
typedef struct aes_meth_st AES_METHOD;
typedef struct hash_meth_st HASH_METHOD;
typedef struct rng_meth_st RNG_METHOD;
typedef struct ec_meth_st EC_METHOD;
typedef struct dsa_meth_st DSA_METHOD;

typedef struct _crypto_instance {
    uint32_t vce_id;
    uint8_t is_created;
    uint8_t vce_in_used;
    uint8_t is_init;
    uint8_t is_enable;
    //aes info
    uint8_t fct;
    uint8_t operation_type;
    uint8_t key_addr_type;
    addr_t aes_iv;
    uint8_t aes_iv_len;
    addr_t aes_key;
    addr_t aes_key_x;
    uint32_t aes_key_len;

    //ecc info
    const void* p_ecc_curve_info;
    block_t group;
    block_t cofactor;
    block_t two_w;
    uint32_t pk_flags;
    uint32_t bytesize;

    //dsa info
    block_t dsa_p;
    block_t dsa_q;
    block_t dsa_g;

    //method
    buff_addr_type_t addr_type;
    RSA_METHOD* rsa_method;
    AES_METHOD* aes_method;
    HASH_METHOD* hash_method;
    RNG_METHOD* rng_method;
    EC_METHOD* ec_method;
    DSA_METHOD* dsa_method;
} crypto_instance_t;


/*resource excel version*/
typedef struct {
    uint32_t    version;
    buff_addr_type_t addr_type;
} crypto_config_t;

//*****************************************************************************
//
//! hal_crypto_creat_handle.
//!
//! @param handle output crypto handle,
//! @param crypto_idx input resource_id define in chipcfg...domain_res.h
//!
//! This function get ce handle.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_creat_handle(void** handle, uint32_t crypto_idx);

//*****************************************************************************
//
//! hal_crypto_delete_handle.
//!
//! @param handle input
//!
//! This function delete  instance hand.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_delete_handle(void* handle);

//*****************************************************************************
//
//! hal_trng_init.
//!
//! @param handle input, ce handle
//! @param crypto_cfg input, ce config for init
//! This function is for init trng
//!
//! @return  status
//
//*****************************************************************************
crypto_hal_status_t hal_trng_init(void* handle, crypto_config_t* crypto_cfg);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __CRYPTO_H__

