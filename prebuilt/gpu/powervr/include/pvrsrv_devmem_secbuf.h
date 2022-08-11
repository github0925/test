/*************************************************************************/ /*!
@File
@Title          Device Memory Management secure
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of device memory management -- This
                file defines the exposed Services API to the secure memory
                management functions.
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(PVRSRV_DEVMEM_SECBUF)
#define PVRSRV_DEVMEM_SECBUF

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "pvrsrv_devmem.h"

/**************************************************************************/ /*!
@Function       PVRSRVAllocSecureBuffer
@Description    Allocate secure device memory.
                The physical memory will come from the OS wide secure allocator
                and can only be accessed by privileged processes.
                It is not allowed to map this memory into the CPU.
                The physical memory can have a sparse mapping or just be a
                normal continuous allocation.

                To get a non-sparse mapping use the following parameters:
                    uiChunkSize = uiSize
                    ui32NumPhysChunks = 1
                    ui32NumVirtChunks = 1
                    pui32MappingTable[0] = 0

                Free with PVRSRVFreeDeviceMem

@Input          hCtx                Memory context that the memory should be
                                    allocated for
@Input          uiSize              The amount of memory to be allocated
@Input          uiChunkSize         uiSize is divided into equal chunks
                                    with a size of uiChunkSize. uiSize must be
                                    a multiple of uiChunkSize. uiChunkSize
                                    should be equal to or a multiple of the OS
                                    page size.
@Input          ui32NumPhysChunks   The number of chunks that will have a
                                    physical backing
@Input          ui32NumVirtChunks   The number of chunks that will form the
                                    virtual allocation space:
                                       uiChunkSize*ui32NumVirtChunks == uiSize
@Input          pui32MappingTable   An array of indices of length
                                    ui32NumPhysChunks that indicates to which
                                    virtual chunk a physical chunk is being
                                    mapped.
@Input          uiFlags             Allocation flags, no CPU flags allowed.
@Input          pszName             Allocation descriptive name, this will
                                    be truncated to the number of characters
                                    specified in the PVR_ANNOTATION_MAX_LEN.
@Output         phMemDescPtr        On success, a PVRSRV_MEMDESC is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocSecureBuffer(PVRSRV_DEVMEMCTX hCtx,
						IMG_DEVMEM_SIZE_T uiSize,
						IMG_DEVMEM_SIZE_T uiChunkSize,
						IMG_UINT32 ui32NumPhysChunks,
						IMG_UINT32 ui32NumVirtChunks,
						IMG_UINT32 *pui32MappingTable,
						PVRSRV_MEMALLOCFLAGS_T uiFlags,
						const IMG_CHAR *pszName,
						PVRSRV_MEMDESC *phMemDescPtr);

#endif	/* !defined(PVRSRV_DEVMEM_SECBUF) */
