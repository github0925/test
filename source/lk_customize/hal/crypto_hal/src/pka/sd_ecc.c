//*****************************************************************************
//
// sd_ecc.c - hal ecc Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_ecc.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define OSSL_NELEM(x)    (sizeof(x)/sizeof(x[0]))
#define curve_list_length OSSL_NELEM(curve_list)

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
crypto_status_t hal_ecc_new_by_curve_name(void* handle, int nid)
{
    crypto_status_t ret = CRYPTO_ERROR;
    crypto_instance_t* l_cryptoinstance = NULL;
    sd_ecc_curve_t* ecc_curve_temp;
    size_t i;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        if (nid <= 0) {
            return ret;
        }

        for (i = 0; i < curve_list_length; i++) {
            if (curve_list[i].nid == nid) {
                l_cryptoinstance->p_ecc_curve_info = curve_list[i].data;
                ecc_curve_temp = (sd_ecc_curve_t*)curve_list[i].data;
                l_cryptoinstance->group = ecc_curve_temp->params;
                l_cryptoinstance->pk_flags = ecc_curve_temp->pk_flags;
                l_cryptoinstance->bytesize = ecc_curve_temp->bytesize;
                ret = CRYPTO_SUCCESS;
                break;
            }
        }

    }

    return ret;
}

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
crypto_status_t hal_ecc_generate_priv_key(void* handle, block_t n, block_t priv_key)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_ERROR;
    crypto_instance_t* l_cryptoinstance = NULL;


    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if ((vce_id & RSA_SUPPORT_PKA_ID_MASK) == 0) {
            if (l_cryptoinstance->ec_method->prikey_gen) {
                ret = l_cryptoinstance->ec_method->prikey_gen(vce_id, n, priv_key);
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }

    }

    return ret;
}

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
crypto_status_t hal_ecc_generate_pub_key(void* handle, ec_key_t* eckey)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_ERROR;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if ((vce_id & RSA_SUPPORT_PKA_ID_MASK) == 0) {
            if (l_cryptoinstance->ec_method->pubkey_gen) {
                ret = l_cryptoinstance->ec_method->pubkey_gen(vce_id, l_cryptoinstance->group,
                        eckey->pub_key, eckey->priv_key,
                        l_cryptoinstance->bytesize, l_cryptoinstance->pk_flags);
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }

    }

    return ret;
}

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
crypto_status_t hal_ecc_generate_key(void* handle, block_t n, ec_key_t* eckey)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_ERROR;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if ((vce_id & RSA_SUPPORT_PKA_ID_MASK) == 0) {

            if (l_cryptoinstance->ec_method->prikey_gen) {
                ret = l_cryptoinstance->ec_method->prikey_gen(vce_id, n, eckey->priv_key);
            }

            if (l_cryptoinstance->ec_method->pubkey_gen) {
                ret = l_cryptoinstance->ec_method->pubkey_gen(vce_id, l_cryptoinstance->group,
                        eckey->pub_key, eckey->priv_key,
                        l_cryptoinstance->bytesize, l_cryptoinstance->pk_flags);
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }

    }

    return ret;
}

