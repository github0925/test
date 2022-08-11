/*
 * Copyright (c) 2019, Semidrive, Inc. All rights reserved
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
#ifndef _SLT_TEST_H_
#define _SLT_TEST_H_
#include <slt_main.h>
#include <slt_message.h>
#include <lib/slt_module_test.h>

typedef struct slt_test_module_info {
    uint32_t domain_id;
    uint32_t test_num;
    uint32_t is_init;
    event_t start_test_signal;
    uint32_t test_index;
    void* parent_ptr;
    slt_test_module_item_info_t item_info[SLT_MODULE_TEST_MODULE_NUM_MAX];
} slt_test_module_info_t;

int slt_test_module_test_start(slt_app_context_t* pcontext);
int slt_test_item_test_start(slt_app_context_t* pcontext, slt_item_test_t* item_test);
int slt_test_init(slt_app_context_t* pcontext);
int slt_test_get_test_num(slt_app_context_t* pcontext);
int slt_test_recode_test_info(slt_app_context_t* pcontext, slt_test_case_result_t* result_info);

#endif  /* _SLT_TEST_H_ */
