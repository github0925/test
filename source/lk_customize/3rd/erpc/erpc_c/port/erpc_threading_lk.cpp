/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "erpc_threading.h"
#include <cassert>

#if ERPC_THREADS_IS(LK)

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_thread(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_thread(0)
{
}

Thread::~Thread(void) {}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
}

void Thread::start(void *arg)
{
    m_arg = arg;

    m_thread = thread_create("erpc_thread", threadEntryPointStub, this, m_priority, DEFAULT_STACK_SIZE);
}

bool Thread::operator==(Thread &o)
{
    return m_thread == o.m_thread;
}

Thread *Thread::getCurrentThread(void)
{
    return reinterpret_cast<Thread *>(tls_get(TLS_ENTRY_OSAL));
}

void Thread::sleep(uint32_t usecs)
{
    thread_sleep(usecs / 1000);
}

void Thread::threadEntryPoint(void)
{
    if (m_entry)
    {
        m_entry(m_arg);
    }
}

int Thread::threadEntryPointStub(void *arg)
{
    Thread *_this = reinterpret_cast<Thread *>(arg);
    assert(_this && "Reinterpreting 'void *arg' to 'Thread *' failed.");

    tls_set(TLS_ENTRY_OSAL, (uintptr_t) _this);

    _this->threadEntryPoint();

    // Handle a task returning from its function.

    return 0;
}

Mutex::Mutex(void)
{
    mutex_init(&m_mutex);
}

Mutex::~Mutex(void) {}

bool Mutex::tryLock(void)
{
    return (mutex_acquire_timeout(&m_mutex, 0) == 0);
}

bool Mutex::lock(void)
{
    return (mutex_acquire(&m_mutex) == 0);
}

bool Mutex::unlock(void)
{
    mutex_release(&m_mutex);
    return true;
}

Semaphore::Semaphore(int count)
{
    // Set max count to highest signed int.
    sem_init(&m_sem, count);
}

Semaphore::~Semaphore(void) {}

void Semaphore::put(void)
{
    sem_post(&m_sem, true);
}

bool Semaphore::get(uint32_t timeout)
{
    return (sem_timedwait(&m_sem, timeout / 1000) == 0);
}

int Semaphore::getCount(void) const
{
    return m_sem.count;
}
#endif /* ERPC_THREADS_IS(FREERTOS) */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
