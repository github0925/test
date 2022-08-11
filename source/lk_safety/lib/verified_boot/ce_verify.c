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

#define DEBUG_ON 0
#include <app.h>
#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <chip_res.h>
#include <trace.h>

#include "crypto_hal.h"
#include "sd_hash.h"
#include "sd_ecc.h"
#include "sd_ecdsa.h"
#include "sd_rsa.h"
#include "res.h"
#include "internal.h"

#define LOCAL_TRACE 0

#define VCE_ID_DEF  (g_ce_mem_res.res_id[0])

#if DEBUG_ON
#define DEBUG_DUMP(ptr, size, format, args...) \
    do{ \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args); \
        hexdump8(ptr, size); \
    }while(0);

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);
#else
#define DEBUG_DUMP(ptr, size, format, args...)
#define ERROR(format, args...)
#endif

extern const domain_res_t g_ce_mem_res;

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

static bool __calc_hash(void *handle, const uint8_t *buf, uint32_t buf_len,
                        uint8_t *dig,
                        uint32_t dig_len, AvbDigestType hash_type)
{
    crypto_status_t status;
    int hash_nid = SD_ALG_SHA256;

    if (hash_type == AVB_DIGEST_TYPE_SHA256) {
        hash_nid = SD_ALG_SHA256;
    }
    else if (hash_type == AVB_DIGEST_TYPE_SHA512) {
        hash_nid = SD_ALG_SHA512;
    }
    else {
        return false;
    }

    status = hal_hash(handle, hash_nid, buf, buf_len, dig, dig_len);
    LTRACEF("hash status:0x%0x hash_nid:%d\n", status, hash_nid);

    return (status ==  CRYPTO_SUCCESS) ? true : false;
}

uint32_t calc_hash(const uint8_t *buf, uint32_t buf_len, uint8_t *dig,
                   uint32_t dig_len, AvbDigestType hash_type)
{
    bool result = false;
    void *crypto_handle = NULL;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_DEF);

    if (crypto_handle == NULL) {
        ERROR("%s create handle error\n", __func__);
        return 1;
    }

    result = __calc_hash(crypto_handle, buf, buf_len, dig, dig_len, hash_type);
    hal_crypto_delete_handle(crypto_handle);

    return result ? 0 : 1;
}

static uint32_t verifiy_rsa_sign(const uint8_t *tbs, uint32_t tbs_len,
                                 const uint8_t *sig, uint32_t sig_len,
                                 const uint8_t *modulus, uint32_t modulus_len,
                                 const uint8_t *expo, uint32_t expo_len, X509_SIGN_T sign_type)
{
    int padding;
    int hash_nid;
    uint32_t ret = 1;
    uint32_t salt_len;
    rsa_pubkey_t pub_key;
    uint8_t *sig_align = NULL;
    crypto_status_t verify;
    void *crypto_handle = NULL;
    bool sign_allocat = false;
    bool modules_allocat = false;
    bool expo_allocat = false;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_DEF);

    if (crypto_handle == NULL) {
        ERROR("%s create handle error\n", __func__);
        goto end;
    }

    if (sign_type == SIGN_SHA256_RSA_PKCS1) {
        hash_nid = SD_ALG_SHA256;
        padding = SD_RSA_PADDING_EMSA_PKCS;
        salt_len = 32;
    }
    else if (sign_type == SIGN_SHA512_RSA_PKCS1) {
        hash_nid = SD_ALG_SHA512;
        padding = SD_RSA_PADDING_EMSA_PKCS;
        salt_len = 64;
    }
    else if (sign_type == SIGN_SHA256_RSASSA_PSS) {
        hash_nid = SD_ALG_SHA256;
        padding = SD_RSA_PADDING_PSS;
        salt_len = 32;
    }
    else if (sign_type == SIGN_SHA512_RSASSA_PSS) {
        hash_nid = SD_ALG_SHA512;
        padding = SD_RSA_PADDING_PSS;
        salt_len = 64;
    }
    else {
        ERROR("unsupport hash type\n");
        goto end;
    }

    pub_key.n_len = modulus_len;
    modules_allocat =  cacheline_aligned_buffer((addr_t)modulus, modulus_len,
                       (addr_t *) & (pub_key.n), pub_key.n_len, false, 0);

    pub_key.e_len = modulus_len;
    expo_allocat =  cacheline_aligned_buffer((addr_t)expo, expo_len,
                    (addr_t *) & (pub_key.e), pub_key.e_len,
                    true, pub_key.e_len - expo_len);


    sign_allocat =  cacheline_aligned_buffer((addr_t)sig, sig_len,
                    (addr_t *)&sig_align, sig_len, false, 0);

#if 0
    DEBUG_DUMP(sig_align, sig_len, "sig_align:\n");

    DEBUG_DUMP(pub_key.e, modulus_len, "expo:\n");

    DEBUG_DUMP(pub_key.n, modulus_len, "modulus:\n");

    DEBUG_DUMP(tbs, tbs_len, "tbs:\n");
#endif
    lk_bigtime_t start = current_time_hires();
    verify = hal_rsa_verify(crypto_handle, hash_nid, tbs, tbs_len,
                            sig_align, sig_len, &pub_key, padding, salt_len);

    start = current_time_hires() - start;
    LTRACEF("RSA verify ret:%d time:%llu\n", verify, start);

    if (verify == CRYPTO_SUCCESS) {
        LTRACEF("RSA verify success\n");
        ret = 0;
    }
    else {
        ERROR("RSA verify  fail ret:%d\n", verify);
    }

end:

    if (modules_allocat && pub_key.n)
        free(pub_key.n);

    if (expo_allocat && pub_key.e)
        free(pub_key.e);

    if (sign_allocat && sig_align)
        free(sig_align);

    if (crypto_handle)
        hal_crypto_delete_handle(crypto_handle);

    return ret;
}

static uint32_t verifiy_ecdsa_sign(const uint8_t *tbs, uint32_t tbs_len,
                                   const uint8_t *sig, uint32_t sig_len,
                                   const uint8_t *pubkey, uint32_t pubkey_len,
                                   X509_SIGN_T sign_type)
{
    int hash_nid;
    int curve_nid;
    ec_key_t eckey;
    uint32_t ret = 1;
    crypto_status_t verify;
    uint8_t *sig_align = NULL;
    void *crypto_handle = NULL;
    uint8_t *pubkey_align = NULL;
    X509_PUBKEY_TYPE pubkey_type;
    bool sign_allocat = false;
    bool pubkey_allocat = false;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_DEF);

    if (crypto_handle == NULL) {
        ERROR("%s create handle error\n", __func__);
        goto end;
    }

    if (sign_type == SIGN_SHA256_ECCDSA) {
        hash_nid = SD_ALG_SHA256;
    }
    else if (sign_type == SIGN_SHA512_ECCDSA) {
        hash_nid = SD_ALG_SHA512;
    }
    else {
        ERROR("unsupport hash type\n");
        goto end;
    }

    pubkey_type = X509ECPubkeyTypeBylen(pubkey_len / 2);

    if (pubkey_type == PUBKEY_EC_P256V1) {
        curve_nid = NID_X9_62_prime256v1;
    }
    else  if (pubkey_type == PUBKEY_EC_SECP384R1) {
        curve_nid = NID_secp384r1;
    }
    else if (pubkey_type == PUBKEY_EC_SECP521R1) {
        curve_nid = NID_secp521r1;
    }
    else {
        ERROR("%s public\n", __func__);
        goto end;
    }

    LTRACEF("curve_nid:%d hash:%d\n", curve_nid, hash_nid);

    pubkey_allocat =  cacheline_aligned_buffer((addr_t)pubkey, pubkey_len,
                      (addr_t *)&pubkey_align, pubkey_len, false, 0);

    sign_allocat =  cacheline_aligned_buffer((addr_t)sig, sig_len,
                    (addr_t *)&sig_align, sig_len, false, 0);

    DEBUG_DUMP(sig_align, sig_len, "sig_align:\n");

    DEBUG_DUMP(pubkey_align, pubkey_len, "pubkey:\n");

    DEBUG_DUMP(tbs, tbs_len, "tbs:\n");

    eckey.priv_key = block_t_convert(NULL, 0, EXT_MEM);
    eckey.pub_key = block_t_convert(pubkey_align, pubkey_len, EXT_MEM);

    lk_bigtime_t start = current_time_hires();

    verify = hal_ecdsa_verify(crypto_handle, curve_nid, tbs, tbs_len,
                              sig_align, sig_len, &eckey, hash_nid);

    start = current_time_hires() - start;
    LTRACEF("ECDSA verify ret:%d time:%llu\n", verify, start);

    if (verify == CRYPTO_SUCCESS) {
        LTRACEF("ECDSA verify success\n");
        ret = 0;
    }
    else {
        ERROR("ECDSA verify  fail ret:%d\n", verify);
    }

end:

    if (pubkey_allocat && pubkey_align)
        free((void *)(addr_t)pubkey_align);

    if (sign_allocat && sig_align)
        free((void *)(addr_t)sig_align);

    if (crypto_handle)
        hal_crypto_delete_handle(crypto_handle);

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
