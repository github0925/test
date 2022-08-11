/*************************************************************************/ /*!
@Title          EGL api definition
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _EGLAPI_H_
#define _EGLAPI_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <powervr/imgyuv.h>

#include "rgxapi.h"
#include "srvcontext.h"
//#include "pds.h"
#include "pixeventpbesetup.h"
#include "buffers.h"
#include "common_tls.h"
#include "imgextensions.h"
#include "rgxfmt_api.h"
#include "resourceman.h"
#include "rgx_hwperf_client.h"
#include "fbcdctable.h"
#include "pvrsrv_sync_km.h"
#include "takickreason.h"
#include "yuvinfo.h"

#if defined(PDUMP)
#include "pvrsrv_devmem_pdump.h"
#endif

#include <stdarg.h>

#if defined(_WIN32)
#pragma warning (disable:4514)
#endif

#if defined (__cplusplus)
extern "C" {
#endif

#define EGL_MAX_SRC_SYNCS RGX_MAX_3D_SYNCS
#define EGL_MAX_PLANES 3 /* This needs to always be equal to WSEGL_MAX_PLANES in wsegl.h */

/**************************************************************
 *
 *         External Z buffer control apphint values
 *
 **************************************************************/
#define EXTERNAL_ZBUFFER_MODE_ALLOC_ONDEMAND_USED_ALWAYS		0
#define EXTERNAL_ZBUFFER_MODE_ALLOC_ONDEMAND_USED_ASNEEDED		1

#define EXTERNAL_ZBUFFER_MODE_ALLOC_UPFRONT_USED_ALWAYS			2
#define EXTERNAL_ZBUFFER_MODE_ALLOC_UPFRONT_USED_ASNEEDED		3

#define EXTERNAL_ZBUFFER_MODE_ALLOC_NEVER_USED_NEVER			4


#define EXTERNAL_ZBUFFER_MODE_DEFAULT							EXTERNAL_ZBUFFER_MODE_ALLOC_ONDEMAND_USED_ASNEEDED




/**************************************************************
 *
 *                       Generic
 *
 **************************************************************/
typedef void (IMG_CALLCONV *GPA_PROC)(void);

/**************************************************************
 *
 *                       EGL --> OpenGL
 *
 **************************************************************/
#define IMG_ANTIALIAS_NONE		0x00000000
#define IMG_ANTIALIAS_1x2		0x00000002
#define IMG_ANTIALIAS_2x2		0x00000004
#define IMG_ANTIALIAS_2x4		0x00000008

#define IMG_MAX_TEXTURE_MIPMAP_LEVELS		11

typedef void* EGLDrawableHandle;
typedef void* EGLContextHandle;
typedef void* EGLRenderSurfaceHandle;

typedef struct EGLRect_TAG
{
	IMG_INT32  i32X;
	IMG_INT32  i32Y;
	IMG_UINT32 ui32Width;
	IMG_UINT32 ui32Height;

} EGLRect;

typedef enum
{
	EGL_DRAWABLETYPE_WINDOW	 = 0,
	EGL_DRAWABLETYPE_PIXMAP	 = 1,
	EGL_DRAWABLETYPE_PBUFFER = 2

} EGLDrawableType;

#define NUM_PBE_TILE_BUFFERS	(8-1)	/* At least one render target will be in the output registers */

typedef struct EGLPixelBEState_TAG
{
	IMG_UINT32			aui32EmitWords[8][STATE_PBE_DWORDS];

	IMG_UINT32			ui32NumTileBuffers;
	PVRSRV_MEMINFO		*apsTileBuffer[NUM_PBE_TILE_BUFFERS];

	IMG_DEV_VIRTADDR	uSPMPixelEventPDSData;

	IMG_UINT32			ui32NumPBEEmits;
	IMG_UINT32			ui32NumOutputRegisters;

	IMG_BOOL			bDither;

#if defined(PDUMP)
	PVRSRV_PDUMP_IMAGE_DESCRIPTOR asPDumpImageDesc[8];
	IMG_UINT32			ui32NumImageDescs;
	IMG_UINT32			ui32BaseNumImageDescs;
#endif

	IMG_UINT32			ui32BaseNumPBEEmits;  // Num PBE Emits in the base setup without any merge renders. Set in ComputeFramebufferCompleteness

} EGLPixelBEState;

typedef struct EGL3DRegisters_TAG
{
    IMG_UINT32 ui32ISPCtrl;
	IMG_UINT64 ui64ISPZLSControl;

} EGL3DRegisters;

/* End moved from the context */

#define MAX_DWORDS_PER_PDS_STATE_BLOCK		3
#define MAX_DWORDS_PER_INDEX_LIST_BLOCK		10
#define MAX_DWORDS_PER_TERMINATE_BLOCK		1
#define MAX_DWORDS_PER_VDMSTATE_BLOCK		19


#define EGL_RENDER_TARGET_NOAA_INDEX		0
#define EGL_RENDER_TARGET_AA_INDEX			1
#define EGL_RENDER_TARGET_MAX_INDEX			2

#define EGL_NUM_PDS_PIXEL_STATE_WORDS		2


typedef struct ZLSBufferTAG
{
	PRGX_ZSBUFFER		hZSBuffer;
	IMG_DEV_VIRTADDR	sZSDevVirtAddr;
	PVRSRV_MEMDESC		hZSMemDesc;
	PVRSRV_MEMINFO		*psZSMemInfo;
	IMG_UINT32			ui32SizeInBytes;
	IMG_UINT32			ui32StencilOffset;
	IMG_BOOL			bZSRequestedPhysicalBacking;
	IMG_BOOL			bDepthWritten;
	IMG_BOOL			bStencilWritten;
	IMG_UINT32			ui32PhysicalWidth;
	IMG_UINT32			ui32PhysicalHeight;
	IMG_FB_COMPRESSION 	eFBCompression;
	PVRSRV_MEMINFO		*psFBCDescriptorMemInfo;
	IMG_PIXFMT 			eZSBufferPixFmt;
#if defined(GTRACE_TOOL)
	IMG_UINT64			ui64AllocationID;
	const IMG_CHAR*		pszAllocationContext;
#endif
} ZLSBuffer;

typedef struct MSAAScratchBufferTAG
{
	PRGX_ZSBUFFER		hMSAAScratchBuffer;
	IMG_DEV_VIRTADDR	sMSAAScratchDevVirtAddr;
	PVRSRV_MEMDESC		hMSAAScratchMemDesc;
	IMG_BOOL			bMSAARequestedPhysicalBacking;
	IMG_UINT32			ui32StrideInTexels;
	IMG_FB_COMPRESSION	eFBCompression;
	PVRSRV_MEMINFO		*psFBCDescriptorMemInfo;

} MSAAScratchBuffer;

/* Optional workload characteristics tracking (per EGLRenderSurface) */
#if defined(SUPPORT_WORKLOAD_ESTIMATION)
typedef struct EGLWorkloadCharacteristicsTAG
{
	IMG_UINT32		ui32NumberOfDrawCalls;
	IMG_UINT32		ui32NumberOfIndices;
	IMG_UINT32		ui32NumberOfMRTs;
} EGLWorkloadCharacteristics;
#endif

typedef struct EGLShadowBufferTAG
{
	IMG_BOOL		bDSShadowRender;
	PVRSRV_MEMINFO *psShadowMemInfo;
	IMG_UINT32		ui32Width;
	IMG_UINT32		ui32Height;
	IMG_FB_COMPRESSION	eFBCompression;
	IMG_BOOL		bHasDepth;

	void			*pvGLES3Texture;
	IMG_UINT32		ui32TextureOffset;

	IMG_UINT64		u3DReg_ISP_ZLS_PIXELS;
	IMG_UINT64		ui64ISPZLoadStoreBase;
	IMG_UINT32		ui32DepthStride;

	IMG_UINT64		ui64ISPStencilLoadStoreBase;
	IMG_UINT32		ui32StencilStride;
} EGLShadowBuffer;

#define IMG_MAX_SRC_SYNCS 16

#define EGL_PIXEL_STATE_DYNAMIC_SA_CODE_BIT		0x1
#define EGL_PIXEL_STATE_DYNAMIC_PRIM_CODE_BIT	0x2
#define EGL_PIXEL_STATE_DYNAMIC_VARYING_BIT		0x4

typedef struct EGLRenderSurfaceTAG
{
	SrvSysContext		*psSysContext;

	IMG_BOOL			bFirstKick;

	PRGX_RTDATASET		ahRenderTarget[IMG_MAX_PARALLEL_RENDERS][EGL_RENDER_TARGET_MAX_INDEX];

#if defined(SUPPORT_BUFFER_SYNC)
	/* Composition source dependency memory descriptors */
	PVRSRV_MEMDESC          ahSrcMemDescs[RGX_MAX_3D_SYNCS];
	IMG_UINT32              ui32NumSrcMemDescs;
#endif

	PVRSRV_FENCE		hFence;

	IMG_BOOL			bAccum;

	/* This flag indicates that the previous frame is accumulated */
	IMG_BOOL			bIsPrevFrameAccumulated;


	/* ZS buffer used by default FBO */
	ZLSBuffer			sZLSBuffer;

	IMG_UINT64			ui64FBOZLSControl;

	/* Last render was an overflow so load ZS */
	IMG_BOOL			bNeedZSLoadAfterOverflowRender;

	CircularBuffer		*psPDSBuffer;
	CircularBuffer		*psPixelStateBuffer;
	CircularBuffer		*psConstantBuffer;
	CircularBuffer		*psUSCBuffer;
	CircularBuffer		*psScissorBuffer;
	CircularBuffer		*psDepthBiasBuffer;
	CircularBuffer		*psTexStateBuffer;
	EGLPixelBEState		sPixelBEState;
	IMG_UINT32			aui32RegionClipWord[IMG_MAX_PARALLEL_RENDERS][2];
	IMG_UINT32			ui32TerminateRegion[IMG_MAX_PARALLEL_RENDERS];
	EGL3DRegisters		s3DRegs;

	/* Flags to tell whether any pixel state (such as secondary PDS code section,
	 * varying etc) in circular buffer. If any state is in circular buffer,
	 * we don't bother hash static pixel state table */
	IMG_UINT32			ui32PixelStateDynamicFlags;

	IMG_BOOL			bIterateCoefficients;
	IMG_UINT64			aui64PDSPixelStateWords[EGL_NUM_PDS_PIXEL_STATE_WORDS];		/* Formally aui32PDSPPPStateWords */
	IMG_DEV_VIRTADDR	uPDSPixelShaderConstantsDataBaseAddr;	/* Is this definitely the right place for this? */

	IMG_BOOL			bRenewPDSPPPStateWords;

	IMG_UINT32			ui32SampleCount;
	IMG_UINT32			ui32NumRTALayers;

	IMG_BOOL			bInFrame;
	IMG_UINT16			ui16SideEffectsIn3D;
	IMG_BOOL			bInFrameWithInvalidateDepth;
	IMG_BOOL			bInFrameWithInvalidateStencil;

	/* TA-side state */
	IMG_BOOL			bHasXFBOrPrimsGenerated;
	IMG_UINT32			ui32NumShareds;
	IMG_UINT32			ui32NumDDMShareds;
	IMG_UINT16			ui16SideEffectsInTA;
	IMG_BOOL			bTAHasDrawcallsWithTess; /* Whether this TA kick contains drawcalls where TESS is used (regardless of whether GS is also present) */
	IMG_BOOL			bTAHasDrawcallsWithBothGSAndTess;	/* Whether this TA kick contains drawcalls where both GS and TESS are used */
	IMG_BOOL			bHas3DEnabledPrim; /* Whether this TA kick contains any primitives which is not stream out only. */
	IMG_BOOL			bRequireCrossPDSSPUInfo;
#if defined(FIX_HW_BRN_68501)
	IMG_BOOL			bPrimIDEnabled;	/* Whether primitive id is enabled */
#endif


	/*
	 * This flag indicates the content of frame is reset, by either a full
	 * screen clear or InvalidateFrameBuffer.
	 */
	IMG_BOOL			bFrameContentReset;
	IMG_BOOL			bInExternalFrame;
	IMG_UINT32			ui32PrimitivesSinceLastTA;
	IMG_BOOL			bOQPresent;
	IMG_BOOL			bDepthWritesSinceLast3D;
	IMG_BOOL			bStencilWritesSinceLast3D;
	IMG_BOOL			bDepthStencilReadsSinceLast3D;

#if defined(SHADER_DEBUG_TOOL)
    void                *psShaderBasedDebugOutBuffersMap;
#endif

	/* This flag indicates that ResetSurface has been called on this surface */
	IMG_BOOL			bSurfaceReset;

	IMG_BOOL			bLastScissorFullScreen;
	EGLRect				sLastScissor[IMG_MAX_PARALLEL_RENDERS];
	IMG_UINT32			ui32LastScissorIndex[IMG_MAX_PARALLEL_RENDERS];
	IMG_UINT32			ui32DrawSurfaceScissorIndex[IMG_MAX_PARALLEL_RENDERS];

	IMG_UINT32			ui32LastDepthBiasIndex;
	IMG_FLOAT			fLastPolyOffsetUnits;
	IMG_FLOAT			fLastPolyOffsetFactor;
	IMG_FLOAT			fLastPolyOffsetClamp;

	PVRSRV_MUTEX_HANDLE hMutex;

	/* Lock to ensure Kicks of render surfaces are single threaded.
	 * If a thread sets bInFrame no other thread can PrepareToDraw on the same
	 * surface until the surface has been kicked. */
	PVRSRV_MUTEX_HANDLE hScheduleTAMutex;

	EGLDrawableHandle	hEGLSurface;
	IMG_BOOL			bDepthStencilBits;
	IMG_BOOL			bStencilBits;

	/* Only used for eglQuerySurface(). Don't overload it. */
	IMG_COLOURSPACE_FORMAT eColourSpace;

	IMG_BOOL            bIsProtectedSurface;

	IMG_BOOL			bZOnlyRender;

#if defined(PDUMP)
	IMG_BOOL			bPDumpTAOnly;
	IMG_BOOL			bFirstTAWasPdumped;
#endif

	/*
	 * Syncs for commands submitted on this rendersurface.
	 *
	 * This list is freed when the render complete.
	 */
	//SyncCommand			*psAllPendingSyncs;

	IMG_UINT32			ui32RenderNumber;

	IMG_BOOL			bFastColorClear;
	IMG_BOOL			bFastDepthClear;
	IMG_BOOL			bFastStencilClear;

	/* CPA: Clear-Only Optimisation
       ui32ClearOnlyClearFlag and ui32PartialClearOnlyClearFlag save clear Flag for latest clear issued for current render
       ui32LastClearOnlyClearFlag saves clear Flag for last render on this render surface, 0 means last render is NOT clear only render.
       bClearOnlyClearConfigChanged marks if latest clear issued for current render has different clear config (clear color and clear mask) compared with that used in last clear only render.
       aui32LastClearOnlyClearValue and ui8LastClearOnlyClearColourType save clear value used for last clear only render
     */
    IMG_UINT32          ui32ClearOnlyClearFlag;
    IMG_UINT32          ui32PartialClearOnlyClearFlag;
    IMG_UINT32          bClearOnlyClearConfigChanged;

    IMG_UINT32          ui32LastClearOnlyClearFlag;
    IMG_UINT32          aui32LastClearOnlyClearValue[6];
    IMG_UINT8           ui8LastClearOnlyClearColourType;

    /* (Non-default) Framebuffer object containing this render surface */
    void                *pvFrameBuffer;

    /* Which gc was the last to start a frame with */
    void				*gc;

	/*
	 * Strided shadow buffer used when rendering to a sub-tile-sized
	 * depth/stencil twiddled texture.
	 */
	EGLShadowBuffer		sShadowBuffer;

	/*
	 * Optional limit on the maximum number of pending 3Ds (passed to
	 * RGXKickTA). 0 means use the driver's default.
	 */
	IMG_UINT32			ui32MaxPending3D;

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	EGLWorkloadCharacteristics sWorkloadCharacteristics;
#endif

	/* references to the current job on the resource manager TA/3D HWQueues */
	RMJob		*psCurrentTAJob;
	RMJob		*psCurrent3DJob;

	RMResource	sRMResource;
	RMResource	sRMGhostResource;

	IMG_UINT32	ui32LoadColourAttachments;

	IMG_BOOL	bMSAAScratchBuffer;
	MSAAScratchBuffer sMSAAScratchBuffer;

#if defined(GLES3_EXTENSION_MULTISAMPLED_RENDER_TO_TEXTURE2)
	IMG_INT32   i32KickReason;
	IMG_BOOL    bUsingMSRTT2;
#endif

#if defined(SUPPORT_STRIP_RENDERING)
	IMG_BOOL    bStripRendering;
#endif

	PRGX_RENDERCONTEXT hRenderContext;

	/* HWPerf info for kick */
	RGX_HWPERF_OGLES_KICK_DATA sGlesKick;

#if defined(GTRACE_TOOL)
	uint64_t						ui64StartFrameIndex;
	uint64_t						ui64RenderSurfaceID;
	struct GMemoryDumpManagerTAG	*psTAMemoryDumpManager;
	struct GMemoryDumpManagerTAG	*ps3DMemoryDumpManager;
#endif

	RGXFWIF_CMDTA               sTACmd;
	RGXFWIF_CMD3D               s3DCmd;

} EGLRenderSurface;

/* EGL drawable flags */

/* Indicates that the drawable has an area of 0 */
#define EGLDRAWABLE_FLAGS_ZERO_AREA         (1 << 0)

/*
 * Indicates to client drivers that the memory descriptor(s) should be passed
 * to all functions that result in HW access to the memory represented by the
 * descriptor(s) in order to perform the necessary synchronisation.
 */
#define EGLDRAWABLE_FLAGS_BUFFER_SYNC       (1 << 1)

typedef struct EGLDrawableParamsTAG
{
	/* Render */

	/*
	 * If eRotationAngle = FLIP_Y which means it is PBUFFER or FBO surface (the layout is flipped
	 * compared with OGL display). The meaning of this variable is the same as on rogueddk v1
	 *
	 * Window surface: 		eRotationAngle = IMG_ROTATION_0DEG, IMG_ROTATION_90DEG, IMG_ROTATION_180DEG, IMG_ROTATION_270DEG
	 * PBUFFER surface: 	eRotationAngle = FLIP_Y
	 * FBO surface: 		eRotationAngle = FLIP_Y
	 *
	 * on S8XT,
	 * if eRotationAngle != FLIP_Y, since our HW is top-origin, we need to perform y flip by PBE
	 * If eRotationAngle == FLIP_Y, we don't need to perform Y FLIP since the memory layout is the same as our HW output
	 */
	IMG_ROTATION			eRotationAngle;

	IMG_UINT32				ui32Width;
	IMG_UINT32				ui32Height;

	/*
	 * This is the allocated sample count for surface, it is 0 if it is
	 * antialiasing render but it is resolved by PBE
	 */
	IMG_UINT32				ui32Samples;

	IMG_UINT32				ui32StrideInBytes;

	/* If YUV, the Y plane stride in texels */
    IMG_UINT32				ui32YPlaneStrideInTexels;

	IMG_UINT32				ui32CpuVirtAddrOffsetInBytes;
	IMG_DEV_VIRTADDR		sRealHWAddress;
	IMG_DEV_VIRTADDR		asHWSurfaceAddress[EGL_MAX_PLANES];

	PVRSRV_MEMDESC			ahMemDesc[EGL_MAX_PLANES];

	FBCDC_STATE_TABLE_ALLOCATION	sFBCDCData;

#if defined(GTRACE_TOOL)
	IMG_UINT64				ui64AllocationID;
	const IMG_CHAR			*pszAlloationContext;
#endif

#if defined(PDUMP)
	PVRSRV_PDUMP_IMAGE_DESCRIPTOR sPDumpImageDesc;
#endif

	IMG_PIXFMT				eIMGPixFmt;
	IMG_YUV_COLORSPACE		eColorspace;
	IMG_MEMLAYOUT			eMemLayout;
	IMG_FB_COMPRESSION		eFBCompression;
	IMG_UINT32              ui32Layers;

	PVRSRV_FENCE			hFence;
	PVRSRV_FENCE			hAccumFence;
#if defined(EGL_EXTENSION_PARTIAL_UPDATES)
	IMG_INT32               i32BufferAge;
#endif

	IMG_BOOL                bSingleBuffered;

	/* Source / accum */

	IMG_ROTATION			eAccumRotationAngle;
	IMG_UINT32				ui32AccumWidth;
	IMG_UINT32				ui32AccumHeight;

	IMG_UINT32				ui32AccumStrideInBytes;

    IMG_UINT32				ui32AccumYPlaneStrideInTexels;

	IMG_DEV_VIRTADDR		sRealAccumHWAddress;
	IMG_DEV_VIRTADDR		asAccumHWAddress[EGL_MAX_PLANES];
	PVRSRV_MEMDESC			ahAccumMemDesc[EGL_MAX_PLANES];

	FBCDC_STATE_TABLE_ALLOCATION	sAccumFBCDCData;

#if defined(GTRACE_TOOL)
	IMG_UINT64				ui64AccumAllocationID;
	const IMG_CHAR			*pszAccumAlloationContext;
#endif

	IMG_PIXFMT				eAccumIMGPixFmt;
	IMG_FB_COMPRESSION		eAccumFBCompression;

	EGLRenderSurface		*psRenderSurface;
	EGLDrawableType			eDrawableType;
	IMG_UINT32				ui32Flags;

	/* Set to non-zero if the rendering should be offset to account for PBE rotation error */
	IMG_INT32				i32XOffset;
	IMG_INT32				i32YOffset;

	/* CPU copy of FBC Descriptor data */
	RGX_FBCDC_STATE_DESCRIPTOR_FORMAT asFBCDescData[EGL_MAX_PLANES];
	RGX_FBCDC_STATE_DESCRIPTOR_FORMAT asAccumFBCDescData[EGL_MAX_PLANES];
} EGLDrawableParams;

#if defined(GTRACE_TOOL)
struct GTraceTAG;
#endif

typedef struct EGLcontextModeRec
{
	IMG_UINT32 ui32ContextPriority;

	IMG_UINT32 ui32MaxViewportX;
	IMG_UINT32 ui32MaxViewportY;

	IMG_UINT32 ui32ContextMajorVersion;
	IMG_UINT32 ui32ContextSupportedMajorVersion;
	IMG_UINT32 ui32ContextMinorVersion;
	IMG_UINT32 ui32ContextFlags;
	IMG_UINT32 ui32ContextProfile;

	/* Does the WSEGL module support getting of image parameters? */
	IMG_BOOL bImageExternal;

	IMG_BOOL   bIsRobustContext;
	IMG_UINT32 ui32ResetBehaviour;

	IMG_BOOL   bIsProtectedContext;

    IMG_UINT32 ui32ContextID;

#if defined(GTRACE_TOOL)
    struct GTraceTAG *psGTrace;
#endif

} EGLcontextMode;


typedef struct EGLsurfaceModeRec
{
	IMG_UINT32 ui32AntiAliasMode;

	IMG_UINT32 ui32RedBits;		/* bits per comp */
	IMG_UINT32 ui32GreenBits;
	IMG_UINT32 ui32BlueBits;
	IMG_UINT32 ui32AlphaBits;

	IMG_UINT32 ui32ColorBits;	/* total bits for rgb */

	IMG_UINT32 ui32DepthBits;

	IMG_UINT32 ui32StencilBits;

#if defined(EGL_EXTENSION_YUV_SURFACE)
	IMG_UINT32 ui32YUVCSC;
	IMG_UINT32 ui32YUVBPP;
	IMG_UINT32 ui32YUVPlanes;
#endif

} EGLsurfaceMode;


/* Flags */
#define EGLIMAGE_FLAGS_COMPOSITION_SYNC     (1 << 0)

/*
 * Indicates to client drivers that the memory descriptor(s) should be passed
 * to all functions that result in HW access to the memory represented by the
 * descriptor(s) in order to perform the necessary synchronisation.
 */
#define EGLIMAGE_FLAGS_BUFFER_SYNC          (1 << 1)

typedef struct IMGEGLImageRec
{
	IMG_UINT32				ui32Width;
	IMG_UINT32				ui32Height;

	IMG_UINT32				ui32Depth;
	IMG_UINT32				ui32ZOffset;

	IMG_PIXFMT				eIMGPixFmt;
	IMG_UINT				uiRequestedFormat;
	IMG_YUV_COLORSPACE		eColorSpace;
	IMG_YUV_CHROMA_INTERP	eChromaUInterp;
	IMG_YUV_CHROMA_INTERP	eChromaVInterp;
	IMG_FB_COMPRESSION		eFBCompression;
	IMG_MEMLAYOUT			eMemLayout;

	/* Used for distinction between compressed RGB/RGBA variants */
	IMG_BOOL				bCompressedRGBOnly;

	/* Stride in bytes. Note: avoid to use ui32Stride for YUV, instead,
	 * try to use sYUVInfo.ui32Plane0StrideInTexels */
	IMG_UINT32				ui32Stride;

	IMG_UINT32				ui32CpuVirtAddrOffsetInBytes;
	IMG_DEV_VIRTADDR		asHWSurfaceAddress[EGL_MAX_PLANES];
	IMG_DEV_VIRTADDR		asRealDeviceAddr[EGL_MAX_PLANES];
	PVRSRV_MEMINFO			*apsMemInfo[EGL_MAX_PLANES];

#if defined(PDUMP)
	/* for Pdump only, it is the actual virtual address specified in the pdump image description
	 * it is ony required for if the image is FBCDC compressed */
	IMG_DEV_VIRTADDR 		sPdumpFBCTexDataAddr;
#endif

	void					*hImage;

	/* Original parameters of source */
	IMG_UINT32				ui32Target;
	uintptr_t				uBuffer;
	IMG_UINT32				ui32Level;
	IMG_UINT32				ui32PlaneOffset;

	/* Flags */
	IMG_UINT32              ui32Flags;

	/* if YUV */
	YUV_INFO				sYUVInfo;

	IMG_BOOL				bIsProtected;

	/* The number of layers if this is a 2D array texture */
	IMG_UINT32				ui32Layers;
	/* The size of a full mipchain, padded to array stride, for a single layer */
	IMG_UINT32				ui32LayerMipChainSize;

#if defined(GTRACE_TOOL)
	IMG_HANDLE              hEGLImageKHR;	/* No use under GTRACE_TOOL */
	IMG_CHAR                *pszSourceName;

	IMG_UINT32              ui32NumPagesTotal;
	IMG_BOOL        		*pbUsedPageMap;

	IMG_CHAR				*pszSourceAllocationTag;
#endif

	IMG_UINT32				ui32CYUVMode;
	IMG_DEV_VIRTADDR		sDevHeaderVirtAddr;

#if RGX_FBC_MAX_DESCRIPTORS > 0
	/* CPU copy of FBC descriptor */
	IMG_BOOL				bFBCDescDataValid;
	RGX_FBCDC_STATE_DESCRIPTOR_FORMAT asFBCDescData[EGL_MAX_PLANES];
#endif

} IMGEGLImage;

typedef struct _IMGEGLBuffer
{
	PVRSRV_MEMDESC		hMemDesc;
	IMG_DEV_VIRTADDR	sDevVirtAddr;
	IMG_UINT32			ui32Size;
} IMGEGLBuffer;

typedef enum IMG_EGLERROR_TAG
{
	IMG_EGL_NO_ERROR			  = 0,
	IMG_EGL_GENERIC_ERROR		  = 1,
	IMG_EGL_SCENE_LOST			  = 2,
	IMG_EGL_MEMORY_INVALID_ERROR  = 3,
	IMG_EGL_BAD_ACCESS			  = 4,
	IMG_EGL_BAD_PARAMETER		  = 5,
	IMG_EGL_BAD_MATCH			  = 6,
	IMG_EGL_OUT_OF_MEMORY		  = 7,

} IMG_EGLERROR;

#if defined(EGL_EXTENSION_IMG_IMAGE_DEBUG_DUMP)
typedef void (*PFNEGLIMAGEREFERENCEDCALLBACK)(const void* pvPrivateData, EGLImageKHR image);
#endif

typedef struct IMG_OGLES1EGL_Interface_TAG
{
	IMG_UINT32	ui32APIVersion;

	GPA_PROC (*pfnGLESGetProcAddress)(const IMG_CHAR *procname);

	IMG_BOOL (*pfnGLESCreateGC)(SrvSysContext *, EGLContextHandle *, EGLcontextMode *, EGLContextHandle);
	IMG_BOOL (*pfnGLESDestroyGC)(EGLContextHandle);
	IMG_EGLERROR (*pfnGLESMakeCurrentGC)(EGLRenderSurface *, EGLRenderSurface *, EGLContextHandle, EGLsurfaceMode *);
	void (*pfnGLESMakeUnCurrentGC)(void);
	IMG_EGLERROR (*pfnGLESFlushBuffersGC)(EGLContextHandle, EGLRenderSurface *, IMG_BOOL, IMG_BOOL, IMG_BOOL, PVRSRV_FENCE *);

#if defined(EGL_EXTENSION_RENDER_TO_TEXTURE)
	IMG_BOOL (*pfnGLESBindTexImage)(EGLContextHandle, EGLDrawableHandle, EGLDrawableHandle*);
	void (*pfnGLESReleaseTexImage)(EGLContextHandle, EGLDrawableHandle, EGLDrawableHandle*);
#endif /* defined(EGL_EXTENSION_RENDER_TO_TEXTURE) */

	IMG_EGLERROR (*pfnGLESGetImageSource)(EGLContextHandle, IMG_UINT32, IMG_UINT32, IMG_UINT32, IMGEGLImage*);

	void (*pfnGLESMarkRenderSurfaceAsInvalid)(EGLContextHandle);

	IMG_BOOL (*pfnGLESInsertFenceSyncGC)(EGLContextHandle, RMTask **, PVRSRV_FENCE*, IMG_BOOL);
	IMG_BOOL (*pfnGLESAddFenceSyncGC)(EGLContextHandle, RMTask *, PVRSRV_FENCE);
#if defined(EGL_EXTENSION_IMG_IMAGE_DEBUG_DUMP)
	void (*pfnGLESSetEGLImageReferencedCallback)(EGLContextHandle, PFNEGLIMAGEREFERENCEDCALLBACK, const void* pvPrivateData);
#endif

} IMG_OGLES1EGL_Interface;


/* According to http://www.opengl.org/registry/api/enum.spec,
 *
 *    If an extension is experimental, allocate temporary enum values in the
 *    range 0x6000-0x8000 during development work.
 *
 * So by using enums in that range we ensure that they are not going to clash
 * with any standard GL enum ever.
 */
#define IMG_OGLES1_FUNCTION_TABLE			0x6500
#define IMG_OGLES1_FUNCTION_TABLE_VERSION	3

typedef enum IMGEGL_DEBUGTYPE_TAG
{
	IMGEGL_DEBUGTYPE_ERROR					= 0,
	IMGEGL_DEBUGTYPE_DEPRECATED_BEHAVIOR	= 1,
	IMGEGL_DEBUGTYPE_UNDEFINED_BEHAVIOR		= 2,
	IMGEGL_DEBUGTYPE_PERFORMANCE			= 3,
	IMGEGL_DEBUGTYPE_PORTABILITY			= 4,
	IMGEGL_DEBUGTYPE_OTHER					= 5,
	IMGEGL_DEBUGTYPE_MARKER					= 6,

} IMGEGL_DEBUGTYPE;

typedef struct IMG_OGLES3EGL_Interface_TAG
{
	IMG_UINT32	ui32APIVersion;

	GPA_PROC (*pfnGLESGetProcAddress)(const IMG_CHAR *procname);

	IMG_BOOL (*pfnGLESCreateGC)(SrvSysContext *, EGLContextHandle *, EGLcontextMode *, EGLContextHandle);
	IMG_BOOL (*pfnGLESDestroyGC)(EGLContextHandle);
	IMG_EGLERROR (*pfnGLESMakeCurrentGC)(EGLRenderSurface *, EGLRenderSurface *, EGLContextHandle, EGLsurfaceMode *);
	void (*pfnGLESMakeUnCurrentGC)(void);

	IMG_EGLERROR (*pfnGLESFlushBuffersGC)(EGLContextHandle, EGLRenderSurface *, IMG_BOOL, IMG_BOOL, IMG_BOOL, PVRSRV_FENCE *);

	IMG_EGLERROR (*pfnGLESGetImageSource)(EGLContextHandle, IMG_UINT32, IMG_UINT32, IMG_UINT32, IMG_UINT32, IMGEGLImage*);

	void (*pfnGLESMarkRenderSurfaceAsInvalid)(EGLContextHandle);

	IMG_BOOL (*pfnGLESInsertFenceSyncGC)(EGLContextHandle, RMTask **, PVRSRV_FENCE*, IMG_BOOL);

	IMG_BOOL (*pfnGLESAddFenceSyncGC)(EGLContextHandle, RMTask *, PVRSRV_FENCE);

	void (*pfnGLESLogEGLDebugMesg)(EGLContextHandle, IMGEGL_DEBUGTYPE, const IMG_CHAR *, ...) __printf(3, 4);

#if defined(EGL_EXTENSION_IMG_IMAGE_DEBUG_DUMP)
	void (*pfnGLESSetEGLImageReferencedCallback)(EGLContextHandle, PFNEGLIMAGEREFERENCEDCALLBACK, const void* pvPrivateData);
#endif

} IMG_OGLES3EGL_Interface;

/* According to http://www.opengl.org/registry/api/enum.spec,
 *
 *    If an extension is experimental, allocate temporary enum values in the
 *    range 0x6000-0x8000 during development work.
 *
 * So by using enums in that range we ensure that they are not going to clash
 * with any standard GL enum ever.
 */
#define IMG_OGLES3_FUNCTION_TABLE		  0x7500
#define IMG_OGLES3_FUNCTION_TABLE_VERSION 1


/**************************************************************
 *
 *         EGL apphints.  (used by Mesa integration)
 *
 **************************************************************/

typedef struct
{
	IMG_UINT32	ui32PDSFragBufferSize;
	IMG_UINT32	ui32ConstFragBufferSize;
	IMG_UINT32	ui32USCFragBufferSize;
	/* Buffer sizes are multiplied by ui32SparseBufferScale to decide how much virtual memory to allocate for each buffer. If 0, non-sparse.*/
	IMG_UINT32	ui32SparseBufferScale;
	IMG_BOOL	bZeroBufferStartingSize;

	IMG_UINT32	ui32PoolBufferListMin;
	IMG_UINT32	ui32PoolBufferListMax;

	IMG_UINT32	ui32ParamBufferSize;
	IMG_UINT32	ui32MaxParamBufferSize;

	IMG_UINT32	ui32ExternalZBufferMode;

	IMG_CHAR	szWindowSystem[APPHINT_MAX_STRING_SIZE];

#if defined (TIMING) || defined (DEBUG)
	IMG_BOOL	bDumpProfileData;
	IMG_UINT32	ui32ProfileStartFrame;
	IMG_UINT32	ui32ProfileEndFrame;
	IMG_BOOL	bDisableMetricsOutput;
#endif /* (TIMING) || (DEBUG) */

	IMG_BOOL	bDisableFBCDC;
	IMG_BOOL	bDisableFBCDCTilePacking;

	IMG_UINT32	ui32TextureCacheMaxEntries;
	IMG_UINT32	ui32TextureCacheEntrySize;

#if defined(DEBUG)
	IMG_UINT32	ui32MemGuardPage;
#endif

	IMG_BOOL	bAsyncTQMFreeing;

	IMG_UINT32 ui32RenderTargetCacheMaxEntries;
	IMG_UINT32 bRenderTargetCacheEnable;
	IMG_UINT32 bRenderTargetCacheStats;

	IMG_UINT32 bMipgenMultipartSubmit;

	IMG_BOOL bEnableZeroOnAlloc;

	IMG_UINT32 bSingleComputeContextPerProcess;

	/* Defer the allocation of the global PB until first use */
	IMG_BOOL bDeferGlobalPB;


#if defined(GTRACE_TOOL)
	IMG_BOOL	bEnableESTraceHWSetup;
	IMG_BOOL	bEnableESTraceBuffer;
#endif

} IMGEGLAppHints;

#define IMG_EGLGL_ALLOC_ATTRIB_MODIFY(bEnableZeroOnAlloc, uiAttribs) (!(bEnableZeroOnAlloc) ? (uiAttribs) : ((uiAttribs) | PVRSRV_MEMALLOCFLAG_ZERO_ON_ALLOC))

/**************************************************************
 *
 *                       EGL -->  OpenGL
 *
 **************************************************************/

typedef struct IMG_OGLEGL_Interface_TAG
{
	IMG_UINT32	ui32APIVersion;

	GPA_PROC (*pfnGLGetProcAddress)(const IMG_CHAR *procname);

	IMG_BOOL (*pfnGLCreateGC)(SrvSysContext *, EGLContextHandle *, EGLcontextMode *, EGLContextHandle);
	IMG_BOOL (*pfnGLDestroyGC)(EGLContextHandle);
	IMG_EGLERROR (*pfnGLMakeCurrentGC)(EGLRenderSurface *, EGLRenderSurface *, EGLContextHandle, EGLsurfaceMode *);
	void (*pfnGLMakeUnCurrentGC)(void);
	IMG_EGLERROR (*pfnGLFlushBuffersGC)(EGLContextHandle, IMG_BOOL, IMG_BOOL);

#if defined(EGL_EXTENSION_RENDER_TO_TEXTURE)
	IMG_BOOL (*pfnGLBindTexImage)(EGLContextHandle, EGLDrawableHandle, EGLDrawableHandle* );
	void (*pfnGLReleaseTexImage)(EGLContextHandle, EGLDrawableHandle,EGLDrawableHandle* );
#endif

	void (*pfnGLMarkRenderSurfaceAsInvalid)(EGLContextHandle);

	IMG_BOOL (*pfnGLFreeResources)(SrvSysContext *);

#if defined(LINUX)
	void (*pfnGLSetDisplayFrontBufferCallback)(void (*pfnCallback)(void *));
#endif
} IMG_OGLEGL_Interface;


/* According to http://www.opengl.org/registry/api/enum.spec,
 *
 *    If an extension is experimental, allocate temporary enum values in the
 *    range 0x6000-0x8000 during development work.
 *
 * So by using enums in that range we ensure that they are not going to clash
 * with any standard GL enum ever.
 */
#define IMG_OGL_FUNCTION_TABLE		   0x7800
#define IMG_OGL_FUNCTION_TABLE_VERSION 1


/**************************************************************
 *
 *                       EGL -->  OpenCL
 *
 **************************************************************/
typedef struct IMG_OCLEGL_Interface_TAG
{
	IMG_UINT32	ui32APIVersion;

	IMG_EGLERROR (*pfnCLGetImageSource)(SrvSysContext *, IMG_UINT32, void *, IMGEGLImage*);

} IMG_OCLEGL_Interface;

#define IMG_OCL_FUNCTION_TABLE		 0x6200

#define IMG_OCL_FUNCTION_TABLE_VERSION 1


void IMG_CALLCONV KEGLSurfaceBind(IMG_HANDLE hSurface);

void IMG_CALLCONV KEGLSurfaceUnbind(SrvSysContext *pvrsrv, IMG_HANDLE hSurface);

__must_check IMG_BOOL IMG_CALLCONV KEGLAcquireSurfaceCPUMapping(IMG_HANDLE hSurface, PVRSRV_MEMDESC hMemDesc, void **ppvCpuVirtAddr);

void IMG_CALLCONV KEGLReleaseSurfaceCPUMapping(IMG_HANDLE hSurface, PVRSRV_MEMDESC hMemDesc);

__must_check IMG_BOOL IMG_CALLCONV KEGLAcquireImageCPUMapping(IMG_HANDLE hImage, void **ppvCpuVirtAddr);

void IMG_CALLCONV KEGLReleaseImageCPUMapping(IMG_HANDLE hImage);

__must_check IMG_BOOL IMG_CALLCONV KEGLCreateRenderSurface(SrvSysContext *psSysContext,
														EGLDrawableParams *psParams,
														IMG_UINT32 ui32SampleCount,
														IMG_UINT32 ui32NumRTALayers,
														IMG_PIXFMT eZSBufferPixFmt,
														IMG_BOOL bCreateMSAAScratchBuffer,
														IMG_BOOL bProtected,
														EGLRenderSurface *psSurface);

IMG_BOOL IMG_CALLCONV KEGLDestroyRenderSurface(SrvSysContext *psSysContext,
											   EGLRenderSurface *psSurface,
											   RMContext *psRMContext);

__must_check IMG_BOOL IMG_CALLCONV KEGLResizeRenderSurface(SrvSysContext *psSysContext,
														 EGLDrawableParams *psParams,
														 IMG_UINT32 ui32SampleCount,
														 IMG_PIXFMT eZSBufferPixFmt,
														 EGLRenderSurface *psSurface,
														 RMContext *psRMContext);

__must_check IMG_BOOL IMG_CALLCONV KEGLGetDrawableParameters(EGLDrawableHandle hDrawable,
														   EGLDrawableParams *ppsParams,
														   IMG_BOOL bAllowSurfaceRecreate);

__must_check IMG_BOOL IMG_CALLCONV KEGLFlagStartFrame(EGLDrawableHandle hDrawable);

void IMG_CALLCONV KEGLBindImage(void *hImage);
void IMG_CALLCONV KEGLUnbindImage(void *hImage);

__must_check IMG_EGLERROR IMG_CALLCONV KEGLGetImageSource(void *hEGLImage, IMGEGLImage **ppsImage);
__must_check IMG_BOOL IMG_CALLCONV KEGLGetPoolBuffers(EGLRenderSurface *psRenderSurface, IMG_UINT32 ui32HWPerfClientFilterCached, void (*pfnGetHWPerfInfo)(void*), RMContext *psRMCtx);
IMG_BOOL IMG_CALLCONV KEGLReleasePoolBuffers(EGLRenderSurface *psRenderSurface
#if defined(GTRACE_TOOL)
	, struct GTraceTAG *psGTrace
#endif
	);

__must_check IMG_EGLERROR IMG_CALLCONV KEGLGetBufferSource(SrvSysContext *psSrvSysContext, EGLClientBuffer hClientBuffer, IMGEGLBuffer *psEGLBuffer);

typedef struct _LargeRenderTargetDesc
{
	IMG_UINT32 ui32NumRenderTargets;
	IMG_UINT32 aui32Dims[IMG_MAX_PARALLEL_RENDERS * 2];
} LargeRenderTargetDesc;

__must_check IMG_BOOL IMG_CALLCONV KEGLAssignRenderTarget(EGLDrawableParams *psParams, EGLRenderSurface *psRenderSurface, PRGX_RENDERCONTEXT hRenderContext, IMG_BOOL bIsGLES1, LargeRenderTargetDesc *psLargeRenderTargetDesc);
IMG_BOOL IMG_CALLCONV KEGLReleaseRenderTarget(EGLRenderSurface *psRenderSurface);
void IMG_CALLCONV KEGLFreeRenderTarget(EGLRenderSurface *psRenderSurface);
void IMG_CALLCONV KEGLCleanPoolRT(SrvSysContext *psSysContext);
void IMG_CALLCONV KEGLEmptyPoolRT(SrvSysContext *psSysContext);

IMG_INTERNAL void KEGLGetAppHints(IMGEGLAppHints *psAppHints);

#if defined(EGL_EXTENSION_ANDROID_BLOB_CACHE) || defined(PVR_BLOB_CACHE)
void IMG_CALLCONV KEGLSetBlob(IMG_UINT64 ui64Key, const void * pvBlob, IMG_UINT32 ui32BlobSize);
__must_check IMG_UINT32 IMG_CALLCONV KEGLGetBlob(IMG_UINT64 ui64Key, void ** pvBlob);
void IMG_CALLCONV KEGLFreeBlob(void *pvBlob);
#endif

__must_check IMG_BOOL IMG_CALLCONV KEGLInitRenderContext(SrvSysContext *psSysContext,
		PRGX_RENDERCONTEXT *phRenderContext,
		IMG_UINT32 ui32ContextPriority,
		IMG_CHAR cAPI,
		IMG_UINT64
		ui64RobustnessAddress,
		IMG_UINT32 ui32MaxTADeadlineMS,
		IMG_UINT32 ui32Max3DDeadlineMS);

#if defined(DEBUG)

__must_check PVRSRV_ERROR IMG_CALLCONV
KEGLAllocDeviceMemTrack(SrvSysContext *psSysContext,
			const IMG_CHAR *pszFile,
			IMG_UINT32 u32Line,
			PVRSRV_HEAP hDevMemHeap,
			PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
			IMG_DEVMEM_SIZE_T uiSize,
			IMG_DEVMEM_ALIGN_T uiAlign,
			const IMG_CHAR *pszText,
			PVRSRV_MEMINFO **ppsMemInfo);

__must_check PVRSRV_ERROR IMG_CALLCONV
KEGLAllocZSBufferMemTrack(SrvSysContext *psSysContext,
						IMG_HANDLE hDevMemHeap,
						const IMG_CHAR *pszFile,
						IMG_UINT32 ui32Line,
						PVRSRV_DEV_CONNECTION *psDevConnection,
						const PVRSRV_DEVMEMCTX hDevMemCtx,
						PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
						IMG_DEVMEM_SIZE_T uiSize,
						IMG_DEVMEM_LOG2ALIGN_T uiLog2Alignment,
						IMG_BOOL bOnDemand,
						IMG_BOOL bIsProtected,
						PVRSRV_MEMDESC *phMemDesc,
						IMG_DEV_VIRTADDR *psDeviceVirtAddr,
						PRGX_ZSBUFFER  *ppsZSBuffer,
						PVRSRV_MEMINFO **ppsMemInfo,
						const IMG_CHAR *pszText);

__must_check PVRSRV_ERROR IMG_CALLCONV
KEGLOverAllocDeviceMemTrack(IMG_UINT32 uiMultiplier,
			SrvSysContext *psSysContext,
			const IMG_CHAR *pszFile,
			IMG_UINT32 u32Line,
			PVRSRV_HEAP hDevMemHeap,
			PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
			IMG_DEVMEM_SIZE_T uiSize,
			IMG_DEVMEM_ALIGN_T uiAlign,
			const IMG_CHAR *pszText,
			PVRSRV_MEMINFO **ppsMemInfo);

__must_check PVRSRV_ERROR IMG_CALLCONV
KEGLAllocDeviceMemSparseTrack(SrvSysContext *psSysContext,
			      const IMG_CHAR *pszFile,
			      IMG_UINT32 u32Line,
			      PVRSRV_HEAP hDevMemHeap,
			      const PVRSRV_DEVMEMCTX hDevMemCtx,
			      IMG_DEVMEM_SIZE_T uiChunkSize,
			      IMG_UINT32 ui32NumPhysChunks,
			      IMG_UINT32 ui32NumVirtChunks,
			      IMG_BOOL *pabMappingTable,
			      IMG_DEVMEM_ALIGN_T uiAlign,
			      DEVMEM_FLAGS_T uiFlags,
			      const IMG_CHAR *pszText,
			      PVRSRV_MEMINFO **ppsMemInfo);

__must_check IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV
KEGLAllocSecureDeviceMem(SrvSysContext *psSysContext,
			      const IMG_CHAR *pszFile,
			      IMG_UINT32 ui32Line,
			      const PVRSRV_DEVMEMCTX hDevMemCtx,
			      PVRSRV_HEAP hDevMemHeap,
			      IMG_DEVMEM_SIZE_T uiSize,
			      IMG_DEVMEM_SIZE_T uiChunkSize,
			      IMG_UINT32 ui32NumPhysChunks,
			      IMG_UINT32 ui32NumVirtChunks,
			      IMG_BOOL *pabMappingTable,
			      DEVMEM_FLAGS_T uiFlags,
			      const IMG_CHAR *pszText,
			      PVRSRV_MEMINFO **ppsMemInfo);

PVRSRV_ERROR IMG_CALLCONV
KEGLAllocDeviceMemExportableTrack(SrvSysContext *psSysContext,
				  const IMG_CHAR *pszFile,
				  IMG_UINT32 u32Line,
				  PVRSRV_HEAP hDevMemHeap,
				  PVRSRV_DEV_CONNECTION *psDevConnection,
				  PVRSRV_MEMALLOCFLAGS_T uiMemAllocFlags,
				  IMG_DEVMEM_SIZE_T uiSize,
				  IMG_DEVMEM_ALIGN_T uiAlign,
				  const IMG_CHAR *pszText,
				  PVRSRV_MEMINFO **ppsMemInfo);

void IMG_CALLCONV
KEGLFreeDeviceMemTrack(SrvSysContext *psSysContext,
		       const IMG_CHAR *pszFile,
		       IMG_UINT32 ui32Line,
		       PVRSRV_MEMINFO *psMemInfo);

void IMG_CALLCONV
KEGLDevmemPDumpSaveToFileVirtual(DEVMEM_MEMDESC *psMemDesc,
                             IMG_DEVMEM_OFFSET_T uiOffset,
                             IMG_DEVMEM_SIZE_T uiSize,
                             const IMG_CHAR *pszFilename);

#endif /* defined(DEBUG) */

#if defined (__cplusplus)
}
#endif

#endif /* _EGLAPI_H_ */

/******************************************************************************
 End of file (eglapi.h)
******************************************************************************/
