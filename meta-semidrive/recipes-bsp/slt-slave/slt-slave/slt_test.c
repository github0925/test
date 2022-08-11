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
#include "slt_test.h"

#define LOCAL_TRACE 1
#define SLT_LIB_PATH_DEFAULT "/lib/slt/"

typedef int (*slt_item_test_entry)(uint times, uint timeout, char* error_str);

int slt_test_start_lib_common(slt_test_module_item_info_t* item_info)
{
    int ret;

    void* handle;

    slt_item_test_entry  entry_func = NULL;

    printf("dlopen slt module lib");
    handle = dlopen(item_info->lib_path, RTLD_LAZY);

    if (!handle) {
        ret = -1;
        printf("lib dlopen failed! %s, exist test!\n", dlerror());
    }

    printf("do slt_test");
    *(void**)(&entry_func) = dlsym(handle, "slt_start");

    if (entry_func) {
        ret = (*entry_func)(item_info->times, item_info->timeout, item_info->result_string);
    }
    else {
        ret = -1;
        printf("could not find symbol , %d IN\n");
    }

    return ret;
}

int slt_test_module_test_start(slt_app_context_t* pcontext)
{
    slt_test_module_info_t* module_info;
    module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    pthread_mutex_lock(&(module_info->start_test_mutex));
    pthread_cond_signal(&(module_info->start_test_cond));
    pthread_mutex_unlock(&(module_info->start_test_mutex));
    return 1;
}

int slt_test_item_byid_test_start(slt_app_context_t* pcontext, slt_item_test_t* item_test)
{
    int ret = 0;
    slt_test_module_info_t* module_info;
    //slt_ipc_msg_t msg;
    //uint32_t chan_id;

    if ((pcontext == NULL) || (item_test == NULL)) {
        return ret;
    }

    module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    if (module_info->domain_id == item_test->domain_id) {
        //start item test in this domain
        ret = 1;
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
        domain_result_temp = (slt_test_domain_result_t*)(&(pcontext->slt_test_result.test_result_info));
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


static int slt_send_ipc_test_result_msg(slt_app_context_t* pcontext, void* msg_value, uint32_t msg_len)
{
    int ret;
    slt_ipc_msg_t msg;

    //init msg_node
    msg.cmd_type = SLT_MESSAGE_FROM_SLAVE_ITEM_TEST_END;

    msg.msg_len = msg_len;

    memcpy(msg.msg, msg_value, msg_len);

    ret = slt_ipc_send_msg(pcontext, &msg);

    return ret;
}

int slt_test_end_cb(void* item_info, int result_value)
{
    slt_test_module_item_info_t* item_info_temp;
    slt_test_module_info_t* module_info_temp;
    slt_test_case_result_t* test_result_info;
    struct timeval time_recode;

    item_info_temp = (slt_test_module_item_info_t*)item_info;

    gettimeofday(&time_recode, NULL);

    item_info_temp->use_time = (time_recode.tv_usec - item_info_temp->use_time); //ms change to s

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

    slt_send_ipc_test_result_msg((slt_app_context_t*)(module_info_temp->parent_ptr), (void*)test_result_info, sizeof(slt_test_case_result_t));

    if (item_info_temp->parallel != 1) {
        pthread_mutex_lock(&(item_info_temp->test_item_mutex));
        pthread_cond_signal(&(item_info_temp->test_item_cond));
        pthread_mutex_unlock(&(item_info_temp->test_item_mutex));
    }

    slt_free_msg_value_buff(test_result_info);
    //
    //event_signal(&(module_info_temp->test_end_signal), false);

    return 0;
}

static void slt_test_item_test_timer_cb(sigval_t arg)
{
    slt_test_module_item_info_t* temp = (slt_test_module_item_info_t*)(arg.sival_ptr);

    timer_delete(temp->module_test_timer);
    printf("timeout %s\n", temp->name);

    temp->test_result = 0xff;
    strcpy(temp->result_string, "fail cause timeout");

    temp->end_hook(temp, 0xff);
//disable run this case
    temp->invalid = 1;

    pthread_kill(temp->t, SIGUSR1);

    return;
}

void* slt_test_item_test_thread_entry(void* arg)
{
    slt_test_module_item_info_t* temp = (slt_test_module_item_info_t*)arg;

    struct sigevent se;
    struct itimerspec its;

    //start a timer,register a timer callback

    memset(&se, 0, sizeof(struct sigevent));
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = (void*)arg;
    se.sigev_notify_function = slt_test_item_test_timer_cb;
    se.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &se, &(temp->module_test_timer)) < 0) {
        printf("Can not create timer!");
    }

    its.it_value.tv_sec = 60 * 10; //10 min
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(temp->module_test_timer, 0, &its, NULL)) {
        printf("timer_settime");
    }

    temp->test_result = slt_test_start_lib_common(temp);

    timer_delete(temp->module_test_timer);

    temp->end_hook(temp, temp->test_result);

    //pthread_cond_destroy(&(temp->test_item_cond));
    //pthread_mutex_destroy(&(temp->test_item_mutex));

    return NULL;
}

static int slt_test_item_test_start(slt_test_module_item_info_t* module)
{
    int ret = 0;

    if (module->invalid == 1) {
        module->test_result = 0xfc;
        strcpy(module->result_string, "fail cause test case invalid");

        module->end_hook(module, 0xfc);
        return ret;
    }

    printf("starting slt_module_test %s\n", module->name);

    //pthread_mutex_init(&(module->test_item_mutex), NULL);
    //pthread_cond_init(&(module->test_item_cond), NULL);

    //creat msg handle thread
    ret = pthread_create(&(module->t), NULL, slt_test_item_test_thread_entry, (void*)(module));

    if (ret != 0) {
        printf("Ctreate Thread ERROR %d\n", ret);
        module->test_result = 0xfe;
        strcpy(module->result_string, "fail cause creat thread error");

        module->end_hook(module, 0xfe);
        //disable run this case
        module->invalid = 1;
        //pthread_cond_destroy(&(module->test_item_cond));
        //pthread_mutex_destroy(&(module->test_item_mutex));
    }

    return ret;
}

void* slt_test_main_handle(void* arg)
{
    slt_app_context_t* pcontext;
    slt_test_module_info_t* module_info;
    struct timeval time_recode;
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

        pthread_mutex_lock(&(module_info->start_test_mutex));
        pthread_cond_wait(&(module_info->start_test_cond), &(module_info->start_test_mutex));
        pthread_mutex_unlock(&(module_info->start_test_mutex));

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
            gettimeofday(&time_recode, NULL);
            module_info->item_info[item_temp].use_time = time_recode.tv_usec;

            ret = slt_test_item_test_start(&(module_info->item_info[item_temp]));

            //recode test result
            //pcontext->slt_test_result.master_result_info.test_case_info[recode_index].case_id = module_info->item_info[item_temp].id;
            //recode_index++;

            if ((module_info->item_info[item_temp].parallel != 1) && (module_info->item_info[item_temp].invalid == 0)) {
                pthread_mutex_lock(&(module_info->item_info[item_temp].test_item_mutex));
                pthread_cond_wait(&(module_info->item_info[item_temp].test_item_cond), &(module_info->item_info[item_temp].test_item_mutex));
                pthread_mutex_unlock(&(module_info->item_info[item_temp].test_item_mutex));
            }

            //get left same level test case item
            for (k = (item_temp + 1); k < module_info->test_num; k++) {

                if (last_test_level == module_info->item_info[k].level) {

                    item_temp = k;

                    //get test start time
                    gettimeofday(&time_recode, NULL);
                    module_info->item_info[item_temp].use_time = time_recode.tv_usec;

                    ret = slt_test_item_test_start(&(module_info->item_info[item_temp]));

                    //recode test result
                    if ((module_info->item_info[item_temp].parallel != 1) && (module_info->item_info[item_temp].invalid == 0)) {
                        pthread_mutex_lock(&(module_info->item_info[item_temp].test_item_mutex));
                        pthread_cond_wait(&(module_info->item_info[item_temp].test_item_cond), &(module_info->item_info[item_temp].test_item_mutex));
                        pthread_mutex_unlock(&(module_info->item_info[item_temp].test_item_mutex));
                    }

                }

            }

        }

        printf("test end\n");
    }

    pthread_cond_destroy(&(module_info->start_test_cond));
    pthread_mutex_destroy(&(module_info->start_test_mutex));
    return NULL;
}

int slt_test_get_test_num(slt_app_context_t* pcontext)
{
    slt_test_module_info_t* g_slt_test_module_info;
    g_slt_test_module_info = (slt_test_module_info_t*)pcontext->test_module_info;

    return g_slt_test_module_info->test_num;
}

//get all lib/slt/ *.so file
static uint32_t slt_test_init_test_item(slt_test_module_info_t* module_info)
{
    int module_num = 0;
    DIR* dir;
    struct dirent* ptr;
    int item_index;

    if ((dir = opendir(SLT_LIB_PATH_DEFAULT)) == NULL) {
        perror("Open dir error...");
        exit(1);
    }

    item_index = 0;

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) { ///current dir OR parrent dir
            continue;
        }
        else if (ptr->d_type == DT_REG) { ///8 This is a regular file
            if (item_index < SLT_MODULE_TEST_MODULE_NUM_MAX) {
                printf("d_name:%s/%s\n", SLT_LIB_PATH_DEFAULT, ptr->d_name);
                memset(module_info->item_info[item_index].lib_path, '\0', 128);
                strcpy(module_info->item_info[item_index].lib_path, SLT_LIB_PATH_DEFAULT);
                strcat(module_info->item_info[item_index].lib_path, ptr->d_name);
                strcpy(module_info->item_info[item_index].name, ptr->d_name);

                module_info->item_info[item_index].id = item_index + 1;
                module_info->item_info[item_index].level = SLT_MODULE_TEST_LEVEL_SAMPLE_1;
                module_info->item_info[item_index].parent_ptr = module_info;
                module_info->item_info[item_index].end_hook = slt_test_end_cb;

                pthread_mutex_init(&(module_info->item_info[item_index].test_item_mutex), NULL);
                pthread_cond_init(&(module_info->item_info[item_index].test_item_cond), NULL);

                item_index++;
            }
        }
        else {
            continue;
        }
    }

    closedir(dir);

    module_info->test_num = item_index;

    printf("item_index =%d \n", item_index);

    return item_index;
}

int slt_test_init(slt_app_context_t* pcontext)
{
    int ret;
    slt_test_module_info_t* g_slt_test_module_info;

    uint32_t i;
    pthread_t t_testhandle;

    g_slt_test_module_info = slt_alloc_msg_value_buff(sizeof(slt_test_module_info_t));

    if (g_slt_test_module_info != NULL) {
        pcontext->test_module_info = (void*)g_slt_test_module_info;
    }
    else {
        return 0;
    }

    g_slt_test_module_info->parent_ptr = pcontext;

    slt_test_init_test_item(g_slt_test_module_info);

    g_slt_test_module_info->domain_id = pcontext->domain_id;
    printf("g_slt_test_module_info->test_num =%d\n", g_slt_test_module_info->test_num);

    pthread_mutex_init(&(g_slt_test_module_info->start_test_mutex), NULL);
    pthread_cond_init(&(g_slt_test_module_info->start_test_cond), NULL);

    //creat msg handle thread
    ret = pthread_create(&t_testhandle, NULL, slt_test_main_handle, (void*)(pcontext));

    if (ret != 0) {
        printf("Ctreate Thread ERROR %d\n", ret);
        return ret;
    }

    //init test result info

    pcontext->slt_test_result.test_result_info.domain_id = pcontext->domain_id;
    pcontext->slt_test_result.test_result_info.test_num = g_slt_test_module_info->test_num;
    pcontext->slt_test_result.test_result_info.selftest_ready = 1;

    g_slt_test_module_info->is_init = 1;

    return 1;
}

