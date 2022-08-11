/*************************************************************************/ /*!
@File
@Title          RGX API Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported RGX API details
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXAPI_H
#define RGXAPI_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__linux__)
	#if defined(__KERNEL__)
		#include <asm/unistd.h>
	#else
		#include <unistd.h>
	#endif
#else
#endif

#include "rgxdefs.h"
#include "rgx_heaps.h"
#include "rgx_hwperf.h"
#include "rgx_common.h"
#include "services.h"
#include "img_3dtypes.h"
#include "pvr_buffer_sync_shared.h"
#include "pvrsrv_devmem.h"	/* Required for PVRSRV_DEVMEMCTX */
#include "imgpixfmts.h"

#include "rgxapi_dm_common.h"

#include "pvrsrv_devvar.h"
#include "pvrsrv_sync_um.h"

#include "rgx_fwif_client.h"
#include "pvrsrv_devmem_pdump.h"


/******************************************************************************
 Structures used in API calls
******************************************************************************/
/* Opaque render context handle used with RGXKickTA */
typedef struct RGX_RENDERCONTEXT_TAG *PRGX_RENDERCONTEXT;

/* Opaque render target handle used with RGXKickTA */
typedef struct RGX_RTDATASET_TAG *PRGX_RTDATASET;

/* Opaque ZS-Buffer handle */
typedef struct RGX_ZSBUFFER_TAG *PRGX_ZSBUFFER;

/* Opaque global parameter buffer handle */
typedef struct RGX_GLOBALPB_TAG *PRGX_GLOBALPB;

#ifndef RGX_DEV_MEM_TYPEDEFS
#define RGX_DEV_MEM_TYPEDEFS
/* Opaque RGX device memory context handle */
typedef struct RGX_DEVMEMCONTEXT_TAG *PRGX_DEVMEMCONTEXT;
#endif

typedef struct RGX_KICKSYNCCONTEXT_TAG *PRGX_KICKSYNCCONTEXT;

typedef struct RMResource_TAG RMResource;
typedef struct RMJob_TAG  RMJob;
typedef struct RMResourceManager_TAG  RMResourceManager;
typedef struct RMContext_TAG  RMContext;
typedef struct RMHWQueue_TAG RMHWQueue;


/*!
   Parameters when making a Create Render-Context Request
*/
typedef struct RGX_CREATERENDERCONTEXT_TAG
{
	IMG_UINT32				ui32ContextFlags;               /*!< Flags which specify properties of the context */
	IMG_HANDLE				hDevCookie;                     /*!< Identifies the device to be associated with the new Render Context */
	PVRSRV_DEVMEMCTX		hDevMemContext;                 /*!< DeviceMem Context */
	IMG_UINT32				ui32VisTestResultBufferSize;    /*!< Occlusion query visibility test results buffer */
	IMG_UINT32				ui32Priority;					/*!< Priority of context */
	IMG_CHAR                cClientAPI;                     /*!< A 1 character tag describing the client API requesting this render
	                                                             context, this will have the DM's resource tag (ie. RC) appended to it.
                                                                 Used in naming resources associated with this context, eg. Timeline. */
	IMG_UINT64				ui64RobustnessAddress;
	IMG_UINT32				ui32MaxTADeadlineMS;
	IMG_UINT32				ui32Max3DDeadlineMS;
} RGX_CREATERENDERCONTEXT,      /*!< Convenience typedef for struct _RGX_CREATERENDERCONTEXT_ */
  *PRGX_CREATERENDERCONTEXT;    /*!< Convenience typedef for ptr to struct _RGX_CREATERENDERCONTEXT_ */


#define RGX_KICKTA_DUMPBITMAP_MAX_NAME_LENGTH		256 /*!< Max Name Length of KickTA Dump Bitmap */

/*!
	Structure for dumping bitmaps
*/
typedef struct RGX_KICKTA_DUMPBITMAP_TAG
{
	IMG_DEV_VIRTADDR	sDevBaseAddr;                       /*!< Device-Virtual Base Address */
	IMG_UINT32			ui32AddrMode;                       /*!< Address mode */
	IMG_UINT32			ui32Width;                          /*!< Bitmap width */
	IMG_UINT32			ui32Height;                         /*!< Bitmap height */
	IMG_UINT32			ui32Stride;                         /*!< Bitmap stride */
	IMG_UINT32			ui32Size;                           /*!< Bitmap size */
	IMG_UINT32			ui32PDUMPFormat;                    /*!< PDump Format */
	IMG_UINT32			ui32BytesPP;                        /*!< Number of bytes per pixel */
	IMG_CHAR			pszName[RGX_KICKTA_DUMPBITMAP_MAX_NAME_LENGTH]; /*!< Bitmap Name */
} RGX_KICKTA_DUMPBITMAP,        /*!< Convenience typedef for struct _RGX_KICKTA_DUMPBITMAP_ */
  *PRGX_KICKTA_DUMPBITMAP;      /*!< Convenience typedef for ptr to struct _RGX_KICKTA_DUMPBITMAP_ */

#define PVRSRV_RGX_PDUMP_CONTEXT_MAX_BITMAP_ARRAY_SIZE	(16) /*!< pdump context bitmap max array element size */


typedef RGX_KICK_DUMP_BUFFER RGX_KICKTA_DUMP_BUFFER;
typedef PRGX_KICK_DUMP_BUFFER PRGX_KICKTA_DUMP_BUFFER;


#define RGX_KICKTA_PDUMP_SYNC_FLAG_FORCESYNC					0x1U
#define RGX_KICKTA_PDUMP_SYNC_FLAG_LASTFRAMEONLYSYNC	0x2U


/*!
	PDUMP version of above kick structure
*/
typedef struct RGX_KICKTA_PDUMP_TAG
{
	/* Bitmaps to dump */
	PRGX_KICKTA_DUMPBITMAP		psPDumpBitmapArray;     /*!< bitmap array */
	IMG_UINT32					ui32PDumpBitmapSize;    /*!< bitmap array */

	IMG_UINT32					  ui32PDumpImageDescSize;
	IMG_CHAR					  **ppsImageFileNameArray;
	PVRSRV_PDUMP_IMAGE_DESCRIPTOR *psPDumpImageDescArray;

	/* Misc buffers to dump (e.g. TA, PDS etc..) */
	PRGX_KICKTA_DUMP_BUFFER		psBufferArray;          /*!< buffer array */
	IMG_UINT32					ui32BufferArraySize;    /*!< buffer array size */
	IMG_UINT32					ui32DumpSyncFlags;		/*!< Flags to force render sync to be pdumped (and bitmap) */
} RGX_KICKTA_PDUMP,     /*!< Convenience typedef for struct _RGX_KICKTA_PDUMP_ */
  *PRGX_KICKTA_PDUMP;   /*!< Convenience typedef for ptr to struct _RGX_KICKTA_PDUMP_ */

/*!
  Parameters for a KickTA Submit Request
 */
typedef struct RGX_KICKTA_SUBMIT_TAG
{
	PRGX_KICKTA_PDUMP		psKickPDUMP;                    /*!< pdump kick structure */
	PRGX_RENDERCONTEXT		hRenderContext;                 /*!< Handle to the render context */
} RGX_KICKTA_SUBMIT,    /*!< Convenience typedef for struct _RGX_KICKTA_SUBMIT */
  *PRGX_KICKTA_SUBMIT;  /*!< Convenience typedef for ptr to struct _RGX_KICKTA_SUBMIT */

/*! Parameters for Add Render-Target Request */
typedef struct RGX_ADDRENDTARG_TAG
{
	PRGX_GLOBALPB					hGlobalPB;						/*!< Handle to the global parameter buffer */
	IMG_HANDLE						hDevMemContext;					/*!< Device Memory Context*/
	IMG_HANDLE						hOSEvent;						/*!< OS event object handle */
	IMG_HANDLE						hDevCookie;                     /*!< Handle identifying a Device */
	IMG_UINT32						ui32NumPixelsX;                 /*!< Number of pixels in X direction, in the render target */
	IMG_UINT32						ui32NumPixelsY;                 /*!< Number of pixels in Y direction, in the render target */
	IMG_UINT16						ui16MSAASamplesInX;             /*!< Number of MSAA Samples in X direction */
	IMG_UINT16						ui16MSAASamplesInY;             /*!< Number of MSAA Samples in Y direction */
	IMG_UINT16						ui16NumRTsInArray;              /*!< Number of Render Targets in Array */
	IMG_UINT32						ui32PerRTPMSize;
	IMG_UINT32						ui32PerRenderPMSize;
} RGX_ADDRENDTARG,      /*!< Convenience typedef for struct _RGX_ADDRENDTARG */
  *PRGX_ADDRENDTARG;    /*!< Convenience typedef for ptr to struct _RGX_ADDRENDTARG */

typedef struct RGX_RT_CONFIG_RES_TAG
{
	IMG_UINT32 ui32RTDataSel; /*!< Applied to psRTDataSet->ui32RTDataSel */
	IMG_UINT16 ui16NumRTsInArray; /*!< Applied to psRTDataSet->ui16NumRTsInArray */
	IMG_UINT32 ui32NumPixelsX; /*!< Applied to psRTDataSet->ui32NumPixelsX */
	IMG_UINT32 ui32NumPixelsY; /*!< Applied to psRTDataSet->ui32NumPixelsY */

	/* Derived values */
	IMG_UINT32 ui32TEAA; /*!< Applied to psRTDataSet->ui32TEAA */
	IMG_UINT32 ui32MTileX1; /*!< Applied to psRTDataSet->ui32MTileX1 */
	IMG_UINT32 ui32MTileY1; /*!< Applied to psRTDataSet->ui32MTileY1 */
	IMG_UINT32 ui32ScreenXMax; /*!< Applied to psRTDataSet->ui32ScreenXMax */
	IMG_UINT32 ui32ScreenYMax; /*!< Applied to psRTDataSet->ui32ScreenYMax */
	IMG_UINT32 ui32RgnPageStride; /*!< Applied to psRTDataSet->ui32RgnPageStride */
	IMG_UINT32 ui32TEStride; /*!< Applied to psRTDataSet->ui32TEStride */
	IMG_UINT32 ui32ScreenPixelMax; /*!< Applied to psRTDataSet->ui32ScreenPixelMax */
	IMG_UINT32_FLOAT uISPMergeLowerX; /*!< ISP triangle merge Lower X */
	IMG_UINT32_FLOAT uISPMergeLowerY; /*!< ISP triangle merge Lower Y */
	IMG_UINT32_FLOAT uISPMergeUpperX; /*!< ISP triangle merge Upper X */
	IMG_UINT32_FLOAT uISPMergeUpperY; /*!< ISP triangle merge Upper Y */
	IMG_UINT32_FLOAT uISPMergeScaleX; /*!< ISP triangle merge Scale X */
	IMG_UINT32_FLOAT uISPMergeScaleY; /*!< ISP triangle merge Scale Y */

	/* Derived by SetupMSAA */
	IMG_UINT32 ui32ISPMtileSize; /*!< Applied to psRTDataSet->ui32ISPMtileSize */
	IMG_UINT64 ui64PPPMultiSampleCtl; /*!< Applied to psRTDataSet->ui64PPPMultiSampleCtl */

	/* Derived PM data */
	IMG_UINT32 ui32PerRTPMSize; /*!< Per-RT parameter buffer size in bytes */
	IMG_UINT32 ui32PerRTDataMListSize; /*!< Per-RT Data MList buffer size required */

	/* Resource sizes */
	IMG_UINT32 ui32RgnHeadersSize; /*!< Total required allocation size for the region headers in bytes */
#if RGX_FEATURE_ALBIORIX_REGISTER_IF_VERSION < 1
	IMG_UINT32 ui32ISPRgnSizeVal; /*!< Applied to psRTDataSet->uiRgnHeaderSize */
#endif
	IMG_UINT32 ui32TailSize; /*!< Total required allocation size for tail pointers in bytes */

	/* Other controls */
	IMG_BOOL	bDisable6X2RegionBlockOrder;
} RGX_RT_CONFIG_RES;

#define RGXMKIF_NUM_RTDATAS 2U
#define RGXFW_MAX_FREELISTS_ 2

typedef struct RGX_RT_DEVVADDRS_TAG
{
	IMG_DEV_VIRTADDR sVHeapTableDevVAddr; /*!< DevVAddr of virtual heap table */
	IMG_DEV_VIRTADDR asRgnHeaderDevVAddr[RGXMKIF_NUM_RTDATAS]; /*!< DevVAddrs of region headers (one allocation per RTData) */
	IMG_DEV_VIRTADDR sTailPtrsDevVAddr; /*!< DevVAddrs of tail pointers */
	IMG_DEV_VIRTADDR asPMMListDevVAddr[RGXMKIF_NUM_RTDATAS]; /*!< Base address of PM MMU list (RGX_CR_PM_MLISTx_BASE) */
	IMG_DEV_VIRTADDR sRTCDevVAddr; /*!< DevVAddr of Render Target Cache */
	IMG_DEV_VIRTADDR asPMRenderStateDevVAddr[RGXMKIF_NUM_RTDATAS];	/*!< Base address of PM render state */
#if defined(SECURE_PM_RENDERSTATE_SHADOW_COPY)
	IMG_DEV_VIRTADDR asPMSecureRenderStateDevVAddr[RGXMKIF_NUM_RTDATAS]; /*!< Base address of PM secure render state */
#endif
#if defined(RGX_FEATURE_TE_RENDER_TARGET_CACHE_AUTO_FLUSH)
	IMG_DEV_VIRTADDR sRTListDevVAddr; /*!< Base addess of the RT List */
#endif

	/* DevVAddrs of Per-context freelist and Per-RT freelist.
	 * Set Per-context freelist DevVAddr=0 to disable per-context freelist.
	 *
	 * This array should actually have type RGXFWIF_FREELIST which is of type
	 * RGXFWIF_DEV_VIRTADDR but RGX_RT_DEVVADDRS does not include Services
	 * data types therefore a generic type must be used.
	 */
	IMG_HANDLE ahFreeList[RGXFW_MAX_FREELISTS_]; /*!< Freelist handle */
} RGX_RT_DEVVADDRS;

/*
 * Kick TA flags.
 */
#define RGX_KICKTA_FLAGS_TERMINATE						(1UL << 0)	/*!< Terminate the TA */
#define RGX_KICKTA_FLAGS_RESETTPC						(1UL << 1)	/*!< Reset TPC */
#define RGX_KICKTA_FLAGS_ZONLY							(1UL << 2)	/*!< Z-only render */
#define RGX_KICKTA_FLAGS_ABORT							(1UL << 3)	/*!< Discard the scene */
#define RGX_KICKTA_FLAGS_NOKICKTA 						(1UL << 4)	/*!< Do not kick the TA */
#define RGX_KICKTA_FLAGS_KICK3D							(1UL << 5)	/*!< Kick the 3D (ISP) */
			/* Free slot */
#define RGX_KICKTA_FLAGS_GETVISRESULTS					(1UL << 7)  /*!< Request occlusion query */
			/* Free slot */
#define RGX_KICKTA_FLAGS_DEPTHBUFFER					(1UL << 9)	/*!< Indicates whether a depth buffer is present */
#define RGX_KICKTA_FLAGS_STENCILBUFFER					(1UL << 10)	/*!< Indicates whether a stencil buffer is present */
#define RGX_KICKTA_FLAGS_DISABLE_PROCESS_EMPTY			(1UL << 11) /*!< Indicates whether empty regions are ignored in ISP */
#define RGX_KICKTA_FLAGS_MEMONLY						(1UL << 12) /*!< Indicates that the TA streams out to memory only. No params are written. */
#define RGX_KICKTA_FLAGS_LM_TESS						(1UL << 13) /*!< Reserve a contiguous LM buffer for domain shader */
#define RGX_KICKTA_FLAGS_SPMSCRATCHBUFFER				(1UL << 14) /*!< Indicates there is a scratch buffer for SPM */
#if defined(SUPPORT_STRIP_RENDERING)
#define RGX_KICKTA_FLAGS_FRAME_STRIP					(1UL << 15) /*!< Indicates this is a frame strip */
#endif
#if defined(FIX_HW_BRN_68501)
#define RGX_KICKTA_FLAGS_PRIM_ID_EN						(1UL << 16) /*!< Primitive ID present in PPP state */
#endif
#if defined(RGX_FEATURE_TESSELLATION) && !defined(FIX_HW_BRN_69123)
#define RGX_KICKTA_FLAGS_TESS_EN						(1UL << 17) /*!< Tessellation/DDM shaders are present */
#endif
#if defined(SUPPORT_SW_TRP)
#define RGX_KICKTA_FLAGS_SWTRP_PROTECTED						(1UL << 18) /*!< Indicates safety critical protected content */
#endif
#if defined(SUPPORT_TRP)
#define RGX_KICKTA_FLAGS_TRP_PROTECTED						(1UL << 18) /*!< Indicates safety critical protected content */
#endif
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGX_KICKTA_FLAGS_SINGLE_CORE					(1UL << 19)	/*!< Use single core in a multi core setup */
#endif
/* Kick TA compound flags */
#define RGX_KICKTA_FLAGS_TERMINATE_AND_KICK3D (RGX_KICKTA_FLAGS_TERMINATE | RGX_KICKTA_FLAGS_KICK3D) /*!< terminate TA and kick 3D */

/*
	Structures for KickTA API
*/

/* Syncs are limited by the maximum number of sync primitives. */
#define RGX_MAX_TA_SYNCS	PVRSRV_MAX_SYNCS	/*!< max TA syncs */
#define RGX_MAX_3D_SYNCS	PVRSRV_MAX_SYNCS	/*!< max 3D syncs */

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
/*! Kick-TA Workload Characteristics */
typedef struct RGX_KICKTA_WORKLOAD_CHARACTERISTICS_TAG
{
	IMG_UINT32	ui32NumberOfDrawCalls;
	IMG_UINT32	ui32NumberOfIndices;
	IMG_UINT32	ui32NumberOfMRTs;
} RGX_KICKTA_WORKLOAD_CHARACTERISTICS;
#endif


/*!
  Parameters for a KickTA Request
 */
typedef struct RGX_KICKTA_TAG
{
	IMG_UINT32			ui32KickFlags;			/*!< Combination of RGX_KICKTA_FLAGS_xxxx flags */
	IMG_UINT32			ui32JobRef;				/*!< client job reference - used in debug for tracking submitted work */

	IMG_UINT32			ui32NumShareds;						/*!< Number of shared registers to be allocated for processing TA command */
#if defined(RGX_FEATURE_TESSELLATION)
	IMG_UINT32			ui32NumDDMShareds;					/*!< Number of shared registers (domain DM) to be allocated for processing TA command */
#endif

    IMG_UINT32              ui32ISPAntiAliasMode;			/*!< ISP AA mode */
	IMG_UINT32				ui32ISPCtl;						/*!< ISP Control */

	IMG_UINT32 ui32PixelOutputWidth;						/*!< Width in DWords of pixel outputs */

	PRGX_RENDERCONTEXT		hRenderContext;                             /*!< Handle to Render Context */
	PRGX_RTDATASET			hRTDataSet;                                 /*!< Handle to RT Data-Set */

#if defined(PDUMP)
	PRGX_KICKTA_PDUMP		psKickPDUMP;                    /*!< pdump kick structure */
#endif

#if defined(SUPPORT_TRUSTED_DEVICE)
	IMG_BOOL				bSecure;						/*!< This kick requires DRM security */
#endif

	PRGX_ZSBUFFER			hZSBuffer;						/*!< OnDemand Depth/Stencil Buffer */
	PRGX_ZSBUFFER			hMSAAScratchBuffer;				/*!< OnDemand MSAA Scratch Buffer */

	PVRSRV_FENCE			iCheckTAFence;					/*!< Fence sync checked before the TA command, or PVRSRV_NO_FENCE */
	PVRSRV_FENCE			iCheck3DFence;					/*!< Fence sync checked before the 3D command, or PVRSRV_NO_FENCE */

#if defined(SUPPORT_BUFFER_SYNC)
	IMG_UINT32				ui32SyncDevMemCount;					/*!< Number of devmem descriptors to sync with */
	PVRSRV_MEMDESC			ahSyncDevMem[RGX_MAX_3D_SYNCS];			/*!< Array of devmem descriptors to sync with */
	IMG_UINT32				aui32SyncDevMemFlags[RGX_MAX_3D_SYNCS];	/*!< Array of devmem descriptor flags */
#endif

	IMG_UINT32				ui32NumTADevVars;
	IMG_UINT32				ui32Num3DDevVars;
	PVRSRV_DEV_VAR_UPDATE	asTADevVarUpdates[PVRSRV_MAX_DEV_VARS];	/*!< dev var update array */
	PVRSRV_DEV_VAR_UPDATE	as3DDevVarUpdates[PVRSRV_MAX_DEV_VARS];	/*!< dev var update array */

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	RGX_KICKTA_WORKLOAD_CHARACTERISTICS	sWorkloadCharacteristics;	/*!< Factors describing the workload*/
#endif

	IMG_DEV_VIRTADDR		sSPUEnableInfo;					/*!< Filled-in by firmware, used by PDS state-update */

	RGXFWIF_CMDTA               *psGeomCmd;
	RGXFWIF_CMD3D               *ps3DCmd;
	RGXFWIF_CMD3D               *ps3DPRCmd;
} RGX_KICKTA;

/*! ***********************************************************************//**
@brief          A surface describes a source or destination for a transfer operation
@note           As surfaces here are used in both source/destination context we
                refer to texels and pixels as they were the same thing.

				The address of the first PBE MRT is pointed to by sDevVAddr.
				With 2LEVELS flag set ui32ChunkStride is added to
				this address, to obtain the base address of the second PBE MRT.
*/ /**************************************************************************/
typedef struct TQ_SURFACE_TAG
{
	IMG_UINT32         ui32Flags;         /*!< Surface flags */
	PVRSRV_MEMDESC     hMemDesc;          /*!< Memory descriptor, to get device and CPU virtual addresses */
	IMG_DEV_VIRTADDR   sDevVAddr;         /*!< Memory address */
	PVRSRV_MEMDESC     ahUVMem[3];        /*!< Memory descriptors for extra YUV planes */
	IMG_DEV_VIRTADDR   asUVAddress[3];    /*!< Memory address for extra U/V planes */
	IMG_UINT32         ui32Width;         /*!< surface width in texels */
	IMG_UINT32         ui32Height;        /*!< surface height in texels */
	IMG_UINT32         ui32Depth;         /*!< surface depth. only applicable if IMG_MEMLAYOUT_3DTWIDDLED */
	IMG_UINT32         ui32StrideInTexels;/*!<  stride in texels */
	IMG_PIXFMT         ePixFormat;        /*!< pixel format*/
	IMG_MEMLAYOUT      eMemLayout;        /*!< memory layout*/
	IMG_FLOAT          fZPosition;        /*!< Z position in a 3D texture. 0.0f <= fZPosition <= ui32Depth */
	IMG_UINT32         ui32ChunkStride;   /*!< used with the 2LEVELS flag - see notes for details */
	IMG_UINT32         ui32SampleCount;   /*!< the number of samples per texel/pixel (0 = 1, 1 = 1) */
	IMG_FB_COMPRESSION eFBCompression;    /*!< Frame buffer compression */
} TQ_SURFACE;

/*! ***********************************************************************//**
@brief          defines a mapping between source and destination sub rectangles
*/ /**************************************************************************/
typedef struct TQ_RECTANGLE_MAPPING_TAG
{
	/*! rotation of the specified mapping*/
	IMG_ROTATION        eRotation;
	IMG_RECT            sSrcRectangle;
	IMG_RECT            sDstRectangle;
} TQ_RECTANGLE_MAPPING;

/*! ***********************************************************************//**
 @brief          Extends the TQ_SURFACE structure with input specific data
*/ /**************************************************************************/
typedef struct TQ_SOURCE_TAG
{
	/*! source surface */
	TQ_SURFACE         sSurface;

	/*! number of mappings for the source */
	IMG_UINT32              ui32NumRectangleMappings;

	/*! mappings array */
	TQ_RECTANGLE_MAPPING * pasRectangleMappings;
} TQ_SOURCE;

/*! ***********************************************************************//**
@brief          Input specific to blit type TQ_BLIT

Depth stencil merge allows merging a depth only / stencil only or depth and stencil
texture against a depth and stencil texture. The following combinations are valid:

 * src0 : depth or depth & stencil, dest : depth & stencil - flags : DSMERGE | PICKD
   Takes the depth of the source and the stencil of the destination and combines the two interleaved.

 * src0 : stencil or depth & stencil, dest : depth & stencil - flags : DSMERGE
   Takes the stencil of the source and the depth of the destination and combines the two interleaved.
*/ /**************************************************************************/
// TQ_BLIT_OP ui32Flags
// Blt type
#define BLTFLAGS_COPY            0x00000000U         /*!< default blt type is source copy*/
#define BLTFLAGS_COLOURFILL      0x00000001U         /*!< colour fill blt type*/
#define BLTFLAGS_ROPCODE         0x00000002U         /*!< enable rop4 for dest/source/pattern/mask*/
#define BLTFLAGS_SOURCE_CKEY     0x00000010U         /*!< enable source colour key*/
#define BLTFLAGS_DEST_CKEY       0x00000020U         /*!< enable dest colour key*/
#define BLTFLAGS_A8MERGE         0x00000040U         /*!< merge a8 source with rgb from argb8888 or ayuv8888 dest */
#define BLTFLAGS_DSMERGE         0x00000080U         /*!< merge a depth or stencil against a depth + stencil texture*/
#define BLTFLAGS_PICKD           0x00000100U         /*!< valid if doing a DS merge with depth + stencil to depth + stencil*/
#define BLTFLAGS_2LEVELS         0x00000400U         /*!< write 2 levels at once of a mip chain - applicable on 3D routes only */
// Colour key type
#define BLT_FLAGS_CKPASS         0x01000000U         /*!< default colour key type is reject*/

/**************************************************************************//**
@brief       TQ Alpha blend
******************************************************************************/
/*! @defgroup       Alpha blend type
*/
/* @{*/
#define TQ_ALPHA_NONE						0x00U		/*!< Alpha blend type 'none' */
#define TQ_ALPHA_SOURCE						0x01U		/*!< Cdst = Csrc*Asrc	+ Cdst*(1-Asrc)*/
#define TQ_ALPHA_PREMUL_SOURCE				0x02U		/*!< Cdst = Csrc		+ Cdst*(1-Asrc)*/
#define TQ_ALPHA_GLOBAL						0x03U		/*!< Cdst = Csrc*Aglob	+ Cdst*(1-Aglob)*/
#define TQ_ALPHA_PREMUL_SOURCE_WITH_GLOBAL	0x04U		/*!< Cdst = Csrc*Aglob	+ Cdst*(1-Asrc)*(1-Aglob)*/

#define TQ_PATTERN void*

/*! @defgroup       TQ surface flags
*/
/* @{*/
#define TQ_SURFACE_FLAG_STRIDE_IN_BYTES             0x00000001U    /*!< stride is in bytes not texels (packed rgb888 format) */
#define TQ_SURFACE_FLAG_RB_SWAP                     0x00000002U    /*!< pixels have red and blue swapped */
#define TQ_SURFACE_FLAG_ENDIAN_SWAP                 0x00000004U    /*!< colour channels are endian swapped */
#define TQ_SURFACE_FLAG_BUFFER_SYNC                 0x00000008U    /*!< use buffer synchronisation */
#define TQ_SURFACE_FLAG_ACCUM_SRC_SURFACE           0x00000010U    /*!< use surface as a source for accumulation */
#define TQ_SURFACE_FLAG_FBC_FLOAT                   0x00000020U    /*!< read/write compressed format as float */
#define TQ_SURFACE_FLAG_PICKD                       0x00000040U    /*!< depth channel operation*/
/* defaults to BT709 conformance range*/
#define TQ_SURFACE_FLAG_YUV_CSC_MASK                0x00000700U    /*!< YUV CSC field mask for the following fields */
#define TQ_SURFACE_FLAG_YUV_CSC_BT601_CONFORM       0x00000200U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
#define TQ_SURFACE_FLAG_YUV_CSC_BT601_FULL          0x00000100U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
#define TQ_SURFACE_FLAG_YUV_CSC_BT709_CONFORM       0x00000000U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
#define TQ_SURFACE_FLAG_YUV_CSC_BT709_FULL          0x00000300U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
#define TQ_SURFACE_FLAG_YUV_CSC_BT2020_CONFORM      0x00000400U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
#define TQ_SURFACE_FLAG_YUV_CSC_BT2020_FULL         0x00000500U    /*!< only applicable to YUV sources. Selects CSC coeff matrix */
/* @}*/

/*! @defgroup       TQ command flags
*/
/* @{*/
#define TQ_CMD_FLAGS_DONTSUBMIT         0x00000001U   /*!< Prepare call doesn't submit */
#define TQ_CMD_FLAGS_FIRSTINBATCH       0x00000002U   /*!< first prepare call after a submit */
#define TQ_CMD_FLAGS_PDUMP_CONTINUOUS   0x00000004U   /*!< PDUMP TQ op even if it's outside of capture range */
#define TQ_CMD_FLAGS_SECURE             0x00000008U   /*!< whether or not to enable DRM security */
#define TQ_CMD_FLAGS_TLA_MERGE          0x00000020U   /*!< when in batched mode, merge TLA commands */
#define TQ_CMD_FLAGS_SUBMITONLY         0x00000080U   /*!< Submit operation without validation */
#define TQ_CMD_FLAGS_CPU_FLUSH_SOURCE	0x00000100U   /*!< If the CPU reads from the TQ source surface, the cache must be flushed first */
#define TQ_CMD_FLAGS_SINGLE_CORE		0x00000200U	  /*!< Use single core in a multi core setup */
/* @}*/

/******************************************************************************
 * API prototypes
 *****************************************************************************/

/* returns RGX HW mappings for the client, e.g. CCB and CCBkicker */

/**************************************************************************/ /*!
@Function       RGXScheduleProcessQueues
@Description    Schedules the firmware
@Input          psDevConnection Pointer to a Device-Data structure
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXScheduleProcessQueues(PVRSRV_DEV_CONNECTION *psDevConnection);


/**************************************************************************/ /*!
@Function       RGXKickTA
@Description    Send a TA command to the RGX firmware
@Input          psDevConnection pointer to device info
@Input          psKickTA        pointer to the kick info
@Output         piTAFence       The returned update fence after the TA command,
                                or PVRSRV_NO_FENCE_PTR
@Input          pcszTAFenceName Name to assign to the created update TA fence,
                                or PVRSRV_NO_FENCE_PTR
@Output         pi3DFence       The returned update fence after the 3D command,
                                or PVRSRV_NO_FENCE_PTR
@Input          pcsz3DFenceName Name to assign to the created update 3D fence,
                                or PVRSRV_NO_FENCE_PTR
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXKickTA(PVRSRV_DEV_CONNECTION	*psDevConnection,
									RGX_KICKTA				*psKickTA,
									PVRSRV_FENCE			*piTAFence,
									const char				*pcszTAFenceName,
									PVRSRV_FENCE			*pi3DFence,
									const char				*pcsz3DFenceName);

/**************************************************************************/ /*!
@Function	    RGXCreateRenderContext / RGXCreateRenderContextCCB
@Description    Creates a rendering context and everything associated with it
                This context is NOT thread-safe, callers must externally
                synchronise all use of this context if shared between threads.
				RGXCreateRenderContextCCB will use custom CCB size.
@Input          psDevConnection				Pointer to a Device-Data structure
@Input          psCreateRenderContext		Parameters required for creating a new
                                			render context.
@Input(for CCB) ui32TACCBAllocSizeLog2		Optional custom CCB size.
                ui32TACCBMaxAllocSizeLog2	Might be 0 for default size.
				ui323DCCBAllocSizeLog2
				ui323DCCBMaxAllocSizeLog2
@Output         phRenderContext         	The new render context
@Return                                 	PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        	error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateRenderContext(PVRSRV_DEV_CONNECTION		*psDevConnection,
 												 PRGX_CREATERENDERCONTEXT	psCreateRenderContext,
 												 PRGX_RENDERCONTEXT			*phRenderContext);
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateRenderContextCCB(PVRSRV_DEV_CONNECTION		*psDevConnection,
 													PRGX_CREATERENDERCONTEXT	psCreateRenderContext,
													IMG_UINT32					ui32TACCBAllocSizeLog2,
													IMG_UINT32					ui32TACCBMaxAllocSizeLog2,
													IMG_UINT32					ui323DCCBAllocSizeLog2,
													IMG_UINT32					ui323DCCBMaxAllocSizeLog2,
													PRGX_RENDERCONTEXT			*phRenderContext);

/**************************************************************************/ /*!
@Function       RGXDestroyRenderContext
@Description    Destroys a rendering context and everything associated with it
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          hRenderContext          The render context to be destroyed
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXDestroyRenderContext(PVRSRV_DEV_CONNECTION	*psDevConnection,
												  PRGX_RENDERCONTEXT	hRenderContext);


/**************************************************************************/ /*!
@Function       RGXAddRenderTarget
@Description    Adds render target to specified PB.
@Input          psDevConection          Pointer to a Device-Data structure
@Input          psAddRTInfo             Parameters required for adding a
                                        Render-Target.
@Output         phRTDataSet             ptr to RT Data set handle
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXAddRenderTarget(PVRSRV_DEV_CONNECTION	*psDevConnection,
 											 RGX_ADDRENDTARG	*psAddRTInfo,
 											 PRGX_RTDATASET		*phRTDataSet);

/**************************************************************************/ /*!
@Function       RGXRemoveRenderTarget
@Description    Remove a render target
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          hRTDataSet              handle for RT Data set
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXRemoveRenderTarget(PVRSRV_DEV_CONNECTION	*psDevConnection,
												PRGX_RTDATASET			hRTDataSet);

/**************************************************************************/ /*!
@Function       RGXCreateZSBuffer
@Description    Creates a Depth and/or Stencil Buffer
@Input          hHeap                   Heap on which the buffer should be allocated
@Input          uiFlags                 Flags for memory allocation
@Input          uiSize                  Buffer size in bytes
@Input          uiLog2Align             Alignment of the buffer
@Input          bOnDemand               Requests on-demand physical page backing
@Input          bProtected              Requests protected physical page backing
@Output         psDeviceVirtualAddr     Device Virtual Address of the ZS buffer
@Output         hZSBuffer               Handle to ZS buffer
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateZSBuffer(PVRSRV_HEAP 			hHeap,
											PVRSRV_DEV_CONNECTION	*psDevConnection,
											PVRSRV_DEVMEMCTX		psDevMemCtx,
											PVRSRV_MEMALLOCFLAGS_T 	uiFlags,
											IMG_DEVMEM_SIZE_T		uiSize,
											IMG_DEVMEM_LOG2ALIGN_T	uiLog2Align,
											IMG_BOOL				bOnDemand,
											IMG_BOOL				bProtected,
											PVRSRV_MEMDESC 			*hMemDesc,
											IMG_DEV_VIRTADDR		*psDeviceVirtualAddr,
 											PRGX_ZSBUFFER			*hZSBuffer,
											PVRSRV_MEMINFO			**psMemInfo,
											const IMG_CHAR			*pszName);

/**************************************************************************/ /*!
@Function       RGXDestroyZSBuffer
@Description    Destroys a Depth and/or Stencil Buffer
@Input          hZSBuffer               Handle to ZS buffer
@Return                                 None
 */ /**************************************************************************/
IMG_EXPORT
void IMG_CALLCONV RGXDestroyZSBuffer(const PVRSRV_DEV_CONNECTION *psConnection,
									 PRGX_ZSBUFFER	hZSBuffer,
									 IMG_HANDLE hOSEvent);


/**************************************************************************/ /*!
@Function       RGXAcquirePhysicalMappingZSBuffer
@Description    Backs the on-demand Depth/Stencil buffer with physical pages
@Input          hZSBuffer               Handle to the Depth/Stencil buffer
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXAcquirePhysicalMappingZSBuffer(PRGX_ZSBUFFER hZSBuffer);

/*************************************************************************/ /*!
@Function       RGXReleasePhysicalMappingZSBuffer
@Description    Releases physical pages of on-demand Depth/Stencil buffer
@Input          hZSBuffer               Handle to the Depth/Stencil buffer
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXReleasePhysicalMappingZSBuffer(PRGX_ZSBUFFER hZSBuffer);

/**************************************************************************/ /*!
@Function       RGXAcquireCPUMappingZSBuffer
@Description    Maps the Depth/Stencil buffer into CPU application space
@Input          hZSBuffer               Handle to the Depth/Stencil buffer
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXAcquireCPUMappingZSBuffer(PRGX_ZSBUFFER hZSBuffer,
										  void **ppvCpuVirtAddr);

/**************************************************************************/ /*!
@Function       RGXReleaseCPUMappingZSBuffer
@Description    Unmaps the Depth/Stencil buffer from CPU application space
@Input          hZSBuffer               Handle to the Depth/Stencil buffer
@Return                                 None
 */ /**************************************************************************/
IMG_EXPORT
void RGXReleaseCPUMappingZSBuffer(PRGX_ZSBUFFER hZSBuffer);


/**************************************************************************/ /*!
@Function       RGXSetRenderContextPriority
@Description    Set the priority for a render context
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          psContext               Handle to the render context.
@Input          ui32Priority            Priority to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXSetRenderContextPriority(PVRSRV_DEV_CONNECTION *psDevConnection,
											PRGX_RENDERCONTEXT psContext,
											IMG_UINT32 ui32Priority);

/**************************************************************************/ /*!
@Function       RGXSetRenderContextFlags
@Description    Set the flags for a render context
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          psContext               Handle to the render context.
@Input          ui32Flags               Flags to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXSetRenderContextFlags(PVRSRV_DEV_CONNECTION *psDevConnection,
									  PRGX_RENDERCONTEXT psContext,
									  IMG_UINT32 ui32Flags);

/**************************************************************************/ /*!
@Function		RGXRetrieveRenderTargetRendersInFlight
@Description	Return an approximate count of how many renders are scheduled
				with this RTDataSet
@Input			psRTDataSet				Pointer to RTDataSet
@Input			pui32NumRendersInFlight	Pointer to the retrieved value
@Return									PVRSRV_OK on success. Otherwise, a PVRSRV_
										error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXRetrieveRenderTargetRendersInFlight(PRGX_RTDATASET psRTDataSet,
													IMG_UINT32 *pui32NumRendersInFlight);

/**************************************************************************/ /*!
@Function		RGXGetScreenXYMaxFromRenderTarget
@Description	Return the screen size in tiles for this RTDataSet
@Input			psRTDataSet				Pointer to RTDataSet
@Input			pui32ScreenXMax			Pointer to the retrieved value
@Input			pui32ScreenYMax			Pointer to the retrieved value
@Return									PVRSRV_OK on success. Otherwise, a PVRSRV_
										error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXGetScreenXYMaxFromRenderTarget(PRGX_RTDATASET psRTDataSet,
													IMG_UINT32 *pui32ScreenXMax,
													IMG_UINT32 *pui32ScreenYMax);

/**************************************************************************/ /*!
@Function		RGXGetSizeInTilesFromRenderTarget
@Description	Return the tile size of the surface associated with this
				RTDataSet taking the MSAA config into account.
@Input			psRTDataSet				Pointer to RTDataSet
@Input			pui32NumTilesX	        Pointer to the X extent in tiles
@Input			pui32NumTilesY	        Pointer to the Y extent in tiles
@Return									PVRSRV_OK on success. Otherwise, a PVRSRV_
										error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXGetSizeInTilesFromRenderTarget(PRGX_RTDATASET psRTDataSet,
											   IMG_UINT32 *pui32NumTilesX,
											   IMG_UINT32 *pui32NumTilesY);

typedef enum _RGX_CONTEXT_RESET_REASON_
{
	RGX_CONTEXT_RESET_REASON_NONE                = 0,	/*!< No reset reason recorded */
	RGX_CONTEXT_RESET_REASON_GUILTY_LOCKUP       = 1,	/*!< Caused a reset due to locking up */
	RGX_CONTEXT_RESET_REASON_INNOCENT_LOCKUP     = 2,	/*!< Affected by another context locking up */
	RGX_CONTEXT_RESET_REASON_GUILTY_OVERRUNING   = 3,	/*!< Overran the global deadline */
	RGX_CONTEXT_RESET_REASON_INNOCENT_OVERRUNING = 4,	/*!< Affected by another context overrunning */
	RGX_CONTEXT_RESET_REASON_HARD_CONTEXT_SWITCH = 5,	/*!< Forced reset to ensure scheduling requirements */
} RGX_CONTEXT_RESET_REASON;

/**************************************************************************/ /*!
@Function       RGXGetLastRenderContextResetReason
@Description    Return the last reset reason for this context and reset the
                value for next time.
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          psContext               Handle to the render context.
@Output         peLastResetReason       Last reset reason if one exists.
@Output         pui32LastResetJobRef    Last reset job reference from the kick API.
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXGetLastRenderContextResetReason(PVRSRV_DEV_CONNECTION *psDevConnection,
											    PRGX_RENDERCONTEXT psContext,
											    RGX_CONTEXT_RESET_REASON *peLastResetReason,
											    IMG_UINT32 *pui32LastResetJobRef);

IMG_EXPORT
void RGXRenderContextStalled(PVRSRV_DEV_CONNECTION	*psConnection,
							 PRGX_RENDERCONTEXT		psRenderContext);

#if defined(SUPPORT_TLT_PERF)
/**************************************************************************/ /*!
@Function       RGXRenderContextGetTLTBuffer
@Description    Return the Tile LifetimeTracking buffer. The buffer is filled
				by the GPU when TLT profiling is enabled in the render kick.
@Input          psRenderContext         Pointer to the current render context.
@Output         ppsTLTBufferMemInfo     Pointer to the TLT buffer object which
										will be a valid object if the call is
										successful.
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXRenderContextGetTLTBuffer(PRGX_RENDERCONTEXT		psRenderContext,
										  PVRSRV_MEMINFO			**ppsTLTBufferMemInfo);
#endif

/*************************************************************************/ /*!
 RGX_CREATECOMPUTECONTEXT - Used for passing parameters to
 RGXCreateComputeContext().
*/ /*************************************************************************/
typedef struct RGX_CREATECOMPUTECONTEXT_TAG
{
	IMG_UINT32				ui32ContextFlags;   /*!< Flags specifying context properties */
	IMG_HANDLE				hDevCookie;         /*!< Handle representing a Device */
	PVRSRV_DEVMEMCTX		hDevMemContext;     /*!< Handle to a Device-Memory Context */
	IMG_UINT32				ui32Priority;		/*!< Priority of context */
	IMG_UINT32				ui32MaxCDMDeadlineMS;
	IMG_CHAR                cClientAPI;         /*!< A 1 character tag representing the client API requesting this
	                                                 context this will have the DM's resource tag (ie. CDM) appended
	                                                 to it. Used in naming resources associated with this context,
	                                                 eg. Timeline. */
	IMG_UINT64				ui64RobustnessAddress;
} RGX_CREATECOMPUTECONTEXT;


/* Opaque compute context handle used with RGXKickCDM */
typedef struct RGX_COMPUTECONTEXT_TAG *PRGX_COMPUTECONTEXT;

/**************************************************************************/ /*!
@Function       RGXCreateComputeContext / RGXCreateComputeContextCCB
@Description    Creates a compute context and everything associated with it
                This context is NOT thread-safe, callers must externally
                synchronise all use of this context if shared between threads.
				RGXCreateComputeContextCCB will use custom CCB size.
@Input          psDevConnection			A Device-Data structure
@Input          psCreateComputeContext	Parameters required for creating a
                                    	Compute Context
@Input(for CCB) ui32CCBAllocSizeLog2	Optional custom CCB size.
                ui32CCBMaxAllocSizeLog2	Might be 0 for default size.
@Output         phComputeContext		On success, the newly-created Compute
                                        Context.
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXCreateComputeContext(PVRSRV_DEV_CONNECTION		*psDevConnection,
									 RGX_CREATECOMPUTECONTEXT	*psCreateComputeContext,
									 PRGX_COMPUTECONTEXT		*phComputeContext);
IMG_EXPORT
PVRSRV_ERROR RGXCreateComputeContextCCB(PVRSRV_DEV_CONNECTION		*psDevConnection,
										RGX_CREATECOMPUTECONTEXT	*psCreateComputeContext,
										IMG_UINT32					ui32CCBAllocSizeLog2,
										IMG_UINT32					ui32CCBMaxAllocSizeLog2,
										PRGX_COMPUTECONTEXT			*phComputeContext);

/**************************************************************************/ /*!
@Function       RGXDestroyComputeContext
@Description    Destroys a compute context and everything associated with it
@Input          psDevConnection         A Device-Data structure
@Input          hComputeContext         Handle to a Compute Context
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXDestroyComputeContext(PVRSRV_DEV_CONNECTION	*psDevConnection,
									  PRGX_COMPUTECONTEXT		hComputeContext);

/**************************************************************************/ /*!
@Function       RGXSetComputeContextPriority
@Description    Set the priority for a compute context
@Input          psDevConnection         A Device-Data structure
@Input          psContext         		Handle to a Compute Context
@Input			ui32Priority			Priority to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXSetComputeContextPriority(PVRSRV_DEV_CONNECTION *psDevConnection,
											PRGX_COMPUTECONTEXT psContext,
											IMG_UINT32 ui32Priority);

/**************************************************************************/ /*!
@Function       RGXSetComputeContextFlags
@Description    Set the flags (properties) for a compute context
@Input          psDevConnection         A Device-Data structure
@Input          psContext               Handle to a Compute Context
@Input          ui32ContextFlags        Flags to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXSetComputeContextFlags(PVRSRV_DEV_CONNECTION *psDevConnection,
									   PRGX_COMPUTECONTEXT   psContext,
									   IMG_UINT32            ui32ContextFlags);

/**************************************************************************/ /*!
@Function       RGXGetLastComputeContextResetReason
@Description    Return the last reset reason for this context and reset the
                value for next time.
@Input          psDevConnection         Pointer to a Device-Data structure
@Input          psContext               Handle to the compute context.
@Output         peLastResetReason       Last reset reason if one exists.
@Output         pui32LastResetJobRef    Last reset job reference from the kick API.
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXGetLastComputeContextResetReason(PVRSRV_DEV_CONNECTION *psDevConnection,
											     PRGX_COMPUTECONTEXT psContext,
											     RGX_CONTEXT_RESET_REASON *peLastResetReason,
												 IMG_UINT32 *pui32LastResetJobRef);

/**************************************************************************/ /*!
 RGX_KICK_CDM - Used for passing parameters to RGXKickCDM().
 */ /**************************************************************************/

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
/*! Kick-Compute Workload Characteristics */
typedef struct RGX_KICK_CDM_WORKLOAD_CHARACTERISTICS_TAG
{
	IMG_UINT32	ui32NumberOfWorkgroups;
	IMG_UINT32	ui32NumberOfWorkitems;
} RGX_KICK_CDM_WORKLOAD_CHARACTERISTICS;
#endif

typedef struct RGX_KICK_CDM_TAG
{
	PRGX_COMPUTECONTEXT			hComputeContext;       	 	/*!< Compute Context */
	IMG_UINT32					ui32NumDevVars;				/*!< number of devvar operations */
	PVRSRV_DEV_VAR_UPDATE		*psDevVarUpdates;			/*!< array of devvar operations */
#if defined(PDUMP)
	PRGX_KICK_DUMP_BUFFER			psKickPDumpBuffers;		/*!< array of buffers to PDump */
	IMG_UINT32					ui32PDumpBufferArraySize;	/*!< number of buffers to PDump */
#endif
	IMG_UINT32					ui32NumShareds;				/*!< Number of shared registers to be allocated for processing CDM command */
	IMG_UINT32					ui32JobRef;					/*!< client job reference - used in debug for tracking submitted work */

	PVRSRV_FENCE				iCheckFence;				/*!< Fence sync checked before this command, or PVRSRV_NO_FENCE */
#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	RGX_KICK_CDM_WORKLOAD_CHARACTERISTICS	sWorkloadCharacteristics;	/*!< Factors describing the workload */
#endif
	RGXFWIF_CMD_COMPUTE			*psComputeCmd;				/*!< CPU copy of FW command */
} RGX_KICK_CDM;


/**************************************************************************/ /*!
@Function       RGXKickCDM
@Description    Schedule a CDM task to the RGX firmware
@Input          psDevConnection   Pointer to the Device-connection
Input           psKickCDM         ptr of CDM kick structure
@Output         piUpdateFence     The returned update fence,
                                  or PVRSRV_NO_FENCE_PTR
@Input          pcszUpdateFenceName   The name to give the returned update
                                      fence, or PVRSRV_NO_FENCE_PTR
@Return                           PVRSRV_OK on success. Otherwise, a PVRSRV_
                                  error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXKickCDM(PVRSRV_DEV_CONNECTION	*psDevConnection,
									 RGX_KICK_CDM			*psKickCDM,
									 PVRSRV_FENCE			*piUpdateFence,
									 const IMG_CHAR			*pcszUpdateFenceName);

/**************************************************************************/ /*!
@Function       RGXFlushComputeData
@Description    Flush the data generated by the CDM
@Input          psDevConnection         Pointer to a Device-Data structure
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXFlushComputeData(PVRSRV_DEV_CONNECTION *psDevConnection, PRGX_COMPUTECONTEXT psContext);


/**************************************************************************/ /*!
@Function       RGXCreateKickSyncContext / RGXCreateKickSyncContextCCB
@Description    Creates a kick sync context
				RGXCreateKickSyncContextCCB will use custom CCB size.
@Input          psDevConnection         Pointer to the Device-connection
@Input          hDevMemContext          The current device memory context
@Input          ui32ContextFlags        Flags specifying properties of the context
@Input          cClientAPI              A 1 character tag describing the
                                        client API requesting this context, this
                                        will have the DM's resource tag (ie. KS)
                                        appended to it. Used in naming resources
                                        associated with this context, eg.
                                        Timeline.
@Input(for CCB) ui32CCBAllocSizeLog2	Optional custom CCB size.
                ui32CCBMaxAllocSizeLog2	Might be 0 for default size.
@Output         phKickSyncContext       The new kick sync context
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateKickSyncContext(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                   PVRSRV_DEVMEMCTX       hDevMemContext,
                                                   IMG_UINT32             ui32ContextFlags,
                                                   IMG_CHAR               cClientAPI,
                                                   PRGX_KICKSYNCCONTEXT  *phKickSyncContext);
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateKickSyncContextCCB(PVRSRV_DEV_CONNECTION  *psDevConnection,
                                                	  PVRSRV_DEVMEMCTX       hDevMemContext,
                                                	  IMG_CHAR               cClientAPI,
													  IMG_UINT32			 ui32CCBAllocSizeLog2,
													  IMG_UINT32			 ui32CCBMaxAllocSizeLog2,
                                                      IMG_UINT32             ui32ContextFlags,
                                                	  PRGX_KICKSYNCCONTEXT   *phKickSyncContext);

/**************************************************************************/ /*!
@Function       RGXDestroyKickSyncContext
@Description    Destroys a kick sync context
@Input          psDevConnection         Pointer to the Device-connection
@Input          hKickSyncContext        The kick sync context to be destroyed
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXDestroyKickSyncContext(PVRSRV_DEV_CONNECTION * psDevConnection,
                                                    PRGX_KICKSYNCCONTEXT    hKickSyncContext);

/**************************************************************************/ /*!
@Function       RGXSetKickSyncContextFlags
@Description    Set the flags (properties) for the specified kick sync context
@Input          psDevConnection         Pointer to the Device-connection
@Input          hKickSyncContext        The kick sync context to be amended
@Input          ui32COntextFlags        Flags to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXSetKickSyncContextFlags(PVRSRV_DEV_CONNECTION * psDevConnection,
                                                     PRGX_KICKSYNCCONTEXT    hKickSyncContext,
                                                     IMG_UINT32              ui32ContextFlags);

typedef struct RGX_KICKSYNC_COMMAND_TAG
{
	IMG_UINT32                 ui32NumDevVars;
	PVRSRV_DEV_VAR_UPDATE      asDevVarUpdates[PVRSRV_MAX_DEV_VARS];        /*!< dev var update array */

	PVRSRV_FENCE               iCheckFence;    /*!< Fence sync checked before this command, or PVRSRV_NO_FENCE */
} RGX_KICKSYNC_COMMAND;


/**************************************************************************/ /*!
@Function       RGXKickSync
@Description    Kicks a sync only command
@Input          psDevConnection   Pointer to the Device-connection
@Input          hKickSyncContext  The kick sync context
@Input          psKickSyncCommand The sync command
@Output         phUpdateFence     The returned update fence,
                                  or PVRSRV_NO_FENCE_PTR
@Input          pcszFenceName     The name to give the returned update fence,
                                  or PVRSRV_NO_FENCE_PTR
@Input          ui32JobRef        Job reference - function up to the caller
@Return                           PVRSRV_OK on success. Otherwise, a PVRSRV_
                                  error code
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
RGXKickSync(PVRSRV_DEV_CONNECTION * psDevConnection,
            IMG_HANDLE              hKickSyncContext,
            RGX_KICKSYNC_COMMAND  * psKickSyncCommand,
            PVRSRV_FENCE          * phUpdateFence,
            const char            * pcszFenceName,
            IMG_UINT32              ui32JobRef);


/**************************************************************************/ /*!
  @Brief        DM selector for breakpoint functions
 */ /**************************************************************************/
typedef enum _RGX_BP_DM_TYPE_
{
	RGX_BP_DM_VERTEX,
	RGX_BP_DM_PIXEL,
	RGX_BP_DM_COMPUTE,
	RGX_BP_DM_TESS_VERTEX,
	RGX_BP_DM_RAY_VERTEX,
	RGX_BP_DM_RAY
} RGX_BP_DM_TYPE;

/**************************************************************************/ /*!
@Function       RGXSetBreakpoint
@Description    Write the address of a breakpoint and its handler into registers.

@Input          hRemoteContext: Device memory context handle. Retrieve via ctx import.
@Input          ui32BreakpointAddr: Offset into the USSC heap to address of breakpoint.
@Input          ui32HandlerAddr: Offset into the USSC heap to address of breakpoint handler.
@Input          eDataMaster: Enumeration representing data master to use.

@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXSetBreakpoint(PVRSRV_REMOTE_DEVMEMCTX hRemoteContext,
                                                      IMG_UINT32 ui32BreakpointAddr,
                                                      IMG_UINT32 ui32HandlerAddr,
                                                      RGX_BP_DM_TYPE eDataMaster);

/**************************************************************************/ /*!
@Function       RGXDisableBreakpoint
@Description    Disable existing breakpoint.

@Input          hRemoteContext: Device memory context handle. Retrieve via ctx import.

@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXDisableBreakpoint(PVRSRV_REMOTE_DEVMEMCTX hRemoteContext);

/**************************************************************************/ /*!
@Function       RGXClearBreakpoint
@Description    Clears breakpoint and handler address from registers.

@Input          hRemoteContext: Device memory context handle. Retrieve via ctx import.

@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXClearBreakpoint(PVRSRV_REMOTE_DEVMEMCTX hRemoteContext);

/**************************************************************************/ /*!
@Function       RGXEnableBreakpoint
@Description    Enable existing breakpoint.

@Input          hRemoteContext: Device memory context handle. Retrieve via ctx import.

@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXEnableBreakpoint(PVRSRV_REMOTE_DEVMEMCTX hRemoteContext);

/**************************************************************************/ /*!
@Function       RGXOverallocateBPRegisters
@Description    Over allocate temporary and shared registers for breakpoint handler.

@Input          hRemoteContext: Device memory context handle. Retrieve via ctx import.
@Input          ui32TempRegs  Number of temporary registers to over allocate.
@Input          ui32SharedRegs  Number of shared registers to over allocate.

@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXOverallocateBPRegisters(PVRSRV_REMOTE_DEVMEMCTX hRemoteContext,
                                                                IMG_UINT32 ui32TempRegs,
                                                                IMG_UINT32 ui32SharedRegs);


/**************************************************************************/ /*!
@Function       RGXCurrentTime
@Description    Returns the current state of the device timer
@Input          psDevConnection  Device data.
@Out            pui64Time
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV RGXCurrentTime(PVRSRV_DEV_CONNECTION * psDevConnection,
                                                    IMG_UINT64      * pui64Time);

#if defined(SUPPORT_USER_REGISTER_CONFIGURATION)
typedef enum _RGX_REG_CONFIG_TYPE_
{
	RGX_REG_CONFIG_TYPE_PWR_ON,
	RGX_REG_CONFIG_TYPE_DUST_CHANGE,
	RGX_REG_CONFIG_TYPE_TA,
	RGX_REG_CONFIG_TYPE_3D,
	RGX_REG_CONFIG_TYPE_CDM,
	RGX_REG_CONFIG_TYPE_TDM,
} RGX_REG_CONFIG_TYPE;

/**************************************************************************/ /*!
@Function       RGXSetRegConfigType
@Description    Set the register config type that the calls to RGXAddRegConfig will
		apply.
@Input		psDevConnection  Device data.
@Input		eRegCfgType      Config type.
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXSetRegConfigType(PVRSRV_DEV_CONNECTION *psDevConnection,
							RGX_REG_CONFIG_TYPE eRegCfgType);

/**************************************************************************/ /*!
@Function       RGXAddRegConfig
@Description    Add a single register configuration record.
@Input          psDevConnection         Device data.
@Input          ui32RegAddr             Register address
@Input          ui64RegValue            Register value
@Input          ui64RegValue            Register mask
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXAddRegConfig(PVRSRV_DEV_CONNECTION *psDevConnection,
							IMG_UINT32 ui32RegAddr,
							IMG_UINT64 ui64RegValue,
							IMG_UINT64 ui64RegMask);

/**************************************************************************/ /*!
@Function       RGXClearRegConfig
@Description    Clear a the complete register configuration.
@Input          psDevConnection  Device data.
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXClearRegConfig(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       RGXEnableRegConfig
@Description    Enable register configuration.
@Input          psDevConnection  Device data.
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXEnableRegConfig(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       RGXDisableRegConfig
@Description    Disable register configuration.
@Input          psDevConnection  Device data.
@Return         PVRSRV_OK on success.
*/ /***************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXDisableRegConfig(PVRSRV_DEV_CONNECTION *psDevConnection);
#endif /* SUPPORT_USER_REGISTER_CONFIGURATION */

/*****************************************************************************
 FUNCTION	: RGXCreateGlobalPB

 PURPOSE	: Creates the global parameter buffer

 PARAMETERS	: ui32PBSize : The size of the global parameter buffer
			  ui32PBSizeLimit : The size limit of the global parameter buffer
			  ppsGlobalPB: The output structure

 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateGlobalPB(PVRSRV_DEV_CONNECTION	*psDevConnection,
							   PVRSRV_DEVMEMCTX					hDevMemContext,                 /*!< DeviceMem Context */
							   IMG_UINT32						ui32GlobalPBSize,                     /*!< Initial Param Buffer Size */
							   IMG_UINT32						ui32GlobalPBSizeLimit,                /*!< Param Buffer Size Limit */
							   IMG_HANDLE						hOSEvent,
							   PRGX_GLOBALPB					*ppsGlobalPB);



/*****************************************************************************
 FUNCTION	: RGXDestroyGlobalPB

 PURPOSE	: Destroys the global parameter context

 PARAMETERS	: psDevConnection
			  psGlobalPB : Pointer to the global parameter buffer to destroy


 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXDestroyGlobalPB(PVRSRV_DEV_CONNECTION		*psDevConnection,
						PRGX_GLOBALPB	psGlobalPB,
						IMG_HANDLE	hOSEvent);


/*****************************************************************************
 FUNCTION	: RGXNotifySignalUpdate

 PURPOSE	: Notifies the device that a signal update has been done by the host

 PARAMETERS	: hDevMemContext: Device memory context
                  sDevSignalAddress : Virtual address of the updated signal


 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXNotifySignalUpdate(PVRSRV_DEVMEMCTX hDevMemContext,
                                                IMG_DEV_VIRTADDR sDevSignalAddress);

/*****************************************************************************
 FUNCTION	: RGXNotifyComputeWriteOffsetUpdate

 PURPOSE	: Notifies the device that a Compute Write Offset has been updated.

 PARAMETERS	: psDevConnection  A Device-Data structure
              psContext  Handle to a Compute Context

 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXNotifyComputeWriteOffsetUpdate(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                            PRGX_COMPUTECONTEXT psContext);


#if defined(RGX_FEATURE_SIGNAL_SNOOPING)
/**************************************************************************/ /*!
 FUNCTION	: RGXGetComputeResumeSignal

 PURPOSE	: Obtain the Signal address used to resume a CDM2 context in the
              firmware.

 PARAMETERS	: psContext           Handle to a Compute Context
              psResumeSignalAddr  Filled on exit address of signal.

 RETURNS	: PVRSRV_ERROR
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXGetComputeResumeSignal(PRGX_COMPUTECONTEXT psContext,
                                                    IMG_DEV_VIRTADDR *psResumeSignalDevVAddr);
#endif


/**************************************************************************/ /*!
 FUNCTION	: RGXInvalidateFBSCTable

 PURPOSE	: Invalidate selected entries in the Framebuffer state cache.
			  Each bit represents 8 cache entries so a mask 0xFF would invalidate
			  entries from 0 to 63 inclusive.

 PARAMETERS	: hDevMemContext      Handle to a GPU memory context.
              ui64FBSCEntries     Bitmask containing the FBSC entries to inval.

 RETURNS	: PVRSRV_ERROR
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXInvalidateFBSCTable(PVRSRV_DEVMEMCTX hDevMemContext,
									IMG_UINT64 ui64FBSCEntries);

/*****************************************************************************
 FUNCTION	: RGXCreateDeviceMemContext

 PURPOSE	: Creates an RGX device memory context

 PARAMETERS	: psDevConnection		Pointer to a Device-Data structure
              phRGXDevMemContext	Handle to the RGX device memory context
              phDevMemContext		Handle to the underlying PVR memory context

 RETURNS	: PVRSRV_ERROR
*****************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCreateDeviceMemContext(PVRSRV_DEV_CONNECTION *psDevConnection,
													PRGX_DEVMEMCONTEXT *phRGXDevMemContext,
													PVRSRV_DEVMEMCTX *phDevMemContext);

/*****************************************************************************
 FUNCTION	: RGXDestroyDeviceMemContext

 PURPOSE	: Destroys an RGX device memory context

 PARAMETERS	: phRGXDevMemContext	Handle to the RGX device memory context
*****************************************************************************/
IMG_EXPORT
void IMG_CALLCONV RGXDestroyDeviceMemContext(PRGX_DEVMEMCONTEXT hRGXDevMemContext);

#if defined(__cplusplus)
}
#endif

#endif /* RGXAPI_H */

/******************************************************************************
 End of file (rgxapi.h)
******************************************************************************/
