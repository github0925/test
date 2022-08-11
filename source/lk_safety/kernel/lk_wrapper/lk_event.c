/*
* lk_event.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implementation of wrapper layer of little kernel(LK)
* of event prototype for interthread communication.
*/

#include <lk_wrapper.h>


/*********************** event API redirection *******************/

WRAPPER_FUNCTION void event_init(event_t *e, bool initial, uint flags)
{

    e->pfrts_evt_handle = xQueueCreate(1,0);
    DEBUG_ASSERT(e->pfrts_evt_handle);

    /*as freertos create event group with zero value, if init state
     *was set by parameter, in this place should call make sure the event
     *bit was set before this event to be waiting.
     */
    if(initial)
    {
        if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
        {
            return;
        }
        /* Invalid to queueing if in spinlocked scetion. */
        RESCHED_ASSERT();

        xQueueSend(e->pfrts_evt_handle,NULL,0);
    }

    e->auto_clear = (flags == EVENT_FLAG_AUTOUNSIGNAL ? pdTRUE : pdFALSE);
}

WRAPPER_FUNCTION void event_destroy(event_t *e)
{
    vQueueDelete(e->pfrts_evt_handle);
}

WRAPPER_FUNCTION status_t event_wait_timeout(event_t *e, lk_time_t timeout) /* wait on the event with a timeout */
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    TickType_t xTicksToWait = (timeout == INFINITE_TIME ? portMAX_DELAY : LKMS_TO_FRTS_TICK(timeout));

    BaseType_t ret = pdFALSE;

    RESCHED_ASSERT();

    if(e->auto_clear)
    {
        ret = xQueueReceive(e->pfrts_evt_handle,NULL,xTicksToWait);
    }
    else
    {
        ret = xQueuePeek(e->pfrts_evt_handle,NULL,xTicksToWait);
    }

    return (ret == pdTRUE ? NO_ERROR : ERR_TIMED_OUT);
}

WRAPPER_FUNCTION status_t event_signal(event_t *e, bool reschedule)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;
    /*in freertos, a set of event may autoresch
     *if there is a blocked task is waiting for
     *this event. So caller should treat this as resched as always.
     */

    if(ulPortInterruptNesting > 0)
    {
        /* Here is in ISR context. call FromISR instead of classic API. */
        /* No need to check if in spinlocked state, as kernel may manage to resched
         * in ISR context. */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(e->pfrts_evt_handle,NULL,&xHigherPriorityTaskWoken);

        /* Notify kernel to do resched after irq over if a higher task was woken.*/
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);


    }
    else
    {
        /* Here is in task context. call classic API. */
        /* We still need to check if in spinlocked state, as no bottom scetion may
         * take responsiblity to do deferred resched. */
        RESCHED_ASSERT();

        xQueueSend(e->pfrts_evt_handle,NULL,0);
    }

    return NO_ERROR;
}

WRAPPER_FUNCTION status_t event_unsignal(event_t *e)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    xQueueReset(e->pfrts_evt_handle);


    return NO_ERROR;
}

