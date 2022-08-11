//*****************************************************************************
//
// sd_ecc.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_ECC_H__
#define __SD_ECC_H__
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

#define SN_X9_62_prime192v1             "prime192v1"
#define NID_X9_62_prime192v1            409

#define SN_X9_62_prime_field            "prime-field"
#define NID_X9_62_prime_field           406

#define SN_X9_62_prime256v1             "prime256v1"
#define NID_X9_62_prime256v1            415

#define SN_secp224r1            "secp224r1"
#define NID_secp224r1           713

#define SN_secp384r1            "secp384r1"
#define NID_secp384r1           715

#define SN_secp521r1            "secp521r1"
#define NID_secp521r1           716

#define SN_sect163k1            "sect163k1"
#define NID_sect163k1           721

#define SN_sect163r2            "sect163r2"
#define NID_sect163r2           723

#define SN_sect233k1            "sect233k1"
#define NID_sect233k1           726

#define SN_sect233r1            "sect233r1"
#define NID_sect233r1           727

#define SN_sect283k1            "sect283k1"
#define NID_sect283k1           729

#define SN_sect283r1            "sect283r1"
#define NID_sect283r1           730

#define SN_sect409k1            "sect409k1"
#define NID_sect409k1           731

#define SN_sect409r1            "sect409r1"
#define NID_sect409r1           732

#define SN_sect571k1            "sect571k1"
#define NID_sect571k1           733

#define SN_sect571r1            "sect571r1"
#define NID_sect571r1           734

#define SN_sece521r1            "sece521r1"
#define NID_sece521r1           1300 //this is not same with openssl

#define SN_sxiptestp256            "sxiptestp256"
#define NID_sxiptestp256           1301 //this is for sx ip test

typedef struct sd_ecc_curve {
    /**< points to predefined curve params
       formatted as {q, n, G<SUB>x</SUB>, G<SUB>y</SUB>, a, b} and
       represented in big number array of uint8 (big endian).
       The fileds are:
       - q, the modulus (or the reduction polynomial for ECC Weierstrass binary)
       - n, the order of the curve
       - G<SUB>x</SUB>, G<SUB>y</SUB>, the x and y coordinates of the generator
       - a, b (or d for Edwards curve), the coefficients defining the ECC curve*/
    block_t  params;
    uint32_t pk_flags; /**< required flags for the PK engine*/
    uint32_t bytesize; /**< size expressed in bytes */
} sd_ecc_curve_t;

typedef struct _ec_list_element_st {
    int nid;
    const void* data;
    const char* comment;
} ec_list_element;

static const ec_list_element curve_list[] = {
    /* prime field curves */
    /* secg curves */
    {
        NID_X9_62_prime192v1, &sx_ecc_curve_p192,
        "NIST/X9.62/SECG curve over a 192 bit prime field"
    },
    {
        NID_X9_62_prime_field, &sx_ecc_sm2_curve_p256,
        "SM2 curve over a 256 bit prime field"
    },
    {
        NID_secp224r1, &sx_ecc_curve_p224,
        "NIST/SECG curve over a 224 bit prime field"
    },
    {
        NID_X9_62_prime256v1, &sx_ecc_curve_p256,
        "X9.62/SECG curve over a 256 bit prime field"
    },
    {
        NID_secp384r1, &sx_ecc_curve_p384,
        "NIST/SECG curve over a 384 bit prime field"
    },
    {
        NID_secp521r1, &sx_ecc_curve_p521,
        "NIST/SECG curve over a 521 bit prime field"
    },
    {
        NID_sect163r2, &sx_ecc_curve_b163,
        "NIST/SECG curve over a 163 bit binary field"
    },
    {
        NID_sect233r1, &sx_ecc_curve_b233,
        "NIST/SECG/WTLS curve over a 233 bit binary field"
    },
    {
        NID_sect283r1, &sx_ecc_curve_b283,
        "NIST/SECG curve over a 283 bit binary field"
    },
    {
        NID_sect409r1, &sx_ecc_curve_b409,
        "NIST/SECG curve over a 409 bit binary field"
    },
    {
        NID_sect571r1, &sx_ecc_curve_b571,
        "NIST/SECG curve over a 571 bit binary field"
    },
    {
        NID_sect163k1, &sx_ecc_curve_k163,
        "NIST/SECG/WTLS curve over a 163 bit binary field"
    },
    {
        NID_sect233k1, &sx_ecc_curve_k233,
        "NIST/SECG/WTLS curve over a 233 bit binary field"
    },
    {
        NID_sect283k1, &sx_ecc_curve_k283,
        "NIST/SECG curve over a 283 bit binary field"
    },
    {
        NID_sect409k1, &sx_ecc_curve_k409,
        "NIST/SECG curve over a 409 bit binary field"
    },
    {
        NID_sect571k1, &sx_ecc_curve_k571,
        "NIST/SECG curve over a 571 bit binary field"
    },
    {
        NID_sece521r1, &sx_ecc_curve_e521,
        "NIST/SECG curve over a 521 bit prime field"
    },
    {
        NID_sxiptestp256, &sx_ecc_curve_p256_iptest,
        "for ip test"
    },
};

//*****************************************************************************
//
//! hal_ecc_new_by_curve_name.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param nid input, the curve id , define in curve_list
//!
//! This function is for init ecc curve info
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_ecc_new_by_curve_name(void* handle, int nid);

//*****************************************************************************
//
//! hal_ecc_generate_priv_key.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param n input, the priv key k, must 1 <= k < n
//! @param priv_key output, the priv key
//!
//! This function is for generate priv key
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_ecc_generate_priv_key(void* handle, block_t n, block_t priv_key);

//*****************************************************************************
//
//! hal_ecc_generate_pub_key.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param ec_key_t input/output, the priv key as input, pub key as output
//!
//! This function is for generate pub key
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_ecc_generate_pub_key(void* handle, ec_key_t* eckey);

//*****************************************************************************
//
//! hal_ecc_generate_key.
//!
//! @param handle input, opration handle for uplayer,creat by hal_crypto_creat_handle
//! @param n input, the priv key k, must 1 <= k < n
//! @param eckey output,  priv and pub key
//!
//! This function is for generate priv and pub key
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_ecc_generate_key(void* handle, block_t n, ec_key_t* eckey);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_ECC_H__

