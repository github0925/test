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
#include "slt_main.h"
#include "slt_test.h"
#include "slt_config.h"
#include "slt_message.h"

#define LOCAL_TRACE 1

#define SLT_MSG_NODE_NUMBER_MAX  5

#define SLT_DOMAIN_TEST_TIMEOUT_DEFAULT_VALUE  5*1000*60 //5 min

slt_app_context_t* g_context;

void* slt_alloc_msg_value_buff(size_t size)
{
    void* p = malloc(size);

    if (!p) {
        return NULL;
    }

    memset(p, 0, size);
    return p;
}

void slt_free_msg_value_buff(void* p)
{
    free(p);
}

//---------msg queue function start --------------------
static int slt_msgqueueinit(slt_app_context_t* pcontext)
{
    int ret = 0;

    pcontext->msg_queue.msg_note = (slt_msg_node_t*)slt_alloc_msg_value_buff(sizeof(slt_msg_node_t) * SLT_MSG_NODE_NUMBER_MAX);
    pcontext->msg_queue.in_pos = 0;
    pcontext->msg_queue.out_pos = 0;
    pcontext->msg_queue.queue_size = SLT_MSG_NODE_NUMBER_MAX;
    pcontext->msg_queue.queue_left_num = SLT_MSG_NODE_NUMBER_MAX;
	if (pthread_spin_init(&(pcontext->msg_queue.lock), PTHREAD_PROCESS_PRIVATE) != 0){
		return ret;
	}

    pcontext->is_msg_init = 1;
    return ret ;
}

int slt_putmsginqueue(slt_app_context_t* pcontext, slt_msg_node_t* msg_node)
{
    int ret;

    if (pcontext->is_msg_init == 0) {
        ret = SLT_MESSAGE_QUE_STATE_NOT_INIT;
        return ret;
    }

    if (pcontext->msg_queue.queue_left_num == 0) { //queue is full
        ret = SLT_MESSAGE_QUE_STATE_FIFO_FULL;
        return ret;
    }

    pthread_spin_lock(&(pcontext->msg_queue.lock));

    msg_node->is_node_empty = 1;

    memcpy((void*)((pcontext->msg_queue.msg_note) + (pcontext->msg_queue.in_pos)), (void*)msg_node, sizeof(slt_msg_node_t));

    pcontext->msg_queue.in_pos++;

    if ((pcontext->msg_queue.in_pos) == (pcontext->msg_queue.queue_size)) {
        pcontext->msg_queue.in_pos = 0;
    }

    pcontext->msg_queue.queue_left_num--;

    pthread_spin_unlock(&(pcontext->msg_queue.lock));

    if (pcontext->is_sleep == 1) {
        pthread_mutex_lock(&(pcontext->msg_handle_mutex));
        pthread_cond_signal(&(pcontext->msg_handle_cond));
        pthread_mutex_unlock(&(pcontext->msg_handle_mutex));
    }

    ret = SLT_MESSAGE_QUE_STATE_OK;

    return ret ;
}

static int slt_getmsgoutqueue(slt_app_context_t* pcontext, slt_msg_node_t* msg_node)
{
    int ret;

    if (pcontext->is_msg_init == 0) {
        ret = SLT_MESSAGE_QUE_STATE_NOT_INIT;
        return ret;
    }

    if ((pcontext->msg_queue.queue_left_num) == (pcontext->msg_queue.queue_size)) { //queue is empty
        ret = SLT_MESSAGE_QUE_STATE_FIFO_EMPTY;
        return ret;
    }

    pthread_spin_lock(&(pcontext->msg_queue.lock));

    memcpy((void*)msg_node, (void*)((pcontext->msg_queue.msg_note) + (pcontext->msg_queue.out_pos)), sizeof(slt_msg_node_t));

    pcontext->msg_queue.out_pos++;

    if (pcontext->msg_queue.out_pos == pcontext->msg_queue.queue_size) {
        pcontext->msg_queue.out_pos = 0;
    }

    pcontext->msg_queue.queue_left_num++;

    pthread_spin_unlock(&(pcontext->msg_queue.lock));

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

static int slt_send_ipc_init_msg(slt_app_context_t* pcontext)
{
    int ret;
    slt_ipc_msg_t msg;
    struct slt_init_info init_info;

    //init msg_node

    msg.cmd_type = SLT_MESSAGE_FROM_SLAVE_INIT;
    init_info.domain_id = pcontext->domain_id;
    init_info.selftesty_ready = 1;
    init_info.test_num = slt_test_get_test_num(pcontext);

    msg.msg_len = sizeof(struct slt_init_info);

    memcpy(msg.msg, &init_info, msg.msg_len);

    ret = slt_ipc_send_msg(pcontext, &msg);

    return ret;
}

void* slt_mainhandle(void* arg)
{
    slt_msg_node_t msg_node;
    slt_app_context_t* pcontext;

    struct slt_init_info* init_info;
    struct slt_config_info* config_info;
    struct timeval time_recode;

    pcontext = (slt_app_context_t*)arg;

    while (1) {

        memset(&msg_node, 0, sizeof(slt_msg_node_t));

        slt_getmsgoutqueue(pcontext, &msg_node);

        if (msg_node.is_node_empty) {

            printf("slt mainhandle msg_type =%d\n", msg_node.msg_item.msg_type);

            //must free msg_value_buff
            switch (msg_node.msg_item.msg_type) {
                case SLT_MESSAGE_FROM_PC_INIT:

                    // return to pc, msg is init state
                    if (pcontext->slt_test_result.test_result_info.selftest_ready == 1) {
                        pcontext->is_test_ready = 1;
                        init_info = slt_alloc_msg_value_buff(sizeof(struct slt_init_info));
                        init_info->selftesty_ready = 1;
                        init_info->software_version = pcontext->software_version;

                        //send msg to pc
                        slt_uart_send_msg(pcontext, init_info);
                        slt_free_msg_value_buff(init_info);
                    }
                    else {
                        printf("domain %d test is not ready, selftest_ready=%d,\n", pcontext->domain_id, pcontext->slt_test_result.test_result_info.selftest_ready);
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
                case SLT_MESSAGE_FROM_MASTER_TEST_START:
                    //recode test time begin
                    gettimeofday(&time_recode, NULL);
                    pcontext->slt_test_result.all_test_usetime = time_recode.tv_usec;
                    slt_test_module_test_start(pcontext);
                    break;

                case SLT_MESSAGE_FROM_MASTER_ITEM_TEST_START:
                    if (msg_node.msg_item.msg_len != 0) {
                        slt_test_item_byid_test_start(pcontext, (slt_item_test_t*)msg_node.msg_item.msg_value);
                        slt_free_msg_value_buff(msg_node.msg_item.msg_value);
                    }

                    break;

                case SLT_MESSAGE_FROM_MASTER_INIT:

                    slt_send_ipc_init_msg(pcontext);

                    break;

                case SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END:

                    if (pcontext->slt_test_result.test_result_info.test_end) {
                        pcontext->slt_test_result.test_result = pcontext->slt_test_result.test_result_info.test_result;

                        //recode test time end
                        gettimeofday(&time_recode, NULL);
                        pcontext->slt_test_result.all_test_usetime = (time_recode.tv_usec - pcontext->slt_test_result.all_test_usetime);
                        //out print test result
                        slt_uart_send_msg(pcontext, NULL);
                    }
                    else {
                        printf("domain test end=%d\n", pcontext->slt_test_result.test_result_info.test_end);
                    }

                    break;

                case SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_END:
                    printf("receive test end\n");
                    slt_send_msg(pcontext, SLT_MESSAGE_FROM_INTERNEL_CHECK_ALL_TEST_END, 0, NULL);
                    break;

                default:
                    break;
            }

        }
        else {
            pcontext->is_sleep = 1;
            pthread_mutex_lock(&(pcontext->msg_handle_mutex));
            pthread_cond_wait(&(pcontext->msg_handle_cond), &(pcontext->msg_handle_mutex));
            pthread_mutex_unlock(&(pcontext->msg_handle_mutex));
        }
    }

    return NULL;
}

slt_app_context_t* slt_get_gcontext(void)
{
    return g_context;
}

int main(int argc, char* argv[])
{
    int ret;
    pthread_t t_mainhandle;
    slt_app_context_t* pcontext;

    g_context = slt_alloc_msg_value_buff(sizeof(slt_app_context_t));

    if (g_context != NULL) {
        pcontext = g_context;
    }
    else {
        //init fail
        return 0;
    }

    //init app context
    pcontext->software_version = 11;

    pcontext->domain_id = SLT_MAIN_TEST_MODULE_DOMAIN_TYPE;

    //init msg handle
    slt_msgqueueinit(pcontext);

    pthread_mutex_init(&(pcontext->msg_handle_mutex), NULL);
    pthread_cond_init(&(pcontext->msg_handle_cond), NULL);

    //creat msg handle thread
    ret = pthread_create(&t_mainhandle, NULL, slt_mainhandle, (void*)(pcontext));

    if (ret != 0) {
        printf("Ctreate Thread ERROR %d\n", ret);
        return 0;
    }

    //init plat config, send plat config msg
    pcontext->slt_test_result.chip_id = slt_config_get_chipid();

    //init ipc
    slt_ipc_init(pcontext);

    //start module test
    slt_test_init(pcontext);

    //send msg to get slave1 test init info
    slt_send_ipc_init_msg(pcontext);

    //start domain test
    //fortest
    //sleep(1);
    //slt_uart_receive_msg_start_test();
    //get test info
    //end
    while (1) {
        sleep(10000);
    }

    return 1;

}

