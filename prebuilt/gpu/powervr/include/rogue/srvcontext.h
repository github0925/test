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
//#include "resourceman.h"
#include "hwbrnwa.h"

#ifndef	EGL_DEFAULT_PARAMETER_BUFFER_SIZE
#define EGL_DEFAULT_PARAMETER_BUFFER_SIZE (2 * 1024 * 1024)
#endif

#ifndef	EGL_DEFAULT_MAX_PARAMETER_BUFFER_SIZE
#define EGL_DEFAULT_MAX_PARAMETER_BUFFER_SIZE	(36 * 1024 * 1024)
#endif

#ifndef EGL_DEFAULT_PDS_FRAG_BUFFER_SIZE
#define EGL_DEFAULT_PDS_FRAG_BUFFER_SIZE	(100 * 1024)
#endif
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

#if defined(FIX_HW_BRN_62493) || defined(FIX_HW_BRN_67805)
typedef struct HWBRN62493_BRN67805Info_TAG
{
	PVRSRV_MEMINFO *psLUTMemInfo;
	IMG_UINT64 aui64TAGControlWord[2][(ROGUE_MAXIMUM_IMAGE_STATE_SIZE >> 1) + (ROGUE_SAMPLER_STATE_SIZE >> 1)];
} HWBRN62493_BRN67805Info;
#endif

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
	IMG_HANDLE hIMGEGLAppHints;

	PVRSRV_HEAP hPDSHeap;
	PVRSRV_HEAP hUSCHeap;
	PVRSRV_HEAP hGeneralHeap;
	IMG_UINT32	ui32Log2GeneralHeapPageSize;
	PVRSRV_HEAP hNon4KHeap;
	IMG_UINT32 ui32Log2Non4KHeapPageSize;

	/* Handles for services contexts */
	PRGX_DEVMEMCONTEXT hRGXDevMemContext;
	PVRSRV_DEVMEMCTX   hDevMemContext;
	PRGX_RENDERCONTEXT hRenderContext;
	PRGX_GLOBALPB	   hGlobalPB;
	IMG_HANDLE		   hTransferContext;
	IMG_BOOL		   bTransferContextInit;
	PDEVVARCTX		   hDevVarContext;
	PRGX_COMPUTECONTEXT hComputeContext;

#if defined(RGX_SW_COMPUTE_PDS_BARRIER) && (RGX_NUM_PHANTOMS == 2)
	/* CDM context switch sync address */
	IMG_DEV_VIRTADDR	sPDSSyncDevVAddr;
#endif

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

#if defined(FIX_HW_BRN_67349) || defined(FIX_HW_BRN_70353)
	HWBRN67349Info		sHWBRN67349Info;
#endif

#if defined(FIX_HW_BRN_62493) || defined(FIX_HW_BRN_67805)
    HWBRN62493_BRN67805Info      sHWBRN62493_BRN67805Info;
#endif

	/* List of all allocated client contexts */
	SrvClientContextNode	*psClientContextList;
	PVRSRV_MUTEX_HANDLE	hClientContextListMutex;

	IMG_HANDLE		hBufPool;

#if defined(EGL_RENDERTARGET_CACHE)
	IMG_HANDLE		hRTPool;
#endif

	IMG_UINT32		ui32JobRef;				 /*!< External JobRef written by clients with HWPerf data */
	PVRSRV_MUTEX_HANDLE hExtJobRefMutex;

	/* For 2-stage TQ Prepare-Submit */
	TQ_CREATE_CONTEXT sCreateTransfer;
	IMG_HANDLE        hTransferSubmit;

	/* TQ Route information */
	IMG_BOOL          bApphintRouteOverriden; /*!< flag indicating that the route is overridden */
	IMG_UINT32        ui32RouteOverride;      /*!< the route from the apphint */

	/* Enable SWTQ from apphint */
	IMG_BOOL bEnableSWTQ;

	//RMResourceManager *psRM;
	RMContext *psDefaultRMContext;

	/* Debug support object handler */
	IMG_HANDLE hDebug;

    /* Mutex for blob cache */
    PVRSRV_MUTEX_HANDLE	hBlobCacheMutex;

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

	/* Count of FBOs created for sizing of RTCache and Fragment CircularBuffer Pool */
	ATOMIC_T atomicintNumFBOs;

} SrvSysContext;

#endif /* !defined(_SRVCONTEXT_H_) */
