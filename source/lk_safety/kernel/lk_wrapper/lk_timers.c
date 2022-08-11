/*
 * lk_timer.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: implementation of wrapper layer of little kernel(LK)
 * of timer prototype.
 */
#include <lk_wrapper.h>

static void lk_timer_general_callback_dispatch(TimerHandle_t timer);

static void timer_config(timer_t *t, lk_time_t delay, timer_callback callback, void *arg)
{
    if(!t->pfrts_tmr_handle)
    {
        t->arg = arg;
        t->callback = callback;
        t->pfrts_tmr_handle = xTimerCreate(
            "sd_tmr", //name
            LKMS_TO_FRTS_TICK(delay), // freertos ticks of period
             pdFALSE, //auto-reload or not, we use our interal flag to maintain
             (void*)t, //use timerID to pass lk_timer structure
             lk_timer_general_callback_dispatch);

        DEBUG_ASSERT(t->pfrts_tmr_handle);

    }
    else
    {
        if(callback != t->callback || arg != t->arg) //if change the callback or arg
        {
            timer_cancel(t);
            t->arg = arg;
            t->callback = callback;
            t->pfrts_tmr_handle = xTimerCreate(
                "sd_tmr", //name
                LKMS_TO_FRTS_TICK(delay), // freertos ticks of period
                pdFALSE, //auto-reload or not, we use our interal flag to maintain
                (void*)t, //use timerID to pass lk_timer structure
                lk_timer_general_callback_dispatch);

            DEBUG_ASSERT(t->pfrts_tmr_handle);
        }
        else if(xTimerGetPeriod(t->pfrts_tmr_handle) != LKMS_TO_FRTS_TICK(delay))
        {
            xTimerChangePeriod(t->pfrts_tmr_handle,LKMS_TO_FRTS_TICK(delay),FRTS_TMR_OP_TICKOUT);
        }
    }
}

/*********************** timer API redirection *******************/

/* as freertos use timer daemon task, which is created statically
 * if configUSE_TIMERS was set to 1.
 */
WRAPPER_FUNCTION void timer_init(void)
{
    return;
}

WRAPPER_FUNCTION void timer_initialize(timer_t *t)
{
    t->pfrts_tmr_handle = NULL;
    t->now = 0;
    t->auto_reload = false;
}

WRAPPER_FUNCTION int timer_set_oneshot(timer_t *t, lk_time_t delay, timer_callback callback, void *arg)
{
    timer_config(t,delay,callback,arg);
    t->auto_reload = false;

    return (pdTRUE == xTimerReset(t->pfrts_tmr_handle,FRTS_TMR_OP_TICKOUT) ? true : false);
}

WRAPPER_FUNCTION int timer_set_periodic(timer_t *t, lk_time_t period, timer_callback callback, void *arg)
{
    timer_config(t,period,callback,arg);
    t->auto_reload = true;

    return (pdTRUE == xTimerReset(t->pfrts_tmr_handle,FRTS_TMR_OP_TICKOUT) ? true : false);
}

WRAPPER_FUNCTION int timer_cancel(timer_t *t)
{
    DEBUG_ASSERT(t->pfrts_tmr_handle);

    int ret = true;

    ret &= xTimerStop(t->pfrts_tmr_handle,0);
    ret &= xTimerDelete(t->pfrts_tmr_handle,0);

    timer_initialize(t);

    return ret;
}

static void lk_timer_general_callback_dispatch(TimerHandle_t timer)
{
    lk_time_t now = current_time();
    timer_t* t = pvTimerGetTimerID(timer);

    ASSERT(t->callback);

    t->now = now;
    
    t->callback(t,t->now,t->arg);

    if(t->auto_reload)
    {
        xTimerReset(t->pfrts_tmr_handle,FRTS_TMR_OP_TICKOUT);
    }
}
