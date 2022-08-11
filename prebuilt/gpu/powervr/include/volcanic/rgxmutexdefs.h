/*************************************************************************/ /*!
@Title          Information about the uses of USC mutuxes.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _RGXMUTEXDEFS_H_
#define _RGXMUTEXDEFS_H_

/*
	Used for compute atomic operations on local memory where the current workgroup
	might span multiple slots.
*/
#define RGX_USC_MUTEX_LOCAL_MEMORY_ATOMIC			(0)

/*
	Used for atomic operations on global memory
*/
#define RGX_USC_MUTEX_GLOBAL_MEMORY_ATOMIC			(1)

/*
	Used for atomic operations on images.
*/
#define RGX_USC_MUTEX_IMAGE_ATOMIC					(2)

#endif /* _RGXMUTEXDEFS_H_ */
