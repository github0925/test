/*
 * lk_semaphore.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: implementation of wrapper layer of little kernel(LK)
 * of semaphore prototype for interthread commuincation.
 */

#include <lk_wrapper.h>

#define lkw_SEM_CEIL (UBaseType_t)(-1) /*default semaphore count ceiling*/

/*********************** semaphore API redirection *******************/

WRAPPER_FUNCTION void sem_init(semaphore_t *sem, unsigned int count)
{
    *sem = xSemaphoreCreateCounting(lkw_SEM_CEIL,count);
    DEBUG_ASSERT(*sem);
}

WRAPPER_FUNCTION void sem_destroy(semaphore_t *sem)
{
    vSemaphoreDelete(*sem);
}

WRAPPER_FUNCTION status_t sem_post(semaphore_t *sem, bool resched)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;
    /*in freertos,  a post of sem may autoresch
    *if there is a blocked task is waiting for
    *this sem. So caller should set resched as true always.
    */
   BaseType_t ret = pdFALSE;
   if(ulPortInterruptNesting > 0)
   {
        /* Here is in ISR context. call FromISR instead of classic API. */
        /* No need to check if in spinlocked state, as kernel may manage to resched
         * in ISR context. */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        ret = xSemaphoreGiveFromISR(*sem,&xHigherPriorityTaskWoken);
        /* Notify to kernel */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
   else
   {
       RESCHED_ASSERT();

       ret = xSemaphoreGive(*sem);
   }



    return ( pdTRUE == ret ) ? NO_ERROR : ERR_GENERIC;
}

WRAPPER_FUNCTION status_t sem_wait(semaphore_t *sem)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    RESCHED_ASSERT();

    return (pdTRUE == xSemaphoreTake(*sem,portMAX_DELAY) ) ? NO_ERROR : ERR_TIMED_OUT;
}

WRAPPER_FUNCTION status_t sem_trywait(semaphore_t *sem)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    RESCHED_ASSERT();

    return (pdTRUE == xSemaphoreTake(*sem,0)) ? NO_ERROR : ERR_TIMED_OUT;
}

WRAPPER_FUNCTION status_t sem_timedwait(semaphore_t *sem, lk_time_t timeout)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    RESCHED_ASSERT();

    return (pdTRUE == xSemaphoreTake(*sem, LKMS_TO_FRTS_TICK(timeout))) ? NO_ERROR : ERR_TIMED_OUT;
}
