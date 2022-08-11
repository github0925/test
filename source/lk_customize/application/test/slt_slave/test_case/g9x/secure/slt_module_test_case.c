/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <arch/ops.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include <lib/slt_module_test.h>
#include <trace.h>
#include <clkgen_hal.h>
#include <crypto_hal.h>
#include <sd_rng.h>
#include <sd_hash.h>
#include <string.h>
#include "image_cfg.h"
#include "res.h"

#define TCM_BASE 0x4b0000
#define TCM_SIZE 0x20000

#define DDR_BASE 0x30000000
#define IRAM1_SIZE (256 * 1024)
#define RAND_BUF_SZ (256)

#define LOCAL_TRACE 1

#define CE_ID_DEF  (g_ce_mem_res.res_id[1])

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

#define DBG(format, args...) dprintf(INFO, \
                               "DBG:%s %d "format"\n", __func__, __LINE__,  ##args);

extern const domain_res_t g_ce_mem_res;

static uint8_t slt_random_n[RAND_BUF_SZ] __ALIGNED(CACHE_LINE);
static int slt_check_memory(uint8_t * src, uint32_t src_len, uint8_t *dst, uint32_t dst_len)
{
    int ret = -1;
    uint32_t remain = 0;
    uint32_t loop_cnt = 0;
    crypto_status_t status;
    void *crypto_handle = NULL;
    uint8_t md5_orig[16] __ALIGNED(CACHE_LINE)= {0};
    uint8_t md5_val[16] __ALIGNED(CACHE_LINE)= {0};
    sd_hash_alg_t hash_alg = SD_ALG_MD5;

    if ((src_len % CACHE_LINE) != 0
        || (dst_len % CACHE_LINE) != 0
        || ((addr_t)src) % CACHE_LINE != 0
        || ((addr_t)dst) % CACHE_LINE != 0)
    {
        ERROR("param error!\n");
        ret = -1;
        goto out;
    }

    hal_crypto_creat_handle(&crypto_handle, CE_ID_DEF);
    if (crypto_handle == NULL) {
        ERROR("create handle error\n");
        ret = -2;
        goto out;
    }

    loop_cnt = dst_len / src_len;
    remain = dst_len % src_len;
    if (loop_cnt && !remain)
    {
        loop_cnt--;
        remain = src_len;
    }

    for(uint32_t i = 0; i < loop_cnt; i++)
    {
        memset(src, 0x0, src_len);
        status = hal_trng_gen(crypto_handle, src, src_len);
#if 0
        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to gen random number!");
            ret = -3;
            goto out;
        }
#endif
        status = hal_hash_update(crypto_handle, hash_alg, i == 0, src, src_len);
        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to calc digest, i:%d", i);
            ret = -4;
            goto out;
        }

        memcpy(dst + i * src_len, src, src_len);
    }

    if(remain)
    {
        memset(src, 0x0, remain);
        status = hal_trng_gen(crypto_handle, src, remain);
#if 0
        hexdump8(src, remain);
        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to gen random number!");
            ret = -5;
            goto out;
        }
#endif
        if (loop_cnt)
            status = hal_hash_finish(crypto_handle, hash_alg, src, remain, md5_orig, 16, dst_len);
        else
        {
            status = hal_hash(crypto_handle, hash_alg, src, remain, md5_orig, 16);
        }

        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to calc digest");
            ret = -6;
            goto out;
        }

        memcpy(dst + loop_cnt * src_len, src, remain);
    }

    arch_clean_cache_range((addr_t)dst, dst_len);

    for (uint32_t i = 0; i < loop_cnt; i++)
    {
        memset(src, 0x0, src_len);
        memcpy(src, dst + i * src_len, src_len);

        status = hal_hash_update(crypto_handle, hash_alg, i == 0, src, src_len);
        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to calc digest, i:%d", i);
            ret = -7;
            goto out;
        }
    }

    if(remain)
    {
        memset(src, 0x0, remain);
        memcpy(src, dst + loop_cnt * src_len, remain);

        if (loop_cnt)
            status = hal_hash_finish(crypto_handle, hash_alg, src, remain, md5_val, 16, dst_len);
        else
        {
            status = hal_hash(crypto_handle, hash_alg, src, remain, md5_val, 16);
        }

        if (status !=  CRYPTO_SUCCESS)
        {
            ERROR("fail to calc digest");
            ret = status;
            goto out;
        }
    }

    if (memcmp(md5_val, md5_orig, 16))
    {
        ERROR("fail to calc digest");
        ret = -9;
        goto out;
    }
    hexdump8(md5_val, 16);
    ret = 0;
out:
    if (crypto_handle)
        hal_crypto_delete_handle(crypto_handle);

    return ret;
}

int TEST_SEC_SS_01(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    uint8_t * ddr_buf = (uint8_t*)_ioaddr (SEC_MEMBASE);

    LTRACEF("entry, times= %d, timeout =%d\n", times, timeout);

    if(!ddr_buf || ((addr_t)ddr_buf) < DDR_BASE)
    {
        ERROR("fail to allocate memory!");
        if (result_string != NULL) {
            strcpy(result_string, "fail: no enough memory!");
        }
        goto out;
    }

    arch_enable_cache(UCACHE);
    ret = slt_check_memory(slt_random_n, sizeof(slt_random_n), ddr_buf, sizeof(slt_random_n));
    if(ret)
    {
        ERROR("enable cache error!");
        if (result_string != NULL) {
            sprintf(result_string, "fail: enable cache error:0x%0x!", ret);
        }
        goto out;
    }

    arch_disable_cache(UCACHE);
    ret = slt_check_memory(slt_random_n, RAND_BUF_SZ, ddr_buf, RAND_BUF_SZ);
    if(ret)
    {
        ERROR("disable cache error!");
        if (result_string != NULL) {
            sprintf(result_string, "fail: disable cache error:%d!", ret);
        }
        goto out;
    }

    arch_enable_cache(UCACHE);
    ret = 0;
out:

    return ret;
}

extern int __tcm_end;
int TEST_SEC_SS_02(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    uint8_t * tcm = (uint8_t*) (((uint32_t)&__tcm_end + CACHE_LINE) & ~(CACHE_LINE - 1));
    uint32_t tcm_sz = TCM_BASE + TCM_SIZE - (uint32_t)tcm;

    if (tcm_sz)
        ret = slt_check_memory(slt_random_n, RAND_BUF_SZ, tcm, tcm_sz);
    if(ret)
    {
        ERROR("check tcm error!");
        if (result_string != NULL) {
            sprintf(result_string, "fail: check error:%d!", ret);
        }
        goto out;
    }

    ret = 0;
out:
    return ret;
}

int TEST_SEC_SS_03(uint times, uint timeout, char* result_string)
{
    DBG("Here, it means that iram2/iram3/iram4 has been checked by DIL!");
    return 0;
}

int TEST_SEC_SS_04(uint times, uint timeout, char* result_string)
{
    DBG("Should tested in ap2!");
    return 0;
}

int TEST_SEC_SS_10(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    bool ckgen_ret = true;
    void *ckgen_handle = NULL;

    ckgen_ret = hal_clock_creat_handle(&ckgen_handle);
    if (!ckgen_ret) {
        ERROR("clkgen creat handle failed\n");
        goto out;
    }

    ckgen_ret = hal_clock_osc_init(ckgen_handle, RES_SCR_L16_SEC_FSREFCLK_SEC, xtal_saf_24M,
                                 true);
    if (!ckgen_ret) {
        ERROR("fail to switch sec fsref clock\n");
        goto out;
    }
    ret = 0;
out:
    if (ckgen_handle)
        hal_clock_release_handle(ckgen_handle);

    return ret;
}

// test case name: module_test_sample1
// test case entry: TEST_SEC_SS_01
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_10, TEST_SEC_SS_10, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_04, TEST_SEC_SS_04, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 1000, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_03, TEST_SEC_SS_03, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 1000, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_02, TEST_SEC_SS_02, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 1000, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_01, TEST_SEC_SS_01, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 1000, 0, 0);

