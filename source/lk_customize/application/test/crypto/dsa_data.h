/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _DSA_DATA_H
#define _DSA_DATA_H

#include <sys/types.h>
#include <string.h>

#include <sx_dsa.h>

/* pattern for signature verification */
//MCOMMA 80007F1A  -- CalcR2, 1024-bit, DSA, Signature Verification
extern uint8_t dsa_p_1024[128]; // -- P
extern uint8_t dsa_q_1024[128]; // -- Q
extern uint8_t dsa_g_1024[128]; // -- G
extern uint8_t dsa_pub_key_1024[128]; // -- Y
extern uint8_t dsa_sig_1024[256]; // -- R // -- S
extern uint8_t dsa_hash_1024[128]; // -- H

//MCOMMA 8000FF1A  -- CalcR2, 2048-bit, DSA, Signature Verification
extern uint8_t dsa_p_2048_256[256]; // -- P
extern uint8_t dsa_q_2048_256[256]; // -- Q
extern uint8_t dsa_g_2048_256[256]; // -- G
extern uint8_t dsa_pub_key_2048_256[256]; // -- Y
extern uint8_t dsa_sig_2048_256[512]; // -- R  // -- S
extern uint8_t dsa_hash_2048_256[256]; // -- H

//MCOMMA 8000FF1A  -- CalcR2, 2048-bit, DSA, Signature Verification
extern uint8_t dsa_p_2048_224[256]; // -- P
extern uint8_t dsa_q_2048_224[256]; // -- Q
extern uint8_t dsa_g_2048_224[256]; // -- G
extern uint8_t dsa_pub_key_2048_224[256]; // -- Y
extern uint8_t dsa_sig_2048_224[512]; // -- R  // -- S
extern uint8_t dsa_hash_2048_224[256]; // -- H

extern uint8_t dsa_pub_key_1024_inv[128];  // -- Y
extern uint8_t dsa_sig_1024_inv[256]; // -- R  // -- S
extern uint8_t dsa_hash_1024_inv[128];  // -- H


/* pattern for key /signature generation */
//MCOMMA 80007F18  -- CalcR2, 1024-bit, DSA, Key Generation //MCOMMA 80007F19  -- CalcR2, 1024-bit, DSA, Signature Generation
extern uint8_t dsa_p_gen_1024[128]; //   -- P
extern uint8_t dsa_q_gen_1024[128]; //   -- Q
extern uint8_t dsa_g_gen_1024[128]; //   -- G
extern uint8_t dsa_prv_key_gen_1024[128]; //   -- X
extern uint8_t dsa_pub_key_gen_1024[128]; //   -- Y
extern uint8_t dsa_hash_gen_1024[128]; //   -- H
extern uint8_t dsa_nonce_gen_1024[128]; //   -- K
extern uint8_t dsa_sig_gen_1024[256]; //   -- R  //   -- S

//MCOMMA 8000FF18  -- CalcR2, 2048-bit, DSA, Key Generation  //MCOMMA 8000FF19  -- CalcR2, 2048-bit, DSA, Signature Generation
extern uint8_t dsa_prv_key_gen_2048_256[256]; // -- X
extern uint8_t dsa_pub_key_gen_2048_256[256]; // -- Y
extern uint8_t dsa_hash_gen_2048_256[256]; // -- H
extern uint8_t dsa_nonce_gen_2048_256[256]; // -- K
extern uint8_t dsa_sig_gen_2048_256[512]; // -- R // -- S

//MCOMMA 8000FF18  -- CalcR2, 2048-bit, DSA, Key Generation  //MCOMMA 8000FF19  -- CalcR2, 2048-bit, DSA, Signature Generation
extern uint8_t dsa_prv_key_gen_2048_224[256]; // -- X
extern uint8_t dsa_pub_key_gen_2048_224[256]; // -- Y
extern uint8_t dsa_hash_gen_2048_224[256]; // -- H
extern uint8_t dsa_nonce_gen_2048_224[256]; // -- K
extern uint8_t dsa_sig_gen_2048_224[512]; // -- R // -- S

#endif
