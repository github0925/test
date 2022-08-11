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
#include <heap.h>
#include <lib/console.h>
#include <app.h>
#include <lk/init.h>

#include <slt_main.h>
#include <slt_test.h>
#include <slt_config.h>
#include <slt_message.h>

#define LOCAL_TRACE 0

#define SLT_MSG_NODE_NUMBER_MAX  5

#define SLT_DOMAIN_TEST_TIMEOUT_DEFAULT_VALUE  5*1000*60 //5 min

#ifdef SLT_PROJECT_REFERENCE_G9
#define SLT_MAIN_SAFETY_MODULE_TEST_ENABLE  1
#define SLT_MAIN_SEC_MODULE_TEST_ENABLE     1
#define SLT_MAIN_MP_MODULE_TEST_ENABLE      0
#define SLT_MAIN_AP1_MODULE_TEST_ENABLE     0
#define SLT_MAIN_AP2_MODULE_TEST_ENABLE     1
#elif defined(SLT_PROJECT_REFERENCE_X9)
#define SLT_MAIN_SAFETY_MODULE_TEST_ENABLE 1
#define SLT_MAIN_SEC_MODULE_TEST_ENABLE 1
#define SLT_MAIN_MP_MODULE_TEST_ENABLE 0
#define SLT_MAIN_AP1_MODULE_TEST_ENABLE 1
#define SLT_MAIN_AP2_MODULE_TEST_ENABLE 1
#else
#define SLT_MAIN_SAFETY_MODULE_TEST_ENABLE 1
#define SLT_MAIN_SEC_MODULE_TEST_ENABLE 1
#define SLT_MAIN_MP_MODULE_TEST_ENABLE 0
#define SLT_MAIN_AP1_MODULE_TEST_ENABLE 1
#define SLT_MAIN_AP2_MODULE_TEST_ENABLE 1
#endif
slt_app_context_t* g_context;

#define slt_malloc(sz)      malloc(sz)
#define slt_free(p)         free(p)

void* slt_alloc_msg_value_buff(size_t size)
{
    void* p = slt_malloc(size);

    if (!p) {
        return NULL;
    }

    memset(p, 0, size);
    return p;
}

void slt_free_msg_value_buff(void* p)
{
    slt_free(p);
}

//---------msg queue function start --------------------
static int slt_msgqueueinit(slt_app_context_t* pcontext)
{
    int ret = 0;

    event_init(&(pcontext->msg_handle_signal), false, EVENT_FLAG_AUTOUNSIGNAL);

    pcontext->msg_queue.msg_note = (slt_msg_node_t*)slt_alloc_msg_value_buff(sizeof(slt_msg_node_t) * SLT_MSG_NODE_NUMBER_MAX);
    pcontext->msg_queue.in_pos = 0;
    pcontext->msg_queue.out_pos = 0;
    pcontext->msg_queue.queue_size = SLT_MSG_NODE_NUMBER_MAX;
    pcontext->msg_queue.queue_left_num = SLT_MSG_NODE_NUMBER_MAX;
    pcontext->msg_queue.lock = 0;

    pcontext->is_msg_init = 1;
    return ret ;
}

int slt_putmsginqueue(slt_app_context_t* pcontext, slt_msg_node_t* msg_node)
{
    int ret;
    spin_lock_saved_state_t states;

    if (pcontext->is_msg_init == 0) {
        ret = SLT_MESSAGE_QUE_STATE_NOT_INIT;
        return ret;
    }

    if (pcontext->msg_queue.queue_left_num == 0) { //queue is full
        ret = SLT_MESSAGE_QUE_STATE_FIFO_FULL;
        return ret;
    }

    spin_lock_irqsave(&(pcontext->msg_queue.lock), states);

    msg_node->is_node_empty = 1;

    memcpy((void*)((pcontext->msg_queue.msg_note) + (pcontext->msg_queue.in_pos)), (void*)msg_node, sizeof(slt_msg_node_t));

    pcontext->msg_queue.in_pos++;

    if ((pcontext->msg_queue.in_pos) == (pcontext->msg_queue.queue_size)) {
        pcontext->msg_queue.in_pos = 0;
    }

    pcontext->msg_queue.queue_left_num--;

    spin_unlock_irqrestore(&(pcontext->msg_queue.lock), states);

    if (pcontext->is_sleep == 1) {
        event_signal(&(pcontext->msg_handle_signal), false);
    }

    ret = SLT_MESSAGE_QUE_STATE_OK;

    return ret ;
}

static int slt_getmsgoutqueue(slt_app_context_t* pcontext, slt_msg_node_t* msg_node)
{
    int ret;
    spin_lock_saved_state_t states;

    if (pcontext->is_msg_init == 0) {
        ret = SLT_MESSAGE_QUE_STATE_NOT_INIT;
        return ret;
    }

    if ((pcontext->msg_queue.queue_left_num) == (pcontext->msg_queue.queue_size)) { //queue is empty
        ret = SLT_MESSAGE_QUE_STATE_FIFO_EMPTY;
        return ret;
    }

    spin_lock_irqsave(&(pcontext->msg_queue.lock), states);

    memcpy((void*)msg_node, (void*)((pcontext->msg_queue.msg_note) + (pcontext->msg_queue.out_pos)), sizeof(slt_msg_node_t));

    pcontext->msg_queue.out_pos++;

    if (pcontext->msg_queue.out_pos == pcontext->msg_queue.queue_size) {
        pcontext->msg_queue.out_pos = 0;
    }

    pcontext->msg_queue.queue_left_num++;

    spin_unlock_irqrestore(&(pcontext->msg_queue.lock), states);

    ret = SLT_MESSAGE_QUE_STATE_OK;

    return ret ;
}

//--------------msg queue function end------------------------------------

static int slt_send_msg(slt_app_context_t* pcontext, int msg_type, int msg_len, void* msg_value)
{
    int ret;
    slt_msg_node_t msg_node;

    //init msg_node
    msg_node.msg_item.msg_type = msg_type;
    msg_node.msg_item.msg_len = msg_len;
    msg_node.msg_item.msg_value = msg_value;

    //send cmd msg
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

static enum handler_return slt_slave_test_timer_cb(struct timer* t, lk_time_t now, void* arg)
{
    int slave_index;
    slt_app_context_t* temp = (slt_app_context_t*)arg;

    //get slave id by timer
    for(slave_index = 0; slave_index < SLT_DOMAIN_TYPE_END; slave_index++){
        if(&(temp->slave_test_timer[slave_index]) == t){
            break;
        }
    }

    if(slave_index < SLT_DOMAIN_TYPE_END){

        temp->slt_test_result.test_result_info[slave_index].test_end = 1;

        if(temp->slave_test_timer_init[slave_index] == 1){
            timer_cancel(&(temp->slave_test_timer[slave_index]));
            temp->slave_test_timer_init[slave_index] = 0;
        }

        //check all test result
        slt_send_msg(temp, SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END, 0, NULL);
    }

    return 0;
}

int __WEAK notify_host_ready(void)
{
    return 0;
}

int __WEAK notify_host_test_result(slt_app_context_t* pcontext)
{
    return 0;
}

bool slt_all_domain_init_done(void)
{
    bool ret = true;
    slt_app_context_t* pcontext = slt_get_gcontext();

    if(pcontext == NULL){
        ret = false;
        return ret;
    }

    for(uint32_t i = 0; i < SLT_DOMAIN_TYPE_END; i++){
        if((pcontext->domain_test_enable[i] == 1)
            && pcontext->slt_test_result.test_result_info[i].selftest_ready != 1){
            ret = false;
            break;
        }
    }
    return ret;
}

static int slt_mainhandle(void* arg)
{
    int i;
    uint32_t domain_id;

    slt_msg_node_t msg_node;
    slt_app_context_t* pcontext;

    struct slt_init_info* init_info;
    struct slt_config_info* config_info;
    struct slt_test_case_result* test_result_info;
    slt_ipc_msg_t msg;

    pcontext = (slt_app_context_t*)arg;

    while (1) {

        memset(&msg_node, 0, sizeof(slt_msg_node_t));

        slt_getmsgoutqueue(pcontext, &msg_node);

        if (msg_node.is_node_empty) {

            LTRACEF("mainhandle msg_type:%d\n", msg_node.msg_item.msg_type);

            //must free msg_value_buff
            switch (msg_node.msg_item.msg_type) {
                case SLT_MESSAGE_FROM_PC_INIT:
                    //check all domain init state

                    pcontext->is_test_ready = 1;

                    for(i = 0; i < SLT_DOMAIN_TYPE_END; i++){
                        if(pcontext->domain_test_enable[i] == 0){
                            LTRACEF("domain is disable, master_id=%d\n", i);
                            continue;
                        }
                        if(pcontext->slt_test_result.test_result_info[i].selftest_ready != 1){
                            LTRACEF("test is not ready, master_id=%d,selftest_ready=%d\n", (i),
                                pcontext->slt_test_result.test_result_info[i].selftest_ready);
                            if(i == SLT_MAIN_MASTER_DOMAIN_ID){
                                //master is not ok, need to do

                            }else
                            {
                                msg.cmd_type = SLT_MESSAGE_FROM_MASTER_INIT;
                                msg.msg_len = 0;
                                slt_ipc_send_msg(pcontext, &msg, pcontext->ipc_chan_slave[i]);
                            }
                            pcontext->is_test_ready = 0;
                        }else{

                        }
                    }

                    // return to pc, msg is init state
                    if (pcontext->is_test_ready == 1) {

                        init_info = slt_alloc_msg_value_buff(sizeof(struct slt_init_info));
                        init_info->selftesty_ready = 1;
                        init_info->software_version = pcontext->software_version;

                        //send msg to pc
                        slt_uart_send_msg(pcontext, init_info);
                        slt_free_msg_value_buff(init_info);
                    }

                    break;

                case SLT_MESSAGE_FROM_PC_CONFIG:
                    if (msg_node.msg_item.msg_len != 0) {
                        config_info = (struct slt_config_info*)msg_node.msg_item.msg_value;
                        pcontext->slt_test_result.test_cfg_vo = config_info->voltage_ap;
                        pcontext->slt_test_result.test_cfg_frq = config_info->freq_ap;
                        slt_config_init_from_pc(pcontext);
                        slt_free_msg_value_buff(config_info);
                    }

                    break;

                case SLT_MESSAGE_FROM_PC_TEST:

                    //start all case test
                    if (msg_node.msg_item.msg_len == 0) {
                        //recode test time begin
                        pcontext->slt_test_result.all_test_usetime = current_time();

                        for(i = 0; i < SLT_DOMAIN_TYPE_END; i++){
                            if(pcontext->domain_test_enable[i] == 1){
                                if(i == SLT_MAIN_MASTER_DOMAIN_ID){
                                    //start internel all item test
                                    slt_test_module_test_start(pcontext);
                                }else{
                                    //start slave test
                                    //init slave recode index, the first recode is master recode
                                    pcontext->slt_test_result.test_result_info[i].test_index = 0;

                                    //send test cmd to slave1
                                    msg.cmd_type = SLT_MESSAGE_FROM_MASTER_TEST_START;
                                    msg.msg_len = 0;
                                    slt_ipc_send_msg(pcontext, &msg, pcontext->ipc_chan_slave[i]);

                                    //start slave timer
                                    timer_initialize(&(pcontext->slave_test_timer[i]));
                                    timer_set_oneshot(&(pcontext->slave_test_timer[i]), SLT_DOMAIN_TEST_TIMEOUT_DEFAULT_VALUE, slt_slave_test_timer_cb, (void*)arg);
                                    pcontext->slave_test_timer_init[i] = 1;
                                }
                            }
                        }

                    }
                    else {
                        //start item case test

                    }

                    break;
                case SLT_MESSAGE_FROM_PC_ITEM_TEST:
                    if (msg_node.msg_item.msg_len != 0) {
                        slt_test_item_test_start(pcontext, (slt_item_test_t*)msg_node.msg_item.msg_value);
                        slt_free_msg_value_buff(msg_node.msg_item.msg_value);
                    }

                    break;
                case SLT_MESSAGE_FROM_SLAVE_INIT:

                    //return slave1 test num, init state
                    if (msg_node.msg_item.msg_len != 0) {
                        init_info = (struct slt_init_info*)msg_node.msg_item.msg_value;

                        domain_id = init_info->domain_id;

                        if((domain_id < SLT_DOMAIN_TYPE_END)&&(pcontext->domain_test_enable[domain_id] == 1)){
                            pcontext->slt_test_result.test_result_info[domain_id].selftest_ready = init_info->selftesty_ready;
                            pcontext->slt_test_result.test_result_info[domain_id].test_num = init_info->test_num;
                        }
                        slt_free_msg_value_buff(init_info);
                    }

                    if (slt_all_domain_init_done())
                    {
                        notify_host_ready();
                    }

                    break;

                case SLT_MESSAGE_FROM_SLAVE_ITEM_TEST_END:

                    if (msg_node.msg_item.msg_len != 0) {

                        test_result_info = (struct slt_test_case_result*)msg_node.msg_item.msg_value;
                        slt_test_recode_test_info(pcontext, test_result_info);
                        slt_free_msg_value_buff(test_result_info);
                    }

                    break;

                case SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END:

                    //check all domain test state
                    pcontext->slt_test_result.test_end = 1;

                    for(i = 0; i < SLT_DOMAIN_TYPE_END; i++){
                        if(pcontext->domain_test_enable[i] == 0){
                            LTRACEF("check test result domain is disable, master_id=%d\n", i);
                            continue;
                        }
                        if(pcontext->slt_test_result.test_result_info[i].test_end == 0){
                            LTRACEF("domain %d test end=%d\n", i, pcontext->slt_test_result.test_result_info[i].test_end);
                            pcontext->slt_test_result.test_end = 0;
                        }else{
                            //recode test result
                            pcontext->slt_test_result.test_result |= pcontext->slt_test_result.test_result_info[i].test_result;
                            LTRACEF("domain %d test end=%d\n", i, pcontext->slt_test_result.test_result_info[i].test_end);
                        }
                    }


                    if (pcontext->slt_test_result.test_end == 1) {
                                            //recode test time end
                        pcontext->slt_test_result.all_test_usetime = (current_time() - pcontext->slt_test_result.all_test_usetime);
                        //out print test result
                        slt_uart_send_msg(pcontext, NULL);
                        notify_host_test_result(pcontext);
                    }else{
                        LTRACEF("all test not end\n");
                    }

                    break;

                case SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_END:
                    LTRACEF("receive test end\n");

                    if (msg_node.msg_item.msg_len != 0) {
                        domain_id = *((uint32_t*)(msg_node.msg_item.msg_value));

                        if (domain_id < SLT_DOMAIN_TYPE_END) {
                            if (pcontext->slave_test_timer_init[domain_id] == 1) {
                                timer_cancel(&(pcontext->slave_test_timer[domain_id]));
                                pcontext->slave_test_timer_init[domain_id] = 0;
                            }

                        }else {

                        }
                        slt_free_msg_value_buff(msg_node.msg_item.msg_value);
                        slt_send_msg(pcontext, SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END, 0, NULL);
                    }

                    break;

                default:
                    break;
            }

        }
        else {
            pcontext->is_sleep = 1;
            event_wait(&(pcontext->msg_handle_signal));
        }
    }

    return 1;
}

slt_app_context_t* slt_get_gcontext(void)
{
    return g_context;
}

static void slt_main(const struct app_descriptor* app, void* args)
{
    int i;
    thread_t* t_mainhandle;
    slt_app_context_t* pcontext;
    slt_ipc_msg_t msg;
    uint32_t default_domain_test_enable[SLT_DOMAIN_TYPE_END] = {SLT_MAIN_SAFETY_MODULE_TEST_ENABLE,
                                                                SLT_MAIN_SEC_MODULE_TEST_ENABLE,
                                                                SLT_MAIN_MP_MODULE_TEST_ENABLE,
                                                                SLT_MAIN_AP1_MODULE_TEST_ENABLE,
                                                                SLT_MAIN_AP2_MODULE_TEST_ENABLE};//safety 1/sec 1/mp 0/ap1 1/ap2 0

    g_context = slt_alloc_msg_value_buff(sizeof(slt_app_context_t));

    if (g_context != NULL) {
        pcontext = g_context;
    }
    else {
        //init fail
        return;
    }

    thread_sleep(1000); //sleep 3 s for ap init

    //init app context
    pcontext->software_version = SLT_MAIN_SOFTWARE_VERSION;

    pcontext->domain_id = SLT_MAIN_MASTER_DOMAIN_ID;

    //init msg handle
    slt_msgqueueinit(pcontext);

    //creat msg handle thread
    t_mainhandle = thread_create("slt_mainhandle", &slt_mainhandle, (void*)(pcontext), DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    //start get test cmd
    thread_resume(t_mainhandle);

    //init plat config, send plat config msg
    slt_config_get_chipid((uint32_t *)&(pcontext->slt_test_result.chip_id));

    //init default salve enable flag
    memcpy(pcontext->domain_test_enable,default_domain_test_enable,(sizeof(uint32_t)*SLT_DOMAIN_TYPE_END));

    //init ipc
    slt_ipc_init(pcontext);

    for(i = 0; i<SLT_DOMAIN_TYPE_END; i++){
        if((pcontext->domain_test_enable[i] == 1)&&(i != SLT_MAIN_MASTER_DOMAIN_ID)){
            //get test init ready state from sec ivi domain
            //send msg to get slave1 test init info
            msg.cmd_type = SLT_MESSAGE_FROM_MASTER_INIT;
            msg.msg_len = 0;
            slt_ipc_send_msg(pcontext, &msg, pcontext->ipc_chan_slave[i]);
            //send msg to get slave2 test init info
        }
    }
    //start module test
    slt_test_init(pcontext);

    //start domain test
    //get test info
    //end
}

static int slt_app_main(void* arg){
    slt_main(NULL, arg);
    return 0;
}

void slt_main_entry(uint level)
{
    thread_t* slt_app_handle;
    //creat app thread
    slt_app_handle = thread_create("slt_app_main", &slt_app_main, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    //start app
    thread_resume(slt_app_handle);
}

#if UART_BAUD_9600
//UART_BAUD 115200->9600 slt app run after ap2 slt init, ipc can not setup, change slt app run before target.
LK_INIT_HOOK(slt_main, slt_main_entry, LK_INIT_LEVEL_TARGET-1);

APP_START(slt_main)
.flags = 0,
APP_END
#else
APP_START(slt_main)
.flags = 0,
.entry = slt_main,
APP_END
#endif
