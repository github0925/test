/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <err.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>
#include <platform.h>
#include <reg.h>
#include <kernel/thread.h>

#include <sd_aes.h>
#include <sd_cmac.h>
#include <trace.h>

#include "hash_data.h"
#include "cipher_data.h"
#include "ce_test.h"
#include <ce.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define AES_OUTPUT_BUFF_LEN_MAX 320
uint8_t cipher_input_inddr[AES_OUTPUT_BUFF_LEN_MAX] __ALIGNED(CACHE_LINE);
uint8_t cipher_output_inddr[AES_OUTPUT_BUFF_LEN_MAX] __ALIGNED(CACHE_LINE);

uint32_t get_plain_cipher(sd_aes_fct_t fct, aes_operation_t operation, aes_key_type_t key_type, buff_addr_type_t dst_type, uint8_t** input, uint8_t** output)
{
    uint8_t* cipher;

    bool hash_res = (HAL_SRAM_SEC == dst_type) && (OPERATION_ENC == operation);

    switch (fct) {
        case SD_FCT_ECB:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_ecb_128_h : cipher_ecb_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_ecb_192_h : cipher_ecb_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_ecb_256_h : cipher_ecb_256;
                    break;

                default:
                    cipher = hash_res ? cipher_ecb_128_h : cipher_ecb_128;
            }

            break;

        case SD_FCT_CBC:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_cbc_128_h : cipher_cbc_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_cbc_192_h : cipher_cbc_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_cbc_256_h : cipher_cbc_256;
                    break;

                default:
                    cipher = hash_res ? cipher_cbc_128_h : cipher_cbc_128;
            }

            break;

        case SD_FCT_CTR:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_ctr_128_h : cipher_ctr_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_ctr_192_h : cipher_ctr_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_ctr_256_h : cipher_ctr_256;
                    break;

                default:
                    cipher = hash_res ? cipher_ctr_128_h : cipher_ctr_128;
            }

            break;

        case SD_FCT_CFB:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_cfb_128_h : cipher_cfb_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_cfb_192_h : cipher_cfb_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_cfb_256_h : cipher_cfb_256;
                    break;

                default:
                    cipher = hash_res ? cipher_cfb_128_h : cipher_cfb_128;
            }

            break;

        case SD_FCT_OFB:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_ofb_128_h : cipher_ofb_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_ofb_192_h : cipher_ofb_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_ofb_256_h : cipher_ofb_256;
                    break;

                default:
                    cipher = hash_res ? cipher_ofb_128_h : cipher_ofb_128;
            }

            break;

        case SD_FCT_CCM:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_ccm_128_h : cipher_ccm_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_ccm_192_h : cipher_ccm_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_ccm_256_h : cipher_ccm_256;
                    break;

                default:
                    cipher = hash_res ? cipher_ccm_128_h : cipher_ccm_128;
            }

            break;

        case SD_FCT_GCM:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_gcm_128_h : cipher_gcm_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_gcm_192_h : cipher_gcm_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_gcm_256_h : cipher_gcm_256;
                    break;

                default:
                    cipher = hash_res ? cipher_gcm_128_h : cipher_gcm_128;
            }

            break;

        case SD_FCT_XTS:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_xts_128_h : cipher_xts_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_xts_192_h : cipher_xts_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_xts_256_h : cipher_xts_256;
                    break;

                default:
                    cipher = hash_res ? cipher_xts_128_h : cipher_xts_128;
            }

            break;

        case SD_FCT_CMAC:
            switch (key_type) {
                case KEY_TYPE_128:
                    cipher = hash_res ? cipher_cmac_128_h : cipher_cmac_128;
                    break;

                case KEY_TYPE_192:
                    cipher = hash_res ? cipher_cmac_192_h : cipher_cmac_192;
                    break;

                case KEY_TYPE_256:
                    cipher = hash_res ? cipher_cmac_256_h : cipher_cmac_256;
                    break;

                default:
                    cipher = hash_res ? cipher_cmac_128_h : cipher_cmac_128;
            }

            break;

        default:
            return -1;
    }

    if (OPERATION_ENC == operation) {
        *output = cipher;
        *input = plain_text;
    }
    else {
        if (HAL_KEY_INT == dst_type || HAL_SRAM_SEC == dst_type) {
            *output = plain_text_h;
        }
        else {
            *output = plain_text;
        }

        *input = cipher;
    }

    return 0;
}
/* sram usage: 1st block(64 bytes) - iv
 *             2-5 block(64 bytes) - dst
 *             6-8 block(64 bytes) - key
 *             9 block -         - src
 */
uint32_t cipher_op(sd_aes_fct_t fct, aes_operation_t operation, buff_addr_type_t key_addr_type, aes_key_type_t key_type,
                   buff_addr_type_t src_type, buff_addr_type_t dst_type)
{
    uint8_t* init_vec;
    uint32_t iv_size;
    uint8_t* key;
    uint8_t* xkey;
    uint32_t key_size;
    addr_t addr_base, addr_dst  = 0;
    addr_t addr_base_begin = 0;
    uint8_t* src = plain_text;
    uint8_t* dst = cipher_ecb_128;
    int result;
    uint32_t dst_size = sizeof(plain_text);
    uint32_t src_size = sizeof(plain_text);
    uint32_t head_len = 0;
    void* crypto_handle = 0;

    LTRACEF("aes test enter fct=%d, operation=%d \n", fct, operation);

    switch (fct) {
        case SD_FCT_ECB:
            init_vec = NULL;
            break;

        case SD_FCT_CBC:
            init_vec = cbc_iv;
            break;

        case SD_FCT_CTR:
            init_vec = ctr_iv;
            break;

        case SD_FCT_CFB:
            init_vec = cfb_iv;
            break;

        case SD_FCT_OFB:
            init_vec = ofb_iv;
            break;

        case SD_FCT_CCM:
            init_vec = NULL;//ccm_iv;
            break;

        case SD_FCT_GCM:
            init_vec = gcm_iv;
            break;

        case SD_FCT_XTS:
            init_vec = xts_iv;
            break;

        case SD_FCT_CMAC:
            init_vec = NULL;//cmac_iv;
            break;

        default:
            return -1;
    }

    switch (key_type) {
        case KEY_TYPE_128:
            key_size = 16;
            key = key_128;
            xkey = key_128_ext;
            break;

        case KEY_TYPE_192:
            key_size = 24;
            key = key_192;
            xkey = key_192_ext;
            break;

        case KEY_TYPE_256:
            key_size = 32;
            key = key_256;
            xkey = key_256_ext;
            break;

        default:
            return -1;
    }

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    if (crypto_handle == NULL) {
        LTRACEF("hal_crypto_creat_handle fail\n");
        return -1;
    }

    if (init_vec == NULL) {
        iv_size = 0;
    }
    else {
        iv_size = 16;
    }

    hal_aes_init(crypto_handle, fct, operation, key_addr_type, key, xkey, key_size, init_vec, iv_size);

    result = get_plain_cipher(fct, operation, key_type, dst_type, &src, &dst);

    if (result) {
        LTRACEF("get plain/cipher fail\n");
        return -1;
    }

    if (SD_FCT_CCM == fct) {
        head_len = sizeof(xcm_aad) + sizeof(ccm_head) + 2 + 14;
    }
    else if (SD_FCT_GCM == fct) {
        head_len = sizeof(xcm_aad);
    }

    addr_base = (addr_t)&cipher_input_inddr;

    addr_base_begin = addr_base;

    if (SD_FCT_GCM == fct) {
        memcpy((void*)addr_base, xcm_aad, head_len);
        addr_base += head_len;
    }
    else if (SD_FCT_CCM == fct) {
        memcpy((void*)addr_base, ccm_head, sizeof(ccm_head));
        addr_base += sizeof(ccm_head);
        memcpy((void*)addr_base, "\x00\x40", 2);
        addr_base += 2;
        memcpy((void*)addr_base, xcm_aad, sizeof(xcm_aad));
        addr_base += sizeof(xcm_aad);
        memcpy((void*)addr_base, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
        addr_base += 14;
    }

    memcpy((void*)addr_base, src, sizeof(plain_text));

    addr_base = addr_base_begin;

    addr_dst = (addr_t)&cipher_output_inddr;

    if ((SD_FCT_CCM == fct)||(SD_FCT_GCM == fct)) {

        if (OPERATION_DEC == operation) {
            src_size += 16;
        }

        dst_size += (head_len + 16);
    }
    else if (SD_FCT_CMAC == fct) {
        dst_size = 16;
    }

    result = hal_aes_final(crypto_handle, (uint8_t*)addr_base, src_size, head_len,
                            (uint8_t*)addr_dst, dst_size);

    LTRACEF("hal_aes_final result= %d\n", result);

    hal_crypto_delete_handle(crypto_handle);

    if (SD_FCT_CCM == fct || SD_FCT_GCM == fct) {
        addr_dst += head_len;
        dst_size -= (head_len + 16);
    }

    result = memcmp(dst, (void*)addr_dst, dst_size);

    if (result) {
        LTRACEF("key: %d, src: %d, dst: %d, key_type: %d, fct: %d, operation: %d, dst_size: %d\n", key_addr_type, src_type, dst_type, key_type, fct, operation, dst_size);
        ce_printf_binary("result: ", (uint8_t*)addr_dst, dst_size);
        ce_printf_binary("should be", dst, dst_size);
    }
    else {
        LTRACEF("aes test success pass! \n");
    }

    return result;
}

uint32_t cipher_cmac(buff_addr_type_t key_addr_type, aes_key_type_t key_type,
                     buff_addr_type_t src_type, buff_addr_type_t dst_type)
{
    uint8_t* key;
    uint32_t key_size;
    addr_t addr_base, addr_dst  = 0;
    uint8_t* src = plain_text;
    uint8_t* dst = cipher_ecb_128;
    int result;
    uint32_t dst_size = sizeof(plain_text);
    uint32_t src_size = sizeof(plain_text);
    void* crypto_handle;

    LTRACEF("cmac test enter! \n");

    switch (key_type) {
        case KEY_TYPE_128:
            key_size = 16;
            key = key_128;
            break;

        case KEY_TYPE_192:
            key_size = 24;
            key = key_192;
            break;

        case KEY_TYPE_256:
            key_size = 32;
            key = key_256;
            break;

        default:
            return -1;
    }

    result = get_plain_cipher(SD_FCT_CMAC, OPERATION_ENC, key_type, dst_type, &src, &dst);

    if (result) {
        LTRACEF("get plain/cipher fail\n");
        return -1;
    }

    addr_base = (addr_t)&cipher_input_inddr;

    memcpy((void*)addr_base, src, sizeof(plain_text));

    addr_dst = (addr_t)&cipher_output_inddr; //iram3

    dst_size = 16;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    result = hal_cmac(crypto_handle, key, key_size, (uint8_t*)addr_base, src_size,
                  (uint8_t*)addr_dst, dst_size);

    LTRACEF("hal_cmac result= %d\n", result);

    hal_crypto_delete_handle(crypto_handle);

    result = memcmp(dst, (void*)addr_dst, dst_size);

    if (result) {
        LTRACEF("key: %d, src: %d, dst: %d, key_type: %d, dst_size: %d\n", key_addr_type, src_type, dst_type, key_type, dst_size);
        ce_printf_binary("result: ", (uint8_t*)addr_dst, dst_size);
        ce_printf_binary("should be", dst, dst_size);

    }
    else {
        LTRACEF("cmac success pass! \n");
    }

    return result;
}

uint32_t cipher_path_test(void)
{
    uint32_t ret;
    sd_aes_fct_t fct;
    aes_operation_t operation;
    buff_addr_type_t key_addr_type = HAL_EXT_MEM;
    buff_addr_type_t src_type = HAL_EXT_MEM;
    buff_addr_type_t dst_type = HAL_EXT_MEM;
    aes_key_type_t key_type;
    uint32_t result_value;
    uint32_t cipher_pass = 0;
    uint32_t cipher_fail = 0;

    for (fct = SD_FCT_ECB; fct <= SD_FCT_CMAC; fct++) {
        //if (SD_FCT_CCM == fct) continue;
        for (operation = OPERATION_ENC; operation <= OPERATION_DEC; operation++) {
            if ((operation == OPERATION_DEC) && (SD_FCT_CMAC == fct)) {
                continue;    //cmac just have enc operation
            }

            for (key_type = KEY_TYPE_128; key_type <= KEY_TYPE_192; key_type++) {
                result_value = cipher_op(fct, operation, key_addr_type, key_type, src_type, dst_type);

                if (fct == SD_FCT_CMAC) {
                    result_value = cipher_cmac(key_addr_type, key_type, src_type, dst_type);
                }

                if(result_value == 0){
                    cipher_pass++;
                }else{
                    cipher_fail++;
                }
            }
        }
    }

    LTRACEF("cipher path test cases total: %u, pass: %u, fail: %u\n", cipher_pass + cipher_fail, cipher_pass, cipher_fail);
    if(cipher_fail == 0){
        ret = 0;
    }else{
        ret = 1;
    }

    return ret;
}

//hal have no context API, just use driver to test this feature
uint32_t context_switch_test(void)
{
    uint8_t * key;
    uint32_t key_size;
    addr_t addr_key, addr_src, addr_dst, addr_ctx;
    uint8_t * dst = cipher_cbc_128;
    int result;
    uint32_t ret;
    uint32_t dst_size = sizeof(plain_text);
    uint32_t head_len = 0;
    uint8_t __attribute__((aligned(CACHE_LINE))) output[64];
    crypto_instance_t* l_cryptoinstance = NULL;
    uint32_t vce_id;

    //case KEY_TYPE_128:
    key_size = 16;
    key = key_128;

    addr_key = SRAM_KEY_OFFSET;
    addr_src = (addr_t)plain_text;
    addr_dst = (addr_t)output;
    addr_ctx = SRAM_KEY_OFFSET + 5 * 64;

    void * crypto_handle;
    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    if (crypto_handle == NULL) {
        LTRACEF("cipher context test fail");
        return 0;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)crypto_handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;
    }

    memcpy_blk(vce_id, block_t_convert((uint8_t *)addr_key, key_size, SRAM_SEC), block_t_convert(key, 16, EXT_MEM), 16);
    memcpy_blk(vce_id, block_t_convert((uint8_t *)SRAM_IV_OFFSET, 16, SRAM_SEC), block_t_convert(cbc_iv, 16, EXT_MEM), 16);

    result = aes_blk(FCT_CBC, vce_id, MODE_ENC, CTX_BEGIN, head_len, false, false,
            block_t_convert((uint8_t *)addr_key, key_size, SRAM_SEC),
            block_t_convert(NULL, 0, 0),
            block_t_convert((uint8_t *)(SRAM_IV_OFFSET), 16, SRAM_SEC),
            block_t_convert((void*)addr_ctx, AES_CTX_SIZE, SRAM_SEC),
            block_t_convert((void*)addr_src, 32, EXT_MEM),
            block_t_convert((void*)addr_dst, 32, EXT_MEM));
    if (result) {
        LTRACEF("ctx_begin test result: 0x%x\n", result);
    }

    addr_src += 32;
    addr_dst += 32;
    result = aes_blk(FCT_CBC, vce_id, MODE_ENC, CTX_END, head_len, false, false,
            block_t_convert((uint8_t *)addr_key, key_size, SRAM_SEC),
            block_t_convert(NULL, 0, 0),
            block_t_convert((uint8_t *)(SRAM_IV_OFFSET), 16, SRAM_SEC),
            block_t_convert((void*)addr_ctx, AES_CTX_SIZE, SRAM_SEC),
            block_t_convert((void*)addr_src, 32, EXT_MEM),
            block_t_convert((void*)addr_dst, 32, EXT_MEM));
    if (result) {
        LTRACEF("ctx_end test result: 0x%x\n", result);
    }

    //hal_crypto_delete_handle(crypto_handle);
    result = memcmp(output, (void *)dst, dst_size);
    if (result) {
        ret = 1;
        printf_binary("result: ", output, dst_size);
        printf_binary("should be", dst, dst_size);
    }else{
        ret = 0;
    }
    hal_crypto_delete_handle(crypto_handle);
    LTRACEF("cipher context test %s\n", result ? "fail" : "pass");
    return ret;
}

uint32_t cipher_test_slt(void* arg)
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

    sram_config();

    //config efuse value by force operation with tcl

    enable_vce_key_interface();

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_CIPHER_PATH_TEST;
    result_value = cipher_path_test();
    ret |= (result_value << CE_TEST_RESULT_OFFSET_CIPHER_PATH_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_CIPHER_CONTEXT_TEST;
    result_value = context_switch_test();
    ret |= (result_value << CE_TEST_RESULT_OFFSET_CIPHER_CONTEXT_TEST);

    return ret;
}

uint32_t cipher_test_uart(void)
{
    uint32_t ret = 0;

    sram_config();

    //config efuse value by force operation with tcl

    enable_vce_key_interface();
    ret = cipher_path_test();
    ret = context_switch_test();

    return ret;
}
#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("cipher_test", "cipher test", (console_cmd)&cipher_test_uart)
STATIC_COMMAND_END(cipher_cipher);

#endif

APP_START(ce_cipher)
.flags = 0
APP_END
