/*
* lk_thread.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implementation of wrapper layer of little kernel(LK)
* of thread prototype.
*/

#include <lk_wrapper.h>

#define lkw_STACK_TYPE_SIZE (sizeof(StackType_t))
#define lkw_STACK_DEPTH(size) (configSTACK_DEPTH_TYPE)(ROUNDUP(size, lkw_STACK_TYPE_SIZE) / lkw_STACK_TYPE_SIZE )

/*********************** thread API redirection *******************/

struct task_dispatcher_t
{
    TaskHandle_t* task_handle;
    thread_start_routine entry;
    void* arg;
};

static void task_dispatch(void* arg)
{
    struct task_dispatcher_t* dispatcher = (struct task_dispatcher_t*) arg;

#if ARM_WITH_VFP==1
    //enable fpu context save/restore
    portTASK_USES_FLOATING_POINT();
#endif

    dispatcher->entry(dispatcher->arg);

    /* Task should not reach here in FreeRTOS programming model in theory,
    But as we are LK now, we could reach here if thread return.
    But we need to recycle the memory we used then delete itself. */

    vPortFree(dispatcher->task_handle);
    vPortFree(dispatcher);
    vTaskDelete(NULL);
}


WRAPPER_FUNCTION thread_t* thread_create(const char *name, thread_start_routine entry, void *arg, int priority, size_t stack_size)
{

    TaskHandle_t* task = pvPortMalloc(sizeof(TaskHandle_t));
    ASSERT(task);

    struct task_dispatcher_t* dispatcher = pvPortMalloc(sizeof(struct task_dispatcher_t));
    ASSERT(dispatcher);

    dispatcher->task_handle = task;
    dispatcher->arg = arg;
    dispatcher->entry = entry;

    /* We used a dispatcher task template to call the wanted task code.
     * In this way we could delete the task by return in default, which was
     * as same as LK programming model.
     */

    /* Pass 0 as priority to make sure the task to be created will not preempt
     * current thread. Setback after suspended. */
    BaseType_t ret =  xTaskCreate((TaskFunction_t)task_dispatch,name, lkw_STACK_DEPTH(stack_size),dispatcher,(UBaseType_t)(0),task);

    if(pdFALSE == ret)
    {
        vPortFree(dispatcher);
        vPortFree(task);
        return NULL;
    }

    /*as lk place new thread in suspend list(not ready),
    * frts place new task in ready list default(waiting for sch),
    * we manually migrate it into suspend list from ready list
    * in FreeRTOS.
    */
    vTaskSuspend(*task);

    /* Set back to wanted priority. */
    vTaskPrioritySet(*task,(UBaseType_t)priority);

    return (thread_t*)task;
}

WRAPPER_FUNCTION void thread_sleep(lk_time_t delay)
{
    ISR_ASSERT();
    RESCHED_ASSERT();

    vTaskDelay(LKMS_TO_FRTS_TICK(delay));
}

WRAPPER_FUNCTION void thread_yield(void)
{

    ISR_ASSERT();

    RESCHED_ASSERT();

    taskYIELD();
}

WRAPPER_FUNCTION status_t thread_resume(thread_t *t)
{
    TaskHandle_t* task = (TaskHandle_t*)t;

    /* No need to care about if scheduler started, cuz
     * this task must be suspended while after created
     * and the pxCurrentTCB will be set back to null while
     * in vTaskSuspend cause scheduler not running and all
     * tasks were moved into suspended list. In this case,
     * resume only move target task into ready list without
     * context switch - cause the context switch happened under
     * the condition if target task has higher priority than
     * pxCurrentTCB which is NULL. Werid concern: No validity check
     * while compare priority between pxTCB and pxCurrentTCB in case of
     * pxCurrentTCB was NULL.
    */

    ISR_ASSERT();

    RESCHED_ASSERT();

    vTaskResume(*task);


    return NO_ERROR;
}

WRAPPER_FUNCTION status_t thread_detach(thread_t *t)
{
    TaskHandle_t* task = (TaskHandle_t*)t;

    ISR_ASSERT();

    RESCHED_ASSERT();

    vTaskSuspend(*task);
    return NO_ERROR;
}

WRAPPER_FUNCTION status_t thread_detach_and_resume(thread_t *t)
{
    thread_detach(t);
    thread_resume(t);
    return NO_ERROR;
}

WRAPPER_FUNCTION void thread_set_priority(int priority)
{
    ISR_ASSERT();
    vTaskPrioritySet(NULL,(UBaseType_t)(priority));
}

WRAPPER_FUNCTION void thread_preempt(void)
{
    thread_yield();
}


WRAPPER_FUNCTION status_t thread_join(thread_t *t, int *retcode, lk_time_t timeout)
{
    thread_sleep(timeout);
    return NO_ERROR;
}
