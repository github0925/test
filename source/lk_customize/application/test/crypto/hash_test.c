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

#include <sd_hash.h>
#include <sd_hmac.h>
#include <trace.h>
#include <sx_hash.h>

#include "hash_data.h"
#include "ce_test.h"

#define LOCAL_TRACE 0 //close local trace 1->0

#define EFUSE_KEY_MEMORY_ADDR 0xf0011070
#define EFUSE_IV1_MEMORY_ADDR 0xf18a0000
#define EFUSE_IV2_MEMORY_ADDR 0xf18b0000
#define PROD_ENABLE_HUK_ADDR 0xf00112a4

#if CE_IN_SAFETY_DOMAIN
#define vce_count 1
#else
#define vce_count 8
#endif

static uint32_t hash_pass = 0;
static uint32_t hash_fail = 0;
const int perf_times = 100;

uint32_t get_digest_size(sd_hash_alg_t hash_alg)
{
    switch (hash_alg) {
        case SD_ALG_MD5:
            return MD5_DIGESTSIZE;

        case SD_ALG_SHA1:
            return SHA1_DIGESTSIZE;

        case SD_ALG_SHA224:
            return SHA224_DIGESTSIZE;

        case SD_ALG_SHA256:
            return SHA256_DIGESTSIZE;

        case SD_ALG_SHA384:
            return SHA384_DIGESTSIZE;

        case SD_ALG_SHA512:
            return SHA512_DIGESTSIZE;

        case SD_ALG_SM3:
            return SM3_DIGESTSIZE;

        default:
            return 0;
    }
}

//test for key_interface as input data
uint32_t key_input_sha256_test(void)
{
    int result;
    uint32_t key_count = 12;
    uint32_t fail_count = 0;
    uint32_t pass_count = 0;

    /* key: password
     * content: passwordpassword
     */
    uint8_t __attribute__((aligned(CACHE_LINE))) digest[SHA256_DIGESTSIZE] =
                        "\x2e\x2b\x24\xf8\xee\x40\xbb\x84"
                        "\x7f\xe8\x5b\xb2\x33\x36\xa3\x9e"
                        "\xf5\x94\x8e\x6b\x49\xd8\x97\x41"
                        "\x9c\xed\x68\x76\x6b\x16\x96\x7a";
    uint8_t __attribute__((aligned(CACHE_LINE))) digest_h[SHA256_DIGESTSIZE] =
                        "\x50\xff\x7e\xbf\x32\x5e\xe4\x57"
                        "\xd5\x2d\xea\x31\xee\xa7\x11\x67"
                        "\xfd\x1e\xed\xf3\x37\x5b\x74\x3a"
                        "\x51\x2a\xbf\x04\xe8\x4c\x96\xe4";

    /* init efuse value by shadow registers */
    addr_t addr_fuse = EFUSE_KEY_MEMORY_ADDR;  //efuse key shadow registers memory base.
    for (uint32_t i = 0; i < (key_count - 2); i++) {
        if (i < 6) {
            memcpy((void*)(_vaddr(addr_fuse + i * 0x20)), (void*)"passwordpasswordpasswordpassword", 32);
        }
        else {
            memcpy((void*)(_vaddr(addr_fuse + 6 * 0x20 + (i - 6) * 0x10)), (void*)"passwordpassword", 16);
        }
    }

    //vlk init, need safety hand over rtc
    memcpy((void*)(_vaddr(EFUSE_IV1_MEMORY_ADDR)), (void*)"passwordpasswordpasswordpassword", 32);
    memcpy((void*)(_vaddr(EFUSE_IV2_MEMORY_ADDR)), (void*)"passwordpasswordpasswordpassword", 32);

    //enable prod_enable bit for HUK using
    uint32_t misc_val = readl(_vaddr(PROD_ENABLE_HUK_ADDR));
    printf("misc val: 0x%x\n", misc_val);
    misc_val |= 0x80;
    writel(misc_val, _vaddr(PROD_ENABLE_HUK_ADDR));

    void* crypto_handle;
    result = hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    if (result) {
        LTRACEF("key_input_sha256_test create handle fail\n");
    }

    uint8_t __attribute__((aligned(CACHE_LINE))) dst_buf[32];
    for (uint32_t i = 0; i < vce_count; i++) {
        addr_t addr_key = 0;
        for (uint32_t j = 0; j < key_count; j++) {
            if (j < 10) {
                addr_key = 0x20 * j;
            }
            else {
                addr_key = 0x40 + j * 0x20;
            }

            memset(dst_buf, 0, 32);
            //todo: for travel all VCEs, just use driver, maybe this part should move to drvier.
            result  = hmac_blk(ALG_SHA256, i, block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t *)(addr_key), 8, KEY_INT),
                    block_t_convert((uint8_t*)(addr_key), 16, KEY_INT),
                    block_t_convert((void*)dst_buf, 32, EXT_MEM));
            if (result) {
                LTRACEF("hmac_blk fail, result: 0x%x\n", result);
                goto _OUT;
            }

            result = memcmp(digest_h, dst_buf, SHA256_DIGESTSIZE);
            if (result) {
                printf_binary("key_input_use_KEY_INT hmac", dst_buf, SHA256_DIGESTSIZE);
                fail_count++;
            }
            else {
                pass_count++;
            }

            memset(dst_buf, 0, 32);
            result = hash_blk(ALG_SHA256, i, block_t_convert(NULL, 0, 0),
                    block_t_convert((uint8_t*)(addr_key), 16, KEY_INT),
                    block_t_convert(dst_buf, SHA256_DIGESTSIZE, EXT_MEM));
            if (result) {
                LTRACEF("hash_blk fail, result : 0x%x\n", result);
                goto _OUT;
            }

            result = memcmp(digest, dst_buf, SHA256_DIGESTSIZE);
            if (result) {
                printf_binary("key_input_use_KEY_INT", dst_buf, SHA256_DIGESTSIZE);
                fail_count++;
            }
            else {
                pass_count++;
            }
        }
    }

    LTRACEF("key and input are use KEY_INT test fail_count: %d, pass_count: %d\n", fail_count, pass_count);

_OUT:
    hal_crypto_delete_handle(crypto_handle);

    return result;
}

uint8_t __attribute__((aligned(CACHE_LINE))) out_data[MAX_DIGESTSIZE];
/* sram usage: 1st block(64 bytes) - iv
 *             2nd block(64 bytes) - dst
 *             3-5 block(64 bytes) - key
 *             6 block -         - src
 */
uint32_t hash_op(sd_hash_alg_t alg, bool iv_en, buff_addr_type_t iv_type, bool hmac_en, buff_addr_type_t key_type,
                 buff_addr_type_t src_type, buff_addr_type_t dst_type)
{

    uint32_t digest_size;
    addr_t addr_base;
    uint8_t* digest;
    addr_t addr_dst;
    void* crypto_handle;
    int result;

    if (HAL_KEY_INT == dst_type && (alg >= SD_ALG_SHA384 && alg != SD_ALG_SM3)) { //key size is 256bit
        return 0;
    }

    digest_size = get_digest_size(alg);

    addr_base = (addr_t)input_data;
    addr_dst = (addr_t)&out_data;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    if (hmac_en) {
        LTRACEF("hmac test enter :\n");
        hal_hmac(crypto_handle, alg, key, key_size, (uint8_t*)addr_base, inputdata_size, (uint8_t*)addr_dst, digest_size);
    }
    else {
        LTRACEF("hash test enter :\n");
        hal_hash(crypto_handle, alg, (uint8_t*)addr_base, inputdata_size, (uint8_t*)addr_dst, digest_size);
    }

    hal_crypto_delete_handle(crypto_handle);

    switch (alg) {
        case SD_ALG_MD5:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_md5_h : expected_digest_md5;
            }
            else {
                digest = hmac_en ? expected_digest_md5_h_2times : expected_digest_md5_2times;
            }

            break;

        case SD_ALG_SHA1:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sha1_h : expected_digest_sha1;
            }
            else {
                digest = hmac_en ? expected_digest_sha1_h_2times : expected_digest_sha1_2times;
            }

            break;

        case SD_ALG_SHA224:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sha224_h : expected_digest_sha224;
            }
            else {
                digest = hmac_en ? expected_digest_sha224_h_2times : expected_digest_sha224_2times;
            }

            break;

        case SD_ALG_SHA256:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sha256_h : expected_digest_sha256;
            }
            else {
                digest = hmac_en ? expected_digest_sha256_h_2times : expected_digest_sha256_2times;
            }

            break;

        case SD_ALG_SHA384:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sha384_h : expected_digest_sha384;
            }
            else {
                digest = hmac_en ? expected_digest_sha384_h_2times : expected_digest_sha384_2times;
            }

            break;

        case SD_ALG_SHA512:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sha512_h : expected_digest_sha512;
            }
            else {
                digest = hmac_en ? expected_digest_sha512_h_2times : expected_digest_sha512_2times;
            }

            break;

        case SD_ALG_SM3:
            if (HAL_SRAM_SEC != dst_type) {
                digest = hmac_en ? expected_digest_sm3_h : expected_digest_sm3;
            }
            else {
                digest = hmac_en ? expected_digest_sm3_h_2times : expected_digest_sm3_2times;
            }

            break;

        default:
            return 0;
    }

    result = memcmp(digest, (uint8_t*)addr_dst, digest_size);


    if (0 == result) {
        LTRACEF("hash test pass :\n");
        hash_pass++;
    }
    else {
        LTRACEF("hash test fail :\n");
        hash_fail++;
    }

    if (HAL_EXT_MEM != src_type) {
        addr_dst = SRAM_BASE_ADDR + SRAM_DST_OFFSET;
    }

    if (result) {
        ce_printf(alg, iv_en, iv_type, hmac_en, key_type, src_type, dst_type,
                  (0 == result), (uint8_t*)addr_dst, digest_size);
    }

    return 0;
}

uint32_t hash_path_test(void)
{
    sd_hash_alg_t alg;
    int ret = 0;

    for (alg = SD_ALG_MD5; alg <= SD_ALG_SM3; alg++) {

        //hmac not support sm3
        if (SD_ALG_SM3 == alg) {
            hash_op(alg, false, HAL_SRAM_PUB, false, HAL_SRAM_PUB, HAL_EXT_MEM, HAL_EXT_MEM);
        }else{
            hash_op(alg, false, HAL_SRAM_PUB, false, HAL_SRAM_PUB, HAL_EXT_MEM, HAL_EXT_MEM);
            hash_op(alg, false, HAL_SRAM_PUB, true, HAL_SRAM_PUB, HAL_EXT_MEM, HAL_EXT_MEM);
        }
    }

    LTRACEF("hash path test cases total: %u, pass: %u, fail: %u\n", hash_pass + hash_fail, hash_pass, hash_fail);

    if(hash_fail == 0){
        ret = 0;
    }else{
        ret = 1;
    }
    return ret;
}

uint8_t __attribute__((aligned(CACHE_LINE))) out_data_multi[MAX_DIGESTSIZE];
//multi-part test
uint32_t hash_multi_test(void)
{
    uint32_t offset = 0;
    uint32_t ret = 0;
    uint32_t input_size = inputdata_multi_size;
    uint32_t part_num = input_size / SHA256_BLOCKSIZE;
    int result1, result2;
    void* crypto_handle = NULL;
    addr_t addr_dst;
    addr_t addr_dst_multi;

    addr_dst = (addr_t)&out_data;
    addr_dst_multi = (addr_t)&out_data_multi;
    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    for (uint32_t i = 0; i < part_num; i++) {

        hal_hash_update(crypto_handle, SD_ALG_SHA256, (0 == i), (void*)(input_data_multi + offset), SHA256_BLOCKSIZE);
        offset += SHA256_BLOCKSIZE;
    }

    hal_hash_finish(crypto_handle, SD_ALG_SHA256, (void*)(input_data_multi + offset), (input_size - offset), (uint8_t*)addr_dst_multi, SHA256_DIGESTSIZE, input_size);

    hal_hash(crypto_handle, SD_ALG_SHA256, input_data_multi, input_size, (uint8_t*)addr_dst, SHA256_DIGESTSIZE);

    hal_crypto_delete_handle(crypto_handle);

    result1 = memcmp((uint8_t*)(addr_dst_multi), expected_digest_multi, SHA256_DIGESTSIZE);
    result2 = memcmp((uint8_t*)(addr_dst), expected_digest_multi, SHA256_DIGESTSIZE);

    ce_printf(SD_ALG_SHA256, false, 0, false, 0, HAL_SRAM_PUB, HAL_SRAM_PUB, (0 == result1), (uint8_t*)(addr_dst_multi), SHA256_DIGESTSIZE);
    ce_printf(SD_ALG_SHA256, false, 0, false, 0, HAL_SRAM_PUB, HAL_SRAM_PUB, (0 == result2), (uint8_t*)(addr_dst), SHA256_DIGESTSIZE);
    LTRACEF("multi part test result: %s, one time test result: %s\n",
            0 == result1 ? "PASS" : "FAIL", 0 == result2 ? "PASS" : "FAIL");
    if((result1 == 0)&&(result2 == 0)){
        ret = 0;
    }else{
        ret = 1;
    }
    return ret;
}

void reg_trav(void)
{
    uint32_t i, j;
    int read_val;
    int result = 0;
    uint32_t pass_cnt = 0;
    uint32_t fail_cnt = 0;
    addr_t grp_end = 0x14C;

    /*
     * 0-7 : 00x-0x148, 8 : 0x0 - 0x158
     */
    for (i = 0; i < 9; i++) {
        if (8 == i) {
            grp_end = 0x15C;
        }

        for (j = 0; j < grp_end;) {
            read_val = readl(CE_SOC_BASE_ADDR + j);

            if ((-1 == read_val)) {
                LTRACEF("access register 0x%x fail\n", CE_SOC_BASE_ADDR + j);
                result = 1;
                fail_cnt++;
            }
            else {
                pass_cnt++;
            }

            j = j + 4;
        }

        if (1 == result) {
            LTRACEF("last addr : 0x%x \n", CE_SOC_BASE_ADDR + j);
        }
    }

    LTRACEF("scan_conn pass : %d, fail : %d\n", pass_cnt, fail_cnt);
}

uint32_t key_port_test(void)
{
    sram_config();
    enable_vce_key_interface();
    key_input_sha256_test();

    return 0;
}

uint32_t hash_test_slt(void* arg)
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

    enable_vce_key_interface();

    //path test
    ce_test_s->current_index = CE_TEST_ITEM_INDEX_HASH_PATH_TEST;
    result_value = hash_path_test();
    ret |= (result_value << CE_TEST_RESULT_OFFSET_HASH_PATH_TEST);
    //multi-part test
    ce_test_s->current_index = CE_TEST_ITEM_INDEX_HASH_MULT_TEST;
    result_value = hash_multi_test();
    ret |= (result_value << CE_TEST_RESULT_OFFSET_HASH_MULT_TEST);
#if CE_TEST_SECURE_STORAGE
    key_input_sha256_test();
#endif
    return ret;
}

uint32_t hash_test_uart(void)
{
    uint32_t ret = 0;
    sram_config();

    enable_vce_key_interface();
    ret = hash_path_test();
    ret |= hash_multi_test();

    return ret;
}

void hashtest_entry(const struct app_descriptor* app, void* args)
{
    hash_test_uart();
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("hash_test", "hash test", (console_cmd)&hash_test_uart)
STATIC_COMMAND("key_port_test", "key_port test", (console_cmd)&key_port_test)
STATIC_COMMAND_END(ce_hash);

#endif

APP_START(ce_hash)
.flags = 0,
//.entry=hashtest_entry,
APP_END
