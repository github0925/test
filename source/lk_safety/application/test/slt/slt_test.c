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
#include <slt_test.h>

#define LOCAL_TRACE 0

int slt_test_module_test_start(slt_app_context_t* pcontext)
{
    slt_test_module_info_t* module_info;
    module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    event_signal(&(module_info->start_test_signal), false);
    return 1;
}

int slt_test_item_test_start(slt_app_context_t* pcontext, slt_item_test_t* item_test)
{
    int ret = 0;
    slt_test_module_info_t* module_info;
    slt_ipc_msg_t msg;
    uint32_t domain_id;


    if ((pcontext == NULL) || (item_test == NULL)) {
        return ret;
    }

    module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    if(module_info->domain_id == item_test->domain_id){
        //start item test in this domain
    }else{

        domain_id = item_test->domain_id;

        if((domain_id < SLT_DOMAIN_TYPE_END)&&(pcontext->domain_test_enable[item_test->domain_id] == 1)){
            //send test cmd to slave
            msg.cmd_type = SLT_MESSAGE_FROM_MASTER_ITEM_TEST_START;
            msg.msg_len = sizeof(slt_item_test_t);
            memcpy(msg.msg, item_test, msg.msg_len);

            slt_ipc_send_msg(pcontext, &msg, pcontext->ipc_chan_slave[domain_id]);
            ret = 1;
        }else{
            return ret;
        }


    }
    return ret;
}

int slt_test_recode_test_info(slt_app_context_t* pcontext, slt_test_case_result_t* result_info)
{
    int ret = 0;
    uint32_t i;
    uint32_t find_index;
    uint32_t is_find;
    slt_msg_node_t msg_node;
    slt_test_domain_result_t* domain_result_temp;

    if ((pcontext == NULL) || (result_info == NULL)) {
        return ret;
    }

    if (result_info->domain_id < SLT_DOMAIN_TYPE_END) {
        domain_result_temp = (slt_test_domain_result_t*)(&(pcontext->slt_test_result.test_result_info[result_info->domain_id]));
      }
    else {
        return ret;
    }

    is_find = 0;

    //find case id recode item
    for (i = 0; i < SLT_MAIN_TEST_CASE_MAX_IN_DOMAIN; i++) {
        if (domain_result_temp->test_case_info[i].case_id == result_info->case_id) {
            find_index = i;
            is_find = 1;
            break;
        }
    }

    if (is_find == 1) {
        //update pass fail num
        if (result_info->result_value != domain_result_temp->test_case_info[find_index].result_value) {
            if (result_info->result_value == 0) {
                domain_result_temp->test_pass_num++;
                pcontext->slt_test_result.all_test_pass_num++;
                domain_result_temp->test_fail_num--;
                pcontext->slt_test_result.all_test_fail_num--;
            }
            else {
                domain_result_temp->test_pass_num--;
                pcontext->slt_test_result.all_test_pass_num--;
                domain_result_temp->test_fail_num++;
                pcontext->slt_test_result.all_test_fail_num++;
            }
        }

        memcpy(&(domain_result_temp->test_case_info[find_index]), result_info, sizeof(slt_test_case_result_t));
        ret = 1;
    }
    else {
        is_find = 0;

        //find empty recode item
        for (i = 0; i < SLT_MAIN_TEST_CASE_MAX_IN_DOMAIN; i++) {
            if (domain_result_temp->test_case_info[i].case_id == 0) {
                find_index = i;
                is_find = 1;
                break;
            }
        }

        if (is_find == 1) {
            //update pass fail and all num
            if (result_info->result_value == 0) {
                domain_result_temp->test_pass_num++;
                pcontext->slt_test_result.all_test_pass_num++;
            }
            else {
                domain_result_temp->test_fail_num++;
                pcontext->slt_test_result.all_test_fail_num++;
            }

            pcontext->slt_test_result.all_test_num++;

            (domain_result_temp->test_index)++;

            memcpy(&(domain_result_temp->test_case_info[find_index]), result_info, sizeof(slt_test_case_result_t));

            ret = 1;
        }
    }

    //if recode is update
    if (ret == 1) {
        //if all test item end
        if (domain_result_temp->test_index == domain_result_temp->test_num) {

            if (domain_result_temp->test_fail_num > 0) {
                domain_result_temp->test_result = 1;
            }
            else {
                domain_result_temp->test_result = 0;
            }

            domain_result_temp->test_end = 1;

            msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_INTERNEL_MODULE_TEST_END;
            msg_node.msg_item.msg_len = sizeof(uint32_t);
            msg_node.msg_item.msg_value = slt_alloc_msg_value_buff(msg_node.msg_item.msg_len);
            *((uint32_t*)(msg_node.msg_item.msg_value)) = result_info->domain_id;

            ret = slt_putmsginqueue(pcontext, &msg_node);

        }
    }

    return ret;
}

int slt_test_end_cb(void* item_info, int result_value)
{
    slt_test_module_item_info_t* item_info_temp;
    slt_test_module_info_t* module_info_temp;
    slt_test_case_result_t* test_result_info;

    item_info_temp = (slt_test_module_item_info_t*)item_info;

    item_info_temp->use_time = (current_time() - item_info_temp->use_time); //ms change to s

    module_info_temp = (slt_test_module_info_t*)(item_info_temp->parent_ptr);

    //init test result info
    test_result_info = slt_alloc_msg_value_buff(sizeof(slt_test_case_result_t));

    test_result_info->domain_id = module_info_temp->domain_id;
    test_result_info->case_id = item_info_temp->id;
    test_result_info->level = item_info_temp->level;
    test_result_info->result_value = item_info_temp->test_result;
    test_result_info->use_time = item_info_temp->use_time;

    strncpy(test_result_info->test_name, item_info_temp->name,
            MIN(SLT_MAIN_TEST_RESULT_NAME_LEN_MAX, strlen(item_info_temp->name)));

    if (test_result_info->result_value != 0) {

        strncpy(test_result_info->result_string, item_info_temp->result_string,
                MIN(SLT_MAIN_TEST_RESULT_STRING_LEN_MAX, strlen(item_info_temp->result_string)));
    }

    //init test result info end

    //recode test result
    slt_test_recode_test_info((slt_app_context_t*)module_info_temp->parent_ptr, test_result_info);

    if (item_info_temp->parallel != 1) {
        event_signal(&(item_info_temp->wait_signal), false);
    }

    slt_free_msg_value_buff(test_result_info);
    //
    //event_signal(&(module_info_temp->test_end_signal), false);

    return 0;
}

static int slt_test_main_handle(void* arg)
{
    slt_app_context_t* pcontext;
    slt_test_module_info_t* module_info;
    uint32_t i;
    uint32_t j;
    uint32_t k;
    uint32_t level_temp;
    uint32_t item_temp;
    uint32_t last_test_level;
    uint32_t is_found;
    int ret;

    pcontext = (slt_app_context_t*)arg;
    module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    while (1) {

        event_wait(&(module_info->start_test_signal));

        if (module_info->is_init == 0) {
            continue;
        }

        item_temp = 0;
        last_test_level = 0;
        //recode already test num
        module_info->test_index = 0;
        //sort test item

        for (i = 0; i < module_info->test_num; i++) {
            //get level smallest item
            level_temp = 0xffffffff;
            is_found = 0;

            for (j = 0; j < module_info->test_num; j++) {

                if ((level_temp > module_info->item_info[j].level) && (last_test_level < module_info->item_info[j].level)) {
                    level_temp = module_info->item_info[j].level;
                    item_temp = j;
                    is_found = 1;
                }
            }

            last_test_level = level_temp;

            if (is_found == 0) {
                continue;
            }

            //get test start time
            module_info->item_info[item_temp].use_time = current_time();

            ret = slt_run_module_test(&(module_info->item_info[item_temp]));

            //recode test result
            //pcontext->slt_test_result.master_result_info.test_case_info[recode_index].case_id = module_info->item_info[item_temp].id;
            //recode_index++;

            if ((module_info->item_info[item_temp].parallel != 1) && (module_info->item_info[item_temp].invalid == 0)) {
                event_wait(&(module_info->item_info[item_temp].wait_signal));
            }

            //get left same level test case item
            for (k = (item_temp + 1); k < module_info->test_num; k++) {

                if (last_test_level == module_info->item_info[k].level) {

                    item_temp = k;

                    //get test start time
                    module_info->item_info[item_temp].use_time = current_time();

                    ret = slt_run_module_test(&(module_info->item_info[item_temp]));

                    //recode test result
                    if ((module_info->item_info[item_temp].parallel != 1) && (module_info->item_info[item_temp].invalid == 0)) {
                        event_wait(&(module_info->item_info[item_temp].wait_signal));
                    }

                }

            }

        }

        LTRACEF("test end\n");
    }

    return ret;
}

int slt_test_init(slt_app_context_t* pcontext)
{
    const struct slt_module_test_struct* module = NULL;
    slt_test_module_info_t* g_slt_test_module_info;

    uint32_t i;
    uint32_t test_id;
    uint32_t module_num;
    thread_t* t_testhandle;

    g_slt_test_module_info = slt_alloc_msg_value_buff(sizeof(slt_test_module_info_t));

    if (g_slt_test_module_info != NULL) {
        pcontext->test_module_info = (void*)g_slt_test_module_info;
    }
    else {
        return 0;
    }

    g_slt_test_module_info->parent_ptr = pcontext;

    test_id = 1;

    //get register module info
    module_num = MIN(SLT_MODULE_TEST_MODULE_NUM_MAX, slt_get_module_num());

    for (i = 0; i < module_num; i++) {

        module = slt_get_module_byindex(i);

        if (module != NULL) {
            g_slt_test_module_info->item_info[i].id = test_id;
            g_slt_test_module_info->item_info[i].level = module->level;
            g_slt_test_module_info->item_info[i].times = module->times;
            g_slt_test_module_info->item_info[i].timeout = module->timeout;
            g_slt_test_module_info->item_info[i].name = module->name;
            g_slt_test_module_info->item_info[i].module_item = module;
            g_slt_test_module_info->item_info[i].parallel = module->parallel;
            g_slt_test_module_info->item_info[i].end_hook = slt_test_end_cb;
            g_slt_test_module_info->item_info[i].result_string = slt_alloc_msg_value_buff(SLT_MODULE_TEST_RESULT_STRING_MAX_LEN);
            g_slt_test_module_info->item_info[i].parent_ptr = pcontext->test_module_info;

            if (g_slt_test_module_info->item_info[i].parallel != 1) {
                event_init(&(g_slt_test_module_info->item_info[i].wait_signal), false, EVENT_FLAG_AUTOUNSIGNAL);
            }

            test_id++;
        }
    }

    g_slt_test_module_info->test_num = test_id - 1; // test id is start from 1
    g_slt_test_module_info->domain_id = pcontext->domain_id;
    LTRACEF("g_slt_test_module_info->total_num =%d\n", test_id);

    event_init(&(g_slt_test_module_info->start_test_signal), false, EVENT_FLAG_AUTOUNSIGNAL);

    //creat msg handle thread
    t_testhandle = thread_create("slt_testhandle", &slt_test_main_handle, (void*)(pcontext), DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    //start get test cmd
    thread_resume(t_testhandle);

    //init test result info
    if (pcontext->domain_id < SLT_DOMAIN_TYPE_END) {
        pcontext->slt_test_result.test_result_info[pcontext->domain_id].domain_id = pcontext->domain_id;
        pcontext->slt_test_result.test_result_info[pcontext->domain_id].test_num = g_slt_test_module_info->test_num;
        pcontext->slt_test_result.test_result_info[pcontext->domain_id].selftest_ready = 1;
    }
    g_slt_test_module_info->is_init = 1;

    return 1;
}

