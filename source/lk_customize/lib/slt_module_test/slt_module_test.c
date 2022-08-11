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

#include <debug.h>
#include <trace.h>

#include <lib/slt_module_test.h>

#define LOCAL_TRACE 0

extern const struct slt_module_test_struct __slt_module_test[];
extern const struct slt_module_test_struct __slt_module_test_end[];

static enum handler_return module_test_timer_cb(struct timer *t, lk_time_t now, void *arg)
{
    slt_test_module_item_info_t *temp = (slt_test_module_item_info_t *)arg;

    if(temp->is_module_test_timer_init == 1){
        timer_cancel(&(temp->module_test_timer));
        temp->is_module_test_timer_init = 0;
    }

    LTRACEF("timeout %s\n", temp->name);

    temp->test_result = 0xff;

    if(((temp->result_string) != NULL) && (strlen(temp->result_string) == 0)){
        strcpy(temp->result_string,"fail cause timeout, case no info return");
    }

    temp->end_hook(temp,0xff);

    temp->invalid = 1;

    thread_detach(temp->t);

    return 0;
}

static int module_test_thread_entry(void *arg)
{
    slt_test_module_item_info_t *temp = (slt_test_module_item_info_t *)arg;

    //start a timer,register a timer callback
    timer_initialize(&(temp->module_test_timer));
    timer_set_oneshot(&(temp->module_test_timer), temp->timeout,module_test_timer_cb, (void *)arg);
    temp->is_module_test_timer_init = 1;

    temp->test_result = temp->module_item->hook(temp->times, temp->timeout, temp->result_string);

    if(temp->is_module_test_timer_init == 1){
        timer_cancel(&(temp->module_test_timer));
        temp->is_module_test_timer_init = 0;
    }

    temp->end_hook(temp,temp->test_result);

    thread_exit(0);

    return 0;
}

static int slt_module_test_start(slt_test_module_item_info_t *module)
{
    int ret = 0;

    uint32_t stack_size = (module->module_item->flags & SLT_FLAG_CUSTOM_STACK_SIZE) ? module->module_item->stack_size : DEFAULT_STACK_SIZE;

    if(module->invalid == 1){
        module->test_result = 0xfc;
        strcpy(module->result_string,"fail cause test case invalid");

        module->end_hook(module,0xfc);
        return ret;
    }

    LTRACEF("starting slt_module_test %s\n", module->name);
    //init test result
    module->test_result = 0;

    module->t = thread_create(module->name, &module_test_thread_entry, (void *)module, DEFAULT_PRIORITY, stack_size);
    if(module->t != NULL){
        thread_detach(module->t);
        thread_resume(module->t);
        ret = 1;
    }else{
        module->test_result = 0xfe;
        strcpy(module->result_string, "fail cause creat thread error");

        module->end_hook(module,0xfe);
    //disable run this case
        module->invalid = 1;
    }

    return ret;
}

const struct slt_module_test_struct* slt_get_module(uint level)
{
    const struct slt_module_test_struct* ptr = NULL;
    const struct slt_module_test_struct* found = NULL;

    for (ptr = __slt_module_test; ptr != __slt_module_test_end; ptr++) {

        LTRACEF("looking at %p (%s) level %#x \n", ptr, ptr->name, ptr->level);

        if (ptr->level == level) {
            found = ptr;
            break;
        }
        else {
            continue;
        }
    }

    return found;
}

uint32_t slt_get_module_num(void)
{
    int module_num = 0;
    const struct slt_module_test_struct* ptr = NULL;

    for (ptr = __slt_module_test; ptr != __slt_module_test_end; ptr++) {
        module_num++;
    }

    LTRACEF("module_num =%d \n", module_num);

    return module_num;
}

const struct slt_module_test_struct* slt_get_module_byindex(uint index)
{
    uint index_temp = 0;
    const struct slt_module_test_struct* ptr = NULL;
    const struct slt_module_test_struct* found = NULL;

    for (ptr = __slt_module_test; ptr != __slt_module_test_end; ptr++) {

        LTRACEF("looking at %p (%s) index %#x \n", ptr, ptr->name, index);

        if (index_temp == index) {
            found = ptr;
            break;
        }
        else {
            index_temp++;
        }
    }

    return found;
}

int slt_run_module_test(slt_test_module_item_info_t* item_info)
{
    int ret = -1;

    if (item_info->module_item == NULL) {
        return ret;
    }

    ret = slt_module_test_start(item_info);

    return ret;
}
