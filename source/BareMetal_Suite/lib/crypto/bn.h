/********************************************************
 *      Copyright(c) 2020   Semidrive                   *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __BN_H__
#define __BN_H__

#include <common_hdr.h>

bool bn_is_zero(uint8_t *bn, uint32_t sz);
int32_t bn_cmp(uint8_t *A, uint8_t *B, uint32_t sz);

#endif  /* __BN_H__ */
