//*****************************************************************************
//
// sd_sm2.c - .
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_sm2.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define RSA_SUPPORT_PKA_ID_MASK 0x04 // 0~3 support pka 4~7 donot support

#define SM2_MAX_ID_LENGTH 16
#define SM2_ID_DIGEST_MSG_LEN_MAX 210 // id_(16) + id_len(2) + a/b(64) + x/y(64) + pubkey(64)

uint8_t __attribute__((aligned(CACHE_LINE))) sd_sm2_z_msg[SM2_ID_DIGEST_MSG_LEN_MAX];
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
                         unsigned out_len, const ec_key_t* eckey, sd_hash_alg_t hash_fct)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    uint8_t * z_msg_ptr;
    uint32_t z_msg_len = 0;
    block_t out_block = block_t_convert(out, out_len, HAL_EXT_MEM);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        hal_ecc_new_by_curve_name(handle, curve_nid);

        vce_id = l_cryptoinstance->vce_id;

        if (strlen((char *)in) != in_len) {
            return CRYPTO_ERROR;
        }
        if (in_len > SM2_MAX_ID_LENGTH || in_len <= 0) {
            return CRYPTO_ERROR;
        }

        if (l_cryptoinstance->ec_method->sm2_generate_digest) {

            /*ZA= H256(ENTLA||IDA||a||b||xG||yG||xA||yA)*/
            z_msg_len = in_len + 194;

            z_msg_ptr = sd_sm2_z_msg;
            memset(z_msg_ptr, 0x0, z_msg_len);

            /* 2-byte id length in bits */
            sd_sm2_z_msg[0] = ((in_len * 8) >> 8) % 256;
            sd_sm2_z_msg[1] = (in_len * 8) % 256;

            memcpy(z_msg_ptr+2, in, in_len);
            memcpy(z_msg_ptr+2+in_len, ((sd_ecc_curve_t*)(l_cryptoinstance->p_ecc_curve_info))->params.addr+128, 64); //copy a b
            memcpy(z_msg_ptr+2+in_len+64, ((sd_ecc_curve_t*)(l_cryptoinstance->p_ecc_curve_info))->params.addr+64, 64); //copy x y
            memcpy(z_msg_ptr+2+in_len+64+64, eckey->pub_key.addr, 64); //copy pubkey

            ret = l_cryptoinstance->ec_method->sm2_generate_digest(vce_id, l_cryptoinstance->p_ecc_curve_info,
                    block_t_convert(z_msg_ptr, z_msg_len, addr_type),
                    hash_fct,
                    &out_block);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! SM2_sign.
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

        if (l_cryptoinstance->ec_method->sm2_sign) {
            ret = l_cryptoinstance->ec_method->sm2_sign(vce_id, l_cryptoinstance->p_ecc_curve_info,
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
//! SM2_verify.
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

        if (l_cryptoinstance->ec_method->sm2_verify) {
            ret = l_cryptoinstance->ec_method->sm2_verify(vce_id, l_cryptoinstance->p_ecc_curve_info,
                    block_t_convert(msg, msg_len, addr_type),
                    block_t_convert(eckey->pub_key.addr, eckey->pub_key.len, HAL_EXT_MEM),
                    block_t_convert(sig, sig_len, addr_type),
                    hash_fct);
        }
    }

    return ret;
}

uint8_t sm2_cofactor_exc_ip_test[32] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"; //-- CoFactor
uint8_t sm2_pointa_exc_ip_test[64] = "\x6c\xb5\x63\x38\x16\xf4\xdd\x56\x0b\x1d\xec\x45\x83\x10\xcb\xcc\x68\x56\xc0\x95\x05\x32\x4a\x6d\x23\x15\x0c\x40\x8f\x16\x2b\xf0" //-- RAx
                                     "\x0d\x6f\xcf\x62\xf1\x03\x6c\x0a\x1b\x6d\xac\xcf\x57\x39\x92\x23\xa6\x5f\x7d\x7b\xf2\xd9\x63\x7e\x5b\xbb\xeb\x85\x79\x61\xbf\x1a"; //-- RAy
uint8_t sm2_two_w_exc_ip_test[32] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"; //-- Two_w

//*****************************************************************************
//
//! SM2_key_exchange.
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
                                 const uint8_t* key, size_t key_len)
{
    uint32_t vce_id;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        hal_ecc_new_by_curve_name(handle, curve_nid);

        //need init 2w and cofactor by nid
        if (curve_nid == NID_sxiptestp256) {
            l_cryptoinstance->cofactor = block_t_convert(sm2_cofactor_exc_ip_test, 32, addr_type);
            l_cryptoinstance->two_w = block_t_convert(sm2_two_w_exc_ip_test, 32, addr_type);
        }

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->ec_method->sm2_key_exchange) {
            ret = l_cryptoinstance->ec_method->sm2_key_exchange(vce_id, l_cryptoinstance->p_ecc_curve_info,
                    block_t_convert(eckey->priv_key.addr, eckey->priv_key.len, addr_type),
                    block_t_convert(eckey->pub_key.addr, eckey->pub_key.len, addr_type),
                    block_t_convert(remote_point, remote_point_len, addr_type),
                    l_cryptoinstance->cofactor,
                    block_t_convert(sm2_pointa_exc_ip_test, 64, addr_type),
                    l_cryptoinstance->two_w,
                    block_t_convert(key, key_len, addr_type));
        }
    }

    return ret;
}
