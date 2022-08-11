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
#include <slt_message.h>

#define LOCAL_TRACE 0
/*
typedef enum {
    IPCC_RRPOC_SAF  = 0,
    IPCC_RRPOC_SEC  = 1,
    IPCC_RRPOC_MP   = 2,
    IPCC_RRPOC_AP1  = 3,
    IPCC_RRPOC_AP2  = 4,
    IPCC_RRPOC_VDSP = 5,
    IPCC_RRPOC_MAX  = IPCC_RRPOC_VDSP,
} hal_mb_proc_t;
*/

#define SLT_TEST_EPT_PAYSZ               (4028)

#define SLT_TEST_IPC_RECEIVE_TIMEOUT     INFINITE_TIME //1000
#define SLT_TEST_IPC_SEND_TIMEOUT        100

//------uart send-------------
int slt_uart_send_msg(slt_app_context_t* pcontext, void* msg)
{
    uint32_t i;
    struct slt_init_info* init_info;

    if (msg != NULL) {
        init_info = (struct slt_init_info*)msg;
        printf("software_version:          %d\n", init_info->software_version);
        printf("is_test_ready:             %d\n", init_info->selftesty_ready);
        return 1;
    }

    printf("chip_id:          %d\n", pcontext->slt_test_result.chip_id);
    printf("test_result:      %s\n", (pcontext->slt_test_result.test_result) ? "fail" : "pass");
    printf("test_num:         all:%d,pass:%d,fail:%d\n", pcontext->slt_test_result.all_test_num,
           pcontext->slt_test_result.all_test_pass_num,
           pcontext->slt_test_result.all_test_fail_num);
    printf("use_time:         %d ms\n", pcontext->slt_test_result.all_test_usetime);
    printf("test_cfg:         voltage:%d\n", pcontext->slt_test_result.test_cfg_vo);
    printf("test_cfg:         freq:%d\n", pcontext->slt_test_result.test_cfg_frq);
    printf("software_version: %d\n", pcontext->software_version);
    printf("test_info:\n");
    printf("                  domain: test_num:%d result:%s\n", pcontext->slt_test_result.test_result_info.test_num,
           (pcontext->slt_test_result.test_result_info.test_result) ? "fail" : "pass");

    for (i = 0; i < pcontext->slt_test_result.test_result_info.test_num; i++) {
        printf("                  case%d level:%d name:%s result:%s result_code:%d use_time:%d ms\n", pcontext->slt_test_result.test_result_info.test_case_info[i].case_id,
               pcontext->slt_test_result.test_result_info.test_case_info[i].level,
               pcontext->slt_test_result.test_result_info.test_case_info[i].test_name,
               (pcontext->slt_test_result.test_result_info.test_case_info[i].result_value) ? "fail" : "pass",
               pcontext->slt_test_result.test_result_info.test_case_info[i].result_value,
               pcontext->slt_test_result.test_result_info.test_case_info[i].use_time);

        if (pcontext->slt_test_result.test_result_info.test_case_info[i].result_value != 0) {
            printf("                         fail reason:%s\n", pcontext->slt_test_result.test_result_info.test_case_info[i].result_string);
        }
    }

    return i;
}
//------uart send-------------

//------uart receive-------------

//get init info
int slt_uart_receive_msg_init(int argc, const cmd_args* argv)
{
    int ret;
    slt_app_context_t* pcontext;
    slt_msg_node_t msg_node;

    //init msg_node
    msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_PC_INIT;
    msg_node.msg_item.msg_len = 0;
    msg_node.msg_item.msg_value = NULL;

    //send cmd msg
    pcontext = slt_get_gcontext();
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

int slt_uart_receive_msg_config(int argc, const cmd_args* argv)
{
    int ret;
    int config_info_size;
    slt_app_context_t* pcontext;
    slt_msg_node_t msg_node;

    struct slt_config_info* config_info;

    config_info_size = sizeof(struct slt_config_info);

    //init config struct
    config_info = slt_alloc_msg_value_buff(config_info_size);

    config_info->voltage_ap = argv[1].u;
    config_info->freq_ap = argv[2].u;

    //init msg_node
    msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_PC_INIT;
    msg_node.msg_item.msg_len = config_info_size;
    msg_node.msg_item.msg_value = config_info;

    //send cmd msg
    pcontext = slt_get_gcontext();
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

int slt_uart_receive_msg_start_test(int argc, const cmd_args* argv)
{
    int ret;
    slt_app_context_t* pcontext;
    slt_msg_node_t msg_node;

    //init msg_node
    msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_PC_TEST;
    msg_node.msg_item.msg_len = 0;
    msg_node.msg_item.msg_value = NULL;

    //send cmd msg
    pcontext = slt_get_gcontext();
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

int slt_uart_receive_test_cmd(int argc, const cmd_args* argv)
{

    if (argc < 2) {
        return 0;
    }

    if (!strcmp(argv[1].str, "init")) {
        slt_uart_receive_msg_init(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "set_config")) {
        // slt_set_config(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "start_test")) {
        slt_uart_receive_msg_start_test(argc - 1, &argv[1]);
    }
    else {
        printf("error cmd\n");
    }

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("slt", "slt test cmd", (console_cmd)&slt_uart_receive_test_cmd)
STATIC_COMMAND_END(slt_uart_receive_test_cmd);

//------uart receive-------------

//------ipc send-------------
//hal_mb_proc_t remout_id
//chan_id == SLT_TEST_EPT_SAF/SEC/AP1
int slt_ipc_send_msg(slt_app_context_t* pcontext, slt_ipc_msg_t* msg, struct ipcc_channel* chan)
{

    int ret = 0;
    char send_buf[IPCC_MB_MTU] = {0,};
    int len = SLT_TEST_EPT_PAYSZ;

    if (chan == NULL) {
        printf("error chan: NULL\n");
        return ret;
    }

    len = sizeof(slt_ipc_msg_t);

    memcpy(send_buf, msg, len);

    ret = ipcc_channel_sendto(chan, chan->addr, send_buf, len, SLT_TEST_IPC_SEND_TIMEOUT);

    if (ret < 0) {
        printf("ping: send failed %d\n", ret);
    }

    return ret;
}
//------ipc send-------------

//------ipc receive-------------

static int slt_ipc_receive_handle(void* arg)
{
    slt_msg_node_t msg_node;
    slt_app_context_t* pcontext;
    slt_ipc_msg_t* ipc_msg;
    void* msg;
    struct ipcc_channel* chan;
    unsigned long src;
    int ret = 0;
    char rx_buf[IPCC_MB_MTU] = {0,};
    int len = SLT_TEST_EPT_PAYSZ;

    pcontext = (slt_app_context_t*)arg;

    chan = pcontext->ipc_chan;

    while (1) {

        len = SLT_TEST_EPT_PAYSZ;

        memset(rx_buf, 0, IPCC_MB_MTU);

        ret = ipcc_channel_recvfrom(chan, &src, rx_buf, &len, SLT_TEST_IPC_RECEIVE_TIMEOUT);

        if (ret < 0) {
            LTRACEF("wait for reply timeout ret=%d\n", ret);
        }
        else {

            ipc_msg = (slt_ipc_msg_t*)&rx_buf;
            msg = slt_alloc_msg_value_buff(ipc_msg->msg_len);
            memcpy(msg, ipc_msg->msg, ipc_msg->msg_len);

            //init msg_node
            msg_node.msg_item.msg_type = ipc_msg->cmd_type;
            msg_node.msg_item.msg_len = ipc_msg->msg_len;
            msg_node.msg_item.msg_value = msg;

            ret = slt_putmsginqueue(pcontext, &msg_node);
        }
    }

    ipcc_channel_stop(chan);
    ipcc_channel_destroy(chan);
    return 1;
}

int slt_ipc_init(slt_app_context_t* pcontext)
{

    thread_t* t_ipchandle;

    //start saf chan
    pcontext->ipc_dev = ipcc_device_gethandle(SLT_MAIN_MASTER_IPC_DEV_RRPOC, SLT_TEST_IPC_RECEIVE_TIMEOUT);
    pcontext->ipc_chan = ipcc_channel_create(pcontext->ipc_dev, SLT_MAIN_TEST_EPT, SLT_MAIN_TEST_EPT_NAME, false);

    if (!(pcontext->ipc_chan)) {
        LTRACEF("create channel fail\n");
        return -1;
    }

    ipcc_channel_start(pcontext->ipc_chan, NULL);

    //creat msg handle thread
    t_ipchandle = thread_create("slt_mainhandle", &slt_ipc_receive_handle, (void*)(pcontext), DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    //start get test cmd
    thread_resume(t_ipchandle);
    return 0;
}



//------ipc receive-------------


