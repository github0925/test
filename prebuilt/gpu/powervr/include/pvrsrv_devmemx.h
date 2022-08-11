/*************************************************************************/ /*!
@File
@Title          X Device Memory Management core
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of extended device memory management.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_DEVMEMX_H
#define PVRSRV_DEVMEMX_H

#if defined __cplusplus
extern "C" {
#endif

#include "pvrsrv_devmem.h"
#include "img_types.h"
#include "img_defs.h"
#include "devicemem_typedefs.h"
#include "pvrsrv_error.h"
#include "pvrsrv_memallocflags.h"
#include "services.h"	/* For PVRSRV_DEV_CONNECTION */

typedef DEVMEMX_PHYSDESC *PVRSRV_MEMDESC_PHYSICAL;
typedef DEVMEMX_VIRTDESC *PVRSRV_MEMDESC_VIRTUALRANGE;


/**************************************************************************/ /*!
@Function       PVRSRVDevMemXAllocPhysical
@Description    Allocate physical device memory.
                Function will create a separate physical allocation and pass
                back a handle for it. This handle may be used to map the
                allocation to CPU or a specific device virtual range that
                was allocated with PVRSRVDevMemXAllocVirtualRange().


                Valid flags to pass in: PVRSRV_MEMALLOCFLAGS_DEVMEMX_PHYSICAL_MASK

@Input          hCtx                DevMem context to allocate from
@Input          uiLog2PageSize      The log2 of the required page-size/contiguity
@Input          uiNumPages          The allocation size in number of heap pages
@Input          uiMemAllocFlags     Physical allocation flags
@Input          pszText             Text to describe the allocation
@Output         hMemDescPhys        Handle for this allocation

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXAllocPhysical(PVRSRV_DEVMEMCTX hCtx,
                           IMG_UINT32 uiLog2PageSize,
                           IMG_UINT32 uiNumPages,
                           PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
                           const IMG_CHAR *pszText,
                           PVRSRV_MEMDESC_PHYSICAL *hMemDescPhys);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXReleasePhysical
@Description    Frees an allocation done with PVRSRVDevMemXAllocPhysical().
                To make sure the physical memory is actually freed all CPU
                and device mappings have to be removed through calls
                to PVRSRVDevMemXUnmapPhysicalToCPU() and
                PVRSRVDevMemXUnmapVirtualRange(). The allocation will exist as long
                as there are valid mappings for it but will be destroyed
                immediately after the last reference was dropped once free has
                been called.

@Input          hMemDescPhys       Physical allocation to free

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXReleasePhysical(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys);

/**************************************************************************/ /*!
@Function       PVRSRVAllocSecureBufferPhysical
@Description    Allocate physical secure device memory.
                Function will create a separate physical allocation and pass
                back a handle for it. This handle may be used to map the
                allocation to CPU or a specific device virtual range that
                was allocated with PVRSRVDevMemXAllocVirtualRange().


                Valid flags to pass in: PVRSRV_MEMALLOCFLAGS_DEVMEMX_PHYSICAL_MASK

@Input          hCtx                DevMem context to allocate from
@Input          uiLog2PageSize      The log2 of the required page-size/contiguity
@Input          uiNumPages          The allocation size in number of heap pages
@Input          uiMemAllocFlags     Physical allocation flags
@Output         hMemDescPhys        Handle for this allocation
@Output         pui64SecBufHandle   Fd for this allocation

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXAllocSecureBufferPhysical(PVRSRV_DEVMEMCTX hCtx,
                        IMG_UINT32 uiLog2PageSize,
                        IMG_UINT32 uiNumPages,
                        PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
                        const IMG_CHAR *pszText,
                        PVRSRV_MEMDESC_PHYSICAL *phMemDescPhys,
                        IMG_UINT64 *pui64SecBufHandle);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXReleaseSecureBufferPhysical
@Description    Frees an allocation done with PVRSRVDevMemXAllocSecureBufferPhysical().
                To make sure the physical memory is actually freed all CPU
                and device mappings have to be removed through calls
                to PVRSRVDevMemXUnmapPhysicalToCPU() and
                PVRSRVDevMemXUnmapVirtualRange(). The allocation will exist as long
                as there are valid mappings for it but will be destroyed
                immediately after the last reference was dropped once free has
                been called.

@Input          hMemDescPhys       Physical allocation to free
@Input          iFd                Fd of secured import allocation to free

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXReleaseSecureBufferPhysical(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys, IMG_UINT64 ui64SecBufHandle);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXAllocVirtualRange
@Description    Allocates a range in the device virtual space of a heap.
                Physical allocations done with PVRSRVDevMemXAllocPhysical() can
                be mapped partially or completely into this range.

                Valid flags to pass in: PVRSRV_MEMALLOCFLAGS_DEVMEMX_VIRTUAL_MASK

@Input          hHeap               The heap to allocate the virtual range from
@Input          uiNumPages          The allocation size in number of pages the
                                    page size is determined from the passed heap
@Input          uiMemAllocFlags     Virtual allocation flags
@Input          pszText             A description for the allocation
@Output         hMemDescVirt        Handle for this allocation
@Output         psVirtAddr          Device virtual address that was allocated

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXAllocVirtualRange(PVRSRV_HEAP hHeap,
                               IMG_UINT32 uiNumPages,
                               PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
                               const IMG_CHAR *pszText,
                               PVRSRV_MEMDESC_VIRTUALRANGE *hMemDescVirt,
                               IMG_DEV_VIRTADDR *psVirtAddr);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXAllocVirtualRangeAddress
@Description    Allocates given address range in the device virtual space of a heap.
                Physical allocations done with PVRSRVDevMemXAllocPhysical() can
                be mapped partially or completely into this range.

                Valid flags to pass in: PVRSRV_MEMALLOCFLAGS_DEVMEMX_VIRTUAL_MASK

@Input          hHeap               The heap to allocate the virtual range from
@Input          uiNumPages          The allocation size in number of pages the
                                    page size is determined from the passed heap
@Input          uiMemAllocFlags     Virtual allocation flags
@Input          pszText             A description for the allocation
@Input          psVirtAddr          Device virtual address to be allocated
@Output         hMemDescVirt        Handle for this allocation

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ERROR error code
				PVRSRV_ERROR_RA_REQUEST_VIRT_ADDR_FAIL on failure
				PVRSRV_ERROR_RA_REQUEST_ALLOC_FAIL on failure
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXAllocVirtualRangeAddress(PVRSRV_HEAP hHeap,
                               IMG_UINT32 uiNumPages,
                               PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
                               const IMG_CHAR *pszText,
                               IMG_DEV_VIRTADDR sVirtAddr,
                               PVRSRV_MEMDESC_VIRTUALRANGE *hMemDescVirt);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXFreeVirtualRange
@Description    Frees an allocation done with PVRSRVDevMemXAllocVirtualRange().
                The virtual descriptor has to be unmapped completely with
                PVRSRVDevMemXUnmapVirtualRange() before this operation is allowed.
                Hence it is not valid to free the virtual descriptor first and
                then unmap it later as it is possible with the physical
                descriptor.

@Input          hMemDescVirt       Handle for this allocation

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXFreeVirtualRange(PVRSRV_MEMDESC_VIRTUALRANGE hMemDescVirt);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXMapVirtualRange
@Description    This function takes a physical allocation and maps it partially
                or completely to a partial or complete virtual range.

                The caller must pass in a valid range for the physical and virtual
                allocations otherwise the function will return an error.
                Apart from that no range checks are done and therefore it is the
                callers responsibility to not overwrite existing mappings that are
                still in use. Of course when the mappings are not needed anymore
                the caller is free to overwrite them.

                The page size of the virtual and physical allocation has to match
                otherwise an error is returned.

@Input          hMemDescPhys        A handle to the physical allocation to map
@Input          ui32PhysPgOffset    The offset into the physical allocation
                                    in pages
@Input          hMemDescVirt        A handle to the device virtual range
@Input          ui32VirtPgOffset    The offset into the virtual range in pages
@Input          ui32PageCount       The amount of pages to map from the offsets
                                    onward


@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXMapVirtualRange(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                             IMG_UINT32 ui32PhysPgOffset,
                             PVRSRV_MEMDESC_VIRTUALRANGE hMemDescVirt,
                             IMG_UINT32 ui32VirtPgOffset,
                             IMG_UINT32 ui32PageCount);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXUnmapVirtualRange
@Description    Unmaps a certain range of a virtual device allocation.

                The range to unmap must not exceed the range in the given
                virtual handle, an error is returned otherwise.

                The function will invalidate the whole range and will overwrite
                all existing (and non existing) mappings in it.

                The virtual allocation will not be destroyed even if everything
                is unmapped, to achieve that PVRSRVDevMemXFreeVirtualRange() has to
                be called.

@Input          hMemDescVirt        A handle to the device virtual range
@Input          ui32VirtPgOffset    The offset into the virtual range in pages
@Input          ui32PageCount       The amount of pages to unmap from the offset
                                    onward

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXUnmapVirtualRange(PVRSRV_MEMDESC_VIRTUALRANGE hMemDescVirt,
                               IMG_UINT32 ui32VirtPgOffset,
                               IMG_UINT32 ui32PageCount);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXMapPhysicalToCPU
@Description    Maps a whole physical device allocation into CPU virtual space.

@Input          hMemDescPhys       A handle to the physical allocation to map
@Output         psVirtAddr         CPU base address the allocation was mapped in

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXMapPhysicalToCPU(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                              IMG_CPU_VIRTADDR *psVirtAddr);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXUnmapPhysicalToCPU
@Description    Unmaps a whole physical device allocation from CPU virtual space.

@Input          hMemAllocPhys       A handle to the physical allocation to unmap

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXUnmapPhysicalToCPU(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXCreateDevmemMemDescVA
@Description    (DEPRECATED and will be removed in future)
                Takes a virtual devmem address and creates an incomplete devmem
                memdesc to pass into other services functions that only
                retrieve the VA from a memdesc.

                NEVER use with other devmem functions!!!
                ONLY destroy with PVRSRVDevMemXFreeDevmemMemDesc!!!


@Input          sVirtualAddress  A device virtual address
@Output         psMemDesc        The new MemDesc

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXCreateDevmemMemDescVA(const IMG_DEV_VIRTADDR sVirtualAddress,
                                   DEVMEM_MEMDESC **psMemDesc);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXCreateDevmemMemDesc
@Description    Takes a physical and virtual descriptor and creates a devmem
                memdesc to pass into other services functions that only
                retrieve the VA from a memdesc.

                NEVER use with other devmem functions!!!
                ONLY destroy with PVRSRVDevMemXFreeDevmemMemDesc!!!


@Input          psPhysDesc       A handle to the physical allocation
@Input          psVirtDesc       A handle to the virtual allocation
@Output         psMemDesc        The new MemDesc

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXCreateDevmemMemDesc(PVRSRV_MEMDESC_PHYSICAL psPhysDesc,
                                 PVRSRV_MEMDESC_VIRTUALRANGE psVirtDesc,
                                 DEVMEM_MEMDESC **psMemDesc);

/**************************************************************************/ /*!
@Function       PVRSRVDevMemXFreeDevmemMemDesc
@Description    Destroys the MemDesc created with PVRSRVDevMemXCreateDevmemMemDesc()
                or PVRSRVDevMemXCreateDevmemMemDescVA().
                Not compatible with MemDescs created by other functions.

@Input          psMemDesc        The MemDesc to destroy. Do ONLY pass MemDescs
                                 from the GLCommon functions.

@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevMemXFreeDevmemMemDesc(DEVMEM_MEMDESC *psMemDesc);

/**************************************************************************/ /*!
@Function      PVRSRVDevmemXGetImportUID
@Description   Get the UID of the PMR that backs this MemDescPhysical

@Input         hMemDescPhys      MemDesc Physical
@Output        pui64UID          UID of PMR

@Return        PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDevmemXGetImportUID(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                          IMG_UINT64              *pui64UID);

#if defined __cplusplus
};
#endif
#endif /* PVRSRV_DEVMEMX_H */

