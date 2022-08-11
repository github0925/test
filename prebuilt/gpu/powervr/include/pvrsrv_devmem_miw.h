/*************************************************************************/ /*!
@File
@Title          Device Memory Management
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of device memory management -- This
                file defines the library wrapper above the exposed
                Services API, for the "meminfo" holding structure.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_DEVMEM_MIW_H
#define PVRSRV_DEVMEM_MIW_H

#if defined __cplusplus
extern "C" {
#endif

#include <powervr/mem_types.h>

#include "img_types.h"
#include "img_defs.h"
#include "devicemem_typedefs.h"
#include "pdumpdefs.h"
#include "pvrsrv_devmem.h"
#include "pvrsrv_error.h"
#include "pvrsrv_memallocflags.h"
#include "lock_types.h"
#include "services_km.h" /* for PVRSRV_DEV_CONNECTION */

/*!
	This data structure is for use by the users of this module,
	i.e. client drivers.  The memory management implementation below
	this API will not use it.  In this shim, we populate the fields as
	required in the call to PVRSRVAllocDeviceMem().
*/
typedef struct PVRSRV_MEMINFO_TAG
{
	/*! Memory "descriptor" - this is the handle (opaque to callers of
		memory mgt i/f) that is for use inside the mem mgt stack */
	PVRSRV_MEMDESC hMemDesc;

	/*! GPU virt addr, where applicable */
	IMG_DEV_VIRTADDR sDevVirtAddr;

	/*! Store a copy of the allocation size.  Redundant in the case of
		dev mem allocation as the caller would know it, but it is
		convenient to have a copy of it here, and is useful in the case
		of mapping imported memory. */
	IMG_DEVMEM_SIZE_T uiAllocationSize;

	/*! CPU virt addr, where applicable, NULL where cpu virt addr
		is not required. */
	void *pvCpuVirtAddr;

	/*! A copy of the allocation flags.  This is for use by this
		wrapper layer.  The wrapper will make certain assumptions when
		freeing the allocation.  We trust the caller not to meddle with
		these flags. */
	PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags;
} PVRSRV_MEMINFO;

/*
 *
 *  API functions
 *
 */

/**************************************************************************/ /*!
@Function       PVRSRVChangeSparseDeviceMemMIW
@Description    Resize the physical backing of sparse memory allocated before.
                This function either allocates/deallocates the physical backing
                depending on the arguments.

@Input          psMemInfo           The memory allocation to be resized
@Input          ui32AllocPageCount  Count in client chunk size to be allocated
                                    0 if none.
@Input          pai32AllocIndices   Indices of allocations to be done into mem map
                                    NULL if none.
@Input          ui32FreePageCount   Count in client chunk size to be freed.
                                    0 if none.
@Input          pai32FreeIndices    Indices of chunks to be freed into mem map
                                    NULL if none.
@Input          uiFlags             Flags indicating the operation. free/alloc/both
@Return         PVRSRV_ERROR        Error status of the call.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR
PVRSRVChangeSparseDeviceMemMIW(PVRSRV_MEMINFO *psMemInfo,
                               IMG_UINT32 ui32AllocPageCount,
                               IMG_UINT32 *pai32AllocIndices,
                               IMG_UINT32 ui32FreePageCount,
                               IMG_UINT32 *pai32FreeIndices,
                               SPARSE_MEM_RESIZE_FLAGS uiFlags);


/**************************************************************************/ /*!
@Function       PVRSRVMapDeviceClassMemoryMIW
@Description    Map the given Device Class buffer to the specified heap.  We
                allocate a meminfo on behalf of the caller, and pass the call
                down to PVRSRVMapDeviceClassMemory() to do the business.
@Input          hHeap               The heap from which memory will be mapped
@Input          hDCBuffer           The Device Class Buffer to be mapped
@Input          uiMemAllocFlags     Mem Alloc Flags
@Output         ppsMemInfoOut       On success, the returned PVRSRV_MEMINFO for
                                    the newly-mapped Device Class buffer
@Input          pszAnnotation       Annotation string for this allocation/import
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVMapExternalMemoryMIW(const PVRSRV_DEVMEMCTX psContext,
						   PVRSRV_HEAP hHeap,
                           IMG_HANDLE hDCBuffer,
						   PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
						   PVRSRV_MEMINFO **ppsMemInfoOut,
						   const IMG_CHAR *pszAnnotation);

/**************************************************************************/ /*!
@Function       PVRSRVUnmapDeviceClassMemoryMIW
@Description    Unmap the DC buffer represented by the memory descriptor.  The
                mem info structure will be freed.
@Input          psMemInfo           The PVRSRV_MEMINFO for the memory which is
                                    to be unmapped
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVUnmapExternalMemoryMIW(PVRSRV_MEMINFO *psMemInfo);

/**************************************************************************/ /*!
@Function       PVRSRVSubAllocDeviceMemMIW
@Description    Allocate memory from the specified heap, acquiring physical
                memory from OS as we go, and mapping this into the GPU
                (required) and CPU (optional).

                We allocate a meminfo on behalf of the caller and populate it
                with cpu virtual address (or NULL if absent), gpu virtual
                address, and size.
@Input          uiPreAllocMultiplier Size factor for internal pre-allocation of
                                     memory to make subsequent calls with the
                                     same flags faster. Independently if a value
                                     is set, the function will try to allocate
                                     from any pre-allocated memory first and -if
                                     successful- not pre-allocate anything more.
                                     That means the factor can always be set and
                                     the correct thing will be done internally.
@Input          hHeap                The heap from which memory will be allocated
@Input          uiSize               The amount of memory to be allocated
@Input          uiAlign              The required alignment for the memory that
                                     will be allocated
@Input          uiMemAllocFlags      Mem Alloc Flags
@Input          pszText              Text describing the allocation
@Output         ppsMemInfoOut        On success, a PVRSRV_MEMINFO is returned,
                                     representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVSubAllocDeviceMemMIW(IMG_UINT8 uiPreAllocMultiplier,
                           PVRSRV_HEAP hHeap,
                           IMG_DEVMEM_SIZE_T uiSize,
                           IMG_DEVMEM_ALIGN_T uiAlign,
                           PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
                           const IMG_CHAR *pszText,
                           PVRSRV_MEMINFO **ppsMemInfoOut);


#define PVRSRVAllocDeviceMemMIW(...) \
    PVRSRVSubAllocDeviceMemMIW(PVRSRV_DEVMEM_PRE_ALLOC_MULTIPLIER_NONE, __VA_ARGS__)

/**************************************************************************/ /*!
@Function       PVRSRVAllocSparseDevMemMIW2
@Description    Allocate sparse memory without mapping into device memory context.
                Sparse memory is used where you have an allocation that has a
                logical size (i.e. the amount of VM space it will need when
                mapping it into a device) that is larger then the amount of
                physical memory that allocation will use.

                We allocate a meminfo on behalf of the caller and populate it
                with cpu virtual address (or NULL if absent), gpu virtual
                address, and size.

                Size must be a positive integer multiple of the page size
@Input          psDevConnection     Device to allocation the memory for
@Input          hHeap               The heap from which memory will be allocated
@Input          uiSize              The logical size of allocation
@Input          uiChunkSize         The size of the chunk
@Input          ui32NumPhysChunks   The number of physical chunks required
@Input          ui32NumVirtChunks   The number of virtual chunks required
@Input			pui32MappingTable	index based Mapping table of size of
									no. of physical chunks
@Input          uiAlign             The required alignment for the memory that
                                    will be allocated
@Input          uiMemAllocFlags     Allocation flags
@Input          pszText             Text describing the allocation
@Output         ppsMemInfoOut       On success, a PVRSRV_MEMINFO is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocSparseDeviceMemMIW2(const PVRSRV_DEVMEMCTX psDevMemCtx,
							  PVRSRV_HEAP hHeap,
							  IMG_DEVMEM_SIZE_T uiSize,
							  IMG_DEVMEM_SIZE_T uiChunkSize,
							  IMG_UINT32 ui32NumPhysChunks,
							  IMG_UINT32 ui32NumVirtChunks,
							  IMG_UINT32 *pui32MappingTable,
							  IMG_DEVMEM_ALIGN_T uiAlign,
							  DEVMEM_FLAGS_T uiMemAllocFlags,
							  const IMG_CHAR *pszText,
							  PVRSRV_MEMINFO **ppsMemInfoOut);

/**************************************************************************/ /*!
@Function       PVRSRVAllocSparseDevMemMIW (DEPRECATED)
@Description    Allocate sparse memory without mapping into device memory context.
                Sparse memory is used where you have an allocation that has a
                logical size (i.e. the amount of VM space it will need when
                mapping it into a device) that is larger then the amount of
                physical memory that allocation will use.

                We allocate a meminfo on behalf of the caller and populate it
                with cpu virtual address (or NULL if absent), gpu virtual
                address, and size.

                Size must be a positive integer multiple of the page size
@Input          psDevConnection     Device to allocation the memory for
@Input          hHeap               The heap from which memory will be allocated
@Input          uiSize              The logical size of allocation
@Input          uiChunkSize         The size of the chunk
@Input          ui32NumPhysChunks   The number of physical chunks required
@Input          ui32NumVirtChunks   The number of virtual chunks required
@Input			pabMappingTable		boolean based Mapping table
@Input          uiAlign             The required alignment for the memory that
                                    will be allocated
@Input          uiMemAllocFlags     Allocation flags
@Input          pszText             Text describing the allocation
@Output         ppsMemInfoOut       On success, a PVRSRV_MEMINFO is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocSparseDeviceMemMIW(const PVRSRV_DEVMEMCTX psDevMemCtx,
							  PVRSRV_HEAP hHeap,
							  IMG_DEVMEM_SIZE_T uiSize,
							  IMG_DEVMEM_SIZE_T uiChunkSize,
							  IMG_UINT32 ui32NumPhysChunks,
							  IMG_UINT32 ui32NumVirtChunks,
							  IMG_BOOL *pabMappingTable,
							  IMG_DEVMEM_ALIGN_T uiAlign,
							  DEVMEM_FLAGS_T uiMemAllocFlags,
							  const IMG_CHAR *pszText,
							  PVRSRV_MEMINFO **ppsMemInfoOut);

/**************************************************************************/ /*!
@Function       PVRSRVAllocExportableDeviceMemMIW
@Description    Allocate memory from the specified heap that can be exported to
                other processes, acquiring physical  memory from OS as we go,
                and mapping this into the GPU (required) and CPU (optional).

                We allocate a meminfo on behalf of the caller and populate it
                with cpu virtual address (or NULL if absent), gpu virtual
                address, and size.
@Input          psDevConnection     Device that the memory should be allocated
                                    from
@Input          hHeap               The heap from which memory will be allocated
@Input          uiSize              The amount of memory to be allocated
@Input          uiAlign             The required alignment for the memory that
                                    will be allocated
@Input          uiMemAllocFlags     Mem Alloc Flags
@Input          pszText             Text describing the allocation
@Output         ppsMemInfoOut       On success, a PVRSRV_MEMINFO is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocExportableDeviceMemMIW(PVRSRV_DEV_CONNECTION *psDevConnection,
								  PVRSRV_HEAP hHeap,
								  IMG_DEVMEM_SIZE_T uiSize,
								  IMG_DEVMEM_ALIGN_T uiAlign,
								  PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
								  const IMG_CHAR *pszText,
								  PVRSRV_MEMINFO **ppsMemInfoOut);

/**************************************************************************/ /*!
@Function       PVRSRVFreeDeviceMemMIW
@Description    Free that allocated by PVRSRVSubAllocDeviceMemMIW,
                PVRSRVAllocExportableDeviceMemMIW or PVRSRVAllocSparseDeviceMemMIW
                (meminfo will be freed)
@Input          psMemInfo           Ptr to the PVRSRV_MEMINFO which describes
                                    the memory to be freed
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVFreeDeviceMemMIW(PVRSRV_MEMINFO *psMemInfo);

/**************************************************************************/ /*!
@Function       PVRSRVAllocSecureDeviceMemMIW
@Description    Allocate secure memory from the specified heap. The physical
                memory will come from the OS wide secure allocator and can only
                be accessed by privileged processes.
                It is not allowed to map this memory into the CPU.
                The physical memory can have a sparse mapping or just be a
                normal continuous allocation.

                To get a non-sparse mapping use the following parameters:
                    uiChunkSize = uiSize
                    ui32NumPhysChunks = 1
                    ui32NumVirtChunks = 1
                    pui32MappingTable[0] = 0

                Free with PVRSRVFreeDeviceMemMIW

@Input          hCtx                Memory context that the memory should be
                                    allocated for
@Input          hHeap               The heap from which memory will be allocated
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
@Input          pabMappingTable     An array of booleans of length
                                    ui32NumVirtChunks that indicates whether
                                    the virtual chunks is backed by a physical
                                    chunk (IMG_TRUE) or not (IMG_FALSE)
@Input          uiFlags             Allocation flags, no CPU flags allowed.
@Input          pszName             Text describing the allocation
@Output         ppsMemInfoOut       On success, a PVRSRV_MEMINFO is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocSecureDeviceMemMIW(PVRSRV_DEVMEMCTX hCtx,
                              PVRSRV_HEAP hHeap,
                              IMG_DEVMEM_SIZE_T uiSize,
                              IMG_DEVMEM_SIZE_T uiChunkSize,
                              IMG_UINT32 ui32NumPhysChunks,
                              IMG_UINT32 ui32NumVirtChunks,
                              IMG_BOOL *pabMappingTable,
                              PVRSRV_MEMALLOCFLAGS_T uiFlags,
                              const IMG_CHAR *pszName,
                              PVRSRV_MEMINFO **ppsMemInfoOut);

/**************************************************************************/ /*!
@Function       PVRSRVAllocSecureDeviceMemMIW2
@Description    Same as PVRSRVAllocSecureDeviceMemMIW but taking a potentially
                smaller mapping table based on indices rather than booleans.

@Input          hCtx                Memory context that the memory should be
                                    allocated for
@Input          hHeap               The heap from which memory will be allocated
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
@Input          pszName             Text describing the allocation
@Output         ppsMemInfoOut       On success, a PVRSRV_MEMINFO is returned,
                                    representing the new allocation
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAllocSecureDeviceMemMIW2(PVRSRV_DEVMEMCTX hCtx,
                               PVRSRV_HEAP hHeap,
                               IMG_DEVMEM_SIZE_T uiSize,
                               IMG_DEVMEM_SIZE_T uiChunkSize,
                               IMG_UINT32 ui32NumPhysChunks,
                               IMG_UINT32 ui32NumVirtChunks,
                               IMG_UINT32 *pui32MappingTable,
                               PVRSRV_MEMALLOCFLAGS_T uiFlags,
                               const IMG_CHAR *pszName,
                               PVRSRV_MEMINFO **ppsMemInfoOut);

#if defined(PDUMP)
/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemLoadMemMIW
@Description    writes the current contents of the referenced memory to the
                PDump PARAMS stream, and emits an LDB to to the script stream to
                load these bytes to the memory when the pdump script is played
                back
@Input          psMemInfo           The PVRSRV_MEMINFO representing the memory
                                    to be PDumped
@Input          uiOffset            Offset into the memory region at which
                                    dumping to the PDump stream is to occur
@Input          uiSize              Size of the area to be dumped to the PDump
                                    stream
@Input          uiPDumpFlags        PDump flags
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVPDumpDevmemLoadMemMIW(const PVRSRV_MEMINFO *psMemInfo,
							IMG_DEVMEM_OFFSET_T uiOffset,
							IMG_DEVMEM_SIZE_T uiSize,
							IMG_UINT32 uiPDumpFlags);
/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemPol32
@Description    Emits a PDUMP POL command relative to the underlying PMR

                This version can only poll on 32-bit memory locations.
@Input          psMemInfo
@Input          ui32Offset
@Input          ui32Value
@Input          ui32Mask
@Input          eOperator
@Input          ui32PDumpFlags
@Return
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVPDumpDevmemPol32MIW(PVRSRV_MEMINFO *psMemInfo,
						  IMG_DEVMEM_OFFSET_T ui32Offset,
						  IMG_UINT32 ui32Value,
						  IMG_UINT32 ui32Mask,
						  PDUMP_POLL_OPERATOR eOperator,
						  IMG_UINT32 ui32PDumpFlags);
#else	/* PDUMP */

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemLoadMemMIW)
#endif
static INLINE void
PVRSRVPDumpDevmemLoadMemMIW(const PVRSRV_MEMINFO *psMemInfo,
							IMG_DEVMEM_OFFSET_T uiOffset,
							IMG_DEVMEM_SIZE_T uiSize,
							IMG_UINT32 uiPDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psMemInfo);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(uiSize);
	PVR_UNREFERENCED_PARAMETER(uiPDumpFlags);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemPol32MIW)
#endif
static INLINE void
PVRSRVPDumpDevmemPol32MIW(PVRSRV_MEMINFO *psMemInfo,
						  IMG_DEVMEM_OFFSET_T ui32Offset,
						  IMG_UINT32 ui32Value,
						  IMG_UINT32 ui32Mask,
						  PDUMP_POLL_OPERATOR eOperator,
						  IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psMemInfo);
	PVR_UNREFERENCED_PARAMETER(ui32Offset);
	PVR_UNREFERENCED_PARAMETER(ui32Value);
	PVR_UNREFERENCED_PARAMETER(ui32Mask);
	PVR_UNREFERENCED_PARAMETER(eOperator);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
}
#endif	/* PDUMP */

/**************************************************************************/ /*!
@Function       PVRSRVAcquireCPUMappingMIW
@Description    Maps the specified PVRSRV_MEMINFO into CPU address space and
                returns the CPU-virtual address pointing to this mapping.
@Input          psMemInfo           The PVRSRV_MEMINFO to be mapped into CPU
                                    virtual address space.
@Output         ppvCpuVirtAddrOut   The CPU-virtual address pointing to this
                                    mapping.
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAcquireCPUMappingMIW(const PVRSRV_MEMINFO *psMemInfo,
						   void **ppvCpuVirtAddrOut);

/**************************************************************************/ /*!
@Function       PVRSRVReleaseCPUMappingMIW
@Description    Unmaps the specified PVRSRV_MEMINFO from CPU address space.
@Input          psMemInfo           The PVRSRV_MEMINFO to be unmapped from CPU
                                    virtual address space.
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVReleaseCPUMappingMIW(const PVRSRV_MEMINFO *psMemInfo);

#if defined __cplusplus
};
#endif
#endif /* #ifndef PVRSRV_DEVMEM_MIW_H */
