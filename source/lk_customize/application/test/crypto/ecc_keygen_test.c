/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>

#include <sd_ecc.h>
#include <trace.h>

#include "ecc_data.h"
#include "ce_test.h"

#define LOCAL_TRACE 0 //close local trace 1->0

uint32_t ecc_keygen_test_p256(void)
{
    uint32_t status;
    uint8_t __attribute__((aligned(CACHE_LINE))) prv_key[] = "\x51\x9b\x42\x3d\x71\x5f\x8b\x58\x1f\x4f\xa8\xee\x59\xf4\x77\x1a\x5b\x44\xc8\x13\x0b\x4e\x3e\xac\xca\x54\xa5\x6d\xda\x72\xb4\x64";
    uint8_t __attribute__((aligned(CACHE_LINE))) pub_key[64];
    uint8_t __attribute__((aligned(CACHE_LINE))) ref[]     = "\x1c\xcb\xe9\x1c\x07\x5f\xc7\xf4\xf0\x33\xbf\xa2\x48\xdb\x8f\xcc\xd3\x56\x5d\xe9\x4b\xbf\xb1\x2f\x3c\x59\xff\x46\xc2\x71\xbf\x83"
                        "\xce\x40\x14\xc6\x88\x11\xf9\xa2\x1a\x1f\xdb\x2c\x0e\x61\x13\xe0\x6d\xb7\xca\x93\xb7\x40\x4e\x78\xdc\x7c\xcd\x5c\xa8\x9a\x4c\xa9";

    void* crypto_handle;
    ec_key_t eckey_test;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    eckey_test.priv_key = block_t_convert(prv_key, 32, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, 64, HAL_EXT_MEM);

    status = hal_ecc_new_by_curve_name(crypto_handle, NID_X9_62_prime256v1);
    status = hal_ecc_generate_pub_key(crypto_handle, &eckey_test);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("ecc_keygen_test_p256 status: %d\n", status);
        return status;
    }

    status = memcmp(pub_key, ref, 64);

    if (status) {
        ce_printf_binary("ecc_keygen_test_p256 result:", pub_key, 64);
    }
    else {
        LTRACEF("ecc_keygen_test_p256 pass !\n");
    }

    return status;
}

uint32_t ecc_gen_pub_key(int nid, uint32_t prv_key_len,
                         const uint8_t* prv_key,
                         const uint8_t* pub_ref)
{
    uint32_t status;
    uint32_t ret;
    void* crypto_handle;
    ec_key_t eckey_test;
    uint8_t pub_key[66 * 2];

    LTRACEF("ecc gen pub key enter nid = %d prv_key_len= %d\n", nid, prv_key_len);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    eckey_test.priv_key = block_t_convert(prv_key, prv_key_len, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, prv_key_len * 2, HAL_EXT_MEM);

    status = hal_ecc_new_by_curve_name(crypto_handle, nid);
    status = hal_ecc_generate_pub_key(crypto_handle, &eckey_test);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("ecc_generate_public_key fail for size: %d, status: %d\n", prv_key_len, status);
        return status;
    }

    status = memcmp(eckey_test.pub_key.addr, pub_ref, prv_key_len);

    if (status) {
        ret = 1;
        ce_printf_binary("ecc_generate_public_key result:", pub_key, prv_key_len * 2);
    }
    else {
        ret = 0;
        LTRACEF("ecc_generate_public_key nid=%d pass !\n", nid);
    }

    return ret;
}

uint32_t ecc_keygen_test_slt(void* arg)
{
    uint32_t ret = 0;
    uint32_t result_value;
    ce_test_t* ce_test_s;

    if(arg != NULL){
        ce_test_s = (ce_test_t*)arg;
    }else{
        ret |= (0x1 << CE_TEST_RESULT_OFFSET_UNKOWN_TEST);
        return ret;
    }

    //generate public key
    ce_test_s->current_index = CE_TEST_ITEM_INDEX_ECC_KEY_GEN_TEST;
    result_value = ecc_gen_pub_key(NID_X9_62_prime192v1, 24, prv_key_p192, pub_key_p192);
    result_value |= ecc_gen_pub_key(NID_X9_62_prime256v1, 32, prv_key_p256, pub_key_p256);
    result_value |= ecc_gen_pub_key(NID_secp384r1, 48, prv_key_p384, pub_key_p384);
    result_value |= ecc_gen_pub_key(NID_secp521r1, 66, prv_key_p521, pub_key_p521);
    result_value |= ecc_gen_pub_key(NID_sece521r1, 66, prv_key_e521, pub_key_e521);

    ret |= (result_value << CE_TEST_RESULT_OFFSET_ECC_KEY_GEN_TEST);

    return ret;
}

uint32_t ecc_keygen_test_uart(void)
{
    uint32_t ret = 0;

    ret = ecc_gen_pub_key(NID_X9_62_prime192v1, 24, prv_key_p192, pub_key_p192);
    ret |= ecc_gen_pub_key(NID_X9_62_prime256v1, 32, prv_key_p256, pub_key_p256);
    ret |= ecc_gen_pub_key(NID_secp384r1, 48, prv_key_p384, pub_key_p384);
    ret |= ecc_gen_pub_key(NID_secp521r1, 66, prv_key_p521, pub_key_p521);
    ret |= ecc_gen_pub_key(NID_sece521r1, 66, prv_key_e521, pub_key_e521);

    return ret;
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("ecc_keygen_test_p256", "ecc generate public key", (console_cmd)&ecc_keygen_test_p256)
STATIC_COMMAND("ecc_keygen_test", "ecc generate public key", (console_cmd)&ecc_keygen_test_uart)

STATIC_COMMAND_END(ecc_keygen_test);

#endif

APP_START(ecc_keygen_test)
.flags = 0
APP_END
