/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>

#include <sd_dsa.h>
#include <trace.h>

#include "ce_test.h"
#include "dsa_data.h"

#define LOCAL_TRACE 0 //close local trace 1->0
/* type 0 -- 1024
        1 -- 2048_256
        2 -- 2048_224
*/

uint32_t dsa_key_gen_single(uint32_t vce_id, uint32_t type)
{
    uint8_t pub_key[256];
    uint32_t size, status;
    uint8_t* str_prv_key, *str_pub_key;

    LTRACEF("dsa key generation enter vce_id=%d type=%d\n", vce_id, type);

    switch (type) {
        case 0:
            size = 128;
            str_prv_key = dsa_prv_key_gen_1024;
            str_pub_key = dsa_pub_key_gen_1024;
            break;

        case 1:
            size = 256;
            str_prv_key = dsa_prv_key_gen_2048_256;
            str_pub_key = dsa_pub_key_gen_2048_256;
            break;

        case 2:
            size = 256;
            str_prv_key = dsa_prv_key_gen_2048_224;
            str_pub_key = dsa_pub_key_gen_2048_224;
            break;

        default:
            return -1;
    }

    void* crypto_handle;
    dsa_key_t dsakey_test;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    dsakey_test.priv_key = block_t_convert(str_prv_key, size, HAL_EXT_MEM);
    dsakey_test.pub_key = block_t_convert(pub_key, size, HAL_EXT_MEM);

    status = hal_dsa_init(crypto_handle, type);
    status = hal_dsa_generate_pub_key(crypto_handle, &dsakey_test);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("dsa key generation for %d fail\n", type);
        return status;
    }

    status = memcmp(pub_key, str_pub_key, size);

    if (status) {
        ce_printf_binary("dsa pub key generation", pub_key, size);

    }
    else {
        LTRACEF("dsa key generation pass, type= %d result: %d\n", type, status);
    }


    return status;
}

uint32_t dsa_sig_gen_single(uint32_t vce_id, uint32_t type)
{
    uint8_t sig[512];
    uint32_t size, status;
    uint8_t* str_prv_key, *msg;
#if WITH_SIMULATION_PLATFORM
    uint8_t *str_sig;
#else
    uint8_t *str_pub_key;
#endif

    LTRACEF("dsa sign enter vce_id=%d type=%d\n", vce_id, type);

    switch (type) {
        case 0:
            size = 128;
            str_prv_key = dsa_prv_key_gen_1024;
            msg = dsa_hash_gen_1024;
#if WITH_SIMULATION_PLATFORM
            str_sig = dsa_sig_gen_1024;
#else
            str_pub_key = dsa_pub_key_gen_1024;
#endif
            break;

        case 1:
            size = 256;
            str_prv_key = dsa_prv_key_gen_2048_256;
            msg = dsa_hash_gen_2048_256;
#if WITH_SIMULATION_PLATFORM
            str_sig = dsa_sig_gen_2048_256;
#else
            str_pub_key = dsa_pub_key_gen_2048_256;
#endif
            break;

        case 2:
            size = 256;
            str_prv_key = dsa_prv_key_gen_2048_224;
            msg = dsa_hash_gen_2048_224;
#if WITH_SIMULATION_PLATFORM
            str_sig = dsa_sig_gen_2048_224;
#else
            str_pub_key = dsa_pub_key_gen_2048_224;
#endif
            break;

        default:
            return -1;
    }

    void* crypto_handle;
    dsa_key_t dsakey_test;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    dsakey_test.priv_key = block_t_convert(str_prv_key, size, HAL_EXT_MEM);
    dsakey_test.pub_key = block_t_convert(NULL, 0, HAL_EXT_MEM);

    status = hal_dsa_init(crypto_handle, type);
    status = hal_dsa_sign(crypto_handle, SD_ALG_SHA256, msg, size, sig, size * 2, &dsakey_test);
    if (status) {
        LTRACEF("dsa signature generation for %d fail\n", type);
        hal_crypto_delete_handle(crypto_handle);
        return status;
    }

#if (WITH_SIMULATION_PLATFORM)
    hal_crypto_delete_handle(crypto_handle);
    status = memcmp(sig, str_sig, size * 2);
    if (status) {
        LTRACEF("dsa signature generation fail\n");
        ce_printf_binary("dsa signature generation", sig, size * 2);
    }
    else {
        LTRACEF("dsa signature generation pass for %d \n", type);
    }
#else
    dsakey_test.priv_key = block_t_convert(NULL, 0, HAL_EXT_MEM);
    dsakey_test.pub_key = block_t_convert(str_pub_key, size, HAL_EXT_MEM);

    status = hal_dsa_verify(crypto_handle, SD_ALG_SHA256, msg, size, sig, size * 2, &dsakey_test);
    hal_crypto_delete_handle(crypto_handle);
    if (status) {
        LTRACEF("dsa verify after signature generation for %d fail\n", type);
    }
#endif

    return status;
}

uint32_t dsa_sig_ver_single(uint32_t vce_id, uint32_t type)
{
    uint32_t size, status;
    uint8_t* str_pub_key, *msg, *str_sig;

    LTRACEF("dsa verify enter vce_id=%d type=%d\n", vce_id, type);

    switch (type) {
        case 0:
            size = 128;
            str_pub_key = dsa_pub_key_gen_1024;
            msg = dsa_hash_gen_1024;
            str_sig = dsa_sig_gen_1024;
            break;

        case 1:
            size = 256;
            str_pub_key = dsa_pub_key_gen_2048_256;
            msg = dsa_hash_gen_2048_256;
            str_sig = dsa_sig_gen_2048_256;
            break;

        case 2:
            size = 256;
            str_pub_key = dsa_pub_key_gen_2048_224;
            msg = dsa_hash_gen_2048_224;
            str_sig = dsa_sig_gen_2048_224;
            break;

        default:
            return -1;
    }

    void* crypto_handle;
    dsa_key_t dsakey_test;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    dsakey_test.priv_key = block_t_convert(NULL, 0, HAL_EXT_MEM);
    dsakey_test.pub_key = block_t_convert(str_pub_key, size, HAL_EXT_MEM);

    status = hal_dsa_init(crypto_handle, type);
    status = hal_dsa_verify(crypto_handle, SD_ALG_SHA256, msg, size, str_sig, size * 2, &dsakey_test);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("dsa sigature verify for %d fail\n", type);
    }
    else {
        LTRACEF("dsa sigature verify for %d success\n", type);
    }


    return status;
}

uint32_t dsa_key_gen(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (uint32_t j = 0; j < 3; j++) {
        status = dsa_key_gen_single(vce_id, j);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }
    LTRACEF("dsa key generation test finish, last staus: %d\n", status);
    return ret;
}

uint32_t dsa_signature_gen(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;
    for (uint32_t j = 0; j < 3; j++) {
        status = dsa_sig_gen_single(vce_id, j);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("dsa signature generation test finish, last staus: %d\n", status);
    return ret;
}

uint32_t dsa_signature_ver(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (uint32_t j = 0; j < 3; j++) {
        status = dsa_sig_ver_single(vce_id, j);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("dsa signature verification test finish, last staus: %d\n", status);
    return ret;
}

uint32_t dsa_test_slt(void* arg)
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

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_DSA_KEY_GEN_TEST;
    result_value = dsa_key_gen(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_DSA_KEY_GEN_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_DSA_SIGN_GEN_TEST;
    result_value = dsa_signature_gen(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_DSA_SIGN_GEN_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_DSA_SIGN_VERY_TEST;
    result_value= dsa_signature_ver(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_DSA_SIGN_VERY_TEST);

    return ret;
}

uint32_t dsa_test_uart(void)
{
    uint32_t ret = 0;

    ret = dsa_key_gen(0);
    ret = dsa_signature_gen(0);
    ret = dsa_signature_ver(0);

    return ret;
}

void dsatest_entry(const struct app_descriptor* app, void* args)
{
    dsa_test_uart();
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("dsa_test", "sm2 generate public key", (console_cmd)&dsa_test_uart)

STATIC_COMMAND_END(dsa_test);

#endif

APP_START(dsa_test)
.flags = 0,
//.entry=dsatest_entry,
APP_END
