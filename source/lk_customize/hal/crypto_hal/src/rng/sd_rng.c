//*****************************************************************************
//
// sd_rng.c - Driver for rng.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_rng.h>
#include <trace.h>
#include <string.h>

#define LOCAL_TRACE 0 //close local trace 1->0

//*****************************************************************************
//
//! hal_trng_gen.
//!
//! @param handle input
//! @param out output
//! @param out_len input
//!
//! This function is for hal_trng_gen
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_trng_gen(void* handle, uint8_t* out, uint32_t out_len)
{
    uint32_t vce_id;
    crypto_status_t ret;
    crypto_instance_t* l_cryptoinstance = NULL;

    LTRACEF("enter out_len= %d\n", out_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->rng_method->trng) {
            uint32_t driver_return;
            driver_return = l_cryptoinstance->rng_method->trng(vce_id, out, out_len);

            if (driver_return == 1) {
                ret = CRYPTO_SUCCESS;
            }
            else {
                ret = CRYPTO_ERROR;
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_prng_gen.
//!
//! @param handle input
//! @param out output
//! @param out_len input
//!
//! This function is for prng gen
//!
//! @return crypto_status_t
//
//*****************************************************************************

crypto_status_t hal_prng_gen(void* handle, uint8_t* out, uint32_t out_len)
{
    uint32_t vce_id;
    crypto_status_t ret;
    crypto_instance_t* l_cryptoinstance = NULL;

    LTRACEF("enter out_len= %d\n", out_len);

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->rng_method->prng) {
            uint32_t driver_return;
            driver_return = l_cryptoinstance->rng_method->prng(vce_id, out, out_len);

            if (driver_return == 1) {
                ret = CRYPTO_SUCCESS;
            }
            else {
                ret = CRYPTO_ERROR;
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}
