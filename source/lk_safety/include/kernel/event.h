/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
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
#ifndef __KERNEL_EVENT_H
#define __KERNEL_EVENT_H

#include <compiler.h>
#include <stdbool.h>
#include <sys/types.h>
#include <kernel/thread.h>
#include <queue.h>

__BEGIN_CDECLS;

#define EVENT_MAGIC (0x65766E74)  // "evnt"
#define EVENT_FLAG_AUTOUNSIGNAL 1

typedef struct event {
    QueueHandle_t pfrts_evt_handle;
    bool auto_clear;
} event_t;

static inline bool event_initialized(event_t *e)
{
    return (NULL != e->pfrts_evt_handle) ? true : false;
}

/***************** API DECLARATION *****************/


/* Rules for Events:
 * - Events may be signaled from interrupt context *but* the reschedule
 *   parameter must be false in that case.
 * - Events may not be waited upon from interrupt context.
 * - Events without FLAG_AUTOUNSIGNAL:
 *   - Wake up any waiting threads when signaled.
 *   - Continue to do so (no threads will wait) until unsignaled.
 * - Events with FLAG_AUTOUNSIGNAL:
 *   - If one or more threads are waiting when signaled, one thread will
 *     be woken up and return.  The signaled state will not be set.
 *   - If no threads are waiting when signaled, the Event will remain
 *     in the signaled state until a thread attempts to wait (at which
 *     time it will unsignal atomicly and return immediately) or
 *     event_unsignal() is called.
*/

/*
*   API: event_init
*   desc: initialize an event handle.
*   para: event handle, flag if auto-signal after initialzed,
*   flag if this event is an autounsignal event.
*/
void event_init(event_t *, bool initial, uint flags);

/*
*   API: event_destroy
*   desc: destroy an event handle. Destroyed event could not
*   be used.
*   para: event handle
*/
void event_destroy(event_t *);

/*
*   API: event_wait_timeout
*   desc: wait an event with specific timeout.
*   para: event handle, waiting timeout, in ms.
*/
status_t event_wait_timeout(event_t *, lk_time_t); /* wait on the event with a timeout */

/*
*   API: event_signal
*   desc: signal an event then awake the waiting thread.
*   para: event handle, flag if need resch.
*   Note: In FreeRTOS, this flag was ignored as FreeRTOS
*   defaultly do resch on its IPC sync API.
*   return:NO_ERROR if signal succ or others if failure.
*/
status_t event_signal(event_t *, bool reschedule);

/*
*   API: event_unsignal
*   desc: unsignal an event. This was useful while event
*   type is not autounsignal.
*   para: event handle.
*   return： NO_ERROR if unsignal succ or thers if failure.
*/
status_t event_unsignal(event_t *);

/*
*   API: event_wait
*   desc: wait an event with infinite timeout.
*   para: event handle.
*   return： NO_ERROR if unsignal succ or blocked forever.
*/
static inline status_t event_wait(event_t *e)
{
    return event_wait_timeout(e, INFINITE_TIME);
}

__END_CDECLS;

#endif

