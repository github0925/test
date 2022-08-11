/* semaphore.h
 *
 * Copyright 2012 Christopher Anderson <chris@nullcode.org>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __KERNEL_SEMAPHORE_H
#define __KERNEL_SEMAPHORE_H

#include <compiler.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <FreeRTOS.h>

__BEGIN_CDECLS;

#include <semphr.h>

typedef SemaphoreHandle_t semaphore_t;

/***************** API DECLARATION *****************/

/*
*   API: sem_init
*   desc: initialize a semaphore
*   para: semaphore handle, initial semaphore value.
*/
void sem_init(semaphore_t *, unsigned int);

/*
*   API: sem_destroy
*   desc: destroy a semaphore. Destroyed semaphore could not be used.
*   para: semaphore handle
*/
void sem_destroy(semaphore_t *);

/*
*   API: sem_post
*   desc: post(give) a semaphore.
*   para: semaphore handle, flag if trigger scheduler to resch asap.
*   this para will be ignored in FreeRTOS and user should take it as
*   `true` in default, as FreeRTOS defaultly do resch in every sync IPC API.
*   return:NO_ERROR if post succ or ERR_GENERIC if others failure.
*/
status_t sem_post(semaphore_t *, bool resched);

/*
*   API: sem_wait
*   desc: wait(take) a semaphore with infinite timeout.
*   para: semaphore handle
*   return: NO_ERROR if waited or blocked forever.
*/
status_t sem_wait(semaphore_t *);

/*
*   API: sem_trywait
*   desc: wait(take) a semaphore with no timeout (query once).
*   para: semaphore handle
*   return: NO_ERROR if waited or ERR_TIMED_OUT if semaphore not given.
*/
status_t sem_trywait(semaphore_t *);

/*
*   API: sem_timedwait
*   desc: wait(take) a semaphore with specific timeout
*   para: semaphore handle, timeout value, in ms.
*   return: NO_ERROR if waited or ERR_TIMED_OUT if semaphore not given.
*/
status_t sem_timedwait(semaphore_t *, lk_time_t);

__END_CDECLS;
#endif
