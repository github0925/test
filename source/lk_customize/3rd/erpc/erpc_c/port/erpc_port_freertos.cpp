/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#include "erpc_port.h"
#include <new>

extern "C" {
#include "FreeRTOS.h"
};

using namespace std;
void *operator new(std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(std::size_t count, const std::nothrow_t &tag) THROW
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count, const std::nothrow_t &tag) THROW

{
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr) THROW
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) THROW
{
    erpc_free(ptr);
}

void *erpc_malloc(size_t size)
{
    void *p = pvPortMalloc(size);
    return p;
}

void erpc_free(void *ptr)
{
    vPortFree(ptr);
}

/* Provide function for pure virtual call to avoid huge demangling code being linked in ARM GCC */
#if 0 //((defined(__GNUC__)) && (defined(__arm__)))
extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}
#endif
