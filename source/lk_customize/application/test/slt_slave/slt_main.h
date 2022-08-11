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
#ifndef _SLT_MAIN_H_
#define _SLT_MAIN_H_

#include <platform.h>
#include <kernel/event.h>
#include <ipcc_device.h>
#include <ipcc_rpc.h>
#include <kernel/timer.h>
#include <lib/slt_module_test.h>

#define SLT_MAIN_TEST_CASE_MAX_IN_DOMAIN 10
#define SLT_MAIN_TEST_RESULT_NAME_LEN_MAX 64
#define SLT_MAIN_TEST_RESULT_STRING_LEN_MAX 256

enum slt_msgque_state {
    /** */
    SLT_MESSAGE_QUE_STATE_OK = 0x0,
    SLT_MESSAGE_QUE_STATE_NOT_INIT,
    SLT_MESSAGE_QUE_STATE_FIFO_FULL,
    SLT_MESSAGE_QUE_STATE_FIFO_EMPTY,
    SLT_MESSAGE_QUE_STATE_END
};

//--------------------- domain id define start-------------------------------------
enum slt_all_domain_type {
    /** */
    SLT_DOMAIN_TYPE_SAFETY = 0x0,
    SLT_DOMAIN_TYPE_SEC,
    SLT_DOMAIN_TYPE_MP,
    SLT_DOMAIN_TYPE_AP1,
    SLT_DOMAIN_TYPE_AP2,
    SLT_DOMAIN_TYPE_END
};
//-----------------------domain id define end-----------------------------------
typedef enum slt_messages_type {
    /** */
    SLT_MESSAGE_FROM_PC_INIT = 0x0,
    SLT_MESSAGE_FROM_PC_CONFIG,
    SLT_MESSAGE_FROM_PC_TEST,
    SLT_MESSAGE_FROM_PC_ITEM_TEST,
    SLT_MESSAGE_FROM_MASTER_INIT = 0x10000,
    SLT_MESSAGE_FROM_MASTER_TEST_START,
    SLT_MESSAGE_FROM_MASTER_ITEM_TEST_START,
    SLT_MESSAGE_FROM_MASTER_ITEM_TEST_END,
    SLT_MESSAGE_FROM_MASTER_TEST_END,
    SLT_MESSAGE_FROM_SLAVE_INIT = 0x20000,
    SLT_MESSAGE_FROM_SLAVE_ITEM_TEST_END,
    SLT_MESSAGE_FROM_SLAVE_TEST_END,
    SLT_MESSAGE_FROM_INTERNEL_START = 0x30000,
    SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_START,
    SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_STOP,
    SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_END,
    SLT_MESSAGE_FROM_INTERNEL_SLAVE_TEST_START,
    SLT_MESSAGE_FROM_INTERNEL_SLAVE_TEST_STOP,
    SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END,
    SLT_MESSAGE_FROM_INTERNEL_ALL_TEST_END,
    SLT_MESSAGE_FROM_INTERNEL_END,
    SLT_MESSAGE_END
} slt_messages_type_t;

#ifdef SLT_RUN_IN_SEC_DOMAIN

#define SLT_MAIN_TEST_EPT                  SLT_TEST_EPT_SEC
#define SLT_MAIN_TEST_EPT_NAME             "slt-test-sec"
#define SLT_MAIN_TEST_MODULE_DOMAIN_TYPE   SLT_DOMAIN_TYPE_SEC

#elif SLT_RUN_IN_AP1_DOMAIN

#define SLT_MAIN_TEST_EPT                  SLT_TEST_EPT_AP1
#define SLT_MAIN_TEST_EPT_NAME             "slt-test-ap1"
#define SLT_MAIN_TEST_MODULE_DOMAIN_TYPE   SLT_DOMAIN_TYPE_AP1

#elif SLT_RUN_IN_MP_DOMAIN

#define SLT_MAIN_TEST_EPT                  SLT_TEST_EPT_MP
#define SLT_MAIN_TEST_EPT_NAME             "slt-test-mp"
#define SLT_MAIN_TEST_MODULE_DOMAIN_TYPE   SLT_DOMAIN_TYPE_MP

#else

#define SLT_MAIN_TEST_EPT                  SLT_TEST_EPT_SAF
#define SLT_MAIN_TEST_EPT_NAME             "slt-test-saf"
#define SLT_MAIN_TEST_MODULE_DOMAIN_TYPE   SLT_DOMAIN_TYPE_SAFETY

#endif

#define SLT_MAIN_MASTER_DOMAIN_ID  SLT_DOMAIN_TYPE_SAFETY
#define SLT_MAIN_MASTER_IPC_DEV_RRPOC  IPCC_RRPOC_SAF

typedef struct slt_item_test {
    uint32_t domain_id;
    uint32_t case_id;
    uint32_t times;
} slt_item_test_t;

typedef struct slt_config_info {
    uint32_t voltage_ap;
    uint32_t freq_ap;
} slt_config_info_t;

typedef struct slt_init_info {
    uint32_t domain_id;
    uint32_t test_num;
    uint32_t selftesty_ready;
    uint32_t software_version;
} slt_init_info_t;

typedef struct slt_test_case_result {
    uint32_t domain_id;
    uint32_t case_id;
    uint32_t level;
    uint32_t result_value;
    uint32_t use_time;
    char test_name[SLT_MAIN_TEST_RESULT_NAME_LEN_MAX];
    char result_string[SLT_MAIN_TEST_RESULT_STRING_LEN_MAX];//if result_value !=0 can recode error string
} slt_test_case_result_t;

typedef struct slt_test_domain_result {
    uint32_t domain_id;
    uint32_t selftest_ready;
    uint32_t test_result;// 0 sucess other fail
    uint32_t test_num;
    uint32_t test_index;
    uint32_t test_pass_num;
    uint32_t test_fail_num;
    uint32_t test_end;
    slt_test_case_result_t test_case_info[SLT_MAIN_TEST_CASE_MAX_IN_DOMAIN];
} slt_test_domain_result_t;

typedef struct slt_test_result {
    uint32_t chip_id;
    uint32_t test_result; // 0 sucess other fail
    uint32_t all_test_num;
    uint32_t all_test_pass_num;
    uint32_t all_test_fail_num;
    uint32_t all_test_usetime;
    uint32_t test_end;
    uint32_t test_cfg_vo;
    uint32_t test_cfg_frq;
    slt_test_domain_result_t test_result_info;
} slt_test_result_t;

typedef struct slt_msg_item {
    int msg_type;
    int msg_len;
    void* msg_value;
} slt_msg_item_t;

typedef struct slt_msg_node {
    int is_node_empty; //0 node is empty
    slt_msg_item_t msg_item;
} slt_msg_node_t;

typedef struct slt_msg_queue_fifo {
    slt_msg_node_t* msg_note;
    int in_pos;
    int out_pos;
    int queue_size;
    int queue_left_num;
    spin_lock_t lock;
} slt_msg_queue_fifo_t;

typedef struct slt_app_context {
    uint8_t app_id;
    void* name;
    uint8_t chip_id;
    uint8_t is_test_ready;
    uint32_t software_version;
    uint32_t domain_id;

//msg queue info
    uint8_t is_msg_init;
    uint8_t is_sleep;
    event_t msg_handle_signal;
    slt_msg_queue_fifo_t msg_queue;
//slt ipc dev
    struct ipcc_device* ipc_dev;
    struct ipcc_channel* ipc_chan;
//slt module test
    void* test_module_info;
//slt test result
    slt_test_result_t slt_test_result;

} slt_app_context_t;

slt_app_context_t* slt_get_gcontext(void);
void* slt_alloc_msg_value_buff(size_t size);
void slt_free_msg_value_buff(void* p);
int slt_putmsginqueue(slt_app_context_t* pcontext, slt_msg_node_t* msg_node);

#endif  /* _SLT_MAIN_H_ */
