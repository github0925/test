/*************************************************************************/ /*!
@File
@Title          Services DDK Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported Services DDK function details
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef SERVICES_CLIENT_PORTING_H
#define SERVICES_CLIENT_PORTING_H


#if defined(__cplusplus)
extern "C" {
#endif

#include "img_defs.h"

/* The comment below is the front page for code-generated doxygen documentation */
/*!
 ******************************************************************************
 @mainpage
 This document lists the Client, Kernel and System functions which require an
 OS-specific implementation, and thus would need to be ported in order to
 support the DDK on another Operating System.

 The following sections list functions grouped by the header files in which
 they are defined.

 NB. Please note that this document is in a draft form and is still being
     finalised. It does not currently include functions required when PDUMP
     is enabled.
 *****************************************************************************/

/*! Max length of an App-Hint string */
#define APPHINT_MAX_STRING_SIZE 256

/*!
 ******************************************************************************
 * IMG data types
 *****************************************************************************/
typedef enum
{
	IMG_STRING_TYPE     = 1,                /*!< String type */
	IMG_FLOAT_TYPE      ,                   /*!< Float type */
	IMG_UINT_TYPE       ,                   /*!< Unsigned Int type */
	IMG_INT_TYPE        ,                   /*!< (Signed) Int type */
	IMG_FLAG_TYPE                           /*!< Flag Type */
}IMG_DATA_TYPE;

/*!
 ******************************************************************************
 * User Module type
 *****************************************************************************/
typedef enum
{
	IMG_EGL             = 0x00000001,      /*!< EGL Module */
	IMG_OPENGLES1       = 0x00000002,      /*!< OGLES1 Module */
	IMG_OPENGLES3       = 0x00000003,      /*!< OGLES3 Module */
	IMG_D3DM            = 0x00000004,      /*!< D3DM Module */
	IMG_SRV_UM          = 0x00000005,      /*!< Services User-Mode */
	IMG_SRV_INIT        = 0x00000006,      /*!< Services initialisation */
	IMG_SRVCLIENT       = 0x00000007,      /*!< Services Client */
	IMG_OPENGL          = 0x00000008,      /*!< OpenGL */
	IMG_D3D             = 0x00000009,      /*!< D3D */
	IMG_OPENCL          = 0x0000000A,      /*!< OpenCL */
	IMG_ANDROID_HAL     = 0x0000000B,      /*!< Graphics HAL */
	IMG_WEC_GPE         = 0x0000000C,      /*!< WinEC-specific GPE */
	IMG_PVRGPE          = 0x0000000D,      /*!< WinEC/WinCE GPE */
	IMG_RSCOMPUTE       = 0x0000000E,      /*!< RenderScript Compute */
	IMG_RESERVED1       = 0x0000000F,      /*!< Reserved for backwards compat */
	IMG_PDUMPCTRL       = 0x00000010,      /*!< PDump control client */
	IMG_USC2            = 0x00000011,      /*!< Uniflex compiler */

} IMG_MODULE_ID;

/*************************************************************************/ /*!
@Function       PVRSRVCreateAppHintState
@Description    Create app hint state
@Input          eModuleID       module id
@Input          pszAppName      app name
@Output         ppvState        state
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void IMG_CALLCONV PVRSRVCreateAppHintState(IMG_MODULE_ID eModuleID,
                                                      const IMG_CHAR *pszAppName,
                                                      void **ppvState);
/*************************************************************************/ /*!
@Function       PVRSRVFreeAppHintState
@Description    Free the app hint state, if it was created
@Input          eModuleID       module id
@Input          pvHintState     app hint state
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void IMG_CALLCONV PVRSRVFreeAppHintState(IMG_MODULE_ID eModuleID,
                                                    void *pvHintState);

/*************************************************************************/ /*!
@Function       PVRSRVGetAppHint
@Description    Return the value of this hint from state or use default
@Input          pvHintState     hint state
@Input          pszHintName     hint name
@Input          eDataType       data type
@Input          pvDefault       default value
@Output         pvReturn        hint value
@Return         True if hint read, False if used default.
*/ /**************************************************************************/
IMG_EXPORT IMG_BOOL IMG_CALLCONV PVRSRVGetAppHint(void *pvHintState,
                                                  const IMG_CHAR *pszHintName,
                                                  IMG_DATA_TYPE eDataType,
                                                  const void *pvDefault,
                                                  void *pvReturn);

/*************************************************************************/ /*!
@Function       PVRSRVGetAppHintWithAllocation
@Description    Return the value of this hint from state or use default
@Input          pvHintState     hint state
@Input          pszHintName     hint name
@Input          eDataType       data type
@Input          pvDefault       default value
@Output         ppvReturn       hint value
@Return         True if hint read, False if used default
*/ /**************************************************************************/
IMG_EXPORT IMG_BOOL IMG_CALLCONV PVRSRVGetAppHintWithAllocation(void *pvHintState,
                                                                const IMG_CHAR *pszHintName,
                                                                IMG_DATA_TYPE eDataType,
                                                                const void *pvDefault,
                                                                void **ppvReturn);


/*************************************************************************/ /*!
@Function       PVRSRVLoadLibrary
@Description    Load the named Dynamic-Link (Shared) Library. This will perform
                reference counting in association with PVRSRVUnloadLibrary(),
                so for example if the same library is loaded twice and unloaded
                once, a reference to the library will remain.
@Input          pszLibraryName      the name of the library to load
@Return         On success, the handle of the newly-loaded library.
                      Otherwise, zero.
*/ /**************************************************************************/
IMG_EXPORT IMG_HANDLE PVRSRVLoadLibrary(const IMG_CHAR *pszLibraryName);

/*************************************************************************/ /*!
@Function       PVRSRVUnloadLibrary
@Description    Unload the Dynamic-Link (Shared) Library which had previously
                been loaded using PVRSRVLoadLibrary(). See PVRSRVLoadLibrary()
                for information regarding reference counting.
@Input          hExtDrv             handle of the Dynamic-Link / Shared library
                                    to unload, as returned by PVRSRVLoadLibrary().
@Return         PVRSRV_OK if successful. Otherwise,
                      PVRSRV_ERROR_UNLOAD_LIBRARY_FAILED.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVUnloadLibrary(IMG_HANDLE hExtDrv);

/*************************************************************************/ /*!
@Function       PVRSRVGetLibFuncAddr
@Description    Returns the address of a function in a Dynamic-Link / Shared
                Library.
@Input          hExtDrv             handle of the Dynamic-Link / Shared Library
                                    in which the function resides
@Input          pszFunctionName     the name of the function
@Output         ppvFuncAddr         on success, the address of the function
                                    requested. Otherwise, NULL.
@Return         PVRSRV_OK if successful. Otherwise,
                      PVRSRV_ERROR_UNABLE_TO_GET_FUNC_ADDR.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVGetLibFuncAddr(IMG_HANDLE hExtDrv,
                                             const IMG_CHAR *pszFunctionName,
                                             void **ppvFuncAddr);

/*************************************************************************/ /*!
@Function       PVRSRVClockus
@Description    Returns the current system clock time, in microseconds. Note
                that this does not necessarily guarantee microsecond accuracy.
@Return         The current system clock time, in microseconds
*/ /**************************************************************************/
IMG_EXPORT IMG_UINT32 PVRSRVClockus(void);

/*************************************************************************/ /*!
@Function       PVRSRVClockus64
@Description    Returns the current system clock time, in microseconds. Note
                that this does not necessarily guarantee microsecond accuracy.
                Note: This function should work exactly like PVRSRVClockus
                      with the difference in return type. The 64bit return
                      type should ensure that the timestamp value doesn't
                      wrap around.
@Return         The current system clock time, in microseconds
*/ /**************************************************************************/
IMG_EXPORT IMG_UINT64 PVRSRVClockus64(void);

/*************************************************************************/ /*!
@Function       PVRSRVClockns64
@Description    Returns the current system clock time, in nanoseconds. Note
                that this does not necessarily guarantee nanosecond accuracy.
@Return         The current system clock time, in nanoseconds
*/ /**************************************************************************/
IMG_EXPORT IMG_UINT64 PVRSRVClockns64(void);

/*************************************************************************/ /*!
@Function       PVRSRVClockMonotonicRawus64
@Description    Returns the current system clock time, in microseconds. Note
                that this does not necessarily guarantee microsecond accuracy.
                Note: This function should return the same time as the server
                      counterpart.
@Return         The current system clock time, in microseconds
*/ /**************************************************************************/
IMG_EXPORT IMG_UINT64 PVRSRVClockMonotonicRawus64(void);

/*************************************************************************/ /*!
@Function       PVRSRVWaitus
@Description    Waits for the specified number of microseconds
@Input          ui32Timeus          the time to wait for, in microseconds
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVWaitus(IMG_UINT32 ui32Timeus);

/*************************************************************************/ /*!
@Function       PVRSRVGetCurrentProcessName
@Description    Returns name for current process
@Return         Name of current process
*/ /**************************************************************************/
IMG_EXPORT const IMG_CHAR *PVRSRVGetCurrentProcessName(void);

/*************************************************************************/ /*!
@Function       PVRSRVGetCurrentProcessID
@Description    Returns handle for current process
@Return         ID of current process
*/ /**************************************************************************/
IMG_EXPORT IMG_PID IMG_CALLCONV PVRSRVGetCurrentProcessID(void);

/*************************************************************************/ /*!
@Function       PVRSRVGetCurrentThreadName
@Description    Returns name for current thread or the name of the current
                process if running on the main thread, the string will be
                allocated on the heap and so \b must be freed with OSFreeMem()
@Return         Name of current thread
*/ /**************************************************************************/
IMG_EXPORT const IMG_CHAR *PVRSRVGetCurrentThreadName(void);

/*************************************************************************/ /*!
@Function       PVRSRVGetCurrentThreadID
@Description    Returns ID for current thread
@Return         ID of current thread
*****************************************************************************/
IMG_EXPORT uintptr_t PVRSRVGetCurrentThreadID(void);

/*************************************************************************/ /*!
@Function       PVRSRVSetCpuAffinity

@Description    Sets CPU affinity (wrapper for sched_setaffinity). This
                function binds execution of the current process to only one
                CPU/core. The CPU/core can be given either by the function
                argument or by the 'MetricsCpuAffinity' AppHint. AppHint takes
                priority over the function argument.

                If a non-existent CPU/core number is given, the function will
                return an error status.

                NB. If this function is used for metrics purposes it should be
                noted that if the measurement is performed on big.LITTLE ARM
                architecture (or similar) binding process to the slower core
                will affect measurement outcome.

@Input          ui32CpuId   The ID of the CPU
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVSetCpuAffinity(IMG_UINT32 ui32CpuId);

/*************************************************************************/ /*!
@Function       PVRSRVSetLocale
@Description    Thin wrapper on posix setlocale
@Input          pszLocale
@Return         String returned by setlocale
*/ /**************************************************************************/
IMG_EXPORT IMG_CHAR * IMG_CALLCONV PVRSRVSetLocale(const IMG_CHAR *pszLocale);


/*************************************************************************/ /*!
@Function       PVRSRVAllocUserModeMem
@Description    Allocate a block of user-mode memory
@Input          ui32Size    the amount of memory to allocate
@Return         A pointer to the memory allocated on success, otherwise NULL
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVAllocUserModeMem(size_t ui32Size);

/*************************************************************************/ /*!
@Function       PVRSRVCallocUserModeMem
@Description    Allocate a block of user-mode memory
@Input          ui32Size    the amount of memory to allocate
@Return         A pointer to the memory allocated on success, otherwise NULL
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVCallocUserModeMem(size_t ui32Size);

/*************************************************************************/ /*!
@Function       PVRSRVReallocUserModeMem
@Description    Re-allocate a block of memory
@Input          pvBase      the address of the existing memory, previously
                            allocated with PVRSRVAllocUserModeMem()
@Input          uNewSize    the newly-desired size of the memory chunk
@Return         On success, a pointer to the memory block. If the size of the
                      block could not be changed, NULL is returned.
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVReallocUserModeMem(void *pvBase, size_t uNewSize);
/*************************************************************************/ /*!
@Function       PVRSRVFreeUserModeMem
@Description    Free a block of memory previously allocated with
                PVRSRVAllocUserModeMem()
@Input          pvMem       pointer to the block of memory to be freed
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void IMG_CALLCONV PVRSRVFreeUserModeMem(void *pvMem);

/*************************************************************************/ /*!
@Function       PVRSRVMemCopy
@Description    Copy a block of memory
                Safe implementation of memset for use with device memory.
@Input          pvDst       Pointer to the destination
@Input          pvSrc       Pointer to the source location
@Input          uiSize      The amount of memory to copy in bytes
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVMemCopy(void *pvDst, const void *pvSrc, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVCachedMemCopy
@Description    Copy a block of memory between two cached memory allocations.
                For use only when source and destination are both cached
                memory allocations.
@Input          pvDst       Pointer to the destination
@Input          pvSrc       Pointer to the source location
@Input          uiSize      The amount of memory to copy in bytes
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVCachedMemCopy(void *pvDst, const void *pvSrc, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVDeviceMemCopy
@Description    Copy a block of memory to/from a device memory allocation.
                For use when one or both of the allocations is a device
                memory allocation.
@Input          pvDst       Pointer to the destination
@Input          pvSrc       Pointer to the source location
@Input          uiSize      The amount of memory to copy in bytes
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDeviceMemCopy(void *pvDst, const void *pvSrc, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVMemSet
@Description    Set all bytes in a region of memory to the specified value.
                Safe implementation of memset for use with device memory.
@Input          pvDest      Pointer to the start of the memory region
@Input          ui8Value    The value to be written
@Input          uiSize      The number of bytes to be set to ui8Value
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVMemSet(void *pvDest, IMG_UINT8 ui8Value, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVCachedMemSet
@Description    Set all bytes in a region of cached memory to the specified
                value. For use only when the destination is a cached memory
                allocation.
@Input          pvDest      Pointer to the start of the memory region
@Input          ui8Value    The value to be written
@Input          uiSize      The number of bytes to be set to ui8Value
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVCachedMemSet(void *pvDest, IMG_UINT8 ui8Value, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVDeviceMemSet
@Description    Set all bytes in a region of device memory to the specified
                value. The destination pointer should be a device memory
                buffer.
@Input          pvDest      Pointer to the start of the memory region
@Input          ui8Value    The value to be written
@Input          uiSize      The number of bytes to be set to ui8Value
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDeviceMemSet(void *pvDest, IMG_UINT8 ui8Value, size_t uiSize);

/*************************************************************************/ /*!
@Function       PVRSRVLockProcessGlobalMutex
@Description    Locking function for non-recursive coarse-grained mutex shared
                between all threads in a process.
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void IMG_CALLCONV PVRSRVLockProcessGlobalMutex(void);

/*************************************************************************/ /*!
@Function       PVRSRVUnlockProcessGlobalMutex
@Description    Unlocking function for non-recursive coarse-grained mutex shared
                between all threads in a process.
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void IMG_CALLCONV PVRSRVUnlockProcessGlobalMutex(void);

/*! Pointer type to opaque \_OS_MUTEX\_ structure. */
typedef struct _OS_MUTEX_ *PVRSRV_MUTEX_HANDLE;

/*************************************************************************/ /*!
@Function       PVRSRVCreateMutex
@Description    creates a mutex
@Output         phMutex     ptr to mutex handle
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code.
*/ /**************************************************************************/
#if !defined(PVR_DEBUG_MUTEXES)
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVCreateMutex(PVRSRV_MUTEX_HANDLE *phMutex);
#else
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVCreateMutex(PVRSRV_MUTEX_HANDLE *phMutex,
                                                       IMG_CHAR pszMutexName[],
                                                       IMG_CHAR pszFilename[],
                                                       IMG_INT iLine);
#define PVRSRVCreateMutex(phMutex) \
    PVRSRVCreateMutex(phMutex, #phMutex, __FILE__, __LINE__)
#endif

/*************************************************************************/ /*!
@Function       PVRSRVDestroyMutex
@Description    Create a mutex.
@Input          hMutex      On success, filled with the new mutex
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
 */ /**********************************************************************/
#if !defined(PVR_DEBUG_MUTEXES)
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVDestroyMutex(PVRSRV_MUTEX_HANDLE hMutex);
#else
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVDestroyMutex(PVRSRV_MUTEX_HANDLE hMutex,
                                                        IMG_CHAR pszMutexName[],
                                                        IMG_CHAR pszFilename[],
                                                        IMG_INT iLine);
#define PVRSRVDestroyMutex(hMutex) \
    PVRSRVDestroyMutex(hMutex, #hMutex, __FILE__, __LINE__)
#endif

/*************************************************************************/ /*!
@Function       PVRSRVLockMutex
@Description    Lock the mutex passed
@Input          hMutex      handle of the mutex to be locked
@Return         None
 */ /**********************************************************************/
#if !defined(PVR_DEBUG_MUTEXES)
IMG_EXPORT void IMG_CALLCONV PVRSRVLockMutex(PVRSRV_MUTEX_HANDLE hMutex);
#else
IMG_EXPORT void IMG_CALLCONV PVRSRVLockMutex(PVRSRV_MUTEX_HANDLE hMutex,
                                             IMG_CHAR pszMutexName[],
                                             IMG_CHAR pszFilename[],
                                             IMG_INT iLine);
#define PVRSRVLockMutex(hMutex) \
    PVRSRVLockMutex(hMutex, #hMutex, __FILE__, __LINE__)
#endif

/*************************************************************************/ /*!
@Function       PVRSRVUnlockMutex
@Description    Unlock the mutex passed
@Input          hMutex      handle of the mutex to be unlocked
@Return         None
 */ /**********************************************************************/
#if !defined(PVR_DEBUG_MUTEXES)
IMG_EXPORT void IMG_CALLCONV PVRSRVUnlockMutex(PVRSRV_MUTEX_HANDLE hMutex);
#else
IMG_EXPORT void IMG_CALLCONV PVRSRVUnlockMutex(PVRSRV_MUTEX_HANDLE hMutex,
                                               IMG_CHAR pszMutexName[],
                                               IMG_CHAR pszFilename[],
                                               IMG_INT iLine);
#define PVRSRVUnlockMutex(hMutex) \
    PVRSRVUnlockMutex(hMutex, #hMutex, __FILE__, __LINE__)
#endif

/* Non-exported APIs */
#if defined(DEBUG) && (defined(__linux__) || defined(_WIN32) || defined(__QNXNTO__) || defined(INTEGRITY_OS))
/*************************************************************************/ /*!
@Function       PVRSRVAllocUserModeMemTracking
@Description    Wrapper function for malloc, used for memory-leak detection
@Input          ui32Size        number of bytes to be allocated
@Input          pszFileName     filename of the calling code
@Input          ui32LineNumber  line number of the calling code
@Return         A pointer to the memory allocated on success, otherwise NULL
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVAllocUserModeMemTracking(size_t ui32Size,
                                                              const IMG_CHAR *pszFileName,
                                                              IMG_UINT32 ui32LineNumber);

/*************************************************************************/ /*!
@Function       PVRSRVCallocUserModeMemTracking
@Description    Wrapper function for calloc, used for memory-leak detection
@Input          ui32Size        number of bytes to be allocated
@Input          pszFileName     filename of the calling code
@Input          ui32LineNumber  line number of the calling code
@Return         A pointer to the memory allocated on success, otherwise NULL
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVCallocUserModeMemTracking(size_t ui32Size,
                                                               const IMG_CHAR *pszFileName,
                                                               IMG_UINT32 ui32LineNumber);

/*************************************************************************/ /*!
@Function       PVRSRVFreeUserModeMemTracking
@Description    Wrapper for free - see PVRSRVAllocUserModeMemTracking()
@Input          pvMem       pointer to the memory to be freed
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void  IMG_CALLCONV PVRSRVFreeUserModeMemTracking(void *pvMem);

/*************************************************************************/ /*!
@Function       PVRSRVReallocUserModeMemTracking
@Description    Wrapper for realloc, used in memory-leak detection
@Input          pvMem           pointer to the existing memory block
@Input          ui32NewSize     the desired new size of the block
@Input          pszFileName     the filename of the calling code
@Input          ui32LineNumber  the line number of the calling code
@Return         On success, a pointer to the memory block. This may not
                      necessarily be the same location as the block was at
                      before the call. On failure, NULL is returned.
*/ /**************************************************************************/
IMG_EXPORT void * IMG_CALLCONV PVRSRVReallocUserModeMemTracking(void *pvMem,
                                                                size_t ui32NewSize,
                                                                const IMG_CHAR *pszFileName,
                                                                IMG_UINT32 ui32LineNumber);
#endif /* defined(DEBUG) && (defined(__linux__) || defined(_WIN32)) */

/*************************************************************************/ /*!
@Function       PVRSRVDumpDebugInfo
@Description    Dump debug information to kernel log
@Input          psConnection    Services connection
@Input          ui32VerbLevel   Specifies the level of verbosity, which can
                                be used to filter out less critical debug
                                messages
@Return         None
*/ /**************************************************************************/
IMG_EXPORT void
PVRSRVDumpDebugInfo(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32VerbLevel);

#if defined(__cplusplus)
}
#endif
#endif /* SERVICES_CLIENT_PORTING_H */

/******************************************************************************
 End of file (services_client_porting.h)
******************************************************************************/
