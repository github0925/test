//*****************************************************************************
//
// sd_rng.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_RNG_H__
#define __SD_RNG_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <crypto_hal.h>

crypto_status_t hal_trng_gen(void* handle, uint8_t* out, uint32_t out_len);
crypto_status_t hal_prng_gen(void* handle, uint8_t* out, uint32_t out_len);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_RNG_H__

