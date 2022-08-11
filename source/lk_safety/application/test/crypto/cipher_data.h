/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _CIPHER_DATA_H
#define _CIPHER_DATA_H

#include <sys/types.h>
#include <string.h>
#include <sd_hash.h>
/* sram usage: 1st block(64 bytes) - iv
 *             2nd block(64 bytes) - dst
 *             3-5 block(64 bytes) - key
 *             6 block -         - src
 */

#define NORMAL_DATA_LEN   64
extern uint8_t cbc_iv[16];
extern uint8_t ctr_iv[16];
extern uint8_t cfb_iv[16];
extern uint8_t ofb_iv[16];
extern uint8_t ccm_iv[16];
extern uint8_t gcm_iv[16];
extern uint8_t xts_iv[16];
extern uint8_t cmac_iv[16];

extern uint8_t ccm_head[16];
extern uint8_t xcm_aad[64];

extern uint8_t plain_text[NORMAL_DATA_LEN];
extern uint8_t plain_text_h[SHA256_DIGESTSIZE];

extern uint8_t key_128[16];
extern uint8_t key_192[24];
extern uint8_t key_256[32];
extern uint8_t key_128_ext[16];
extern uint8_t key_192_ext[24];
extern uint8_t key_256_ext[32];

extern uint8_t cipher_ecb_128[NORMAL_DATA_LEN];
extern uint8_t cipher_ecb_192[NORMAL_DATA_LEN];
extern uint8_t cipher_ecb_256[NORMAL_DATA_LEN];
extern uint8_t cipher_ecb_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ecb_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ecb_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_cbc_128[NORMAL_DATA_LEN];
extern uint8_t cipher_cbc_192[NORMAL_DATA_LEN];
extern uint8_t cipher_cbc_256[NORMAL_DATA_LEN];
extern uint8_t cipher_cbc_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cbc_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cbc_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_ctr_128[NORMAL_DATA_LEN];
extern uint8_t cipher_ctr_192[NORMAL_DATA_LEN];
extern uint8_t cipher_ctr_256[NORMAL_DATA_LEN];
extern uint8_t cipher_ctr_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ctr_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ctr_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_cfb_128[NORMAL_DATA_LEN];
extern uint8_t cipher_cfb_192[NORMAL_DATA_LEN];
extern uint8_t cipher_cfb_256[NORMAL_DATA_LEN];
extern uint8_t cipher_cfb_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cfb_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cfb_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_ofb_128[NORMAL_DATA_LEN];
extern uint8_t cipher_ofb_192[NORMAL_DATA_LEN];
extern uint8_t cipher_ofb_256[NORMAL_DATA_LEN];
extern uint8_t cipher_ofb_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ofb_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ofb_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_ccm_128[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_ccm_192[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_ccm_256[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_ccm_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ccm_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_ccm_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_gcm_128[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_gcm_192[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_gcm_256[NORMAL_DATA_LEN + 16];
extern uint8_t cipher_gcm_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_gcm_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_gcm_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_xts_128[NORMAL_DATA_LEN];
extern uint8_t cipher_xts_192[NORMAL_DATA_LEN];
extern uint8_t cipher_xts_256[NORMAL_DATA_LEN];
extern uint8_t cipher_xts_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_xts_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_xts_256_h[SHA256_DIGESTSIZE];

extern uint8_t cipher_cmac_128[16];
extern uint8_t cipher_cmac_192[16];
extern uint8_t cipher_cmac_256[16];
extern uint8_t cipher_cmac_128_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cmac_192_h[SHA256_DIGESTSIZE];
extern uint8_t cipher_cmac_256_h[SHA256_DIGESTSIZE];

#endif
