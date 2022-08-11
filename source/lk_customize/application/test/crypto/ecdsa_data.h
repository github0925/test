/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_ECDSA_DATA_H
#define SX_ECDSA_DATA_H

#include <stdint.h>

extern uint8_t  ecdsa_msg[128];
extern uint8_t  ecdsa_ref_p192_sha1[48];
extern uint8_t  ecdsa_ref_p256_sha256[64];
extern uint8_t  ecdsa_ref_p384_sha256[96];
extern uint8_t  ecdsa_ref_p521_sha256[132];
extern uint8_t  ecdsa_ref_e521_sha256[132];

#endif
