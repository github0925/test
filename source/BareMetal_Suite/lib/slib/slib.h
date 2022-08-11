/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <_assert.h>

#if !defined(CFG_SEC_SNUM_DIS)
#define SN_DCLR(v)    volatile unsigned int scnt = (v)
#define SN_ADD(v)     scnt += (v)
#define SN_SUB(v)     scnt -= (v)
#define SN_ASSERT(v)  assert((v) == scnt)
#else
#define SN_DCLR(v)
#define SN_ADD(v)
#define SN_SUB(v)
#define SN_ASSERT(v)
#endif
