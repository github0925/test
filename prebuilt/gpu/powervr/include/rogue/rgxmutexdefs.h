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
#define	RGX_USC_MUTEX_LOCAL_MEMORY_ATOMIC  	0
/*
	Used for atomic operations on global memory
*/
#define RGX_USC_MUTEX_GLOBAL_MEMORY_ATOMIC 	1
/*
	Used for atomic operations on images.
*/
#define RGX_USC_MUTEX_IMAGE_ATOMIC 			2
/*
	Used for the debugger handler.
*/
#define RGX_USC_MUTEX_DEBUGGER 				3
/*
	The first used by the barrier. A barrier can use up all values
	starting from this one up to RGX_USC_MUTEX_COUNT - 1.
*/
#define RGX_USC_MUTEX_FIRST_FOR_BARRIER 	4

#if defined(FIX_HW_BRN_62269) || defined(FIX_HW_BRN_66972)
#define RGX_USC_MUTEX_BRN62269_RESUME_FINISHED	RGX_USC_MUTEX_FIRST_FOR_BARRIER
#endif /* defined(FIX_HW_BRN_62269) || defined(FIX_HW_BRN_66972) */

#endif /* _RGXMUTEXDEFS_H_ */
