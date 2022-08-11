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
#ifndef _SLT_MODULE_TEST_H_
#define _SLT_MODULE_TEST_H_

#include <sys/types.h>
#include <thread.h>
#include <timer.h>
#include <event.h>

#define SLT_FLAG_DEFAULT_STACK_SIZE 0x0
#define SLT_FLAG_CUSTOM_STACK_SIZE 0x8

#define SLT_MODULE_TEST_PARALLEL_DEFAULT 0

#define SLT_MODULE_TEST_TIMEOUT_DEFAULT 1000*60 //60S
#define SLT_MODULE_TEST_TIME_DEFAULT 1

#define SLT_MODULE_TEST_RESULT_STRING_MAX_LEN 256

#define SLT_MODULE_TEST_MODULE_NUM_MAX 20

typedef int (*slt_module_test_hook)(uint times, uint timeout, char* error_str);
typedef int (*slt_module_test_end_hook)(void * item_info, int test_result);

//module test and level id in this enum
enum slt_module_test_level {
    SLT_MODULE_TEST_LEVEL_EARLIEST = 1,
    SLT_MODULE_TEST_LEVEL_SAMPLE_1,
    SLT_MODULE_TEST_LEVEL_SAMPLE_2,
    SLT_MODULE_TEST_LEVEL_SAMPLE_3,
    SLT_MODULE_TEST_LEVEL_SAMPLE_4,
    SLT_MODULE_TEST_LEVEL_DDR,

    SLT_MODULE_TEST_LEVEL_LAST,
};

typedef struct slt_module_test_struct {
    uint level;
    uint parallel;
    uint times;
    uint timeout;
    uint flags;
    size_t stack_size;
    slt_module_test_hook hook;
    const char *name;
}slt_module_test_struct_t;

typedef struct slt_test_module_item_info {
    uint id;
    uint level;
    uint times;
    uint timeout;
    uint parallel;
    uint invalid;
    int test_result;
    uint use_time;
    thread_t *t;
    uint is_module_test_timer_init;
    timer_t module_test_timer;
    event_t wait_signal;
    slt_module_test_end_hook end_hook;
    const char* name;
    char* result_string;
    const slt_module_test_struct_t* module_item;
    void* parent_ptr;
} slt_test_module_item_info_t;

#if MODULE_STATIC_LIB
#define SLT_MODULE_TEST_HOOK_DETAIL(a,b,c,d,e) _Pragma("GCC error \"slt hooks are not fully compatible with static libraries\"")
#else
#define SLT_MODULE_TEST_HOOK_DETAIL(_name, _hook, _level, _parallel, _times, _timeout, _flags, _stack_size) \
    __USED const struct slt_module_test_struct _slt_module_test_struct_##_name __ALIGNED(sizeof(void *)) __SECTION(".slt_module_test") = { \
        .level = _level, \
        .parallel = _parallel, \
        .times = _times, \
        .timeout = _timeout, \
        .flags = _flags, \
        .stack_size = _stack_size, \
        .hook = _hook, \
        .name = #_name, \
    };
#endif

#define SLT_MODULE_TEST_HOOK(_name, _hook) \
    SLT_MODULE_TEST_HOOK_DETAIL(_name, _hook, SLT_MODULE_TEST_LEVEL_DDR, \
                                              SLT_MODULE_TEST_PARALLEL_DEFAULT, \
                                              SLT_MODULE_TEST_TIME_DEFAULT, \
                                              SLT_MODULE_TEST_TIMEOUT_DEFAULT, \
                                              SLT_FLAG_DEFAULT_STACK_SIZE, \
                                              DEFAULT_STACK_SIZE)

int slt_run_module_test(slt_test_module_item_info_t* item_info);
uint32_t slt_get_module_num(void);
const struct slt_module_test_struct * slt_get_module( uint level);
const struct slt_module_test_struct* slt_get_module_byindex(uint index);
#endif  /* _SLT_MODULE_TEST_H_ */
