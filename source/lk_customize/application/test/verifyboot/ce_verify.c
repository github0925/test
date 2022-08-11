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

#include "crypto_hal.h"
#include "sd_hash.h"
#include "sd_rsa.h"
#include "res.h"
#include "internal.h"

int ce_verify_cert(uint8_t *tbs, uint32_t tbs_len,
                   uint8_t *sig, uint32_t sig_len,
                   uint8_t *modulus, uint32_t modulus_len,
                   uint8_t *expo, uint32_t expo_len, hash_type hash)
{
    int ret = 0;
    rsa_pubkey_t pub_key;
    void *crypto_handle;
    int hash_nid = SD_ALG_SHA256;
    uint32_t vce_id = RES_CE_MEM_CE2_VCE1;
    uint8_t digest[SHA256_INITSIZE] __attribute__((aligned(CACHE_LINE))) = {0};

    pub_key.n = modulus;
    pub_key.n_len = modulus_len;

    pub_key.e = expo;
    pub_key.e_len = expo_len;

    hal_crypto_creat_handle(&crypto_handle, vce_id);

    if (crypto_handle == NULL) {
        dprintf(CRITICAL, "%s create handle error\n", __func__);
        ret = -1;
        goto end;
    }

    if (hash == HASH_TYPE_SHA256) {
        hash_nid = SD_ALG_SHA256;
    }
    else if (hash == HASH_TYPE_SHA512) {
        hash_nid = SD_ALG_SHA512;
    }

    hal_hash(crypto_handle, hash_nid, tbs, tbs_len, digest,
             sizeof digest);
    dprintf(CRITICAL, "%s %d sha256:\n", __func__, __LINE__);
    hexdump8(digest, sizeof digest);

    ret = hal_rsa_verify(crypto_handle, SD_ALG_SHA256, digest, sizeof digest,
                         sig, sig_len, &pub_key, SD_RSA_PADDING_PSS);

    hal_crypto_delete_handle(crypto_handle);

    if (ret == CRYPTO_SUCCESS) {
        dprintf(CRITICAL, "RSA verify success\n");
        ret = 0;
    }
    else {
        dprintf(CRITICAL, "RSA verify  fail res:%d\n", ret);
        ret = -1;
    }

end:
    return ret;
}
