/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#ifndef _ERPC_DETECT_H_
#define _ERPC_DETECT_H_

#include "erpc_config.h"
//#include "FreeRTOSConfig.h"

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////
/* clang-format off */

// Determine if this is a POSIX system.
#if !defined(ERPC_HAS_POSIX)
    // Detect Linux, BSD, Cygwin, and Mac OS X.
    #if defined(__linux__) || defined(__GNU__) || defined(__FreeBSD__) || defined(__NetBSD__) || \
        defined(__OpenBSD__) || defined(__DragonFly__) || defined(__CYGWIN__) || defined(__MACH__)
        #define ERPC_HAS_POSIX (1)
    #else
        #define ERPC_HAS_POSIX (0)
    #endif
#endif

// Safely detect FreeRTOSConfig.h.
#define ERPC_HAS_FREERTOSCONFIG_H (0)
#if defined(__has_include)
    #if __has_include("FreeRTOSConfig.h")
        #undef ERPC_HAS_FREERTOSCONFIG_H
        #define ERPC_HAS_FREERTOSCONFIG_H (1)
    #endif
#endif

// Detect threading model if not already set.
#if !defined(ERPC_THREADS)
    #if ERPC_HAS_POSIX
        // Default to pthreads for POSIX systems.
        #define ERPC_THREADS (ERPC_THREADS_PTHREADS)
    #elif ERPC_HAS_FREERTOSCONFIG_H
        // Use FreeRTOS if we can auto detect it.
        #define ERPC_THREADS (ERPC_THREADS_FREERTOS)
    #else
        // Otherwise default to no threads.
        #define ERPC_THREADS (ERPC_THREADS_NONE)
    #endif
#endif

// Handy macro to test threading model. You can also ERPC_THREADS directly to test for threading
// support, i.e. "#if ERPC_THREADS", because ERPC_THREADS_NONE has a value of 0.
#define ERPC_THREADS_IS(_n_) (ERPC_THREADS == (ERPC_THREADS_##_n_))

// Set default buffer size.
#if !defined(ERPC_DEFAULT_BUFFER_SIZE)
    //! @brief Size of buffers allocated by BasicMessageBufferFactory in setup functions.
    #define ERPC_DEFAULT_BUFFER_SIZE (256)
#endif

// Set default buffers count.
#if !defined(ERPC_DEFAULT_BUFFERS_COUNT)
    //! @brief Count of buffers allocated by StaticMessageBufferFactory.
    #define ERPC_DEFAULT_BUFFERS_COUNT (2)
#endif

// Disable/enable noexcept.
#if !defined(ERPC_NOEXCEPT)
    #if ERPC_HAS_POSIX
        #define ERPC_NOEXCEPT (ERPC_NOEXCEPT_ENABLED)
    #else
        #define ERPC_NOEXCEPT (ERPC_NOEXCEPT_DISABLED)
    #endif
#endif

//NOEXCEPT support
#if defined(__cplusplus) && __cplusplus >= 201103 && ERPC_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif // NOEXCEPT

// Disabling nesting calls support as default.
#if !defined(ERPC_NESTED_CALLS)
    #define ERPC_NESTED_CALLS (ERPC_NESTED_CALLS_DISABLED)
#endif

#if ERPC_NESTED_CALLS && !ERPC_THREADS
    #error "Nested calls currently working only with Threads."
#endif

// Enabling nesting calls detection as default for debug.
#if !defined(ERPC_NESTED_CALLS_DETECTION)
    #if defined(NDEBUG) || (ERPC_NESTED_CALLS == ERPC_NESTED_CALLS_ENABLED)
        #define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_DISABLED)
    #else
        #define ERPC_NESTED_CALLS_DETECTION (ERPC_NESTED_CALLS_DETECTION_ENABLED)
    #endif
#endif

// Disabling tracing the eRPC.
#if !defined(ERPC_MESSAGE_LOGGING)
    #define ERPC_MESSAGE_LOGGING (ERPC_MESSAGE_LOGGING_DISABLED)
#endif

#if defined(__CC_ARM) /* Keil MDK */
#define THROW_BADALLOC throw(std::bad_alloc)
#define THROW throw()
#else
#define THROW_BADALLOC
#define THROW
#endif

#ifndef ERPC_TRANSPORT_MU_USE_MCMGR
    #if defined(__has_include)
        #if (__has_include("mcmgr.h"))
            #define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED)
        #else
            #define ERPC_TRANSPORT_MU_USE_MCMGR (ERPC_TRANSPORT_MU_USE_MCMGR_DISABLED)
        #endif
    #endif
#else
    #if defined(__has_include)
        #if ((!(__has_include("mcmgr.h"))) && (ERPC_TRANSPORT_MU_USE_MCMGR == ERPC_TRANSPORT_MU_USE_MCMGR_ENABLED))
            #error "Do not forget to add the MCMGR library into your project!"
        #endif
    #endif
#endif

/* clang-format on */
#endif // _ERPC_DETECT_H_
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
