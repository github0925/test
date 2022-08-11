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
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include <lib/slt_module_test.h>
#include <trace.h>

#define LOCAL_TRACE 1

int slt_module_test_sample_hook_1(uint times, uint timeout, char* result_string)
{
    int ret = 0x12;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    if (result_string != NULL) {
        strcpy(result_string, "fail cause no memory");
    }

    return ret;
}

int slt_module_test_sample_hook_2(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);
    thread_sleep(1000);
    LTRACEF("timeout end\n");

    return ret;
}

int slt_module_test_sample_hook_3(uint times, uint timeout, char* result_string)
{
    int ret = 0x23;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    if (result_string != NULL) {
        strcpy(result_string, "fail cause xx timeout");
    }

    return ret;
}

int slt_module_test_sample_hook_4(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    return ret;
}

int slt_module_test_sample_hook_5(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    return ret;
}

int slt_module_test_sample_hook_6(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    return ret;
}

// test case name: module_test_sample1
// test case entry: slt_module_test_sample_hook_1
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size
SLT_MODULE_TEST_HOOK_DETAIL(module_test_sample1, slt_module_test_sample_hook_1, SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);

SLT_MODULE_TEST_HOOK_DETAIL(module_test_sample2, slt_module_test_sample_hook_2, SLT_MODULE_TEST_LEVEL_SAMPLE_2, 0, 1, 500, 0, 0);

SLT_MODULE_TEST_HOOK_DETAIL(module_test_sample3, slt_module_test_sample_hook_3, SLT_MODULE_TEST_LEVEL_SAMPLE_3, 0, 1, 500, 0, 0);

SLT_MODULE_TEST_HOOK_DETAIL(module_test_sample4, slt_module_test_sample_hook_4, SLT_MODULE_TEST_LEVEL_SAMPLE_4, 0, 1, 500, 0, 0);

SLT_MODULE_TEST_HOOK(module_test_sample5, slt_module_test_sample_hook_5);
SLT_MODULE_TEST_HOOK(module_test_sample6, slt_module_test_sample_hook_6);
