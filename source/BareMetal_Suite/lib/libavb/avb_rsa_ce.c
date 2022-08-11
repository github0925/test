/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Implementation of RSA signature verification which uses a pre-processed
 * key for computation. The code extends libmincrypt RSA verification code to
 * support multiple RSA key lengths and hash digest algorithms.
 */

#include <debug.h>
#include "avb_rsa.h"
#include "avb_sha.h"
#include "avb_util.h"
#include "avb_vbmeta_image.h"

#include "atb_crypto.h"

typedef struct IAvbKey {
  unsigned int len; /* Length of n[] in number of uint32_t */
  uint32_t n0inv;   /* -1 / n[0] mod 2^32 */
  uint32_t* n;      /* modulus as array (host-byte order) */
  uint32_t* rr;     /* R^2 as array (host-byte order) */
} IAvbKey;

static IAvbKey* iavb_parse_key_data(IAvbKey* key, const uint8_t* data, size_t length) {
  AvbRSAPublicKeyHeader h;
  size_t expected_length;
  const uint8_t* n;

  if (!avb_rsa_public_key_header_validate_and_byteswap(
          (const AvbRSAPublicKeyHeader*)data, &h)) {
    avb_error("Invalid key.\n");
    goto fail;
  }

  if (!(h.key_num_bits == 2048 || h.key_num_bits == 4096 ||
        h.key_num_bits == 8192)) {
    avb_error("Unexpected key length.\n");
    goto fail;
  }

  expected_length = sizeof(AvbRSAPublicKeyHeader) + 2 * h.key_num_bits / 8;
  if (length != expected_length) {
    avb_error("Key does not match expected length.\n");
    goto fail;
  }

  n = data + sizeof(AvbRSAPublicKeyHeader);

  key->len = h.key_num_bits / 32;
  key->n0inv = h.n0inv;
  key->n = (uint32_t*)n;
  return key;

fail:
  return NULL;
}

/* Verify a RSA PKCS1.5 signature against an expected hash.
 * Returns false on failure, true on success.
 */
bool avb_rsa_verify(const uint8_t* key,
                    size_t key_num_bytes,
                    const uint8_t* sig,
                    size_t sig_num_bytes,
                    const uint8_t* hash,
                    size_t hash_num_bytes,
                    const uint8_t* msg_buf,
                    size_t msg_len) {
  uint32_t ret;
  IAvbKey parsed_key;
  bool success = false;
  rsa_key_t *rsa_key = NULL;
  crypto_alg_hash_e hash_nid = ALG_HASH_SHA256;

  if (key == NULL || sig == NULL || hash == NULL || msg_buf == NULL) {
    avb_error("Invalid input.\n");
    goto out;
  }

  if ( hash_num_bytes == AVB_SHA256_DIGEST_SIZE)
    hash_nid = ALG_HASH_SHA256;
  else if ( hash_num_bytes == AVB_SHA512_DIGEST_SIZE)
    hash_nid = ALG_HASH_SHA512;
  else
  {
    avb_error("Error unkown hash alg.\n");
    goto out;
  }

  if (iavb_parse_key_data(&parsed_key, key, key_num_bytes) == NULL) {
    avb_error("Error parsing key.\n");
    goto out;
  }

  if (sig_num_bytes != (parsed_key.len * sizeof(uint32_t))) {
    avb_error("Signature length does not match key length.\n");
    goto out;
  }

  rsa_key = avb_malloc(sizeof(rsa_key_t));
  if (!rsa_key){
    avb_error("Fail to allocate memory.\n");
    goto out;
  }

  memset(&(rsa_key->e[0]), 0x0, sig_num_bytes);
  memcpy(&(rsa_key->n[0]), parsed_key.n, sig_num_bytes);

  /* The public key exponent must be 0x10001 */
  rsa_key->e[sig_num_bytes - 1] = 0x01;
  rsa_key->e[sig_num_bytes - 3] = 0x01;
  rsa_key->n_sz = sig_num_bytes;

  ret = crypto_rsa_verify(hash_nid, (addr_t)msg_buf, msg_len,
                            (addr_t)sig, sig_num_bytes, (addr_t)rsa_key);

  if (ret) {
    avb_error("Error verify signature fail.\n");
    goto out;
  }

  success = true;

out:
  if (rsa_key)
    avb_free(rsa_key);

  return success;
}
