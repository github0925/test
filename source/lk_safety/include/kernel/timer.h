/*
 * Copyright (c) 2008-2009 Travis Geiselbrecht
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
#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include <compiler.h>
#include <list.h>
#include <sys/types.h>
#include <timers.h>
#include <FreeRTOS.h>
__BEGIN_CDECLS;


struct timer;

typedef enum handler_return (*timer_callback)(struct timer *, lk_time_t now, void *arg);

typedef struct timer
{
    TimerHandle_t pfrts_tmr_handle;
    timer_callback callback;
    void* arg;
    lk_time_t now;
    bool auto_reload;
}timer_t;




/* Rules for Timers:
 * - Timer callbacks occur from task context
 * - Timers MUST NOT be programmed or canceled from interrupt context
 * - Timers may be canceled or reprogrammed from within their callback
*/
void timer_init(void);

/* override:
*As freertos use timer daemon task for timer services, which means all sw timers
*were linked to a list which was maintained by this timer daemon task. In this way,
*timer-related API using a command queue to manipulate to sw timers, which have the uncertainty
*on the API result due to command queue full or not. Use FRTS_TMR_OP_TICKOUT macro to set
*the max block time (in freertos tick units) on the pending of API result.
*use 0  without timeout, use portMAX_DELAY for infinitely timeout.
* return: true: manipulation on success. false: maipulation fail.
*/
#define FRTS_TMR_OP_TICKOUT 0

/*
*   API: timer_initialize
*   desc: initialize a timer handle
*   para: timer handle.
*/
void timer_initialize(timer_t *);

/*
*   API: timer_set_oneshot
*   desc: set a one-shot timer. The timer will automatically start after called.
*   This function could be used for reconfig an active timer.
*   para: timer handle, expired delay, callback, argument of callback.
*   return: true: manipulation on success. false: maipulation fail.
*/
int timer_set_oneshot(timer_t *, lk_time_t delay, enum handler_return (*)(struct timer *, lk_time_t, void*), void *arg);

/*
*   API: timer_set_periodic
*   desc: set a periodic timer. The timer will automatically start after called.
*   This function could be used for reconfig an active timer.
*   para: timer handle, trigger period, callback, argument of callback.
*   return: true: manipulation on success. false: maipulation fail.
*/
int timer_set_periodic(timer_t *, lk_time_t period, enum handler_return (*)(struct timer *, lk_time_t, void*), void *arg);

/*
*   API: timer_cancel
*   desc: Cancel an active timer. The cancelled timer handle could not
*   be used.
*   para: timer handle
*   return: true: manipulation on success. false: maipulation fail.
*/
int timer_cancel(timer_t *);
lk_time_t current_time(void);

__END_CDECLS;

#endif

