/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_MATH_H
#define SX_MATH_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief a = a + value (value can be positive or negative).
 * Increment (time constant) of an unsigned value stored as byte array a.
 * @param a      unsigned integer to increment, stored as a big endian byte array
 * @param length size of \p a
 * @param value  the value to add to \p a
 */
void math_array_incr(uint8_t* a, const size_t length, int8_t value);

#endif
