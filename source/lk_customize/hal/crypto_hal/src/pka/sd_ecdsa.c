//*****************************************************************************
//
// sd_ecdsa.c - hal for ecdsa.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_ecdsa.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

//*****************************************************************************
//
//! hal_ecdsa_sign.
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
//! This function is for ecdsa signature
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_ecdsa_sign(void* handle, int curve_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                           unsigned out_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        hal_ecc_new_by_curve_name(handle, curve_nid);

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->ec_method->ecdsa_sign) {
            ret = l_cryptoinstance->ec_method->ecdsa_sign(vce_id, l_cryptoinstance->p_ecc_curve_info,
                    block_t_convert(in, in_len, addr_type),
                    block_t_convert(eckey->priv_key.addr, eckey->priv_key.len, HAL_EXT_MEM),
                    block_t_convert(out, out_len, HAL_EXT_MEM),
                    hash_fct);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_ecdsa_verify.
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
//! This function is for ecdsa verify
//!
//! @return crypto_status_t CRYPTO_SUCCESS: verify pass
//
//*****************************************************************************
crypto_status_t hal_ecdsa_verify(void* handle, int curve_nid, const uint8_t* msg, size_t msg_len,
                             const uint8_t* sig, size_t sig_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        hal_ecc_new_by_curve_name(handle, curve_nid);

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->ec_method->ecdsa_verify) {
            ret = l_cryptoinstance->ec_method->ecdsa_verify(vce_id, l_cryptoinstance->p_ecc_curve_info,
                    block_t_convert(msg, msg_len, addr_type),
                    block_t_convert(eckey->pub_key.addr, eckey->pub_key.len, HAL_EXT_MEM),
                    block_t_convert(sig, sig_len, addr_type),
                    hash_fct);
        }
    }

    return ret;
}