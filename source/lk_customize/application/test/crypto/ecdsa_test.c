/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>

#include <sd_ecdsa.h>
#include <trace.h>

#include "ecc_data.h"
#include "ecdsa_data.h"
#include "ce_test.h"

#define LOCAL_TRACE 0 //close local trace 1->0

static uint32_t ecdsa_pass = 0;
static uint32_t ecdsa_fail = 0;

uint32_t ecdsa_signature_gen_single(uint32_t vce_id, ecdsa_test_type_t type)
{
    uint32_t status;

    uint8_t sig[132]={0};
    sd_ecc_curve_t* ecc_curve;
#if WITH_SIMULATION_PLATFORM
    uint8_t* sig_ref;
#else
    uint8_t * pub_key;
#endif
    uint8_t* prv_key;
    uint32_t size;
    void* crypto_handle;
    int curve_nid;

    LTRACEF("ecdsa signature enter vce_id=%d type=%d\n", vce_id, type);

    switch (type) {
        case ECDSA_P192:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p192;
#if WITH_SIMULATION_PLATFORM
            sig_ref = ecdsa_ref_p192_sha1;
#else
            pub_key = (uint8_t*)pub_key_p192;
#endif
            prv_key = (uint8_t*)prv_key_p192;
            curve_nid = NID_X9_62_prime192v1;
            break;

        case ECDSA_P256:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p256;
#if WITH_SIMULATION_PLATFORM
            sig_ref = ecdsa_ref_p256_sha256;
#else
            pub_key = (uint8_t*)pub_key_p256;
#endif
            prv_key = (uint8_t*)prv_key_p256;
            curve_nid = NID_X9_62_prime256v1;
            break;

        case ECDSA_P384:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p384;
#if WITH_SIMULATION_PLATFORM
            sig_ref = ecdsa_ref_p384_sha256;
#else
            pub_key = (uint8_t*)pub_key_p384;
#endif
            prv_key = (uint8_t*)prv_key_p384;
            curve_nid = NID_secp384r1;
            break;

        case ECDSA_P521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p521;
#if WITH_SIMULATION_PLATFORM
            sig_ref = ecdsa_ref_p521_sha256;
#else
            pub_key = (uint8_t*)pub_key_p521;
#endif
            prv_key = (uint8_t*)prv_key_p521;
            curve_nid = NID_secp521r1;
            break;

        case ECDSA_E521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_e521;
#if WITH_SIMULATION_PLATFORM
            sig_ref = ecdsa_ref_e521_sha256;
#else
            pub_key = (uint8_t*)pub_key_e521;
#endif
            prv_key = (uint8_t*)prv_key_e521;
            curve_nid = NID_sece521r1;
            break;

        default:
            return -1;
    }

    size = ecc_curve->bytesize;

    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(prv_key, size, EXT_MEM);
    eckey_test.pub_key = block_t_convert(NULL, 0, EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    status = hal_ecdsa_sign(crypto_handle, curve_nid, ecdsa_msg, 128, sig,
                        2 * size, &eckey_test, ECDSA_P192 == type ? SD_ALG_SHA1 : SD_ALG_SHA256);
    if (status) {
        hal_crypto_delete_handle(crypto_handle);
        LTRACEF("sigature gen result: %d\n", status);
        return status;
    }
#if WITH_SIMULATION_PLATFORM
    hal_crypto_delete_handle(crypto_handle);
    status = memcmp(sig, sig_ref, 2 * size);
    if (status) {
        ce_printf_binary("signature gen result", sig, 2 * size);
        ecdsa_fail++;
        return status;
    }
    else {
        LTRACEF("ecdsa sigature pass !\n");
        ecdsa_pass++;
    }
#else
    eckey_test.priv_key = block_t_convert(NULL, 0, EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, 2 * size, EXT_MEM);

    status = hal_ecdsa_verify(crypto_handle, curve_nid, ecdsa_msg, 128,
                           sig, 2 * size, &eckey_test, ECDSA_P192 == type ? SD_ALG_SHA1 : SD_ALG_SHA256);
    hal_crypto_delete_handle(crypto_handle);
    if (status) {
        LTRACEF("ECDSA_verify after generate fail status=%d \n", status);
        ecdsa_fail++;
    }
    else {
        LTRACEF("ECDSA_verify after generate pass \n");
        ecdsa_pass++;
    }
#endif

    return status;
}

uint32_t ecdsa_signature_ver_single(uint32_t vce_id, ecdsa_test_type_t type)
{
    uint32_t status;
    sd_ecc_curve_t* ecc_curve;
    uint8_t* sig_ref;
    uint8_t* pub_key;
    uint32_t size;
    void* crypto_handle;
    int curve_nid;

    LTRACEF("ecdsa verify enter vce_id=%d type=%d\n", vce_id, type);

    switch (type) {
        case ECDSA_P192:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p192;
            sig_ref = ecdsa_ref_p192_sha1;
            pub_key = (uint8_t*)pub_key_p192;
            curve_nid = NID_X9_62_prime192v1;
            break;

        case ECDSA_P256:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p256;
            sig_ref = ecdsa_ref_p256_sha256;
            pub_key = (uint8_t*)pub_key_p256;
            curve_nid = NID_X9_62_prime256v1;
            break;

        case ECDSA_P384:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p384;
            sig_ref = ecdsa_ref_p384_sha256;
            pub_key = (uint8_t*)pub_key_p384;
            curve_nid = NID_secp384r1;
            break;

        case ECDSA_P521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p521;
            sig_ref = ecdsa_ref_p521_sha256;
            pub_key = (uint8_t*)pub_key_p521;
            curve_nid = NID_secp521r1;
            break;

        case ECDSA_E521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_e521;
            sig_ref = ecdsa_ref_e521_sha256;
            pub_key = (uint8_t*)pub_key_e521;
            curve_nid = NID_sece521r1;
            break;

        default:
            return -1;
    }

    size = ecc_curve->bytesize;

    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(NULL, 0, EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, 2 * size, EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    status = hal_ecdsa_verify(crypto_handle, curve_nid, ecdsa_msg, 128,
                           sig_ref, 2 * size, &eckey_test, ECDSA_P192 == type ? SD_ALG_SHA1 : SD_ALG_SHA256);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("ECDSA_verify fail status=%d \n", status);
        ecdsa_fail++;
    }
    else {
        LTRACEF("ECDSA_verify pass \n");
        ecdsa_pass++;
    }


    return status;
}

uint32_t ecdsa_signature_gen(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (int i = ECDSA_P192; i <= ECDSA_E521; i++) {
        status = ecdsa_signature_gen_single(vce_id, i);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("signature generation test finish, last staus: %d\n", status);
    return ret;
}

uint32_t ecdsa_signature_ver(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (int i = ECDSA_P192; i <= ECDSA_E521; i++) {
        status = ecdsa_signature_ver_single(vce_id, i);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("signature verification test finish, last staus: %d\n", status);
    return ret;
}

uint32_t ecdsa_test_slt(void* arg)
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

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_ECDSA_SIG_GEN_TEST;
    result_value = ecdsa_signature_gen(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_ECDSA_SIG_GEN_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_ECDSA_SIGN_VERY_TEST;
    result_value = ecdsa_signature_ver(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_ECDSA_SIGN_VERY_TEST);

    LTRACEF("test result, pass: %d, fail: %d\n", ecdsa_pass, ecdsa_fail);
    return ret;
}

uint32_t ecdsa_test_uart(void)
{
    uint32_t ret = 0;

    ret = ecdsa_signature_gen(0);
    ret |= ecdsa_signature_ver(0);

    return ret;
}

void ecdsatest_entry(const struct app_descriptor* app, void* args)
{
    ecdsa_test_uart();
}


#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("ecdsa_test", "ecc generate public key", (console_cmd)&ecdsa_test_uart)

STATIC_COMMAND_END(ecdsa_test);

#endif

APP_START(ecdsa_test)
.flags = 0,
//.entry=ecdsatest_entry,
APP_END
