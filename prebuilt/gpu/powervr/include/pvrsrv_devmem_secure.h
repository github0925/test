/*************************************************************************/ /*!
@File
@Title          Device Memory Management secure
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of device memory management -- This
                file defines the exposed Services API to the secure memory
                management functions.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRSRV_DEVMEM_SECURE_
#define _PVRSRV_DEVMEM_SECURE_

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "pvrsrv_devmem.h"

/*************************************************************************/ /*!
@Function       PVRSRVSecureExportDevMem

@Description    Export memory to another process using a secure mechanism

@Input          psMemDesc               MemDesc to export

@Output         pExport                 Secure export token

@Return         PVRSRV_OK is successful
*/
/*****************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVSecureExportDevMem(PVRSRV_MEMDESC hMemDesc,
									  IMG_SECURE_TYPE *pExport);

/*************************************************************************/ /*!
@Function       PVRSRVSecureUnexportDevMem

@Description    Unexport memory previously exported using a secure mechanism

@Input          hMemDesc             	MemDesc to unexport

@Input          Export                  Secure export token

@Return         PVRSRV_OK is successful
*/
/*****************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVSecureUnexportDevMem(PVRSRV_MEMDESC hMemDesc,
										IMG_SECURE_TYPE Export);

/*************************************************************************/ /*!
@Function       PVRSRVSecureImportDevMem

@Description    Import memory from another process using a secure mechanism

@Input          psConnection            Services connection

@Input          Import                  Secure import token

@Input          uiFlags                 Import flags

@Output         phMemDescPtr            Created MemDesc

@Output         puiSizePtr              Size of the created MemDesc

@Return         PVRSRV_OK is successful
*/
/*****************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVSecureImportDevMem(const PVRSRV_DEV_CONNECTION *psConnection,
									  IMG_SECURE_TYPE Import,
									  PVRSRV_MEMMAP_FLAGS_T uiFlags,
									  PVRSRV_MEMDESC *phMemDescPtr,
									  IMG_DEVMEM_SIZE_T *puiSizePtr);

#endif	/* _PVRSRV_DEVMEM_SECURE_ */
