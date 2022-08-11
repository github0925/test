/*************************************************************************/ /*!
@File
@Title          EGL api structures and constants.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(_SRVCONTEXT_H_)
#define _SRVCONTEXT_H_


#include "imgextensions.h"
#include "pvrsrv_devmem_miw.h"
#include "rgxapi.h"
#include "rgxapi_tqtdm.h"
//#include "resourceman.h"

#ifndef	EGL_DEFAULT_PARAMETER_BUFFER_SIZE
#define EGL_DEFAULT_PARAMETER_BUFFER_SIZE (2 * 1024 * 1024)
#endif

#ifndef	EGL_DEFAULT_MAX_PARAMETER_BUFFER_SIZE
#define EGL_DEFAULT_MAX_PARAMETER_BUFFER_SIZE	(36 * 1024 * 1024)
#endif

#define EGL_DEFAULT_PDS_FRAG_BUFFER_SIZE	(100 * 1024)
#define EGL_DEFAULT_USC_FRAG_BUFFER_SIZE	(100 * 1024)
#define EGL_DEFAULT_CONST_FRAG_BUFFER_SIZE	(400 * 1024)
#define EGL_DEFAULT_FRAG_BUFFER_SCALE		(8)

#if defined(DEBUG)
typedef struct DevMemllocation_TAG
{
	PVRSRV_MEMINFO			*psMemInfo;
	IMG_UINT32             u32Line;
	const IMG_CHAR         *pszFile;
	IMG_UINT32				u32Sequence;

} DevMemAllocation;
#endif /* defined(DEBUG) */


typedef enum
{
	SRV_CONTEXT_OPENGLES1 = 0,
	SRV_CONTEXT_OPENGLES3 = 1,
	SRV_CONTEXT_OPENGL    = 2,
	SRV_CONTEXT_OPENCL    = 3,
	SRV_CONTEXT_TYPEMAX   = 4

}	SRV_CONTEXT_TYPE;


typedef struct SrvClientContextNode_TAG
{
	void* pvContext;
	SRV_CONTEXT_TYPE eType;
	struct SrvClientContextNode_TAG* psNext;

}	SrvClientContextNode;

typedef struct SrvSyncFreeNode_TAG
{
	void *pvSync;
	struct SrvSyncFreeNode_TAG *psNext;
}	SrvSyncFreeListNode;

typedef struct SrvImageFreeNode_TAG
{
	void *pvKEGLImage;
	void *pvMemDesc;
	struct SrvImageFreeNode_TAG *psNext;
}	SrvImageFreeListNode;

typedef struct TQM_QUEUE_ITEM_TAG TQM_QUEUE_ITEM;

typedef struct SrvSysContext_TAG
{
	PVRSRV_DEV_CONNECTION	*psDevConnection;
	PVRSRV_TASK_CONTEXT	*psTaskContext;

	IMG_HANDLE 				hOSEvent;
 	IMG_DEV_VIRTADDR		uPDSExecBase;
 	IMG_DEV_VIRTADDR		uUSCExecBase;
 	IMG_DEV_VIRTADDR		uCompCtrlBase;
 	IMG_DEV_VIRTADDR		uPixelStateBase;
 	IMG_DEV_VIRTADDR		uTexStateBase;
 	IMG_DEV_VIRTADDR		uFBCDescBase;
	IMG_HANDLE hIMGEGLAppHints;

	PVRSRV_HEAP hPDSHeap;
	PVRSRV_HEAP hUSCHeap;
	PVRSRV_HEAP hGeneralHeap;
	IMG_UINT32	ui32Log2GeneralHeapPageSize;
	PVRSRV_HEAP hNon4KHeap;
	IMG_UINT32 ui32Log2Non4KHeapPageSize;
	PVRSRV_HEAP hCompCtrlHeap;
	PVRSRV_HEAP hPixelStateHeap;
	PVRSRV_HEAP hTexStateHeap;
	PVRSRV_HEAP hFBCDescHeap;

	/* Handles for services contexts */
	PRGX_DEVMEMCONTEXT hRGXDevMemContext;
	PVRSRV_DEVMEMCTX   hDevMemContext;
	PRGX_RENDERCONTEXT hRenderContext;
	PRGX_GLOBALPB	   hGlobalPB;
	IMG_HANDLE		   hTransferContext;
	IMG_BOOL		   bTransferContextInit;
	IMG_HANDLE         hFBCDCContext;
	PDEVVARCTX		   hDevVarContext;
	PRGX_COMPUTECONTEXT hComputeContext;

	/* Client sync stuff for TQ */
	PVRSRV_MUTEX_HANDLE	 hTQClientMutex;

	/* Current Job status on the Resource Manager for the TQ HWQueue */
	RMHWQueue *psHWQueue_TQ;
	RMHWQueue *psSWQueue;

	/* TQM buffer queue */
	TQM_QUEUE_ITEM		*psTQMBufferQueueFirst;
	TQM_QUEUE_ITEM		*psTQMBufferQueueLast;

	/* TQM cache for temporary texture related allocations */
	PVRSRV_MEMINFO		**ppsTQMTextureCacheMemInfoList;
	IMG_UINT32		ui32TQMTextureCacheMaxEntries;
	IMG_UINT32		ui32TQMTextureCacheEntrySize;

#if defined(DEFERRED_WORKER_THREAD)
	/* TQM deferred free thread handle */
	IMG_HANDLE		hTQMFreeMemoryTask;

	/* EGL sync deferred free thread handle */
	IMG_HANDLE			hFreeSyncTask;
	PVRSRV_MUTEX_HANDLE	hFreeSyncMutex;
	SrvSyncFreeListNode	*psSyncFreeList;

	/* EGL image deferred free thread handle */
	IMG_HANDLE			hFreeImageTask;
	PVRSRV_MUTEX_HANDLE	hFreeImageMutex;
	SrvImageFreeListNode	*psImageFreeList;
#endif

#if defined(DEBUG) || defined(TIMING)
	/* TQM memory stats */
	IMG_DEVMEM_SIZE_T	uiTQMMemCurrent;
	IMG_DEVMEM_SIZE_T	uiTQMMemHWM;
#endif

#if defined(DEBUG)
	/* Track device memory allocations */
	PVRSRV_MUTEX_HANDLE	hMemTrackMutex;

	IMG_UINT32		uiMemTrackAllocsCurrent;
	IMG_UINT32		uiMemTrackAllocsMax;
	IMG_UINT32		uiMemTrackCurSequence;

	DevMemAllocation	*psMemTrackAllocs;

	IMG_UINT32		uiDumpAllResultsIdx;
#endif /* defined(DEBUG) */

	/* List of all allocated client contexts */
	SrvClientContextNode	*psClientContextList;
	PVRSRV_MUTEX_HANDLE	hClientContextListMutex;

	IMG_HANDLE		hBufPool;

#if defined(EGL_RENDERTARGET_CACHE)
	IMG_HANDLE		hRTPool;
#endif

	IMG_UINT32		ui32JobRef;				 /*!< External JobRef written by clients with HWPerf data */
	PVRSRV_MUTEX_HANDLE hExtJobRefMutex;

	TQ_TDM_CREATE_CONTEXT sCreateTransfer;

	RMContext *psDefaultRMContext;

    /* Mutex for blob cache */
    PVRSRV_MUTEX_HANDLE	hBlobCacheMutex;

	/* Count of FBOs created for sizing of RTCache and Fragment CircularBuffer Pool */
	ATOMIC_T atomicintNumFBOs;

	/* Enable SWTQ from apphint */
	IMG_BOOL bEnableSWTQ;

	/* Debug support object handler */
	IMG_HANDLE hDebug;

#if defined(SHADER_DEBUG_TOOL)
    /* Total amount memory allocated for shader debug out buffers */
    IMG_UINT64			ui64ShaderDebugTotalSizeInBytes;
    PVRSRV_MUTEX_HANDLE	hShaderDebugTotalSizeMutex;
#endif

#if defined(GTRACE_TOOL)
    struct GTraceSetupTAG				*psGTraceSetup;
    struct GMemoryDumpManagerGroupTAG	*psMemoryDumpManagerGroup;
    uint64_t							ui64AllocationID;
    uint64_t							ui64RenderSurfaceID;
    uint64_t							ui64CCBID;
    uint64_t							ui64MemHeapID;
    struct GMemoryDumpEngineTAG			*psMemoryDumpEngineES;
    struct GMemoryDumpEngineTAG			*psMemoryDumpEngineTransferDest;
#endif

} SrvSysContext;

#endif /* !defined(_SRVCONTEXT_H_) */
