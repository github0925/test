/*
 * boringssl_verify.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <app.h>
#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "internal.h"

#define LOCAL_TRACE 0

uint32_t calc_hash(const uint8_t *buf, uint32_t buf_len, uint8_t *dig,
                   uint32_t dig_len, AvbDigestType hash_type)
{
    int result = 0;

    if (hash_type == AVB_DIGEST_TYPE_SHA256) {
        if (dig_len < SHA256_DIGEST_LENGTH) {
            dprintf(CRITICAL, "dig_len is too small!\n");
            return 1;
        }

        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, buf, buf_len);
        result = SHA256_Final(dig, &ctx);
    }
    else if (hash_type == AVB_DIGEST_TYPE_SHA512) {
        if (dig_len < SHA512_DIGEST_LENGTH) {
            dprintf(CRITICAL, "dig_len is too small!\n");
            return 1;
        }

        SHA512_CTX ctx;
        SHA512_Init(&ctx);
        SHA512_Update(&ctx, buf, buf_len);
        result = SHA512_Final(dig, &ctx);
    }
    else {
        dprintf(CRITICAL, "unsupport hash type:%d\n", hash_type);
        return 1;
    }

    return (result == 1) ? 0 : 1;
}

uint32_t verify_cert_sign(const uint8_t *tbs, uint32_t tbs_len,
                          const uint8_t *sig, uint32_t sig_len,
                          const uint8_t *modulus, uint32_t modulus_len,
                          const uint8_t *expo, uint32_t expo_len, X509_SIGN_T sign_type)
{
    int hash_nid;
    int verify = 0;
    uint32_t ret = 1;
    uint8_t *em = NULL;
    uint32_t digest_len;
    size_t em_len = 0;
    bool rssssapss = false;
    AvbDigestType hash_type;
    uint8_t *digest = NULL;
    RSA *public_key = NULL;
    BIGNUM *public_key_e = NULL;
    BIGNUM *public_key_n = NULL;

    if (sign_type == SIGN_SHA256_RSA_PKCS1) {
        digest_len = SHA256_DIGEST_LENGTH;
        hash_nid = NID_sha256;
        hash_type = AVB_DIGEST_TYPE_SHA256;
    }
    else if (sign_type == SIGN_SHA512_RSA_PKCS1) {
        digest_len = SHA512_DIGEST_LENGTH;
        hash_nid = NID_sha512;
        hash_type = AVB_DIGEST_TYPE_SHA512;
    }
    else if (sign_type == SIGN_SHA256_RSASSA_PSS) {
        digest_len = SHA256_DIGEST_LENGTH;
        hash_nid = NID_sha256;
        hash_type = AVB_DIGEST_TYPE_SHA256;
        rssssapss = true;
    }
    else if (sign_type == SIGN_SHA512_RSASSA_PSS) {
        digest_len = SHA512_DIGEST_LENGTH;
        hash_nid = NID_sha512;
        hash_type = AVB_DIGEST_TYPE_SHA512;
        rssssapss = true;
    }
    else {
        dprintf(CRITICAL, "unsupport signature alg\n");
        goto out;
    }

    digest = calloc(1, digest_len);

    if (!digest) {
        dprintf(CRITICAL, "allocat memory fail\n");
        goto out;
    }

    calc_hash(tbs, tbs_len, digest, digest_len, hash_type);
    public_key    = RSA_new();
    public_key_e  = BN_new();
    public_key_n  = BN_new();

    BN_bin2bn(modulus, modulus_len, public_key_n);
    BN_bin2bn(expo, expo_len, public_key_e);

    verify = RSA_set0_key(public_key, public_key_n, public_key_e, NULL);
#if 0
    dprintf(CRITICAL, "expo:\n");
    hexdump8(expo, expo_len);

    dprintf(CRITICAL, "modulus:\n");
    hexdump8(modulus, modulus_len);

    dprintf(CRITICAL, "digest:\n");
    hexdump8(digest, digest_len);

    dprintf(CRITICAL, "tbs:\n");
    hexdump8(tbs, tbs_len);

    dprintf(CRITICAL, "sig:\n");
    hexdump8(sig, sig_len);
#endif
    em = calloc(1, modulus_len);

    if (!em) {
        dprintf(CRITICAL, "allocat memory for em fail\n");
        goto out;
    }

    if (rssssapss) {
#if 1
        verify = RSA_verify_raw(public_key, &em_len, em,
                                modulus_len, sig, sig_len, RSA_NO_PADDING);

        if (em_len != modulus_len) {
            dprintf(CRITICAL, "em_len %llu error\n", (uint64_t)em_len);
            goto out;
        }

        verify = RSA_verify_PKCS1_PSS_mgf1(public_key,
                                           digest,
                                           EVP_sha256(),
                                           EVP_sha256(),
                                           em, -1);
        dprintf(CRITICAL, "plain:\n");
        hexdump8(em, em_len);
#else
        verify = RSA_verify_pss_mgf1(public_key, digest,
                                     digest_len, EVP_sha256(),
                                     EVP_sha256(), -1,
                                     sig, sig_len);
#endif
        LTRACEF("ret:%d\n", verify);
    }
    else {
#if 0
        verify = RSA_verify_raw(public_key, &em_len, em,
                                modulus_len, sig, sig_len, RSA_NO_PADDING);

        if (em_len != modulus_len) {
            dprintf(CRITICAL, "em_len %d error\n", em_len);
            goto out;
        }

        dprintf(CRITICAL, "plain:\n");
        hexdump8(em, em_len);
#endif
        lk_bigtime_t start = current_time_hires();
        verify = RSA_verify(hash_nid, digest, digest_len,
                            sig, sig_len, public_key);
        start = current_time_hires() - start;
        LTRACEF("RSA verify ret:%d time:%llu\n", verify, start);
    }

    LTRACEF("ret:%d\n", verify);
    ret = verify ? 0 : 1;
out:

    if (public_key_n)
        BN_free(public_key_n);

    if (public_key_e)
        BN_free(public_key_e);

    if (public_key)
        RSA_free(public_key);

    if (digest)
        free(digest);

    if (em)
        free(em);

    return ret;
}

