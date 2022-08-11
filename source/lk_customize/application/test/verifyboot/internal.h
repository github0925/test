/*
 * internal.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _INTERNAL_H
#define _INTERNAL_H

typedef enum HASH_TYPE {
    HASH_TYPE_SHA256,
    HASH_TYPE_SHA512,
} hash_type;

uint32_t boringssl_parse_x509(uint8_t *buffer, uint32_t size);

int boringssl_verify_cert(uint8_t *tbs, uint32_t tbs_len,
                   uint8_t *sig, uint32_t sig_len,
                   uint8_t *modulus, uint32_t modulus_len,
                   uint8_t *expo, uint32_t expo_len, hash_type hash);

int ce_verify_cert(uint8_t *tbs, uint32_t tbs_len,
                   uint8_t *sig, uint32_t sig_len,
                   uint8_t *modulus, uint32_t modulus_len,
                   uint8_t *expo, uint32_t expo_len, hash_type hash);

#endif
