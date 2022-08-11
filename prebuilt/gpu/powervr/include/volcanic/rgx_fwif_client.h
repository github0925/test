/*************************************************************************/ /*!
@File
@Title          RGX firmware interface structures
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX firmware interface structures used by srvclient
@License        Strictly Confidential.
*/ /**************************************************************************/
#if !defined (RGX_FWIF_CLIENT_H)
#define RGX_FWIF_CLIENT_H

#include "img_types.h"
#include "img_defs.h"

#include "rgxdefs.h"
#include "rgx_fwif_alignchecks.h"
#include "rgx_fwif_shared.h"


/* Indicates the number of RTDATAs per RTDATASET */
#define RGXMKIF_NUM_RTDATAS			2U


#define RGXMKIF_RENDERFLAGS_ABORT				0x00000002UL	/*!< The scene has been aborted free the parameters and dummy
																	 process to completion */
#define RGXMKIF_RENDERFLAGS_3D_ONLY				0x00000004UL	/*!< The TA before this was not marked as LAST. */
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_RENDERFLAGS_SINGLE_CORE			0x00000008UL	/*!< Use single core in a multi core setup. */
#endif
#define RGXMKIF_RENDERFLAGS_DEPTHBUFFER			0x00000080UL	/*!< Indicates whether a depth buffer is present */
#define RGXMKIF_RENDERFLAGS_STENCILBUFFER		0x00000100UL	/*!< Indicates whether a stencil buffer is present */
#define RGXMKIF_RENDERFLAGS_SECURE				0x00002000UL	/*!< This render needs DRM Security */
#define RGXMKIF_RENDERFLAGS_ABORT_NOFREE		0x00004000UL	/*!< This flags goes in hand with ABORT and explicitly ensures no mem free is issued in case of first TA kick */
#define RGXMKIF_RENDERFLAGS_DISABLE_PIXELMERGE	0x00008000UL	/*!< Force disabling of pixel merging */
#define RGXMKIF_RENDERFLAGS_SPMSCRATCHBUFFER	0x00080000UL	/*!< Partial render must write to scratch buffer */
#if defined(SUPPORT_STRIP_RENDERING)
#define RGXMKIF_RENDERFLAGS_FRAME_STRIP			0x00100000UL	/*!< Render is a frame strip and should communicate with display via GPIO */
#endif

/*
	The host must indicate if this is the first and/or last command to
	be issued for the specified task
*/
#define RGXMKIF_TAFLAGS_FIRSTKICK				0x00000001UL
#define RGXMKIF_TAFLAGS_LASTKICK				0x00000002UL

#if defined(SUPPORT_TRP)
/*
 * Enable Tile Region Protection for this TA
 */
#define RGXMKIF_TAFLAGS_TRP						0x00000004UL

#endif

#if defined(SUPPORT_SW_TRP)
/*
 * Enable Tile Region Protection for this TA
 */
#define RGXMKIF_TAFLAGS_SWTRP						0x00000008UL
#endif

#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_TAFLAGS_SINGLE_CORE				0x00000010UL /*!< Use single core in a multi core setup. */
#endif

/*
 * 	Indicates the particular TA needs to be aborted
 */
#define RGXMKIF_TAFLAGS_TA_ABORT				0x00000100UL

/*
 * 	Indicates that the TA streams out to memory only. No params are written.
 */
#define RGXMKIF_TAFLAGS_TA_MEMONLY				0x00000200UL

/*
 * 	Reserve a contiguous local memory buffer for the domain shader.
 */
#define RGXMKIF_TAFLAGS_LM_TESS					0x00000400UL

#define RGXMKIF_TAFLAGS_SECURE					0x00000800UL
#define RGXMKIF_TAFLAGS_PPP_PRIMID_EN			0x00001000UL
#define RGXMKIF_TAFLAGS_TESS_EN					0x00002000UL

#define RGXMKIF_TAFLAGS_PHR_TRIGGER				0x01000000UL

#define RGXMKIF_CMDTDM_FLAG_SECURE              0x00000001U
#define RGXMKIF_CMDTDM_FLAG_SKIP_FLUSH_INVAL    0x00000002U
#define RGXMKIF_CMDTDM_FLAG_TRP					0x00000040UL
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_CMDTDM_FLAG_SINGLE_CORE			0x00000080UL /*!< Use single core in a multi core setup. */
#endif

/* flags for compute commands */
#define RGXMKIF_COMPUTE_FLAG_SECURE				0x00000001U
#define RGXMKIF_COMPUTE_FLAG_USES_BARRIERS		0x00000002U
#define RGXMKIF_COMPUTE_FLAG_EXPLICIT_OFFSET	0x00000004U
#define RGXMKIF_COMPUTE_FLAG_USRM_RECONFIG		0x00000008U
#define RGXMKIF_COMPUTE_FLAG_STACK_DISABLED		0x00000010U
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_COMPUTE_FLAG_SINGLE_CORE		0x00000020U /*!< Use single core in a multi core setup. */
#endif

/*****************************************************************************
 Parameter/HWRTData control structures.
*****************************************************************************/

/*!
	Configuration registers which need to be loaded by the firmware before a TA
	kick can be started.
*/
typedef struct
{
	IMG_UINT64	uTAReg_TE_PSGREGION_ADDR;
	IMG_UINT64	uTAReg_DCE_ROOT_CTRL_STREAM;
	IMG_UINT64	uTAReg_DCE_INDEX_FORMAT0;
	IMG_UINT64	uTAReg_TA_RTC_ADDR_BASE;
#if defined(RGX_FEATURE_TE_RENDER_TARGET_CACHE_AUTO_FLUSH)
	IMG_UINT64	uTAReg_GEOM_RT_LIST_ADDR_BASE;
#endif
	IMG_UINT64	uTAReg_TPU_BORDER_COLOUR_TABLE;
	IMG_UINT64	uTAReg_DCE_SCISSOR_BASE;
	IMG_UINT64	uTAReg_DCE_DBIAS_BASE;
	IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_VDM1_SIZE;
	IMG_UINT64	uTAReg_DCE_CONTEXT_RESUME_TASK_DDM1_SIZE;

	IMG_UINT32	uTAReg_DCE_INDEX_FORMAT1;

	IMG_UINT32	uTAReg_PPP_CTRL;
	IMG_UINT32	uTAReg_TE_CTRL;

#if (RGX_FEATURE_USC_NUM_GLOBAL_SPECIAL_REGISTERS > 0)
	IMG_UINT32  uVDMViewIndex;
#if defined(RGX_FEATURE_TESSELLATION)
	IMG_UINT32	uDDMViewIndex;
#endif
#endif

} RGXFWIF_TAREGISTERS;

/*!
	TA command. The RGX TA can be used to tile a whole scene's objects
	as per TA behaviour on RGX.
*/
typedef struct
{
	/* CMDTA3D_SHARED field MUST BE ALWAYS AT THE BEGINNING OF THE CMD STRUCT !!
	 *
	 * The command struct (RGXFWIF_CMDTA) is shared between Client and Firmware.
	 * Kernel is unable to perform read/write operations on the command struct,
	 * the SHARED region is our only exception from that rule.
	 * This region must be the first member so Kernel can easily access it.
	 * For more info, see CMDTA3D_SHARED definition.
	 */
	CMDTA3D_SHARED sCmdShared;

	RGXFWIF_TAREGISTERS			RGXFW_ALIGN sGeomRegs;/*!< TA registers */
	IMG_UINT32					RGXFW_ALIGN ui32Flags; /*!< command control flags */
	IMG_UINT32					ui32FrameNum;/*!< associated frame number */
#if defined(RGX_FEATURE_TESSELLATION) && !defined(FIX_HW_BRN_69123)
	IMG_UINT32					uiPDSTessReserve; /*!< PDS store reserve for tessellation */
#endif
	RGXFWIF_UFO					sPartialRenderTA3DFence; /* Holds the TA/3D fence value to allow the 3D partial render command to go through */

	IMG_UINT64				RGXFW_ALIGN ui64SPUEnableInfoAddr;

} RGXFWIF_CMDTA,*PRGXFWIF_CMDTA;

static_assert(offsetof(RGXFWIF_CMDTA, sCmdShared) == 0U,
				"CMDTA3D_SHARED must be the first member of RGXFWIF_CMDTA");

static_assert(sizeof(RGXFWIF_CMDTA) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current TA command size");



/*!
	Configuration registers which need to be loaded by the firmware before ISP
	can be started.
*/
typedef struct
{
	/* All 32 bit values should be added in the top section. This then requires only a
	 * single RGXFW_ALIGN to align all the 64 bit values in the second section */
	IMG_UINT32  u3DReg_USC_PIXEL_OUTPUT_CTRL;
	IMG_UINT32  au3DReg_USC_CLEAR_REGISTER[RGX_NUM_CLEAR_REGISTERS];

#if !defined(RGX_FEATURE_ALBIORIX_TOP_INFRASTRUCTURE)
	IMG_UINT32  u3DReg_ISP_MTILE_SIZE;
#endif
	IMG_UINT32  u3DReg_ISP_BGOBJDEPTH;
	IMG_UINT32  u3DReg_ISP_BGOBJVALS;
	IMG_UINT32  u3DReg_ISP_MSAA;
#if !defined(RGX_FEATURE_ALBIORIX_TOP_INFRASTRUCTURE)
	IMG_UINT32  u3DReg_ISP_RGN;
#endif
	IMG_UINT32  u3DReg_ISP_CTL;
	IMG_UINT32  u3DReg_ISP_RENDER;
	IMG_UINT32  u3DReg_EVENT_PIXEL_PDS_DATA;
#if (RGX_FEATURE_USC_NUM_GLOBAL_SPECIAL_REGISTERS > 0)
	IMG_UINT32  u3DViewIndex;
#endif

	/* All values below the RGXFW_ALIGN must be 64 bit */
	IMG_UINT64	RGXFW_ALIGN u3DReg_ISP_RGN_BASE;
	IMG_UINT64	u3DReg_ISP_SCISSOR_BASE;
	IMG_UINT64	u3DReg_ISP_DBIAS_BASE;
    IMG_UINT64  u3DReg_ISP_OCLQRY_BASE;
    IMG_UINT64  u3DReg_ISP_ZLSCTL;
    IMG_UINT64  u3DReg_ISP_ZLOAD_STORE_BASE;
    IMG_UINT64  u3DReg_ISP_ZLOAD_STORE_BASE2;        /* Duplicate of above to program two registers in one kick */
    IMG_UINT64  u3DReg_ISP_STENCIL_LOAD_STORE_BASE;
    IMG_UINT64  u3DReg_ISP_STENCIL_LOAD_STORE_BASE2; /* Duplicate of above to program two registers in one kick */
#if (RGX_FEATURE_ZLS_VERSION==2)
	IMG_UINT64  u3DReg_ISP_ZLS_MSAA_SINGLE_SAMPLE_ADDRESS;
	IMG_UINT64  u3DReg_ISP_ZLS_STATE0;
	IMG_UINT64  u3DReg_ISP_ZLS_STATE1;
#if (RGX_FEATURE_ZLS_STATE_VERSION>1)
	IMG_UINT64	u3DReg_ISP_ZLS_STATE2;
#endif
#else
	IMG_UINT64  u3DReg_ISP_ZLS_PIXELS;
#endif
	IMG_UINT64	au3DReg_PBE_WORD[8][RGX_PBE_WORDS_REQUIRED_FOR_RENDERS];
	IMG_UINT64	u3DReg_TPU_BORDER_COLOUR_TABLE;
	IMG_UINT64	au3DReg_PDS_BGND[3];
	IMG_UINT64	au3DReg_PDS_PR_BGND[3];
#if defined(SUPPORT_TLT_PERF) && defined(RGX_FEATURE_ISP_TILE_LIFETIME_TRACKING)	
	IMG_UINT64	u3DReg_ISP_TILE_LIFETIME;
#endif
} RGXFWIF_3DREGISTERS;

typedef struct RGXFWIF_CMD3D_STRUCT
{
	/* CMDTA3D_SHARED field MUST BE ALWAYS AT THE BEGINNING OF THE CMD STRUCT !!
	 *
	 * The command struct (RGXFWIF_CMD3D) is shared between Client and Firmware.
	 * Kernel is unable to perform read/write operations on the command struct,
	 * the SHARED region is our only exception from that rule.
	 * This region must be the first member so Kernel can easily access it.
	 * For more info, see CMDTA3D_SHARED definition.
	 */
	CMDTA3D_SHARED RGXFW_ALIGN sCmdShared;


	RGXFWIF_3DREGISTERS		RGXFW_ALIGN s3DRegs;		/*!< 3D registers */
	IMG_UINT32				ui32Flags; /*!< command control flags */
	IMG_UINT32				ui32FrameNum;/*!< associated frame number */
	IMG_UINT32				ui32ZLSStride; /* Stride IN BYTES for Z-Buffer in case of RTAs */
	IMG_UINT32				ui32SLSStride; /* Stride IN BYTES for S-Buffer in case of RTAs */
#if (RGX_FEATURE_ZLS_VERSION==2)
	IMG_UINT32				ui32ZLSResolveStride; /* Stride IN BYTES for Z/S-Buffer MSAA resolve in case of RTAs */
#endif
#if defined(SUPPORT_STRIP_RENDERING)
	IMG_UINT8				ui8FrameStripBuffer;
	IMG_UINT8				ui8FrameStripIndex;
	IMG_UINT8				ui8FrameStripMode;
#endif
} RGXFWIF_CMD3D,*PRGXFWIF_CMD3D;

static_assert(offsetof(RGXFWIF_CMD3D, sCmdShared) == 0U,
				"CMDTA3D_SHARED must be the first member of RGXFWIF_CMD3D");

static_assert(sizeof(RGXFWIF_CMD3D) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current 3D command size");

typedef struct
{
	IMG_UINT64 uTDMReg_TDM_CONTEXT_STATE_BASE;
	IMG_UINT64 uTDMReg_TDM_CB_QUEUE;
#if defined(SUPPORT_TRUSTED_DEVICE)
	IMG_UINT64 uTDMReg_TDM_CB_SECURE_QUEUE;
#endif
	IMG_UINT64 uTDMReg_TDM_CB_BASE;

	IMG_UINT32 uTDMReg_EVENT_TDM_PDS_INFO;
	IMG_UINT32 uTDMReg_EVENT_TDM_PDS_CODE;
	IMG_UINT32 uTDMReg_EVENT_TDM_PDS_DATA;

	IMG_UINT32 uTDMReg_TDM_CB;

} RGXFWIF_TDMREGISTERS;


typedef struct
{
	RGXFWIF_TDMREGISTERS			RGXFW_ALIGN sTDMRegs;

	IMG_UINT32				RGXFW_ALIGN ui32FrameNum;
	IMG_UINT32				ui32Flags;/*!< flags */
	IMG_UINT32				ui32StreamStartOffset;	/*!< Stream starting point within the circular buffer */

} RGXFWIF_CMDTDM;

static_assert(sizeof(RGXFWIF_CMDTDM) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current TDM command size");

/*****************************************************************************
 Host interface structures.
*****************************************************************************/

/*!
	Configuration registers which need to be loaded by the firmware before CDM
	can be started.
*/
typedef struct
{
	IMG_UINT64	uCDMReg_TPU_BORDER_COLOUR_TABLE;

	IMG_UINT64	uCDMReg_CDM_CB_QUEUE;
#if defined(SUPPORT_TRUSTED_DEVICE)
	IMG_UINT64	uCDMReg_CDM_CB_SECURE_QUEUE;
#endif
	IMG_UINT64	uCDMReg_CDM_CB_BASE;
	IMG_UINT64	uCDMReg_CDM_STACK_STATE_BASE;
	IMG_UINT64	uCDMReg_CDM_RESUME_PDS1;

	IMG_UINT32	uCDMReg_CDM_CB;

} RGXFWIF_CDM_REGISTERS;

/*!
	RGX Compute command.
*/
typedef struct
{
	RGXFWIF_CDM_REGISTERS			RGXFW_ALIGN sCDMRegs;				/*!< CDM registers */
	IMG_UINT32						RGXFW_ALIGN ui32Flags;				/*!< Control flags */
	IMG_UINT32						uiNumTempRegions;					/*!< Number of temporary register regions to pre-allocate */
	IMG_UINT64						RGXFW_ALIGN ui64StreamStartOffset;	/*!< Stream starting point within the circular buffer */
	IMG_UINT32						ui32FrameNum;						/*!< Client frame number */
} RGXFWIF_CMD_COMPUTE;

static_assert(sizeof(RGXFWIF_CMD_COMPUTE) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current COMPUTE command size");


/*
 * Minimum/Maximum PB size supported by RGX and Services.
 *
 * Base page size is dependent on core:
 *   S8XT                     = 50 pages
 *   S8XT with BRN66011 fixed = 38 pages
 *
 * Minimum PB = Base Pages + (NUM_TE_PIPES-1)*16K + (NUM_VCE_PIPES-1)*64K + IF_PM_PREALLOC(NUM_TE_PIPES*16K + NUM_VCE_PIPES*16K)
 *
 * Maximum PB size must ensure that no PM address space can be fully used,
 * because if the full address space was used it would wrap and corrupt itself.
 * Since there are two freelists (local is always minimum sized) this can be
 * described as following three conditions being met:
 *
 *   (Minimum PB + Maximum PB)  <  ALIST PM address space size (16GB)
 *   (Minimum PB + Maximum PB)  <  TE PM address space size (16GB) / NUM_TE_PIPES
 *   (Minimum PB + Maximum PB)  <  VCE PM address space size (16GB) / NUM_VCE_PIPES
 *
 * Since the max of NUM_TE_PIPES and NUM_VCE_PIPES is 4, we have a hard limit
 * of 4GB minus the Minimum PB. For convenience we take the smaller power-of-2
 * value of 2GB. This is far more than any normal application would request
 * or use.
 */
#if defined(FIX_HW_BRN_66011)
#define		RGX_PM_BASE_MINIMUM_PAGES			(50U)
#else
#define		RGX_PM_BASE_MINIMUM_PAGES			(38U)
#endif

#define		RGX_PM_FLPAGES_FOR_SCALABLE_TE		((RGX_FEATURE_SCALABLE_TE_ARCH-1U)*4U)

#define		RGX_PM_FLPAGES_FOR_SCALABLE_VCE		((RGX_FEATURE_SCALABLE_VCE-1U)*16U)

/* With PM Pre-Alloc each requester can take an additional page, for VCE2 + TE2 config, it is 64KB. */
#define		RGX_PM_FLPAGES_FOR_TE_PREALLOC		(RGX_FEATURE_SCALABLE_TE_ARCH*4U)
#define		RGX_PM_FLPAGES_FOR_VCE_PREALLOC		(RGX_FEATURE_SCALABLE_VCE*4U)

#define		RGX_PM_MIN_FLSIZE					((RGX_PM_FLPAGES_FOR_SCALABLE_TE  +    \
                                                  RGX_PM_FLPAGES_FOR_SCALABLE_VCE +    \
                                                  RGX_PM_FLPAGES_FOR_TE_PREALLOC  +    \
                                                  RGX_PM_FLPAGES_FOR_VCE_PREALLOC +    \
                                                  RGX_PM_BASE_MINIMUM_PAGES) * RGX_BIF_PM_PHYSICAL_PAGE_SIZE)

#define		RGX_PM_MAX_FLSIZE					IMG_UINT64_C(2U*1024U*1024U*1024) /* 2GB */

#if defined(FIX_HW_BRN_68497)
#define TDM_CONTEXT_STATE_SIZE 96
#define TDM_CB_QUEUE_SIZE RGX_CR_TDM_CB_QUEUE_CTRL_BASE_ADDR_ALIGNSIZE
#define TDM_BRN68497_TDMCB_SIZE RGX_CR_TDM_CB_BASE_ADDR_ALIGNSIZE
#endif

/* Applied to RGX_CR_VDM_SYNC_PDS_DATA_BASE */
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_VDM_SYNC_OFFSET_BYTES	0
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_VDM_SYNC_MAX_SIZE_BYTES	128

/* Applied to RGX_CR_EVENT_PIXEL_PDS_CODE */
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_EOT_OFFSET_BYTES			128
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_EOT_MAX_SIZE_BYTES		128

/* Applied to RGX_CR_PDS_LOCAL_FREE_PROG_CODE */
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_LOCAL_FREE_OFFSET_BYTES		256
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_LOCAL_FREE_MAX_SIZE_BYTES	128

#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_TOTAL_BYTES				4096


/* Pointed to by PDS code at RGX_CR_VDM_SYNC_PDS_DATA_BASE */
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_VDM_SYNC_OFFSET_BYTES	0
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_VDM_SYNC_MAX_SIZE_BYTES	128

/* Used for DM kill */
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_DM_KILL_OFFSET_BYTES		128
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_DM_KILL_MAX_SIZE_BYTES	128

#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_TOTAL_BYTES				4096


/* Applied to RGX_CR_TPU_YUV_CSC_COEFFICIENTS */
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_YUV_CSC_OFFSET_BYTES		128
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_YUV_CSC_MAX_SIZE_BYTES	1024

#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_TOTAL_BYTES			4096

#endif /*  RGX_FWIF_CLIENT_H */

/******************************************************************************
 End of file (rgx_fwif_client.h)
******************************************************************************/
