/*************************************************************************/ /*!
@File           lock_types.h
@Title          Locking types
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Locking specific enums, defines and structures
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef LOCK_TYPES_H
#define LOCK_TYPES_H

/* In Linux kernel mode we are using the kernel mutex implementation directly
 * with macros. This allows us to use the kernel lockdep feature for lock
 * debugging. */
#if defined(LINUX) && defined(__KERNEL__)

#include <linux/types.h>
#include <linux/mutex.h>
/* The mutex is defined as a pointer to be compatible with the other code. This
 * isn't ideal and usually you wouldn't do that in kernel code. */
typedef struct mutex *POS_LOCK;
typedef struct rw_semaphore *POSWR_LOCK;
typedef spinlock_t *POS_SPINLOCK;
typedef atomic_t ATOMIC_T;

#else /* defined(LINUX) && defined(__KERNEL__) */
#include "img_types.h" /* needed for IMG_INT */
typedef struct _OS_LOCK_ *POS_LOCK;

#if defined(LINUX) || defined(__QNXNTO__) || defined(INTEGRITY_OS)
typedef struct _OSWR_LOCK_ *POSWR_LOCK;
#else /* defined(LINUX) || defined(__QNXNTO__) || defined(INTEGRITY_OS) */
typedef struct _OSWR_LOCK_ {
	IMG_UINT32 ui32Dummy;
} *POSWR_LOCK;
#endif /* defined(LINUX) || defined(__QNXNTO__) || defined(INTEGRITY_OS) */

#if defined(LINUX)
	typedef struct _OS_ATOMIC {IMG_INT32 counter;} ATOMIC_T;
#elif defined(__QNXNTO__)
	typedef struct _OS_ATOMIC {IMG_INT32 counter;} ATOMIC_T;
#elif defined(_WIN32)
	/*
	 * Dummy definition. WDDM doesn't use Services, but some headers
	 * still have to be shared. This is one such case.
	 */
	typedef struct _OS_ATOMIC {IMG_INT32 counter;} ATOMIC_T;
#elif defined(INTEGRITY_OS)
	/* Only lower 32bits are used in OS ATOMIC APIs to have consistent behaviour across all OS */
	typedef struct _OS_ATOMIC {IMG_INT64 counter;} ATOMIC_T;
#else
	#error "Please type-define an atomic lock for this environment"
#endif

#endif /* defined(LINUX) && defined(__KERNEL__) */

#endif /* LOCK_TYPES_H */
