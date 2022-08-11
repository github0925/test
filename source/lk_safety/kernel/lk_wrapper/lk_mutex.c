/*
* lk_mutex.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implementation of wrapper layer of little kernel(LK)
* of mutex prototype for interthread communication.
*/
#include <lk_wrapper.h>

/*********************** mutex API redirection *******************/

WRAPPER_FUNCTION void mutex_init(mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();
    DEBUG_ASSERT(*mutex);
}

WRAPPER_FUNCTION void mutex_destroy(mutex_t *mutex)
{
    vSemaphoreDelete(*mutex);
    *mutex = NULL;
}

WRAPPER_FUNCTION status_t mutex_acquire_timeout(mutex_t *mutex, lk_time_t timeout) /* try to acquire the mutex with a timeout value */
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    RESCHED_ASSERT();

    BaseType_t ret = xSemaphoreTake(*mutex, (timeout == INFINITE_TIME ? portMAX_DELAY : LKMS_TO_FRTS_TICK(timeout) ));
    return (pdTRUE == ret ? NO_ERROR : ERR_TIMED_OUT);
}

WRAPPER_FUNCTION status_t mutex_release(mutex_t *mutex)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    return NO_ERROR;

    ISR_ASSERT();

    RESCHED_ASSERT();

    BaseType_t ret = xSemaphoreGive(*mutex);
    return (pdTRUE == ret ? NO_ERROR : ERR_TIMED_OUT);
}
