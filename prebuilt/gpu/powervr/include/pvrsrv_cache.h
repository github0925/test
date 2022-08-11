/*************************************************************************/ /*!
@File
@Title          Services cache management interface
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side interface for cache management
@License        Strictly Confidential.
*/ /**************************************************************************/
#include "services.h"
#include "cache_ops.h"
#include "pvrsrv_devmem.h"
#include "pvrsrv_devmemx.h"
#include "pvrsrv_sync_um.h"

#ifndef _PVRSRV_CACHE_H_
#define _PVRSRV_CACHE_H_

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpExec
@Description    Execute an array of cache operations (batchless interface).

@Input          psDevConnection          Device connection
@Input          phMemDesc                The phMemDesc holding the memory that
                                         is going to be manipulated.
@Input          phPhysDesc               The physical MemDesc holding the memory
										 that is going to be manipulated. If
										 this is set to non-NULL, it will be
										 used instead of hMemDesc.
@Input          puiOffset                The offset inside the descriptor to the
                                         addresses.
@Input          puiSize                  The range from offset + size will be
                                         handled.
@Input          eCacheOp                 The operation that should be executed
                                         on the defined address range.
@Input          hCacheOpTimeline         Associated timeline

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpExec(PVRSRV_DEV_CONNECTION *psDevConnection,
				  PVRSRV_MEMDESC *phMemDesc,
				  PVRSRV_MEMDESC_PHYSICAL *phPhysDesc,
				  IMG_DEVMEM_OFFSET_T *puiOffset,
				  IMG_DEVMEM_SIZE_T *puiSize,
				  PVRSRV_CACHE_OP *peCacheOp,
				  IMG_UINT32 ui32Count,
				  PVRSRV_TIMELINE hCacheOpTimeline);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchCreate
@Description    Creates a batch for cache operations. This batch instance should
                not be shared across multiple threads as state will be corrupted.
                For a batch, operations are added using PVRSRVCacheOpBatchAdd()
                and are executed together using PVRSRVCacheOpBatchExec().
                Call PVRSRVCacheOpBatchStart() to reset the batch for further use
                after calling PVRSRVCacheOpBatchExec() but before the first call
                PVRSRVCacheOpBatchAdd().
                Call PVRSRVCacheOpBatchDestroy() to finally destroy the batch if
                it won't be in use anymore

@Input          psDevConnection          Device connection
@Input          phBatch                  A handle to a batch that is going to
                                         be created.

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpBatchCreate(PVRSRV_DEV_CONNECTION *psDevConnection, IMG_HANDLE *phBatch);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchStart
@Description    Prepares the batch for another execution. Call this first before
                adding new bunch of cache operations with PVRSRVCacheOpBatchAdd().
                NOTE: PVRSRVCacheOpBatchStart() is NOT multi-threaded safe and/or
				is NOT re-entrant using the same batch handle with these funcions
				PVRSRVCacheOpBatch[Start,Add,Exec]().

@Input          hBatch                   A handle to the batch

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpBatchStart(IMG_HANDLE hBatch);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchAdd
@Description    Adds a new operation to the batch that is going to be executed
                on PVRSRVCacheOpBatchExec together with the others.
                NOTE: PVRSRVCacheOpBatchAdd() is NOT multi-threaded safe and/or
				is NOT re-entrant using the same batch handle with these funcions
				PVRSRVCacheOpBatch[Start,Add,Exec]().

@Input          hBatch                   A handle to the batch
@Input          hMemDesc                 The MemDesc holding the memory that
                                         is going to be manipulated.
@Input          uiOffset                 The offset inside the MemDesc to the
                                         addresses..
@Input          uiSize                   The range from offset + size will be
                                         handled.
@Input          eCacheOp                 The operation that should be executed
                                         on the defined address range.

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpBatchAdd(IMG_HANDLE hBatch,
					  PVRSRV_MEMDESC hMemDesc,
					  IMG_DEVMEM_OFFSET_T uiOffset,
					  IMG_DEVMEM_SIZE_T uiSize,
					  PVRSRV_CACHE_OP eCacheOp);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchAddPhysMem
@Description    Adds a new operation to the batch that is going to be executed
                on PVRSRVCacheOpBatchExec together with the others.
                NOTE: PVRSRVCacheOpBatchAddPhysMem() is NOT multi-threaded safe and/or
		is NOT re-entrant using the same batch handle with these funcions
		PVRSRVCacheOpBatch[Start,Add,Exec]().

@Input          hBatch                   A handle to the batch
@Input          hPhysDesc                The physical MemDesc holding the memory
@Input          uiOffset                 The offset inside the MemDesc to the
                                         addresses..
@Input          uiSize                   The range from offset + size will be
                                         handled.
@Input          eCacheOp                 The operation that should be executed
                                         on the defined address range.

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpBatchAddPhysMem(IMG_HANDLE hBatch,
					  PVRSRV_MEMDESC_PHYSICAL hPhysDesc,
					  IMG_DEVMEM_OFFSET_T uiOffset,
					  IMG_DEVMEM_SIZE_T uiSize,
					  PVRSRV_CACHE_OP eCacheOp);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchExec
@Description    Executes the operations in the batch.
                NOTE: PVRSRVCacheOpBatchExec() is NOT multi-threaded safe and/or
				is NOT re-entrant using the same batch handle with these funcions
				PVRSRVCacheOpBatch[Start,Add,Exec]().

@Input          hBatch                   A handle to the batch
@Input          hCacheOpTimeline         Associated timeline

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCacheOpBatchExec(IMG_HANDLE hBatch, PVRSRV_TIMELINE hCacheOpTimeline);

/*************************************************************************/ /*!
@Function       PVRSRVCacheOpBatchDestroy
@Description    Empties batch and frees the memory.

@Input          hBatch                   A handle to the batch

@Return         Standard PVRSRV_ERROR error code.
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVCacheOpBatchDestroy(IMG_HANDLE hBatch);

#endif	/* _PVRSRV_CACHE_H_ */
