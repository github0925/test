/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 * Copyright (c) 2012 Shantanu Gupta
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
#ifndef __KERNEL_MUTEX_H
#define __KERNEL_MUTEX_H

#include <compiler.h>
#include <debug.h>
#include <stdint.h>
#include <kernel/thread.h>
#include <FreeRTOS.h>

__BEGIN_CDECLS;

#include <semphr.h>

typedef SemaphoreHandle_t mutex_t;

/***************** API DECLARATION *****************/

/* Rules for Mutexes:
 * - Mutexes are only safe to use from thread context.
 * - Mutexes are non-recursive.
*/

/*
*   API: mutex_init
*   desc: initialize a mutex handle.
*   para: mutex handle.
*/
void mutex_init(mutex_t *);

/*
*   API: mutex_destroy
*   desc: destroy a mutex. Destroyed mutex could not be used.
*   para: mutex handle.
*/
void mutex_destroy(mutex_t *);

/*
*   API: mutex_acquire_timeout
*   desc: acquire a mutex with specific timeout
*   para: mutex handle, timeout value, in ms.
*   return: NO_ERROR if acquired succ or ERR_TIMED_OUT if timeout happened.
*/
status_t mutex_acquire_timeout(mutex_t *, lk_time_t); /* try to acquire the mutex with a timeout value */

/*
*   API: mutex_release
*   desc: release a mutex. This function must be called with
*   `mutex_acquire_timeout` or `mutex_acquire` as bracklet.
*   para: mutex handle
*   return: NO_ERROR if release succ or ERR_GENERIC if others' failure.
*/
status_t mutex_release(mutex_t *);

/*
*   API: mutex_acquire
*   desc: acquire a mutex with infinite timeout.
*   para: mutex handle
*   return: NO_ERROR if acquire succ or blocked forever.
*/
static inline status_t mutex_acquire(mutex_t *m)
{
    return mutex_acquire_timeout(m, INFINITE_TIME);
}


__END_CDECLS;
#endif

