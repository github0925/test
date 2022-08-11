/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#include <stdint.h>

uint32_t checksum[8] __attribute__((section(".checksum"))) __attribute__((used)) = {
    0x12345678, 0x12345678, 0x12345678, 0x12345678,
    0x12345678, 0x12345678, 0x12345678, 0x12345678,
};
