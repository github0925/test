/*************************************************************************/ /*!
@File           pvrsrv_atomic.h
@Title          PVRSRV Atomic types
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Atomic specific defines and functions
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRSRV_ATOMIC_H_
#define _PVRSRV_ATOMIC_H_

#if defined __cplusplus
extern "C" {
#endif

#if (defined(LINUX) && defined(__KERNEL__)) || defined(_WIN32)
	#error "Atomic functions are not supported by Services for this environment"
#else
/*************************************************************************/ /*!
@Function       PVRSRVAtomicRead
@Description    Read the value of a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to read
@Return         The value of the atomic variable
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicRead(const ATOMIC_T *pCounter);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicWrite
@Description    Write the value of a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be written to
@Input          v               The value to write
@Return         None
*/ /**************************************************************************/
IMG_EXPORT
void PVRSRVAtomicWrite(ATOMIC_T *pCounter, IMG_INT32 v);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicIncrement
@Description    Increment the value of a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be incremented
@Return         The new value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicIncrement(ATOMIC_T *pCounter);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicDecrement
@Description    Decrement the value of a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be decremented
@Return         The new value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicDecrement(ATOMIC_T *pCounter);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicAdd
@Description    Add a specified value to a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to add the value to
@Input          v               The value to be added
@Return         The new value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicAdd(ATOMIC_T *pCounter, IMG_INT32 v);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicAddUnless
@Description    Add a specified value to a variable atomically unless it
                already equals a particular value.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to add the value to
@Input          v               The value to be added to 'pCounter'
@Input          t               The test value. If 'pCounter' equals this,
                                its value will not be adjusted
@Return         The old value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicAddUnless(ATOMIC_T *pCounter, IMG_INT32 v, IMG_INT32 t);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicSubtract
@Description    Subtract a specified value to a variable atomically.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to subtract the value from
@Input          v               The value to be subtracted
@Return         The new value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicSubtract(ATOMIC_T *pCounter, IMG_INT32 v);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicSubtractUnless
@Description    Subtract a specified value from a variable atomically unless
                it already equals a particular value.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to subtract the value from
@Input          v               The value to be subtracted from 'pCounter'
@Input          t               The test value. If 'pCounter' equals this,
                                its value will not be adjusted
@Return         The old value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicSubtractUnless(ATOMIC_T *pCounter, IMG_INT32 v, IMG_INT32 t);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicCompareExchange
@Description    Set a variable to a given value only if it is currently
                equal to a specified value. The whole operation must be atomic.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be checked and
                                possibly updated
@Input          oldv            The value the atomic variable must have in
                                order to be modified
@Input          newv            The value to write to the atomic variable if
                                it equals 'oldv'
@Return         The value of *pCounter before the function.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicCompareExchange(ATOMIC_T *pCounter, IMG_INT32 oldv, IMG_INT32 newv);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicExchange
@Description    Set a variable to a given value and retrieve previous value.
                The whole operation must be atomic.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be updated
@Input          newv            The value to write to the atomic variable
@Return         The previous value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicExchange(ATOMIC_T *pCounter, IMG_INT32 newv);

/*************************************************************************/ /*!
@Function       PVRSRVAtomicOr
@Description    Set a variable to the bitwise or of its current value and the
                specified value. Equivalent to *pCounter |= iVal.
                The whole operation must be atomic.
                Atomic functions must be implemented in a manner that
                is both symmetric multiprocessor (SMP) safe and has a memory
                barrier around each operation.
@Input          pCounter        The atomic variable to be updated
@Input          iVal            The value to bitwise or against
@Return         The new value of *pCounter.
*/ /**************************************************************************/
IMG_EXPORT
IMG_INT32 PVRSRVAtomicOr(ATOMIC_T *pCounter, IMG_INT32 iVal);

#endif /* (defined(LINUX) && defined(__KERNEL__)) || defined(_WIN32) */

#if defined __cplusplus
}
#endif

#endif	/* _PVRSRV_ATOMIC_H_ */
