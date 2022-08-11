/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include "sx_math.h"

void math_array_incr(uint8_t* a, const size_t length, int8_t value)
{
    int32_t carry = value;

    /* The LSB are at the end of the array so start there. */
    for (int i = length - 1; i >= 0; i--) {
        int32_t byte = a[i];
        int32_t sum = byte + carry;
        a[i] = (uint8_t)(sum & 0xFF);
        carry = sum >> 8;
    }
}
