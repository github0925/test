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

#include "libavb.h"
#include "sd_x509.h"

void crypto_eng_init(void);
void crypto_eng_deinit(void);
uint32_t calc_hash(const uint8_t *buf, uint32_t buf_len, uint8_t *dig,
               uint32_t dig_len, AvbDigestType hash_type);

uint32_t verify_cert_sign(const uint8_t *tbs, uint32_t tbs_len,
                const uint8_t *sig, uint32_t sig_len,
                const uint8_t *modulus, uint32_t modulus_len,
                const uint8_t *expo, uint32_t expo_len, X509_SIGN_T sign_type);

#endif
