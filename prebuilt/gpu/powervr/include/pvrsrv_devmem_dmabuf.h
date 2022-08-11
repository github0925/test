/*************************************************************************/ /*!
@File
@Title          Device Memory Management secure
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of device memory management -- This
                file defines the exposed Services API to the secure memory
                management functions.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRSRV_DEVMEM_DMABUF_
#define _PVRSRV_DEVMEM_DMABUF_

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "pvrsrv_devmem.h"
#include "pvrsrv_devmemx.h"


/*************************************************************************/ /*!
@Function       PVRSRVDmaBufExportDevMemX
@Description    Export device memory (allocated using
                PVRSRVDevMemXAllocPhysical) as a DMA-BUF
@Input          hMemDescPhys            Handle of memory to export
@Output         piFd                    Returned DMA-BUF export file descriptor
@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR PVRSRVDmaBufExportDevMemX(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                                       IMG_INT *piFd);

/*************************************************************************/ /*!
@Function       PVRSRVDmaBufImportDevMemX
@Description    Import a DMA-BUF allocation into a DevMemX buffer
@Input          hCtx                    Services memory context
@Input          fd                      DMABUF export file descriptor
@Input          uiFlags                 Import flags
@Output         phPhysDescPtr           Created physical descriptor
@Output         puiSizePtr              Size of the created MemDesc
@Input          pszName                 Allocation descriptive name, this will
                                        be truncated to the number of characters
                                        specified in the PVR_ANNOTATION_MAX_LEN.
@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR PVRSRVDmaBufImportDevMemX(PVRSRV_DEVMEMCTX hCtx,
                                       IMG_INT fd,
                                       PVRSRV_MEMMAP_FLAGS_T uiFlags,
                                       PVRSRV_MEMDESC_PHYSICAL *phPhysDescPtr,
                                       IMG_DEVMEM_SIZE_T *puiSizePtr,
                                       const IMG_CHAR *pszName);

/*************************************************************************/ /*!
@Function       PVRSRVDmaBufExportDevMem
@Description    Export device memory (allocated using
                PVRSRVAllocExportableDevMem) as a DMA-BUF
@Input          hMemDesc                Handle of memory to export
@Output         piFd                    Returned DMA-BUF export file descriptor
@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR PVRSRVDmaBufExportDevMem(PVRSRV_MEMDESC hMemDesc,
                                      IMG_INT *piFd);

/*************************************************************************/ /*!
@Function       PVRSRVDmaBufImportDevMem
@Description    Import a DMA-BUF allocation
@Input          psDevConnection         Services device connection
@Input          fd                      DMABUF export file descriptor
@Input          uiFlags                 Import flags
@Output         phMemDescPtr            Created MemDesc
@Output         puiSizePtr              Size of the created MemDesc
@Input          pszName                 Allocation descriptive name, this will
                                        be truncated to the number of characters
                                        specified in the PVR_ANNOTATION_MAX_LEN.
@Return         PVRSRV_OK if successful
*/
/*****************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR PVRSRVDmaBufImportDevMem(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                      IMG_INT fd,
                                      PVRSRV_MEMMAP_FLAGS_T uiFlags,
                                      PVRSRV_MEMDESC *phMemDescPtr,
                                      IMG_DEVMEM_SIZE_T *puiSizePtr,
                                      const IMG_CHAR *pszName);

/*************************************************************************/ /*!
@Function       PVRSRVDMABufAllocDevMem
@Description    Allocate a DMABuf by doing a services allocation and export it
                to become a DMABuf or on Android do a gralloc and import the
                result into services.
@Input          psDevConnection         Services device connection
@Input          uiSize                  Requested Size
@Input          uiLog2Align             Requested uiLog2Align
@Input          uiLog2HeapPageSize      Page size of the heap we want to map
@Input          uiFlags                 Allocation Flags
@Input          pszName                 Allocation descriptive name, this will
                                        be truncated to the number of characters
                                        specified in the PVR_ANNOTATION_MAX_LEN.
@Output         fd                      File Descriptor of the DMABuf
@Output         phMemDescPtr            Created MemDesc
@Return         PVRSRV_OK is successful
*/
/*****************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR PVRSRVDMABufAllocDevMem(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                     IMG_DEVMEM_SIZE_T uiSize,
                                     IMG_DEVMEM_LOG2ALIGN_T uiLog2Align,
                                     IMG_UINT32 uiLog2HeapPageSize,
                                     PVRSRV_MEMMAP_FLAGS_T uiFlags,
                                     IMG_CHAR *pszName,
                                     IMG_INT *fd,
                                     PVRSRV_MEMDESC *phMemDescPtr);

/*************************************************************************/ /*!
@Function       PVRSRVFreeDeviceMemInt
@Description    Free device memory (allocated using
                PVRSRVAllocExportableDevMem)
@Input          psDevConnection         Services device connection
@Input          hMemDesc                Handle of memory to free
@Return        void
*/
/*****************************************************************************/
__must_check IMG_EXPORT
bool PVRSRVFreeDeviceMemInt(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                 PVRSRV_MEMDESC hMemDesc);

/*************************************************************************/ /*!
@Function       PVRSRVDMABufReleaseDevMem
@Description    Release the DMABuf allocated with PVRSRVDMABufAllocDevMem().
@Input          psDevConnection         Services device connection
@Input          hMemDesc                MemDesc to release
@Input          fd                      File Descriptor of the DMABuf
@Return        nothing
*/
/*****************************************************************************/
__must_check IMG_EXPORT
bool PVRSRVDMABufReleaseDevMem(const PVRSRV_DEV_CONNECTION *psDevConnection,
                          PVRSRV_MEMDESC hMemDesc,
                          IMG_INT fd);
#endif /* _PVRSRV_DEVMEM_DMABUF_ */
