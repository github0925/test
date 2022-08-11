/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_ECC_DATA_H
#define SX_ECC_DATA_H

#include <stdint.h>
#include <stdbool.h>

#include <sx_ecc_keygen.h>

/**
 * @brief Get size of \p curve in bytes
 * @param curve is a pointer to a ::sx_ecc_curve_t structure describing the curve
 */
uint32_t sx_ecc_curve_bytesize(const sx_ecc_curve_t* curve);

extern const uint8_t prv_key_p192[24];
extern const uint8_t pub_key_p192[48 + 16];
extern const uint8_t prv_key_p256[32];
extern const uint8_t pub_key_p256[64];
extern const uint8_t prv_key_p384[48 + 16];
extern const uint8_t pub_key_p384[96];
extern const uint8_t prv_key_p521[66 + 30];
extern const uint8_t pub_key_p521[132 + 28];
extern const uint8_t prv_key_e521[66 + 30];
extern const uint8_t pub_key_e521[132 + 28];

#endif
