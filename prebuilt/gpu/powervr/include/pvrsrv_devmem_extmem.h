/*************************************************************************/ /*!
@File
@Title          External Memory Import
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client side part of the exposed Services API to the external
                memory import functionality.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRSRV_DEVMEM_EXTMEM_
#define _PVRSRV_DEVMEM_EXTMEM_

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "pvrsrv_devmem.h"

/*************************************************************************/ /*!
@Function       PVRSRVWrapExtMem
@Description    This function allows any dynamically allocated memory
				associated with an user mode CPU virtual address mapping
				to be imported in to the GPU domain to be accessed by the GPU.
				Typical use cases would be for example to import a texture
				uploaded by application directly into GPU space.

				The memory descriptor returned by this API allows the memory
				associated to be imported in GPU domain and prevents most of
				the usual device memory operations such as pin, unpin, memory
				layout changes, CPU mapping, write permissions on the memory,
				import and finally export attributes.

				Any given CPU virtual address and size with in the allocated
				range can be mapped using the API as long as the CPU virtual
				address is OS page aligned and the size is aligned to
				mapping heap page size.
				CPU virtual address out side the allocated range, or
				size beyond the allocated boundary fail to map with the
				appropriate error.
				Also GPU access should be with in the mapping parameters and
				any access outside the parameters result in BIF fault.

@Input          psDevMemCtx            Services device memory context
@Input          uiSize                 Size of the allocation.
@Input          pvCpuVAddr             CPU Virtual address of mapped memory to
									   be imported.
@Input          uiAlign                Alignment of the input CPU VAddr.
									   This should always be a mapping heap
									   page size.
@Input          uiFlags                Import flags
@Input          pszText                Debug text

@Output         hMemDesc	           Created MemDesc
@Return         PVRSRV_OK if successful
				the following error code on error
				PVRSRV_ERROR_INVALID_FLAGS implies some options in uiFlags are not
				supported. ex: CPU mapping is not allowed.

				PVRSRV_ERROR_INVALID_PARAMS implies one or more input parameters
				are not valid.

				PVRSRV_ERROR_OUT_OF_MEMORY implies there not enough memory in
				the system to satisfy this request.

				PVRSRV_ERROR_UNSUPPORTED_CACHE_MODE implies the cache mode flags
				in uiFlags are not supported.

				PVRSRV_ERROR_INVALID_CPU_ADDR implies the CPU virtual address
				passed is not valid.

				PVRSRV_ERROR_BAD_PARAM_SIZE implies the size parameter passed
				doesn't reflect the allocation size as seen by the kernel.

				PVRSRV_ERROR_FAILED_TO_ACQUIRE_PAGES implies the pages associated
				with the allocation couldn't be found.

				PVRSRV_ERROR_INVALID_PHYS_ADDR implies physical address of the page
				backing the virtual range exceed device accessible range.
*/
/*****************************************************************************/

IMG_EXPORT PVRSRV_ERROR
PVRSRVWrapExtMem(const PVRSRV_DEVMEMCTX psDevMemCtx,
                IMG_DEVMEM_SIZE_T uiSize,
                IMG_CPU_VIRTADDR pvCpuVAddr,
                IMG_DEVMEM_ALIGN_T uiAlign,
                PVRSRV_MEMALLOCFLAGS_T uiFlags,
                const IMG_CHAR *pszText,
                PVRSRV_MEMDESC *hMemDesc);

#endif /* _PVRSRV_DEVMEM_EXTMEM_ */
