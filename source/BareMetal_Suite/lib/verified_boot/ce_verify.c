/*
 * ce_verify.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#define DEBUG_ON 1
#include <malloc.h>
#include "atb_crypto.h"
#include "verified_boot.h"
#include "sd_x509.h"

#define EC_P521R1_LEN 66
#define EC_P384R1_LEN 48
#define EC_P256V1_LEN 32

#define LOCAL_TRACE 0
#define LTRACEF(x...) do { if (LOCAL_TRACE) { WARN(x);  }  } while (0)

#define ERROR(format, args...) WARN(\
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

/* if src is aligned, *dst = src,
 * otherwise, allocate aligned memory
 * and copy data
 * */
static bool cacheline_aligned_buffer(addr_t src, uint64_t src_len,
                                     addr_t *dst, uint64_t dst_len,
                                     bool clear, uint64_t offset)
{
    if (!src || !dst)
        return false;

    if (IS_ALIGNED(src, CACHE_LINE)) {
        *dst = src;
    }
    else {
        *dst = (addr_t)memalign(CACHE_LINE, dst_len);

        if (!*dst) {
            ERROR("%s allocat memory fail\n", __func__);
            return false;
        }

        if (clear)
            memset((uint8_t *)(*dst), 0x0, dst_len);

        memcpy((uint8_t *)(*dst) + offset, (uint8_t *)src, src_len);
        return true;
    }

    return false;
}

void crypto_eng_init(void)
{
    if (crypto_init()) {
        ERROR("Fail to init crypto eng!\n");
    }
}

void crypto_eng_deinit(void)
{
    crypto_deinit();
}

uint32_t calc_hash(const uint8_t *buf, uint32_t buf_len, uint8_t *dig,
                   uint32_t dig_len, AvbDigestType hash_type)
{
    U32 len = dig_len;
    uint32_t ret;

    crypto_alg_hash_e ce_hash_alg = ALG_HASH_SHA256;

    if (hash_type == AVB_DIGEST_TYPE_SHA256) {
        ce_hash_alg = ALG_HASH_SHA256;
    }
    else if (hash_type == AVB_DIGEST_TYPE_SHA512) {
        ce_hash_alg = ALG_HASH_SHA512;
    }
    else {
        ERROR("unsupport hash alg!\n");
        return 1;
    }

    ret = crypto_hash(ce_hash_alg, (addr_t)buf, buf_len, (addr_t)dig, &len);

    if (ret) {
        ERROR("Fail to calc hash ret:%u!\n", ret);
        return 2;
    }

    return 0;
}

static uint32_t verifiy_rsa_sign(const uint8_t *tbs, uint32_t tbs_len,
                                 const uint8_t *sig, uint32_t sig_len,
                                 const uint8_t *modulus, uint32_t modulus_len,
                                 const uint8_t *expo, uint32_t expo_len, X509_SIGN_T sign_type)
{
    uint32_t ret = 1;
    uint8_t *sig_align = NULL;
    bool sign_allocat = false;
    crypto_alg_hash_e hash_nid = ALG_HASH_SHA256;
    rsa_key_t *rsa_key = NULL;
    uint8_t *expo_r = NULL;

    if (sign_type == SIGN_SHA256_RSA_PKCS1) {
        hash_nid = ALG_HASH_SHA256;
    }
    else if (sign_type == SIGN_SHA512_RSA_PKCS1) {
        hash_nid = ALG_HASH_SHA512;
    }
    else if (sign_type == SIGN_SHA256_RSASSA_PSS) {
        hash_nid = ALG_HASH_SHA256;
    }
    else if (sign_type == SIGN_SHA512_RSASSA_PSS) {
        hash_nid = ALG_HASH_SHA512;
    }
    else {
        ERROR("unsupport hash type\n");
        goto end;
    }

#if 0
    pub_key.n_len = modulus_len;
    modules_allocat =  cacheline_aligned_buffer((addr_t)modulus, modulus_len,
                       (addr_t *) & (pub_key.n), pub_key.n_len, false, 0);

    pub_key.e_len = modulus_len;
    expo_allocat =  cacheline_aligned_buffer((addr_t)expo, expo_len,
                    (addr_t *) & (pub_key.e), pub_key.e_len,
                    true, pub_key.e_len - expo_len);
#endif
    rsa_key = memalign(CACHE_LINE, sizeof(rsa_key_t));

    if (!rsa_key) {
        ERROR("Fail to allocate memory!");
        goto end;
    }

    memset(&(rsa_key->e[0]), 0x0, modulus_len);
    rsa_key->n_sz = modulus_len;
    memcpy(&(rsa_key->n[0]), modulus, modulus_len);
    expo_r = (uint8_t *)(&rsa_key->e[0]);
    expo_r += modulus_len - expo_len;
    memcpy(expo_r, expo, expo_len);

    sign_allocat =  cacheline_aligned_buffer((addr_t)sig, sig_len,
                    (addr_t *)&sig_align, sig_len, false, 0);

    ret = crypto_rsa_verify(hash_nid, (addr_t)tbs, tbs_len,
                            (addr_t)sig_align, sig_len, (addr_t)rsa_key);

    if (ret) {
        ERROR("Fail to verify cert,ret:%d", ret)
    }

end:

    if (rsa_key)
        free(rsa_key);

    if (sign_allocat && sig_align)
        free(sig_align);

    return ret;
}

static uint32_t verifiy_ecdsa_sign(const uint8_t *tbs, uint32_t tbs_len,
                                   const uint8_t *sig, uint32_t sig_len,
                                   const uint8_t *pubkey, uint32_t pubkey_len,
                                   X509_SIGN_T sign_type)
{
    uint32_t ret = 1;
    uint8_t *sig_align = NULL;
    X509_PUBKEY_TYPE pubkey_type;
    bool sign_allocat = false;
    crypto_alg_hash_e hash_nid = ALG_HASH_SHA256;
    ecdsa_key_t *ec_key = NULL;

    ec_key = memalign(CACHE_LINE, sizeof(ecdsa_key_t));

    if (!ec_key) {
        ERROR("Fail to allocate memory!");
        goto end;
    }

    if (sign_type == SIGN_SHA256_ECCDSA) {
        hash_nid = ALG_HASH_SHA256;
    }
    else if (sign_type == SIGN_SHA512_ECCDSA) {
        hash_nid = ALG_HASH_SHA512;
    }
    else {
        ERROR("unsupport hash type\n");
        goto end;
    }

    pubkey_type = X509ECPubkeyTypeBylen(pubkey_len / 2);

    if (pubkey_type == PUBKEY_EC_P256V1) {
        ec_key->prime_sz = EC_P256V1_LEN;
    }
    else  if (pubkey_type == PUBKEY_EC_SECP384R1) {
        ec_key->prime_sz = EC_P384R1_LEN;
    }
    else if (pubkey_type == PUBKEY_EC_SECP521R1) {
        ec_key->prime_sz = EC_P521R1_LEN;
    }
    else {
        ERROR("%s public\n", __func__);
        goto end;
    }

    memcpy(&(ec_key->x[0]), pubkey, ec_key->prime_sz);
    memcpy(&(ec_key->y[0]), pubkey + ec_key->prime_sz, ec_key->prime_sz);

    LTRACEF("hash:%d\n", hash_nid);

    sign_allocat =  cacheline_aligned_buffer((addr_t)sig, sig_len,
                    (addr_t *)&sig_align, sig_len, false, 0);

    ret = crypto_ecdsa_verify(hash_nid, (addr_t)tbs, tbs_len, (addr_t)sig, sig_len,
                              (addr_t)ec_key);

    LTRACEF("ECDSA verify ret:%d\n", ret);

    if (ret) {
        ERROR("ECDSA verify  fail ret:%d\n", ret);
    }
    else {
        LTRACEF("ECDSA verify success\n");
    }

end:

    if (ec_key)
        free(ec_key);

    if (sign_allocat && sig_align)
        free(sig_align);

    return ret;

}

uint32_t verify_cert_sign(const uint8_t *tbs, uint32_t tbs_len,
                          const uint8_t *sig, uint32_t sig_len,
                          const uint8_t *modulus, uint32_t modulus_len,
                          const uint8_t *expo, uint32_t expo_len, X509_SIGN_T sign_type)
{
    if (sign_type == SIGN_SHA256_ECCDSA
            || sign_type == SIGN_SHA512_ECCDSA) {
        return verifiy_ecdsa_sign(tbs, tbs_len, sig, sig_len,
                                  modulus, modulus_len, sign_type);
    }
    else {
        return verifiy_rsa_sign(tbs, tbs_len, sig, sig_len,
                                modulus, modulus_len, expo, expo_len, sign_type);
    }
}
