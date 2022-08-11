//*****************************************************************************
//
// sd_cmac.h - Prototypes for the crypto hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_CMAC_H__
#define __SD_CMAC_H__
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
#include <sd_aes.h>

crypto_status_t hal_cmac(void* handle, const uint8_t* key, uint32_t key_len,
                     const void* src, size_t src_size, uint8_t* dst, uint32_t dst_size);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __SD_CMAC_H__

