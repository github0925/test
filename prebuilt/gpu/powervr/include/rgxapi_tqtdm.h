/*************************************************************************/ /*!
@File           rgxapi_tqtdm.h
@Title          TDM API Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Current blit programming model provides a flexible interface
                where the unit of operation is a blit, that contains a number of
                source surfaces - each can contain a number of sub rectangles
                and a single destination surface, where a corresponding sub
                rectangle is specified for each of the rectangles of the
                sources. Arbitrary scale and rotation are allowed per rectangle
                pairs. The blit also contains a scissor on the
                destination. Pixels outside of the scissor are not updated. The
                area inside the scissor but outside of the the destination
                rectangles are either accumulated or filled with a fill colour.

                Sampler state or texture state changes require different TQ
                sources, even if they refer to the same surface in memory.

@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef TQ_TDM_API_H
#define TQ_TDM_API_H

#include "img_types.h"
#include "img_defs.h"
#include "img_3dtypes.h"
#include "powervr/imgyuv.h"
#include "imgpixfmts.h"
#include "rgx_common.h"
#include "pvrsrv_devmem.h"
#include "pvrsrv_sync_um.h"
#include "pvrsrv_devvar.h"


/*! use buffer synchronisation for the surface */
#define TQ_TDM_SURFACE_FLAG_BUFFER_SYNC 0x00000001U

/*! read/write compressed format as float */
#define TQ_TDM_SURFACE_FLAG_FBC_FLOAT	0x00000002U

/*! valid if doing a DS merge with depth + stencil to depth + stencil*/
#define TQ_TDM_SURFACE_FLAGS_PICKD      0x00000004U

/*! ************************************************************************//**
 @brief         A surface describes a source or destination for a transfer operation

 @note          As surfaces here are used in both source/destination context we
                refer to texels and pixels as they were the same thing.
*/ /**************************************************************************/
typedef struct TQ_TDM_SURFACE_TAG
{
	/*! Surface related flags TQ_TDM_SURFACE_FLAG_<*> */
	IMG_UINT32         ui32Flags;

	/*! Memory address */
	IMG_DEV_VIRTADDR   sDevVAddr;

	/*! Memory descriptor, read for buffer sync information. No mappings are read from this */
	PVRSRV_MEMDESC     hMemDesc;

	/*! Memory address for extra U/V planes */
	IMG_DEV_VIRTADDR   asUVAddress[2];

	/*! Memory descriptors for extra YUV planes, read for buffer sync information, no mappings are read from this */
	PVRSRV_MEMDESC     ahUVMem[3];

	/*! YUV colour space */
	IMG_YUV_COLORSPACE eYUVColourSpace;

	/*! surface width in texels */
	IMG_UINT32         ui32Width;

	/*! surface height in texels */
	IMG_UINT32         ui32Height;

	/*! surface depth. only applicable if IMG_MEMLAYOUT_3DTWIDDLED */
	IMG_UINT32         ui32Depth;

	/*! Z position in a 3D tecture. 0.0f <= fZPosition <= ui32Depth */
	IMG_FLOAT          fZPosition;

	/*! stride in texels */
	IMG_UINT32         ui32StrideInTexels;

	/*! pixel format*/
	IMG_PIXFMT         ePixFormat;

	/*! memory layout*/
	IMG_MEMLAYOUT      eMemLayout;

	/*! the number of samples per texel/pixel (0 = 1, 1 = 1) */
	IMG_UINT32         ui32SampleCount;

	/*! Frame buffer compression */
	IMG_FB_COMPRESSION     eFBCompression;
} TQ_TDM_SURFACE;


/*! ***********************************************************************//**
 @brief         Defines a mapping between source and destination sub rectangles

 Arbitrary coordinate flipping and rotation are supported.  Rotation refers to
 the sampler, the source and destination rectangles are specified in texel/pixel
 positionson, then rotation rotates and scales the content inside the
 subrectangle.  Rotation does not affect where texels are read from and where
 they are written to.
*/ /**************************************************************************/
typedef struct TQ_TDM_RECTANGLE_MAPPING_TAG
{
	/*! rotation of the specified mapping*/
	IMG_ROTATION        eRotation;
	IMG_RECT            sSrcRectangle;
	IMG_RECT            sDstRectangle;
} TQ_TDM_RECTANGLE_MAPPING;


/*! ***********************************************************************//**
 @brief          Extends the TQ_SURFACE structure with input specific data
*/ /**************************************************************************/
typedef struct TQ_TDM_SOURCE_TAG
{
	/*! source surface */
	TQ_TDM_SURFACE          sSurface;

	/*! source addressing mode.
	 *
	 * In case of a simple 1:1 copy this setting does not affect the output but
	 * will affect performance. Use don't care or clamp when possible.
	 */
	IMG_ADDRESSMODE         eAddressMode;

	/*! source filtering method */
	IMG_FILTER              eFilter;

	/*! MSAA resolve operation */
	IMG_RESOLVE_OP          eResolveOp;

	/*! @defgroup rectangle pairs of source -> destination mappings
	 */
	/* @{ */
	/*! number of mappings for the source */
	IMG_UINT32              ui32NumRectangleMappings;

	/*! mappings array */
	TQ_TDM_RECTANGLE_MAPPING  * pasRectangleMappings;
	/* @} */
} TQ_TDM_SOURCE;



/*! ***********************************************************************//**
 @defgroup       TQ command flags

 Depth stencil merge allows merging a depth only / stencil only or depth and
 stencil texture against a depth and stencil texture. The following combinations
 are valid:

   1. source is depth or depth & stencil, destination is depth & stencil
      flags : DSMERGE | PICKD Takes the depth of the source and the stencil of
      the destination and combines the two interleaved.
   2. source is stencil or depth & stencil, destination depth & stencil
      flags : DSMERGE Takes the stencil of the source and the depth of the
      destination and combines the two interleaved.
*/ /**************************************************************************/
/* @{ */
/*! PDUMP TQ op even if it's outside of capture range */
#define TQ_TDM_CMD_FLAGS_PDUMP_CONTINUOUS 0x00000001U
/*! whether or not to enable DRM security */
#define TQ_TDM_CMD_FLAGS_SECURE           0x00000002U
/*! merge a8 source with rgb from argb8888 or ayuv8888 dest */
#define TQ_TDM_CMD_FLAGS_A8MERGE          0x00000004U
/*! merge a depth or stencil against a depth + stencil texture*/
#define TQ_TDM_CMD_FLAGS_DSMERGE          0x00000008U
/*! valid if doing a DS merge with depth + stencil to depth + stencil*/
#define TQ_TDM_CMD_FLAGS_PICKD            0x00000010U
/*! destination pixels not covered by any of the destination rectangles but inside the scissor are filled with the clear colour */
#define TQ_TDM_CMD_FLAGS_FILL             0x00000020U
/*! destination surface coherent */
#define TQ_TDM_CMD_FLAGS_DEST_COHERENT    0x00000040U
/*! skip cache flush/inval at the end of the blit */
#define TQ_TDM_CMD_FLAGS_SKIP_FLUSH_INVAL 0x00000080U
/*! Terminate and flush previously opened stream before this blit */
#define TQ_TDM_CMD_FLAGS_FLUSHCACHE       0x00000100U
/*! submit operation without validation */
#define TQ_TDM_CMD_FLAGS_SUBMITONLY       0x00000200U
/*!< If the CPU reads from the TQ source surface, the cache must be flushed first */
#define TQ_TDM_CMD_FLAGS_CPU_FLUSH_SOURCE 0x00000400U
/*! Safety critical protected content */
#define TQ_TDM_CMD_FLAGS_TRP_PROTECTED    0x00000800U
/*! Use single core in a multi core setup */
#define TQ_TDM_CMD_FLAGS_SINGLE_CORE      0x00001000U
/* @} */


/*! ***********************************************************************//**
 @brief          The transfer command characteristics
*/ /**************************************************************************/
#if defined(SUPPORT_WORKLOAD_ESTIMATION)
typedef struct RGX_TQ_WORKLOAD_CHARACTERISTICS_TAG
{
	IMG_UINT32	ui32DestSizeInTiles;
	IMG_UINT32	ui32PixFormatID;
} RGX_TQ_WORKLOAD_CHARACTERISTICS;
#endif

/*! ***********************************************************************//**
 @brief          The transfer command
*/ /**************************************************************************/
typedef struct TQ_TDM_TRANSFER_COMMAND_TAG
{
	/*! flags (TQ_CMD_FLAGS_*) */
	IMG_UINT32                 ui32Flags;

	/*! frame number*/
	IMG_UINT32                 ui32Frame;

	/*! number of sources in the pasSources array */
	IMG_UINT32                 ui32NumSources;

	/*! array of source surfaces*/
	TQ_TDM_SOURCE              * pasSources;

	/*! clear colour */
   	IMG_CLEAR_COLOUR           uClearColour;

	/*! a destination scissor rectangle */
	IMG_RECT                   sScissor;

	/*! the destination surface*/
	TQ_TDM_SURFACE             sDest;

	/*! Number of DevVar updates */
	IMG_UINT32                 ui32NumDevVars;

	/*! DevVar update array */
	PVRSRV_DEV_VAR_UPDATE      asDevVarUpdates[PVRSRV_MAX_DEV_VARS];   /*!< dev var update array */

	PVRSRV_FENCE               iCheckFence; /*!< Fence sync checked before this command, or PVRSRV_NO_FENCE */

	/*! client job reference - used in debug for tracking submitted work */
	IMG_UINT32                 ui32ExtJobRef;

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	/*! TDM workload characteristics - used to estimate GPU usage by future workloads */
	RGX_TQ_WORKLOAD_CHARACTERISTICS		sWorkloadCharacteristics;
#endif
} TQ_TDM_TRANSFER_COMMAND;


/*! ***********************************************************************//**
 @defgroup       TQ mipgen flags
*/ /**************************************************************************/
/* @{ */
/*! PDUMP TQ op even if it's outside of capture range */
#define TQ_TDM_MIPGEN_FLAGS_PDUMP_CONTINUOUS 0x00000001U
/*! Disable 2 level optimisation if first level width or height uneven */
#define TQ_TDM_MIPGEN_FLAGS_STRICT_BILINEAR  0x00000002U
/*! First level from separate surface */
#define TQ_TDM_MIPGEN_FLAGS_SEPARATE_SOURCE  0x00000004U
/*! whether or not to enable DRM security */
#define TQ_TDM_MIPGEN_FLAGS_SECURE           0x00000008U
/*! submit operation without validation */
#define TQ_TDM_MIPGEN_FLAGS_SUBMITONLY       0x00000010U
/*! Safety critical protected content */
#define TQ_TDM_MIPGEN_FLAGS_TRP_PROTECTED    0x00000020U
/*! Use single core in a multi core setup */
#define TQ_TDM_MIPGEN_FLAGS_SINGLE_CORE      0x00000040U
/* @} */


/*! ***********************************************************************//**
 @brief          The mipgen command
*/ /**************************************************************************/
typedef struct TQ_TDM_MIPGEN_COMMAND_TAG
{
	/*! flags (TQ_MIPGEN_FLAGS_*) */
	IMG_UINT32                 ui32Flags;

	/*! frame number*/
	IMG_UINT32                 ui32Frame;

	/*! source surface,
	  used when flag TQ_TDM_MIPGEN_FLAGS_SEPARATE_SOURCE is set */
	TQ_TDM_SURFACE             sSource;

	/*! mipmap surface */
	TQ_TDM_SURFACE             sDest;

	/*! start generating from level */
	IMG_UINT32                 ui32BaseLevel;

	/*! number of levels to generate */
	IMG_UINT32                 ui32NumLevelsToGenerate;

	/*! Number of DevVar updates */
	IMG_UINT32                 ui32NumDevVars;

	/*! DevVar update array */
	PVRSRV_DEV_VAR_UPDATE      asDevVarUpdates[PVRSRV_MAX_DEV_VARS];

	/*!< Fence sync checked before this command, or PVRSRV_NO_FENCE */
	PVRSRV_FENCE               iCheckFence;

	/*!< client job reference - used in debug for tracking submitted work */
	IMG_UINT32                 ui32ExtJobRef;

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	/*!< TDM workload characteristics - used to estimate GPU usage by future workloads */
	RGX_TQ_WORKLOAD_CHARACTERISTICS		sWorkloadCharacteristics;
#endif
} TQ_TDM_MIPGEN_COMMAND;


/*! ***********************************************************************//**
 @defgroup       TQ submit flags
*/ /**************************************************************************/
/* @{ */
/*! Terminate and flush previously opened stream between each blit */
#define TQ_TDM_SUBMIT_FLAGS_FLUSHCACHE       0x00000001U
/*! Use single core in a multi core setup */
#define TQ_TDM_SUBMIT_FLAGS_SINGLE_CORE      0x00000002U
/* @} */


/*! ***********************************************************************//**
 @brief          The submit command
*/ /**************************************************************************/
typedef struct TQ_TDM_SUBMIT_TAG
{
	/*! flags (TQ_SUBMIT_FLAGS_*) */
	IMG_UINT32                 ui32Flags;

	/*! frame number*/
	IMG_UINT32                 ui32Frame;

	/*! number of prepares in the pasPrepares array */
	IMG_UINT32                 ui32NumPrepares;

	/*! array of prepares */
	IMG_HANDLE                 * pahPrepares;

	/*!< Fence sync checked before this command, or PVRSRV_NO_CHECK_FENCE_REQUIRED */
	PVRSRV_FENCE               iCheckFence;

	/*!< client job reference - used in debug for tracking submitted work */
	IMG_UINT32                 ui32ExtJobRef;
} TQ_TDM_SUBMIT;

/*! ***********************************************************************//**
 @brief          Transfer context type
*/ /**************************************************************************/
typedef enum TQ_TDM_CONTEXT_TYPE_TAG
{
	TQ_TDM_CONTEXT_QUEUE   = 0,
	TQ_TDM_CONTEXT_PREPARE = 1,
	TQ_TDM_CONTEXT_SUBMIT  = 2,
} TQ_TDM_CONTEXT_TYPE;

/*! ***********************************************************************//**
 @brief          Structures passed in as parameters to transfer queue functions
*/ /**************************************************************************/
typedef struct TQ_TDM_CREATE_CONTEXT_TAG
{
	/*! Context creation flags */
	IMG_UINT32          ui32ContextFlags;

	/*! Device memory context*/
	PVRSRV_DEVMEMCTX    hDevMemContext;

	/*! Context switch priority */
	IMG_UINT32          ui32Priority;

	/*! A 1 character tag describing client API requesting tdm context, this
        will have the DM's resource tag (ie. TDM) appended to it. Used in
        naming resources associated with this context, eg. Timeline. */
	IMG_CHAR            cClientAPI;

	/*! Transfer context type */
	TQ_TDM_CONTEXT_TYPE eType;

	/*! TQ shared memory. */
	IMG_HANDLE hStaticMem;
} TQ_TDM_CREATE_CONTEXT;


IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMQueueTransfer(
	IMG_HANDLE                    hTransferContext,
	const TQ_TDM_TRANSFER_COMMAND * psQueueTransfer,
	PVRSRV_FENCE                  * piUpdateFence,
	const IMG_CHAR                * pcszFenceName);

/**************************************************************************/ /*!
@Function       RGXTDMQueueValidate
@Description    Validate TDM TQ command
@Input          psQueueTransfer         Transfer command.
@Return                                 IMG_TRUE on success.
 */ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV RGXTDMQueueValidate(
	const TQ_TDM_TRANSFER_COMMAND * psQueueTransfer);

/**************************************************************************/ /*!
@Function       RGXTDMMipgen
@Description    Creates a mipmap.
@Input          hTransferContext        handle to a transfer context
@Input          psMipgenCommand         Mipmap gen command.
@Input          piUpdateFence           fence sync
@Input          pcszFenceName           fence sync name
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMMipgen(
	IMG_HANDLE                   hTransferContext,
	const TQ_TDM_MIPGEN_COMMAND  * psMipgenCommand,
	PVRSRV_FENCE                 * piUpdateFence,
	const IMG_CHAR               * pcszFenceName);

/*************************************************************************/ /*!
@Function       RGXTDMPrepare
@Description    Create TDM prepare handle.
@Input          hTransferContext  TDM transfer context.
@Input          psCommand         TDM transfer command.
@Output         phPrepare         Pointer to prepare handle.
@Return         PVRSRV_ERROR      PVRSRV_OK or error code.
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMPrepare(
	IMG_HANDLE                    hTransferContext,
	const TQ_TDM_TRANSFER_COMMAND * psCommand,
	IMG_HANDLE                    * phPrepare);

/*************************************************************************/ /*!
@Function       RGXTDMDestroyPrepare
@Description    Destroy TDM prepare handle.
@Input          hPrepare          Prepare handle.
@Return         PVRSRV_ERROR      PVRSRV_OK or error code.
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMDestroyPrepare(IMG_HANDLE hPrepare);

/*************************************************************************/ /*!
@Function       RGXTDMSubmit
@Description    Submit TDM prepares.
@Input          hTransferContext  TDM transfer context.
@Input          psSubmit          TDM Submit command
@Output         piUpdateFence     Set to not NULL to request an update fence.
@Input          pcszFenceName     Name for update fence.
@Return         PVRSRV_ERROR      PVRSRV_OK or error code.
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMSubmit(
	IMG_HANDLE          hTransferContext,
	const TQ_TDM_SUBMIT * psSubmit,
	PVRSRV_FENCE        * piUpdateFence,
	const IMG_CHAR      * pcszFenceName);

/**************************************************************************/ /*!
@Function       RGXTDMMipgenValidate
@Description    Validate TQ Mipgen command
@Input          psMipgenCommand         Mipgen command.
@Return                                 IMG_TRUE on success.
 */ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV RGXTDMMipgenValidate(
	const TQ_TDM_MIPGEN_COMMAND * psMipgenCommand);

/*************************************************************************/ /*!
@Function       RGXTDMCreateStaticMem
@Description    Create shared read-only TQ memory, for re-use in more than
                one TQ context. Handle given by this function can be set
                in TQ_TDM_CREATE_CONTEXT when creating a new TQ context.
                Memory contexts must be same when creating TQ context.
@Input          psDevConnection   Device connection.
@Input          hDevMemContext    Memory context.
@Output         phStaticMem       Pointer to TQ shared memory handle.
@Return         PVRSRV_ERROR      PVRSRV_OK or error code.
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMCreateStaticMem(
	PVRSRV_DEV_CONNECTION       * psDevConnection,
	PVRSRV_DEVMEMCTX              hDevMemContext,
	IMG_HANDLE                  * phStaticMem);

/*************************************************************************/ /*!
@Function       RGXTDMDestroyStaticMem
@Description    Destroy shared read-only TQ memory created
                by RGXTDMCreateStaticMem.
@Input          psDevConnection   Device connection.
@Input          hStaticMem        Shared memory handle.
@Return         PVRSRV_ERROR      PVRSRV_OK or error code.
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMDestroyStaticMem(PVRSRV_DEV_CONNECTION * psDevConnection,
												 IMG_HANDLE              hStaticMem);

/**************************************************************************/ /*!
@Function       RGXTDMCreateTransferContext / RGXTDMCreateTransferContextCCB
@Description    Creates a transfer context.
				RGXTDMCreateTransferContextCCB will use custom CCB size.
@Input          psDevConnection         Pointer to Device-Data structure
@Input          psCreateTransfer        Parameters required for creation of a
                                        new transfer context
@Input(for CCB) ui32CCBAllocSizeLog2    Optional custom CCB size.
                ui32CCBMaxAllocSizeLog2 Might be 0 for default size.
@Output         phTransferContext       On success, points to the newly-created
                                        transfer context.
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMCreateTransferContext(
	PVRSRV_DEV_CONNECTION       * psDevConnection,
	const TQ_TDM_CREATE_CONTEXT * psCreateTransfer,
	IMG_HANDLE                  * phTransferContext);
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMCreateTransferContextCCB(
	PVRSRV_DEV_CONNECTION       * psDevConnection,
	const TQ_TDM_CREATE_CONTEXT * psCreateTransfer,
	IMG_UINT32					  ui32CCBAllocSizeLog2,
	IMG_UINT32					  ui32CCBMaxAllocSizeLog2,
	IMG_HANDLE                  * phTransferContext);

/**************************************************************************/ /*!
@Function       RGXDestroyTransferContext
@Description    Destroy the specified transfer context
@Input          hTransferContext        handle to a transfer context
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXTDMDestroyTransferContext(IMG_HANDLE hTransferContext);


/**************************************************************************/ /*!
@Function       RGXSetTransferContextPriority
@Description    Destroy the specified transfer context
@Input          psDevData               Pointer to Device-Data structure
@Input          hContext                Handle to a transfer context
@Input          ui32Priority            Priority to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXTDMSetTransferContextPriority(
	PVRSRV_DEV_CONNECTION * psDevConnection,
	IMG_HANDLE            hContext,
	IMG_UINT32            ui32Priority);


/**************************************************************************/ /*!
@Function       RGXTDMSetTransferContextFlags
@Description    Set the flags (properties) for the specified transfer context
@Input          psDevConnection         Pointer to Device-Data structure
@Input          hContext                Handle to a transfer context
@Input          ui32ContextFlags        Flags to set
@Return                                 PVRSRV_OK on success. Otherwise, a PVRSRV_
                                        error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR RGXTDMSetTransferContextFlags(PVRSRV_DEV_CONNECTION *psDevConnection,
										   IMG_HANDLE            hContext,
										   IMG_UINT32            ui32ContextFlags);
#endif /* TQ_TDM_API_H */

/**************************************************************************/ /*!
End of file (tqtdmapi.h)
 */ /**************************************************************************/
