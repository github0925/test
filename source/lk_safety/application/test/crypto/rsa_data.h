/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _RSA_DATA_H
#define _RSA_DATA_H

#include <sys/types.h>
#include <string.h>

/* sram usage: 1st block(512 bytes) - key
 *             2nd block(512 bytes) - src
 *             3rd block(512 bytes) - exponent
 *             4th block -          - dst
 */
#define RSA_SRAM_N_OFFSET      (0 + 64)  //-->n
#define RSA_SRAM_SRC_OFFSET    (512 + 64) //src--p
#define RSA_SRAM_KEY_OFFSET    (1024 + 64) //-->private key/exponent
#define RSA_SRAM_DST_OFFSET    (1536 + 64) //dst--q
#define IRAM4_BASE_ADDR        0x001c0000

#define RSA_1024_LEN        128
#define RSA_1280_LEN        160
#define RSA_2048_LEN        256
#define RSA_3072_LEN        384
#define RSA_4096_LEN        512

//RSA_1024
extern uint8_t msg_1024[RSA_1024_LEN];
extern uint8_t n_1024[RSA_1024_LEN];
extern uint8_t p_1024[RSA_1024_LEN];
extern uint8_t q_1024[RSA_1024_LEN];
extern uint8_t public_expo_1024[RSA_1024_LEN];
extern uint8_t private_key_1024[RSA_1024_LEN * 2];
extern uint8_t cipher_1024[RSA_1024_LEN];
extern uint8_t sig_1024[RSA_1024_LEN];
extern uint8_t dp_1024[RSA_1024_LEN];
extern uint8_t dq_1024[RSA_1024_LEN];
extern uint8_t inv_1024[RSA_1024_LEN];

//RSA_2048
extern uint8_t msg_2048[RSA_2048_LEN];
extern uint8_t n_2048[RSA_2048_LEN];
extern uint8_t p_2048[RSA_2048_LEN];
extern uint8_t q_2048[RSA_2048_LEN];
extern uint8_t public_expo_2048[RSA_2048_LEN];
extern uint8_t private_key_2048[RSA_2048_LEN];
extern uint8_t cipher_2048[RSA_2048_LEN];
extern uint8_t sig_2048[RSA_2048_LEN];

//RSA_3072
extern uint8_t msg_3072[RSA_3072_LEN];
extern uint8_t n_3072[RSA_3072_LEN];
extern uint8_t p_3072[RSA_3072_LEN];
extern uint8_t q_3072[RSA_3072_LEN];
extern uint8_t public_expo_3072[RSA_3072_LEN];
extern uint8_t private_key_3072[RSA_3072_LEN];
extern uint8_t cipher_3072[RSA_3072_LEN];
extern uint8_t sig_3072[RSA_3072_LEN];

//RSA_4096
extern uint8_t msg_4096[RSA_4096_LEN];
extern uint8_t n_4096[RSA_4096_LEN];
extern uint8_t p_4096[RSA_4096_LEN];
extern uint8_t q_4096[RSA_4096_LEN];
extern uint8_t public_expo_4096[RSA_4096_LEN];
extern uint8_t private_key_4096[RSA_4096_LEN];
extern uint8_t cipher_4096[RSA_4096_LEN];
extern uint8_t sig_4096[RSA_4096_LEN];

#endif
