/*************************************************************************/ /*!
@File
@Title          RGX firmware interface structures
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX firmware interface structures shared by both host client
                and host server
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(RGX_FWIF_SHARED_H)
#define RGX_FWIF_SHARED_H

#include "img_types.h"
#include "img_defs.h"
#include "rgx_common.h"
#include "powervr/mem_types.h"

/* Maximum number of UFOs in a CCB command.
 * The number is based on having 32 sync prims (as originally), plus 32 sync
 * checkpoints.
 * Once the use of sync prims is no longer supported, we will retain
 * the same total (64) as the number of sync checkpoints which may be
 * supporting a fence is not visible to the client driver and has to
 * allow for the number of different timelines involved in fence merges.
 */
#define RGXFWIF_CCB_CMD_MAX_UFOS			(32U+32U)

/*
 * This is a generic limit imposed on any DM (TA,3D,CDM,TDM,2D,TRANSFER)
 * command passed through the bridge.
 * Just across the bridge in the server, any incoming kick command size is
 * checked against this maximum limit.
 * In case the incoming command size is larger than the specified limit,
 * the bridge call is retired with error.
 */
#define RGXFWIF_DM_INDEPENDENT_KICK_CMD_SIZE	(1024U)

#define	RGXFWIF_PRBUFFER_START        IMG_UINT32_C(0)
#define	RGXFWIF_PRBUFFER_ZSBUFFER      IMG_UINT32_C(0)
#define	RGXFWIF_PRBUFFER_MSAABUFFER   IMG_UINT32_C(1)
#define	RGXFWIF_PRBUFFER_MAXSUPPORTED IMG_UINT32_C(2)

typedef struct RGXFWIF_DEV_VIRTADDR_
{
	IMG_UINT32	ui32Addr;
} RGXFWIF_DEV_VIRTADDR;

typedef struct
{
	IMG_DEV_VIRTADDR        RGXFW_ALIGN psDevVirtAddr;
	RGXFWIF_DEV_VIRTADDR    pbyFWAddr;
} UNCACHED_ALIGN RGXFWIF_DMA_ADDR;

typedef IMG_UINT8	RGXFWIF_CCCB;

typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_UFO_ADDR;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CLEANUP_CTL;


typedef struct
{
	PRGXFWIF_UFO_ADDR	puiAddrUFO;
	IMG_UINT32			ui32Value;
} RGXFWIF_UFO;

typedef struct
{
	IMG_UINT32			ui32SubmittedCommands;	/*!< Number of commands received by the FW */
	IMG_UINT32			ui32ExecutedCommands;	/*!< Number of commands executed by the FW */
} UNCACHED_ALIGN RGXFWIF_CLEANUP_CTL;

/*
 * TA and 3D commands require set of firmware addresses that are stored in the
 * Kernel. Client has handle(s) to Kernel containers storing these addresses,
 * instead of raw addresses. We have to patch/write these addresses in KM to
 * prevent UM from controlling FW addresses directly.
 * Typedefs for TA and 3D commands are shared between Client and Firmware (both
 * single-BVNC). Kernel is implemented in a multi-BVNC manner, so it can't use
 * TA|3D CMD type definitions directly. Therefore we have a SHARED block that
 * is shared between UM-KM-FW across all BVNC configurations.
 */
typedef struct
{
	RGXFWIF_DEV_VIRTADDR sHWRTData; /* RTData associated with this command,
									   this is used for context selection and for storing out HW-context,
									   when TA is switched out for continuing later */

	RGXFWIF_DEV_VIRTADDR asPRBuffer[RGXFWIF_PRBUFFER_MAXSUPPORTED];	/* Supported PR Buffers like Z/S/MSAA Scratch */

} CMDTA3D_SHARED;

/*!
 * Client Circular Command Buffer (CCCB) control structure.
 * This is shared between the Server and the Firmware and holds byte offsets
 * into the CCCB as well as the wrapping mask to aid wrap around. A given
 * snapshot of this queue with Cmd 1 running on the GPU might be:
 *
 *          Roff                           Doff                 Woff
 * [..........|-1----------|=2===|=3===|=4===|~5~~~~|~6~~~~|~7~~~~|..........]
 *            <      runnable commands       ><   !ready to run   >
 *
 * Cmd 1    : Currently executing on the GPU data master.
 * Cmd 2,3,4: Fence dependencies met, commands runnable.
 * Cmd 5... : Fence dependency not met yet.
 */
typedef struct
{
	IMG_UINT32  ui32WriteOffset;    /*!< Host write offset into CCB. This
	                                 *    must be aligned to 16 bytes. */
	IMG_UINT32  ui32ReadOffset;     /*!< Firmware read offset into CCB.
	                                      Points to the command that is
	                                 *    runnable on GPU, if R!=W */
	IMG_UINT32  ui32DepOffset;      /*!< Firmware fence dependency offset.
	                                 *    Points to commands not ready, i.e.
	                                 *    fence dependencies are not met. */
	IMG_UINT32  ui32WrapMask;       /*!< Offset wrapping mask, total capacity
	                                 *    in bytes of the CCB-1 */
} UNCACHED_ALIGN RGXFWIF_CCCB_CTL;


typedef IMG_UINT32 RGXFW_FREELIST_TYPE;

#define RGXFW_LOCAL_FREELIST     IMG_UINT32_C(0)
#define RGXFW_GLOBAL_FREELIST    IMG_UINT32_C(1)
#define RGXFW_FREELIST_TYPE_LAST RGXFW_GLOBAL_FREELIST
#define RGXFW_MAX_FREELISTS      (RGXFW_FREELIST_TYPE_LAST + 1U)


typedef struct
{
	IMG_UINT64	uTAReg_VDM_CONTEXT_STATE_BASE_ADDR;
	IMG_UINT64	uTAReg_VDM_CONTEXT_STATE_RESUME_ADDR;
	IMG_UINT64	uTAReg_TA_CONTEXT_STATE_BASE_ADDR;

	struct
	{
		IMG_UINT64	uTAReg_VDM_CONTEXT_STORE_TASK0;
		IMG_UINT64	uTAReg_VDM_CONTEXT_STORE_TASK1;
		IMG_UINT64	uTAReg_VDM_CONTEXT_STORE_TASK2;

		/* VDM resume state update controls */
		IMG_UINT64	uTAReg_VDM_CONTEXT_RESUME_TASK0;
		IMG_UINT64	uTAReg_VDM_CONTEXT_RESUME_TASK1;
		IMG_UINT64	uTAReg_VDM_CONTEXT_RESUME_TASK2;

		IMG_UINT64	uTAReg_VDM_CONTEXT_STORE_TASK3;
		IMG_UINT64	uTAReg_VDM_CONTEXT_STORE_TASK4;

		IMG_UINT64	uTAReg_VDM_CONTEXT_RESUME_TASK3;
		IMG_UINT64	uTAReg_VDM_CONTEXT_RESUME_TASK4;
	} asTAState[2];

} RGXFWIF_TAREGISTERS_CSWITCH;

#define RGXFWIF_TAREGISTERS_CSWITCH_SIZE sizeof(RGXFWIF_TAREGISTERS_CSWITCH)

typedef struct
{
	IMG_UINT64	uCDMReg_CDM_CONTEXT_STATE_BASE_ADDR;
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS0;
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS1;
	IMG_UINT64	uCDMReg_CDM_TERMINATE_PDS;
	IMG_UINT64	uCDMReg_CDM_TERMINATE_PDS1;

	/* CDM resume controls */
	IMG_UINT64	uCDMReg_CDM_RESUME_PDS0;
	IMG_UINT64	uCDMReg_CDM_CONTEXT_PDS0_B;
	IMG_UINT64	uCDMReg_CDM_RESUME_PDS0_B;

} RGXFWIF_CDM_REGISTERS_CSWITCH;

typedef struct
{
	RGXFWIF_TAREGISTERS_CSWITCH	RGXFW_ALIGN sCtxSwitch_Regs;	/*!< Geom registers for ctx switch */
} RGXFWIF_STATIC_RENDERCONTEXT_STATE;

#define RGXFWIF_STATIC_RENDERCONTEXT_SIZE sizeof(RGXFWIF_STATIC_RENDERCONTEXT_STATE)

typedef struct
{
	RGXFWIF_CDM_REGISTERS_CSWITCH	RGXFW_ALIGN sCtxSwitch_Regs;	/*!< CDM registers for ctx switch */
} RGXFWIF_STATIC_COMPUTECONTEXT_STATE;

#define RGXFWIF_STATIC_COMPUTECONTEXT_SIZE sizeof(RGXFWIF_STATIC_COMPUTECONTEXT_STATE)

typedef IMG_UINT32 RGXFWIF_PRBUFFER_TYPE;

typedef enum
{
	RGXFWIF_PRBUFFER_UNBACKED = 0,
	RGXFWIF_PRBUFFER_BACKED,
	RGXFWIF_PRBUFFER_BACKING_PENDING,
	RGXFWIF_PRBUFFER_UNBACKING_PENDING,
}RGXFWIF_PRBUFFER_STATE;

typedef struct
{
	IMG_UINT32				ui32BufferID;				/*!< Buffer ID*/
	IMG_BOOL				bOnDemand;					/*!< Needs On-demand Z/S/MSAA Buffer allocation */
	RGXFWIF_PRBUFFER_STATE	eState;						/*!< Z/S/MSAA -Buffer state */
	RGXFWIF_CLEANUP_CTL		sCleanupState;				/*!< Cleanup state */
	IMG_UINT32				ui32PRBufferFlags;			/*!< Compatibility and other flags */
} UNCACHED_ALIGN RGXFWIF_PRBUFFER;


#endif /*  RGX_FWIF_SHARED_H */

/******************************************************************************
 End of file (rgx_fwif_shared.h)
******************************************************************************/
