/*
 * Copyright (c) 2019 Semidrive Semiconductor
 * All rights reserved.
 *
 */

/**************************************************************************
 * FILE NAME
 *
 *       ipcc_internal.h
 *
 * DESCRIPTION
 *
 *       This file defines compiler macros
 *
 ***************************************************************************/
#ifndef _IPCC_INTERNAL_H_
#define _IPCC_INTERNAL_H_


/* ARM GCC */
#if defined(__CC_ARM) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))

#if (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#include <arm_compat.h>
#endif

#define MEM_BARRIER() __schedule_barrier()

#ifndef DCF_PACKED_BEGIN
#define DCF_PACKED_BEGIN _Pragma("pack(1U)")
#endif

#ifndef DCF_PACKED_END
#define DCF_PACKED_END _Pragma("pack()")
#endif

/* GNUC */
#elif defined(__GNUC__)

#define MEM_BARRIER() __asm__ volatile("dsb" : : : "memory")

#ifndef DCF_PACKED_BEGIN
#define DCF_PACKED_BEGIN
#endif

#ifndef DCF_PACKED_END
#define DCF_PACKED_END __attribute__((__packed__))
#endif

#elif defined (__ICCARM__)
#define MEM_BARRIER() __asm volatile("dsb" : : : "memory")
#ifndef DCF_PACKED_BEGIN
#define DCF_PACKED_BEGIN
#endif

#ifndef DCF_PACKED_END
#define DCF_PACKED_END __packed
#endif

#else
/* There is no default definition here to avoid wrong structures packing in case of not supported compiler */
#error Please implement the structure packing macros for your compiler here!
#endif

#endif /* _IPCC_INTERNAL_H_ */
