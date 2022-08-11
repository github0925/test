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

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include "internal.h"

#define OUTPUT_BUF_LEN  0x1000
#define TBS_BUF_LEN     0x400

int boringssl_verify_cert(uint8_t *tbs, uint32_t tbs_len,
                          uint8_t *sig, uint32_t sig_len,
                          uint8_t *modulus, uint32_t modulus_len,
                          uint8_t *expo, uint32_t expo_len, hash_type hash)
{
    uint32_t ret = 0;
    RSA *cakey = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *ca_n = NULL;
    //BN_ULONG exp_l = 0x10001;
    uint8_t output[512] = {0};
    uint32_t actual_size = 0;
    SHA256_CTX ctx;
    uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
    int hash_nid = NID_sha256;

    if (!modulus) {
        dprintf(CRITICAL, "%s modulus is null\n", __func__);
        ret = -1;
        goto end;
    }

    if (!tbs) {
        dprintf(CRITICAL, "%s tbs is null\n", __func__);
        ret = -1;
        goto end;
    }

    if (!sig) {
        dprintf(CRITICAL, "%s sig is null\n", __func__);
        ret = -1;
        goto end;
    }

    if (hash == HASH_TYPE_SHA256) {
        hash_nid = NID_sha256;
    }
    else if (hash == HASH_TYPE_SHA512) {
        hash_nid = NID_sha512;
    }

    cakey = RSA_new();
    exp = BN_new();
    ca_n = BN_new();

    BN_bin2bn(modulus, modulus_len, ca_n);
    BN_bin2bn(expo, expo_len, exp);
    //BN_set_word(exp, exp_l);

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, tbs, tbs_len);
    SHA256_Final(digest, &ctx);
    dprintf(CRITICAL, "%s tbs sha nid:%d\n", __func__, hash_nid);
    hexdump8(digest, SHA256_DIGEST_LENGTH);

    ret = RSA_set0_key(cakey, ca_n, exp, NULL);
    dprintf(CRITICAL, "%s ret:%d\n", __func__, ret);
    //RSA_verify(int hash_nid, tbs_align, tbs_len, sig_align, sig_len, cakey);
    dprintf(CRITICAL, "%s %d tbs:\n", __func__, __LINE__ );
    hexdump8(tbs, tbs_len);
    dprintf(CRITICAL, "\n\n%s %d sign:\n", __func__, __LINE__ );
    hexdump8(sig, sig_len);
    //ret = RSA_verify_raw(cakey, &actual_size, output, sizeof output, sig_align, sig_len, RSA_PKCS1_PADDING);
    ret = RSA_verify(hash_nid, digest, SHA256_DIGEST_LENGTH,
                     sig, sig_len, cakey);
    dprintf(CRITICAL, "%s ret:%d\n", __func__, ret);
    hexdump8(output, actual_size);
    ret = 0;
end:
    return ret;
}

uint32_t boringssl_parse_x509(uint8_t *buffer, uint32_t size)
{
    int ret = 0;
    BIO *in = NULL;
    BIO *output = NULL;
    X509 *cert = NULL;
    //X509_NAME *xn = NULL;
    RSA *rsa = NULL;
    RSA *cakey = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *ca_pkey = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *modulus = NULL;
    BIGNUM *ca_n = NULL;
    unsigned char *outstr = NULL;

    in = BIO_new(BIO_s_mem());
    output = BIO_new(BIO_s_mem());
    BIO_puts(in, (char *)buffer);
    cakey = RSA_new();

    dprintf(CRITICAL, "%s %d\n", __func__, __LINE__);

    if (PEM_read_bio_X509(in, &cert, NULL, NULL) == NULL) {
        dprintf(CRITICAL, "%s %d\n", __func__, __LINE__);
        return 1;
    }

#if 0
    xn = X509_get_subject_name(cert);

    if (xn == NULL) {
        dprintf(CRITICAL, "%s %d\n", __func__, __LINE__);
        return 1;
    }

#endif
    outstr = memalign(CACHE_LINE, OUTPUT_BUF_LEN);

    if  (!outstr) {
        dprintf  (CRITICAL, "%s %d allocate memory fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

    modulus = BN_new();
    exp = BN_new();
    ca_n = BN_new();
    ca_pkey = EVP_PKEY_new();

    if  (!modulus || !exp || !ca_n || !ca_pkey) {
        dprintf  (CRITICAL, "%s %d allocate memory fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

    //BN_hex2bn(&ca_n, CA);
    //BN_bin2bn(g_modulus, sizeof(g_modulus), ca_n);
#if 0
    //X509_NAME_print(output, xn, 0);
    X509_print(output, cert);
    pkey = X509_get_pubkey(cert);
    //EVP_PKEY_print_public(output, pkey, 16, NULL);

    rsa = EVP_PKEY_get1_RSA(pkey);
    RSA_get0_key(rsa, (const BIGNUM **)&modulus, (const BIGNUM **)&exp, NULL);
#endif
    BN_ULONG exp_l = 0x10001;
    BN_set_word(exp, exp_l);
    RSA_set0_key(cakey, ca_n, exp, NULL);
    ret = EVP_PKEY_set1_RSA(ca_pkey, cakey);
    dprintf(CRITICAL, "%s %d set key ret:%d\n", __func__, __LINE__, ret);
    ret = X509_verify(cert, ca_pkey);
    dprintf(CRITICAL, "%s %d verify ret:%d\n", __func__, __LINE__, ret);
    //BN_print(output, ca_n);
    //ret = ERR_get_error();
    //ERR_error_string_n(ret , (char*)outstr, OUTPUT_BUF_LEN);
    //dprintf(CRITICAL, "%s %d error:%s\n", __func__, __LINE__, outstr);

    //BIO_read(output, outstr, OUTPUT_BUF_LEN);
    //dprintf(CRITICAL, "%s %d outstr:%s \n", __func__, __LINE__, outstr);
    //dprintf(CRITICAL, "%s %d cert tbs:\n", __func__, __LINE__);
    //hexdump8(outstr, ret);
    //md5(outstr, ret, md5_calc);
    //dprintf(CRITICAL, "%s %d cert tbs md5:\n", __func__, __LINE__);
    //hexdump8(md5_calc, MD5_LEN);

    ret = 0;
end:

    if (in)
        BIO_free(in);

    if (output)
        BIO_free(output);

    if (modulus)
        BN_free(modulus);

    if (exp)
        BN_free(exp);

    if (rsa)
        RSA_free(rsa);

    if (cakey)
        RSA_free(cakey);

    if (pkey)
        EVP_PKEY_free(pkey);

    if (ca_pkey)
        EVP_PKEY_free(ca_pkey);

    if (outstr)
        free(outstr);

    if (cert)
        X509_free(cert);

    return ret;
}

static int rsa_verify_cert(uint8_t *buffer, uint32_t size,
                           uint8_t *modulus, uint32_t modulus_len,
                           uint8_t *expo, uint32_t expo_len)
{
    int ret = 0;
    BIO *in = NULL;
    BIO *output = NULL;
    X509 *cert = NULL;
    uint8_t *tbs = NULL;
    uint32_t tbs_len = 0;
    uint8_t *temp = NULL;
    uint8_t *signature = NULL;
    const X509_ALGOR *alg = NULL;
    const ASN1_BIT_STRING *sig = NULL;

    in = BIO_new(BIO_s_mem());

    if (!in) {
        dprintf(CRITICAL, "%s %d new bio error\n", __func__, __LINE__);
        return -1;
    }

    BIO_puts(in, (char *)buffer);

    if (PEM_read_bio_X509(in, &cert, NULL, NULL) == NULL) {
        dprintf(CRITICAL, "%s %d\n", __func__, __LINE__);
        return -1;
    }

    if (!cert) {
        dprintf(CRITICAL, "%s %d cert is null\n", __func__, __LINE__);
        return -1;
    }

    tbs = memalign(CACHE_LINE, TBS_BUF_LEN);

    if (!tbs) {
        dprintf(CRITICAL, "%s %d allocate memory fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

#if 1
    output = BIO_new(BIO_s_mem());

    if (!output) {
        dprintf(CRITICAL, "%s %d allocat memory fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

#endif
    temp = tbs;
    ret = i2d_re_X509_tbs(cert, &temp);//note, this function modify temp ptr
    dprintf(CRITICAL, "%s %d tbs ret:%d\n", __func__, __LINE__, ret);

    if (!ret) {
        dprintf(CRITICAL, "%s %d get cert tbs fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

    tbs_len = ret;
    //memset(tbs, 0x0, TBS_BUF_LEN);
    //BIO_read(output, tbs, TBS_BUF_LEN);
    dprintf(CRITICAL, "%s %d cert tbs:\n", __func__, __LINE__);
    hexdump8(tbs, ret);

    X509_get0_signature(&sig, &alg, cert);
    signature = memalign(CACHE_LINE, sig->length);

    if (!signature) {
        dprintf(CRITICAL, "%s %d allocate memory fail\n", __func__, __LINE__);
        ret = -1;
        goto end;
    }

    memcpy(signature, sig->data, sig->length);
    dprintf(CRITICAL, "%s %d cert sigature:\n", __func__, __LINE__);
    hexdump8(signature, sig->length);

    //i2a_ASN1_OBJECT(output, alg->algorithm);
    //memset(outstr, 0x0, OUTPUT_BUF_LEN);
    //BIO_read(output, outstr, sizeof(outstr));
    //dprintf(CRITICAL, "%s %d alg:%s\n", __func__, __LINE__, outstr);
    boringssl_verify_cert(tbs, tbs_len, signature, sig->length,
                          modulus, modulus_len,
                          expo, expo_len, HASH_TYPE_SHA256);
end:

    if (output)
        BIO_free(output);

    if (in)
        BIO_free(in);

    if (tbs)
        free(tbs);

    if (signature)
        free(signature);

    if (cert)
        X509_free(cert);

    return ret;
}
