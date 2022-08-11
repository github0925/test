/*************************************************************************/ /*!
@File           pvr_refcount.h
@Title          IMG refcount implementation
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Refcount implementation to avoid opencoding it all over
                the place and introducing hard to understand code and bugs.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVR_REFCOUNT_H
#define _PVR_REFCOUNT_H

#include <stdbool.h>

#include "pvrsrv_atomic.h"
#include "pvr_debug.h"
#include "img_defs.h"

typedef struct {
   ATOMIC_T refs;
} IMG_REFCOUNT;

#define IMG_REFCOUNT_DECLARE(name) \
	IMG_REFCOUNT name = { .refs = 0 }

/* starts at 1, ie. you don't need to call Get() just after Init() */
static inline void
PVRRefCountInit(IMG_REFCOUNT *r)
{
	PVRSRVAtomicWrite(&r->refs, 1);
}

/* returns true when acquiring the first reference */
static inline bool
PVRRefCountAcquire(IMG_REFCOUNT *r)
{
	return PVRSRVAtomicIncrement(&r->refs) == 1;
}

/* returns true when releasing the last reference */
#if defined(__KLOCWORK__) && defined(INTEGRITY_OS)
/* Klocwork bug causes parse errors when both bool and __attribute__ are encountered here in Integrity build. */
static inline bool
#else
static inline bool __must_check
#endif
PVRRefCountRelease(IMG_REFCOUNT *r)
{
	/* Going below zero is a bug */
	PVR_ASSERT(PVRSRVAtomicRead(&r->refs) > 0);

	return PVRSRVAtomicDecrement(&r->refs) == 0;
}

#endif /* _PVR_REFCOUNT_H */
