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
#include <heap.h>
#include <clkgen_hal.h>
#include <crypto_hal.h>
#include <sd_rng.h>
#include <sd_hash.h>
#include "res.h"

#define TCM_BASE 0x4b0000
#define TCM_SIZE 0x20000

#define DDR_BASE 0x30000000
#define IRAM1_SIZE (256 * 1024)
#define RAND_BUF_SZ (256)

#define LOCAL_TRACE 1
#define PAD_VAL 0x95

#define CE_ID_DEF  (g_ce_mem_res.res_id[0])

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

#define DBG(format, args...) dprintf(CRITICAL, \
                               "DBG:%s %d "format"\n", __func__, __LINE__,  ##args);

extern const domain_res_t g_ce_mem_res;

static int check_memory(uint8_t * src, uint32_t src_len, uint8_t *dst, uint32_t dst_len, uint8_t pad_val)
{
    int ret = -1;
    uint32_t remain = 0;
    uint32_t loop_cnt = 0;
    uint32_t * temp_dst = NULL;
    uint32_t temp_val = (pad_val << 24)|(pad_val << 16)|(pad_val << 8)|(pad_val << 0);

    if ((src_len % CACHE_LINE) != 0
        || (dst_len % CACHE_LINE) != 0
        || ((addr_t)src) % CACHE_LINE != 0
        || ((addr_t)dst) % CACHE_LINE != 0)
    {
        ERROR("param error!\n");
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
        memset(src, pad_val, src_len);
        memcpy(dst + i * src_len, src, src_len);
    }

    if(remain)
    {
        memset(src, pad_val, remain);
        memcpy(dst + loop_cnt * src_len, src, remain);
    }

    arch_clean_invalidate_cache_range((addr_t)dst, dst_len);

    temp_dst = (uint32_t*)dst;
    for(uint32_t i = 0;i < dst_len / sizeof(uint32_t);i++, temp_dst++)
    {
        if (*temp_dst != temp_val)
        {
            ERROR("param error!\n");
            goto out;
        }
    }

    ret = 0;
out:
    return ret;
}

static uint8_t _Alignas(CACHE_LINE) slt_ddr_buf[RAND_BUF_SZ];

int TEST_SAFE_SS_01(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    uint8_t * ddr_buf = NULL;
    uint8_t *random_n;

    LTRACEF("entry, times= %d, timeout =%d\n", times, timeout);

    ddr_buf = slt_ddr_buf;
    random_n = memalign(CACHE_LINE, RAND_BUF_SZ);
    if (!random_n)
        return ret;
    memset(random_n, 0, RAND_BUF_SZ);

#if 0
    if(!ddr_buf || ((addr_t)ddr_buf) < DDR_BASE)
    {
        ERROR("fail to allocate memory, ddr_buf:%p!", ddr_buf);
        if (result_string != NULL) {
            strcpy(result_string, "fail: no enough memory!");
        }
        goto out;
    }
#endif
    arch_enable_cache(UCACHE);
    ret = 0;
    ret = check_memory(random_n, RAND_BUF_SZ, ddr_buf, RAND_BUF_SZ, 0x0);
    ret |= check_memory(random_n, RAND_BUF_SZ, ddr_buf, RAND_BUF_SZ, 0xff);
    if(ret)
    {
        ERROR("enable cache error!");
        if (result_string != NULL) {
            strcpy(result_string, "fail: enable cache error!");
        }
        goto out;
    }

    arch_disable_cache(UCACHE);
    ret = 0;
    ret = check_memory(random_n, RAND_BUF_SZ, ddr_buf, RAND_BUF_SZ, 0x0);
    ret |= check_memory(random_n, RAND_BUF_SZ, ddr_buf, RAND_BUF_SZ, 0xff);
    if(ret)
    {
        ERROR("disable cache error!");
        if (result_string != NULL) {
            strcpy(result_string, "fail: disable cache error!");
        }
        goto out;
    }

    arch_enable_cache(UCACHE);
    ret = 0;

out:
    free(random_n);

    return ret;
}

int TEST_SAFE_SS_02(uint times, uint timeout, char* result_string)
{
#if 0
    int ret = -1;
    void * handle = NULL;
    bool clk_ret = false;

    clk_ret = hal_clock_creat_handle(&handle);
    if (!clk_ret) {
        ERROR("clkgen creat handle failed\n");
        goto out;
    }

    clk_ret = hal_clock_osc_init(handle, RES_SCR_L16_SAF_FSREFCLK_SAF,
                            xtal_saf_24M, true);
    if (!clk_ret) {
        ERROR("fail to switch fsrefclk to rc_24\n");
        if (result_string != NULL)
            strcpy(result_string, "fail: switch rc 24m");

        goto out;
    }

    ret = 0;

out:
    if(handle)
        hal_clock_release_handle(handle);
#endif
    DBG("RC_24M is ok if BootROM run!");
    return 0;
}

static uint8_t _Alignas(CACHE_LINE) slt_tcm[RAND_BUF_SZ * 4 * 8] __attribute__((section(".tcmdata")));

int TEST_SAFE_SS_03(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    uint8_t *random_n;
    uint8_t * tcm = slt_tcm;
    uint32_t tcm_sz = sizeof(slt_tcm);

    random_n = memalign(CACHE_LINE, RAND_BUF_SZ);
    if (!random_n)
        return ret;
    memset(random_n, 0, RAND_BUF_SZ);

    ret = 0;
    ret = check_memory(random_n, RAND_BUF_SZ, tcm, tcm_sz, 0x0);
    ret |= check_memory(random_n, RAND_BUF_SZ, tcm, tcm_sz, 0xff);
    if(ret)
    {
        ERROR("check tcm error!");
        if (result_string != NULL) {
            strcpy(result_string, "fail: check error!");
        }
        goto out;
    }

    ret = 0;
out:
    free(random_n);

    return ret;
}

int TEST_SAFE_SS_04(uint times, uint timeout, char* result_string)
{
    int ret = -1;
    uint8_t random_n[RAND_BUF_SZ] __ALIGNED(CACHE_LINE) = {0};
    uint8_t * iram1 = (uint8_t*) IRAM1_BASE;
    uint32_t iram1_sz = IRAM1_SIZE;

    ret = 0;
    ret = check_memory(random_n, sizeof(random_n), iram1, iram1_sz, 0x0);
    ret |= check_memory(random_n, sizeof(random_n), iram1, iram1_sz, 0xff);
    if(ret)
    {
        ERROR("check tcm error!");
        if (result_string != NULL) {
            strcpy(result_string, "fail: check error!");
        }
        goto out;
    }

    ret = 0;
out:
    return ret;
}

int TEST_SAFE_SS_05(uint times, uint timeout, char* result_string)
{
#if 0
    int ret = -1;
    void * handle = NULL;
    bool clk_ret = false;
    clkgen_app_bus_cfg_t bus_app_cfg = {
        .clk_src_select_a_num = 4,
        .clk_src_select_b_num = 4,
        .clk_a_b_select = 1,
        .pre_div_a = 0,
        .pre_div_b = 0,
        .post_div = 0,
        .m_div = 1,
        .n_div = 3,
        .p_div = 0,
        .q_div = 0,
    };
#if 0
    clkgen_app_ip_cfg_t uart4_clk_cfg = {
        .clk_src_select_num = 5,
        .pre_div = 0,
        .post_div = 1,
    };
#endif

    clk_ret = hal_clock_creat_handle(&handle);
    if (!clk_ret) {
        ERROR("clkgen creat handle failed\n");
        goto out;
    }
    clk_ret = hal_clock_busclk_set(handle, RES_BUS_SLICE_SAF_SAF_PLAT_CTL, &bus_app_cfg);

    if (!clk_ret) {
        ERROR("fail to switch safety bus clock!\n");
        goto out;
    }
#if 0
    clk_ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SAF_UART_SAF, &uart4_clk_cfg);
    if (!clk_ret) {
        ERROR("fail to switch uart4 ip clock!\n");
        goto out;
    }
#else
    DBG("safety uart clock src has been switch to clk13 from PLL2!\n");
#endif

    ret = 0;
out:
    if(handle)
        hal_clock_release_handle(handle);
#endif
    DBG("PLL1/PLL2 has been setup by BootROM!");
    return  0;
}

int TEST_SAFE_SS_06(uint times, uint timeout, char* result_string)
{
    DBG("Here, it means that ssystem has run and mailbox is ok!");
    return 0;
}

int TEST_DDR_SS_01(uint times, uint timeout, char* result_string)
{
    return TEST_SAFE_SS_01(times, timeout, result_string);
}

// test case name: module_test_sample1
// test case entry: TEST_SAFE_SS_01
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size

SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_01, TEST_SAFE_SS_01, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_02, TEST_SAFE_SS_02, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_03, TEST_SAFE_SS_03, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_04, TEST_SAFE_SS_04, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_05, TEST_SAFE_SS_05, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_06, TEST_SAFE_SS_06, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(ddr_ss_01, TEST_DDR_SS_01, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);

