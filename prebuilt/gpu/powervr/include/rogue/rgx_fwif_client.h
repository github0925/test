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

#define RGXMKIF_RENDERFLAGS_FLIP_SAMPLE_POSITIONS	0x00000001UL/*!< Render needs flipped sample positions */
#define RGXMKIF_RENDERFLAGS_ABORT				0x00000002UL	/*!< The scene has been aborted free the parameters and dummy
																	 process to completion */
#define RGXMKIF_RENDERFLAGS_3D_ONLY				0x00000004UL	/*!< The TA before this was not marked as LAST. */
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_RENDERFLAGS_SINGLE_CORE			0x00000008UL	/*!< Use single core in a multi core setup. */
#endif
#define RGXMKIF_RENDERFLAGS_GETVISRESULTS		0x00000020UL	/*!< This render has visibility result associated with it.
																	 Setting this flag will cause the firmware to collect the visibility results */
#define RGXMKIF_RENDERFLAGS_DEPTHBUFFER			0x00000080UL	/*!< Indicates whether a depth buffer is present */
#define RGXMKIF_RENDERFLAGS_STENCILBUFFER		0x00000100UL	/*!< Indicates whether a stencil buffer is present */
#define RGXMKIF_RENDERFLAGS_SECURE				0x00002000UL	/*!< This render needs DRM Security */
#define RGXMKIF_RENDERFLAGS_ABORT_NOFREE		0x00004000UL	/*!< This flags goes in hand with ABORT and explicitly ensures no mem free is issued in case of first TA kick */
#define RGXMKIF_RENDERFLAGS_DISABLE_PIXELMERGE	0x00008000UL	/*!< Force disabling of pixel merging */
#if defined(FIX_HW_BRN_51537)
#define RGXMKIF_RENDERFLAGS_CSRM_MAX_COEFFS		0x00020000UL	/*!< Force 4 lines of coeffs on render */
#endif
#if defined(FIX_HW_BRN_59998)
#define RGXMKIF_RENDERFLAGS_FORCE_TPU_CLK		0x00040000UL    /*!< Force TPU clock ON */
#endif
#define RGXMKIF_RENDERFLAGS_SPMSCRATCHBUFFER	0x00080000UL	/*!< Partial render must write to scratch buffer */
#if defined(RGX_FEATURE_PAIRED_TILES)
#define RGXMKIF_RENDERFLAGS_PAIRED_TILES		0x00100000UL	/*!< Render uses paired tile feature, empty tiles must always be enabled */
#endif
#if defined(FIX_HW_BRN_65101)
#define RGXMKIF_RENDERFLAGS_BRN65101			0x00200000UL	/*!< Render needs blit at end to work around BRN65101 */
#define RGXKMIF_RENDERFLAGS_RENDER_STRIPS_IF_BGOBJ	0x00400000UL	/*!< FW must split render into strips of tiles */
#define RGXKMIF_RENDERFLAGS_HORIZONTAL_STRIPS	0x00800000UL	/*!< Strips of tiles are horizontal */
#endif
#if defined(SUPPORT_STRIP_RENDERING)
#define RGXMKIF_RENDERFLAGS_FRAME_STRIP			0x01000000UL	/*!< Render is a frame strip and should communicate with display via GPIO */
#endif
#if defined(FIX_HW_BRN_67182)
#define RGXMKIF_RENDERFLAGS_BRN67182			0x02000000UL	/*!< Partial renders need be split into odd/even tiles to work around BRN65101 */
#endif
#define RGXMKIF_RENDERFLAGS_PREVENT_CDM_OVERLAP	0x04000000UL	/*!< Disallow compute overlapped with this render */
#if defined(FIX_HW_BRN_67001)
#define RGXKMIF_RENDERFLAGS_PREVENT_TDM_OVERLAP	0x08000000UL	/*!< Disallow TDM overlapped with this render */
#endif
#if defined(FIX_HW_BRN_43125) || defined(FIX_HW_BRN_43169) || defined(FIX_HW_BRN_43728) || defined(FIX_HW_BRN_62572)
#define RGXMKIF_RENDERFLAGS_PBE_DISABLE_EDGEOPT	0x10000000UL	/*!< Disallow PBE edge optimisation */
#endif
#if defined(FIX_HW_BRN_69359)
#define RGXMKIF_RENDERFLAGS_DISABLE_MADD_CACHE 	0x20000000UL	/*!< Disable the MADD L0 cache. */
#endif /* defined(FIX_HW_BRN_69359) */
/*
	The host must indicate if this is the first and/or last command to
	be issued for the specified task
*/
#define RGXMKIF_TAFLAGS_FIRSTKICK				0x00000001UL
#define RGXMKIF_TAFLAGS_LASTKICK				0x00000002UL

#define RGXMKIF_TAFLAGS_FLIP_SAMPLE_POSITIONS	0x00000004UL
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_TAFLAGS_SINGLE_CORE				0x00000008UL /*!< Use single core in a multi core setup. */
#endif

/*
 * 	Indicates the particular TA needs to be aborted
 */
#define RGXMKIF_TAFLAGS_TA_ABORT			0x00000100UL

#define RGXMKIF_TAFLAGS_SECURE					0x00080000UL

#if defined(FIX_HW_BRN_67349) || defined(FIX_HW_BRN_70353)
/*
	Indicates that the VCE needs to be halted and restarted as per the WA
*/
#define RGXMKIF_TAFLAGS_APPLY_VCE_PAUSE			0x00100000UL
#endif
#if defined(FIX_HW_BRN_51537)
/*
 * 	Indicates that the CSRM should be reconfigured to support maximum coeff
 *  space before this command is scheduled.
 */
#define RGXMKIF_TAFLAGS_CSRM_MAX_COEFFS			0x00200000UL
#endif

#if defined(FIX_HW_BRN_59998)
#define RGXMKIF_TAFLAGS_FORCE_TPU_CLK			0x00400000UL    /*!< Force TPU clock ON */
#endif

#if defined (FIX_HW_BRN_61484) || defined(FIX_HW_BRN_66333)
#define RGXMKIF_TAFLAGS_BYPASS_BRN61484_BRN66333 0x00800000UL
#endif

#if defined(FIX_HW_BRN_69359)
#define RGXMKIF_TAFLAGS_DISABLE_MADD_CACHE		0x01000000UL	/*!< Disable the MADD L0 cache. */
#endif /* defined(FIX_HW_BRN_69359) */

#define RGXMKIF_TAFLAGS_PHR_TRIGGER				0x02000000UL

/* flags for transfer queue commands */
#define RGXMKIF_CMDTRANSFER_FLAG_SECURE			0x00000001U
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_CMDTRANSFER_SINGLE_CORE			0x00000002U /*!< Use single core in a multi core setup. */
#endif

#if defined(RGX_FEATURE_TLA)
/* flags for 2D commands */
#define RGXMKIF_CMD2D_FLAG_SECURE				0x00000001U
#define RGXMKIF_CMD2D_FLAG_SRC_FBCDC			0x00000002U
#define RGXMKIF_CMD2D_FLAG_DST_FBCDC			0x00000004U
#endif

#if defined(RGX_FEATURE_FASTRENDER_DM)
#define RGXMKIF_CMDTDM_FLAG_SECURE              0x00000001U
#define RGXMKIF_CMDTDM_FLAG_SKIP_FLUSH_INVAL    0x00000002U
#if defined(RGX_FEATURE_TDM_PAIRED_TILES)
#define RGXMKIF_CMDTDM_PAIRED_TILES				0x00000004UL
#define RGXMKIF_CMDTDM_PAIRED_TILES_VERT			0x00000008UL
#define RGXMKIF_CMDTDM_PAIRED_TILES_SHIFT			2U
#define RGXMKIF_CMDTDM_PAIRED_TILES_MASK			0x0000000CUL
#endif
#if defined(FIX_HW_BRN_67001)
#define RGXMKIF_CMDTDM_PREVENT_3D_OVERLAP		0x00000010UL
#endif
#define RGXMKIF_CMDTDM_FLAG_TDMCB_WRAP			0x00000020UL
#if defined(SUPPORT_TRP)
#define RGXMKIF_CMDTDM_FLAG_TRP					0x00000040UL
#endif
#if defined(RGX_FEATURE_GPU_MULTICORE_SUPPORT)
#define RGXMKIF_CMDTDM_FLAG_SINGLE_CORE			0x00000080UL /*!< Use single core in a multi core setup. */
#endif
#endif

#define RGXMKIF_CMD3DTQ_SLICE_WIDTH_MASK        0x00000038UL
#define RGXMKIF_CMD3DTQ_SLICE_WIDTH_SHIFT       (3)
#define RGXMKIF_CMD3DTQ_SLICE_GRANULARITY       (0x10U)

/* flags for compute commands */
#define RGXMKIF_COMPUTE_FLAG_SECURE							0x00000001U
#define RGXMKIF_COMPUTE_FLAG_PREVENT_ALL_OVERLAP			0x00000002U
#define RGXMKIF_COMPUTE_FLAG_FORCE_TPU_CLK					0x00000004U
#if (RGX_FEATURE_CDM_CONTROL_STREAM_FORMAT == 2)
#define RGXMKIF_COMPUTE_FLAG_EXPLICIT_OFFSET				0x00000008U
#endif
#if defined(RGX_SUPPORT_LIMITED_COMPUTE_OVERLAP_WITH_BARRIERS)
#define RGXMKIF_COMPUTE_FLAG_PREVENT_ALL_NON_TAOOM_OVERLAP	0x00000010U
#endif
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
	IMG_UINT64	uTAReg_VDM_CTRL_STREAM_BASE;
	IMG_UINT64	uTAReg_TPU_BORDER_COLOUR_TABLE;

#if defined(RGX_FEATURE_VDM_DRAWINDIRECT)
	IMG_UINT64	uTAReg_VDM_DRAW_INDIRECT0;
	IMG_UINT32	uTAReg_VDM_DRAW_INDIRECT1;
#endif

	IMG_UINT32	uTAReg_PPP_CTRL;
	IMG_UINT32	uTAReg_TE_PSG;
	IMG_UINT32	uTAReg_TPU;

	IMG_UINT32	uTAReg_VDM_CONTEXT_RESUME_TASK0_SIZE;
#if defined(RGX_FEATURE_VDM_OBJECT_LEVEL_LLS)
	IMG_UINT32	uTAReg_VDM_CONTEXT_RESUME_TASK3_SIZE;
#endif

#if defined(FIX_HW_BRN_56279) || defined(FIX_HW_BRN_67381)
	IMG_UINT32	uTAReg_PDS_CTRL;
#endif

	IMG_UINT32	uTAViewIndex;

#if defined(RGX_FEATURE_TESSELLATION)
	IMG_UINT32	uTAReg_PDS_COEFF_FREE_PROG;
#endif

} RGXFWIF_TAREGISTERS;

#if defined(FIX_HW_BRN_44455) || defined(FIX_HW_BRN_63027)
/*!
	Configuration registers which need to be loaded by the firmware before the
	dummy region header is issued.
*/
typedef struct
{
	IMG_UINT64	uTAReg_TE_PSGREGION_ADDR;
} RGXFWIF_DUMMY_RGNHDR_INIT_TAREGISTERS;
#endif

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
	IMG_UINT32					RGXFW_ALIGN ui32FrameNum;/*!< associated frame number */
	RGXFWIF_UFO					sPartialRenderTA3DFence; /* Holds the TA/3D fence value to allow the 3D partial render command to go through */

#if defined(FIX_HW_BRN_44455) || defined(FIX_HW_BRN_63027)
	RGXFWIF_DUMMY_RGNHDR_INIT_TAREGISTERS	RGXFW_ALIGN sDummyRgnHdrInitTARegs;/*!< TA registers for Dummy RgnHdr Init workaround */
#endif

#if defined(FIX_HW_BRN_61484) || defined(FIX_HW_BRN_66333)
	IMG_UINT32					ui32BRN61484_66333LiveRT;
#endif

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
	IMG_UINT32	u3DReg_USC_PIXEL_OUTPUT_CTRL;
	IMG_UINT32  au3DReg_USC_CLEAR_REGISTER[RGX_MAXIMUM_OUTPUT_REGISTERS_PER_PIXEL];

	IMG_UINT32	u3DReg_ISP_BGOBJDEPTH;
	IMG_UINT32	u3DReg_ISP_BGOBJVALS;
    IMG_UINT32  u3DReg_ISP_AA;
#if defined(RGX_FEATURE_S7_TOP_INFRASTRUCTURE)
    IMG_UINT32  u3DReg_ISP_XTP_PIPE_ENABLE;
#endif
    IMG_UINT32  u3DReg_ISP_CTL;

	IMG_UINT32	u3DReg_TPU;

	IMG_UINT32	u3DReg_EVENT_PIXEL_PDS_INFO;

#if defined(RGX_FEATURE_CLUSTER_GROUPING)
	IMG_UINT32  u3DReg_PIXEL_PHANTOM;
#endif

	IMG_UINT32	u3DViewIndex;

	IMG_UINT32	u3DReg_EVENT_PIXEL_PDS_DATA;
#if defined(FIX_HW_BRN_65101)
	IMG_UINT32	u3DReg_BRN65101_EVENT_PIXEL_PDS_DATA;
#endif
#if defined(MULTIBUFFER_OCLQRY)
	IMG_UINT32	u3DReg_ISP_OCLQRY_STRIDE;
#endif

	/* All values below the RGXFW_ALIGN must be 64 bit */
	IMG_UINT64	RGXFW_ALIGN u3DReg_ISP_SCISSOR_BASE;
	IMG_UINT64	u3DReg_ISP_DBIAS_BASE;
    IMG_UINT64  u3DReg_ISP_OCLQRY_BASE;
    IMG_UINT64  u3DReg_ISP_ZLSCTL;
    IMG_UINT64  u3DReg_ISP_ZLOAD_STORE_BASE;
    IMG_UINT64  u3DReg_ISP_STENCIL_LOAD_STORE_BASE;
#if defined(RGX_FEATURE_ZLS_SUBTILE)
    IMG_UINT64  u3DReg_ISP_ZLS_PIXELS;
#endif

#if defined(RGX_HW_REQUIRES_FB_CDC_ZLS_SETUP)
    IMG_UINT64  u3DReg_FB_CDC_ZLS;
#endif

	IMG_UINT64	au3DReg_PBE_WORD[8][RGX_PBE_WORDS_REQUIRED_FOR_RENDERS];
	IMG_UINT64	u3DReg_TPU_BORDER_COLOUR_TABLE;
	IMG_UINT64	au3DReg_PDS_BGND[3];
#if defined(FIX_HW_BRN_65101)
	IMG_UINT64	au3DReg_PDS_BGND_BRN65101[3];
#endif
	IMG_UINT64	au3DReg_PDS_PR_BGND[3];
#if defined(RGX_FEATURE_ISP_ZLS_D24_S8_PACKING_OGL_MODE)
	IMG_UINT64	u3DReg_RGX_CR_BLACKPEARL_FIX;
#endif
#if defined(FIX_HW_BRN_62850) || defined(FIX_HW_BRN_62865)
    IMG_UINT64  u3DReg_ISP_DUMMY_STENCIL_STORE_BASE;
#endif
#if defined(FIX_HW_BRN_66193)
    IMG_UINT64  u3DReg_ISP_DUMMY_DEPTH_STORE_BASE;
#endif
#if defined(FIX_HW_BRN_67182)
	IMG_UINT32	u3DReg_RgnHeaderSingleRTSize;
	IMG_UINT32	u3DReg_RgnHeaderScratchOffset;
#endif
} RGXFWIF_3DREGISTERS;

typedef struct
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
	/* All 32 bit values should be added in the top section. This then requires only a
	 * single RGXFW_ALIGN to align all the 64 bit values in the second section */
	IMG_UINT32 uTransReg_ISP_BGOBJVALS;

	IMG_UINT32 uTransReg_USC_PIXEL_OUTPUT_CTRL;
	IMG_UINT32 uTransReg_USC_CLEAR_REGISTER0;
	IMG_UINT32 uTransReg_USC_CLEAR_REGISTER1;
	IMG_UINT32 uTransReg_USC_CLEAR_REGISTER2;
	IMG_UINT32 uTransReg_USC_CLEAR_REGISTER3;

	IMG_UINT32 uTransReg_ISP_MTILE_SIZE;
	IMG_UINT32 uTransReg_ISP_RENDER_ORIGIN;
	IMG_UINT32 uTransReg_ISP_CTL;

#if defined(RGX_FEATURE_S7_TOP_INFRASTRUCTURE)
    IMG_UINT32 uTransReg_ISP_XTP_PIPE_ENABLE;
#endif
	IMG_UINT32 uTransReg_ISP_AA;

	IMG_UINT32 uTransReg_EVENT_PIXEL_PDS_INFO;

	IMG_UINT32 uTransReg_EVENT_PIXEL_PDS_CODE;
	IMG_UINT32 uTransReg_EVENT_PIXEL_PDS_DATA;

	IMG_UINT32 uTransReg_ISP_RENDER;
	IMG_UINT32 uTransReg_ISP_RGN;

	/* All values below the RGXFW_ALIGN must be 64 bit */
	IMG_UINT64 RGXFW_ALIGN uTransReg_PDS_BGND0_BASE;
	IMG_UINT64 uTransReg_PDS_BGND1_BASE;
	IMG_UINT64 uTransReg_PDS_BGND3_SIZEINFO;

	IMG_UINT64 uTransReg_ISP_MTILE_BASE;

	IMG_UINT64 uTransReg_PBE_WORDX_MRTY[3 * RGX_PBE_WORDS_REQUIRED_FOR_TQS]; /* TQ_MAX_RENDER_TARGETS * PBE_STATE_SIZE */

} RGXFWIF_TRANSFERREGISTERS;

typedef struct
{
	RGXFWIF_TRANSFERREGISTERS			RGXFW_ALIGN sTransRegs;

	IMG_UINT32				ui32FrameNum;
	IMG_UINT32				ui32Flags;/*!< flags */

} RGXFWIF_CMDTRANSFER;

static_assert(sizeof(RGXFWIF_CMDTRANSFER) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current TRANSFER command size");

#if defined(RGX_FEATURE_FASTRENDER_DM)

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

} RGXFWIF_TDMREGISTERS, *PRGXFWIF_TDMREGISTERS;


typedef struct
{
	RGXFWIF_TDMREGISTERS			RGXFW_ALIGN sTDMRegs;

	IMG_UINT32				RGXFW_ALIGN ui32FrameNum;
	IMG_UINT32				ui32Flags;/*!< flags */
	IMG_UINT32				ui32StreamStartOffset;	/*!< Stream starting point within the circular buffer */

} RGXFWIF_CMDTDM, *PRGXFWIF_CMDTDM;

static_assert(sizeof(RGXFWIF_CMDTDM) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current TDM command size");

#endif /* RGX_FEATURE_FASTRENDER_DM */


#if defined(RGX_FEATURE_TLA)

typedef struct
{
	IMG_UINT64	u2DReg_TLA_CMD_STREAM;
	IMG_UINT64	u2DReg_TLA_FBC_MEM_SRC_REGION;
	IMG_UINT64	u2DReg_TLA_FBC_MEM_SRC_CTRL;
	IMG_UINT64	u2DReg_TLA_FBC_MEM_DST_REGION;
	IMG_UINT64	u2DReg_TLA_FBC_MEM_DST_CTRL;
#if defined(FIX_HW_BRN_57193)
	IMG_UINT64	u2DReg_BRN57193_TLA_CMD_STREAM;
#endif
} RGXFWIF_2DREGISTERS;

typedef struct
{
	RGXFWIF_2DREGISTERS		RGXFW_ALIGN s2DRegs;

	IMG_UINT32 				ui32FrameNum;
	IMG_UINT32				ui32Flags;/*!< flags */

} RGXFWIF_CMD2D;

static_assert(sizeof(RGXFWIF_CMD2D) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current 2D command size");

#endif /* RGX_FEATURE_TLA */

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

#if defined(RGX_FEATURE_COMPUTE_MORTON_CAPABLE)
	IMG_UINT64	uCDMReg_CDM_ITEM;
#endif
#if defined(RGX_FEATURE_CLUSTER_GROUPING)
	IMG_UINT64  uCDMReg_COMPUTE_CLUSTER;
#endif

#if defined(RGX_FEATURE_TPU_DM_GLOBAL_REGISTERS)
	IMG_UINT64 uCDMReg_TPU_TAG_CDM_CTRL;
#endif
#if RGX_FEATURE_CDM_CONTROL_STREAM_FORMAT == 2
	IMG_UINT64	uCDMReg_CDM_CB_QUEUE;
#if defined(SUPPORT_TRUSTED_DEVICE)
	IMG_UINT64	uCDMReg_CDM_CB_SECURE_QUEUE;
#endif
	IMG_UINT64	uCDMReg_CDM_CB_BASE;
	IMG_UINT64	uCDMReg_CDM_CB;
#else
	IMG_UINT64	uCDMReg_CDM_CTRL_STREAM_BASE;
#endif

	IMG_UINT32	uCDMReg_TPU;

	IMG_UINT32	uCDMReg_CDM_RESUME_PDS1;
} RGXFWIF_CDM_REGISTERS;

/*!
	RGX Compute command.
*/
typedef struct
{
	RGXFWIF_CDM_REGISTERS			RGXFW_ALIGN sCDMRegs;				/*!< CDM registers */
	IMG_UINT32						RGXFW_ALIGN ui32Flags;				/*!< Control flags */
#if RGX_FEATURE_CDM_CONTROL_STREAM_FORMAT == 2
	IMG_UINT64						RGXFW_ALIGN ui64StreamStartOffset;	/*!< Stream starting point within the circular buffer */
#endif
	IMG_UINT32						ui32FrameNum;						/*!< Frame number */
} RGXFWIF_CMD_COMPUTE;

static_assert(sizeof(RGXFWIF_CMD_COMPUTE) <= RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE,
              	  	  	 "kernel expects command size be increased to match current COMPUTE command size");


/*
 * Minimum/Maximum PB size supported by RGX and Services.
 *
 * Base page size is dependent on core:
 *   S6/S6XT/S7               = 50 pages
 *   S8XE                     = 40 pages
 *   S8XE with BRN66011 fixed = 25 pages
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
#if defined(RGX_FEATURE_ROGUEXE)
#if defined(FIX_HW_BRN_66011)
#define RGX_PM_BASE_MINIMUM_PAGES           (40U)
#else
#define RGX_PM_BASE_MINIMUM_PAGES           (25U)
#endif
#else
#define RGX_PM_BASE_MINIMUM_PAGES           (50U)
#endif

#if defined(RGX_FEATURE_SCALABLE_TE_ARCH)
#define RGX_PM_FLPAGES_FOR_SCALABLE_TE      ((RGX_FEATURE_SCALABLE_TE_ARCH-1U)*4U)
#else
#define RGX_PM_FLPAGES_FOR_SCALABLE_TE      (0U*4U)
#endif

#if defined(RGX_FEATURE_SCALABLE_VCE)
#define RGX_PM_FLPAGES_FOR_SCALABLE_VCE     ((RGX_FEATURE_SCALABLE_VCE-1U)*16U)
#else
#define RGX_PM_FLPAGES_FOR_SCALABLE_VCE     (0U*16U)
#endif

#if defined(HW_ERN_46066)
/* With PM Pre-Alloc each requester can take an additional page, for VCE2 + TE2 config, it is 64KB. */
#if defined(RGX_FEATURE_SCALABLE_TE_ARCH)
#define RGX_PM_FLPAGES_FOR_TE_PREALLOC      (RGX_FEATURE_SCALABLE_TE_ARCH*4U)
#else
#define RGX_PM_FLPAGES_FOR_TE_PREALLOC      (1U*4U)
#endif
#if defined(RGX_FEATURE_SCALABLE_VCE)
#define RGX_PM_FLPAGES_FOR_VCE_PREALLOC     (RGX_FEATURE_SCALABLE_VCE*4U)
#else
#define RGX_PM_FLPAGES_FOR_VCE_PREALLOC     (1U*4U)
#endif
#else
#define RGX_PM_FLPAGES_FOR_TE_PREALLOC      (0U*4U)
#define RGX_PM_FLPAGES_FOR_VCE_PREALLOC     (0U*4U)
#endif

#define RGX_PM_MIN_FLSIZE                   ((RGX_PM_FLPAGES_FOR_SCALABLE_TE  +    \
                                              RGX_PM_FLPAGES_FOR_SCALABLE_VCE +    \
                                              RGX_PM_FLPAGES_FOR_TE_PREALLOC  +    \
                                              RGX_PM_FLPAGES_FOR_VCE_PREALLOC +    \
                                              RGX_PM_BASE_MINIMUM_PAGES) * RGX_BIF_PM_PHYSICAL_PAGE_SIZE)

#define RGX_PM_MAX_FLSIZE                   IMG_UINT64_C(2U*1024U*1024U*1024) /* 2GB */


/* Applied to RGX_CR_VDM_SYNC_PDS_DATA_BASE */
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_VDM_SYNC_OFFSET_BYTES	0
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_VDM_SYNC_MAX_SIZE_BYTES	128

/* Applied to RGX_CR_EVENT_PIXEL_PDS_CODE */
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_EOT_OFFSET_BYTES			128
#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_EOT_MAX_SIZE_BYTES		128

#define RGXFWIF_HEAP_FIXED_OFFSET_PDS_HEAP_TOTAL_BYTES				4096


/* Pointed to by PDS code at RGX_CR_VDM_SYNC_PDS_DATA_BASE */
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_VDM_SYNC_OFFSET_BYTES	0
#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_VDM_SYNC_MAX_SIZE_BYTES	128

#define RGXFWIF_HEAP_FIXED_OFFSET_USC_HEAP_TOTAL_BYTES				4096


/* Applied to RGX_CR_MCU_FENCE (!defined(RGX_FEATURE_SLC_VIVT)), and RGX_CR_PM_MTILE_ARRAY (defined(RGX_FEATURE_SIMPLE_INTERNAL_PARAMETER_FORMAT))*/
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_FENCE_OFFSET_BYTES	0U
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_FENCE_MAX_SIZE_BYTES	128

/* Applied to RGX_CR_TPU_YUV_CSC_COEFFICIENTS */
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_YUV_CSC_OFFSET_BYTES		128U
#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_YUV_CSC_MAX_SIZE_BYTES	1024

#define RGXFWIF_HEAP_FIXED_OFFSET_GENERAL_HEAP_TOTAL_BYTES			4096

#endif /*  RGX_FWIF_CLIENT_H */

/******************************************************************************
 End of file (rgx_fwif_client.h)
******************************************************************************/
