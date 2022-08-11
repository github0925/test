/*************************************************************************/ /*!
@File
@Title          Transfer Queue Manager interface
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Interface for managing various transfer queue operations
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(__TQMANAGER_H__)
#define __TQMANAGER_H__

#include <stdlib.h>

#include "img_types.h"
#include "img_defs.h"
#include "rgxapi.h"
#include "rgxapi_tqtdm.h"
#include "services.h"
#include "pvrsrv_devmem_miw.h"
#include "srvcontext.h"
#include "resourceman.h"

typedef enum SWTQ_COMMAND_TAG
{
	SWTQ_COMMAND_TWIDDLE,
	SWTQ_COMMAND_MEMCPY,
} SWTQ_COMMAND;

typedef struct SWTQCommand_TAG
{
	PVRSRV_TIMELINE hTimeline;
	PVRSRV_DEV_CONNECTION *psDevConnection;
	PVRSRV_FENCE hFence;
	SrvSysContext *psSysContext;

	SWTQ_COMMAND eCommand;

	IMG_UINT8 *pui8SrcCpuVirtAddr;
	IMG_UINT8 *pui8DstCpuVirtAddr;

	PVRSRV_MEMDESC hSrcMemDesc;
	PVRSRV_MEMDESC hDstMemDesc;

	IMG_PIXFMT eIMGFmt;
	IMG_UINT32 ui32Width;
	IMG_UINT32 ui32Height;
	IMG_UINT32 ui32Stride;

	IMG_UINT32 ui32BytesToMemcpy;

	PVRSRV_MEMINFO *psBufferToFree;

	IMG_BOOL bFlushSrcCache;

	IMG_UINT32 ui32JobRef;

#if defined(GTRACE_TOOL)
	struct GTraceTAG *psGTrace;
	struct GMemoryDumpManagerTAG *psMemoryDumpManager;
#endif

} SWTQCommand;

typedef struct TQBatchInfo_TAG
{
	IMG_UINT32 ui32LastBlitter;
	IMG_BOOL   bFirst;
	IMG_BOOL   bLast;
	IMG_BOOL   bWasMiniKick;
} TQBatchInfo;

#define	SKIP_LOCK_RM	0x00000001
#define SKIP_LOCK_TQM	0x00000002

typedef struct TQMArgs_TAG
{
	PVRSRV_MEMINFO *psBufferToFree;
	PVRSRV_FENCE hCheckFence;

	RMResource* apsSrcResources[4];
	RMResource* apsDstResources[4];

	RM_RESOURCE_USAGE aePerLevelSrcUsage[4];
	RM_RESOURCE_USAGE aePerLevelDstUsage[4];

#if defined(GTRACE_TOOL)
	struct GMemoryUsageTAG *apsSrcMemoryUsages[4];
	struct GMemoryUsageTAG *apsDstMemoryUsages[4];
	struct GMemoryUsageTAG *psBufferToFreeMemoryUsage;

	IMG_UINT64 ui64KickTQIndex;
	/* Does this contains valid TQ trace information, if FALSE, an unknown TQ operation will be traced. */
	IMG_BOOL bValidTraceInfo;
	/* Does this TQ operation is traced.  This is useful when TQ operation fails, it is still traced. */
	IMG_BOOL bTraced;
#endif

	IMG_BOOL bTrySWTQFirst;

	IMG_UINT32 ui32SkipLockFlags;

	IMG_UINT32 ui32JobRef;
} TQMArgs;

IMG_BOOL TQMQueueTransfer(RMContext* psCtx,
                 TQ_SOURCE *psSources,
                 TQ_SURFACE *psDestSurface,
                 IMG_RECT *psScissor,
                 IMG_UINT32 ui32NumSrcs,
                 IMG_UINT32 ui32FilterMode,
                 IMG_RESOLVE_OP eResolveOp,
                 IMG_UINT32 ui32Flags,
#if defined(RGX_FEATURE_TLA)
                 IMG_UINT32 ui32TQRoute,
#endif
                 IMG_UINT32 ui32MergeFlags,
                 TQBatchInfo *psTQBatchInfo,
				 TQMArgs* psArgs);

IMG_EXPORT
IMG_BOOL TQMQueueMipgen(RMContext* psCtx,
                 TQ_TDM_SURFACE *psMipgenSource,
                 TQ_TDM_SURFACE *psMipgenSurface,
                 IMG_UINT32 ui32Flags,
				 IMG_UINT32 ui32BaseLevel,
				 IMG_UINT32 ui32NumLevelsToGenerate,
				 TQBatchInfo *psTQBatchInfo,
				 TQMArgs* psArgs);

IMG_EXPORT
IMG_BOOL TQMBlitData(RMContext* psCtx,
			IMG_UINT32 ui32Flags,
            PVRSRV_MEMDESC hSrcMemDesc,
            IMG_UINT32 ui32SrcOffsetInBytes,
            PVRSRV_MEMDESC hDstMemDesc,
            IMG_UINT32 ui32DstOffsetInBytes,
            IMG_UINT32 ui32CopySizeInBytes,
#if defined(RGX_FEATURE_TLA)
            IMG_UINT32 ui32TQRoute,
#endif
            IMG_UINT32 ui32Samples,
			TQMArgs* psArgs,
			IMG_UINT32 *pui32NumTQOps);

IMG_BOOL TQMColourFill(RMContext* psCtx,
              TQ_SURFACE *psDestSurface,
              IMG_RECT *psScissor,
              IMG_UINT32 ui32Flags,
#if defined(RGX_FEATURE_TLA)
              IMG_UINT32 ui32TQRoute,
#endif
              IMG_UINT32* pui32Colour,
			  TQMArgs* psArgs);

PVRSRV_ERROR TQMCacheAllocDeviceMem(SrvSysContext *psSysContext,
					RMContext *psRMCtx,
				    IMG_UINT32 ui32Size,
				    PVRSRV_MEMINFO **ppsMemInfo,
					IMG_CHAR *pszRIAPIString,
					const IMG_CHAR *pszFile,
					IMG_UINT32 u32Line);

#if defined(DEBUG)

#if !defined (INTEGRITY_OS) && !defined(__KLOCWORK__)
#define TQM_ALLOC_DEVICE_MEM(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, eHWPerfMemOpResourceType) \
    ({ \
        PVRSRV_ERROR eError = PVRSRV_OK;\
        if(unlikely(gc->sRMCtx.ui32HWPerfClientFilterCached & RGX_HWPERF_CLIENT_EVENT_MASK(OPENGLES, RGX_HWPERF_CLIENT_EVENT_TYPE_OPENGLES_MEM_OP_START))) HWPerfClientMemOpStartGLES();\
        eError = TQMCacheAllocDeviceMem(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, __FILE__, __LINE__);\
        if(unlikely(gc->sRMCtx.ui32HWPerfClientFilterCached & RGX_HWPERF_CLIENT_EVENT_MASK(OPENGLES, RGX_HWPERF_CLIENT_EVENT_TYPE_OPENGLES_MEM_OP_END))) HWPerfClientMemOpEndGLES(eError == PVRSRV_OK ? (*ppsMemInfo)->uiAllocationSize:0, eHWPerfMemOpResourceType, RGX_HWPERF_MEM_OP_TYPE_DEVICE_ALLOC, "TQ Manager Alloc");\
        eError;\
    }) \

#else /* !defined (INTEGRITY_OS) && !defined(__KLOCWORK__) */

#define TQM_ALLOC_DEVICE_MEM(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, eHWPerfMemOpResourceType) \
    TQMCacheAllocDeviceMem(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, __FILE__, __LINE__)
#endif /* !defined (INTEGRITY_OS) && !defined(__KLOCWORK__) */

#else

#if !defined (INTEGRITY_OS) && !defined(__KLOCWORK__)
#define TQM_ALLOC_DEVICE_MEM(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, eHWPerfMemOpResourceType) \
    ({ \
        PVRSRV_ERROR eError = PVRSRV_OK;\
        if(unlikely(gc->sRMCtx.ui32HWPerfClientFilterCached & RGX_HWPERF_CLIENT_EVENT_MASK(OPENGLES, RGX_HWPERF_CLIENT_EVENT_TYPE_OPENGLES_MEM_OP_START))) HWPerfClientMemOpStartGLES();\
        eError = TQMCacheAllocDeviceMem(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, NULL, 0);\
        if(unlikely(gc->sRMCtx.ui32HWPerfClientFilterCached & RGX_HWPERF_CLIENT_EVENT_MASK(OPENGLES, RGX_HWPERF_CLIENT_EVENT_TYPE_OPENGLES_MEM_OP_END))) HWPerfClientMemOpEndGLES(eError == PVRSRV_OK ? (*ppsMemInfo)->uiAllocationSize:0, eHWPerfMemOpResourceType, RGX_HWPERF_MEM_OP_TYPE_DEVICE_ALLOC, "TQ Manager Alloc");\
        eError;\
    }) \

#else /* !defined (INTEGRITY_OS) && !defined(__KLOCWORK__) */

#define TQM_ALLOC_DEVICE_MEM(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, eHWPerfMemOpResourceType) \
    TQMCacheAllocDeviceMem(psSysContext, psRMCtx, ui32Size, ppsMemInfo, pszRIAPIString, NULL, 0)
#endif /* !defined (INTEGRITY_OS) && !defined(__KLOCWORK__) */
#endif

IMG_UINT32 TQMCleanUpBufferQueue(SrvSysContext *psSysContext,
#if defined(GTRACE_TOOL)
		                         struct GTraceTAG *psGTrace,
#endif
	                             IMG_BOOL bNoDefer);

IMG_BOOL TQMInitAndTakeLock(SrvSysContext *psSysContext, IMG_BOOL bTakeLock);

IMG_BOOL TQMDeInit(SrvSysContext *psSysContext);

#endif /* !defined(__TQMANAGER_H__) */
