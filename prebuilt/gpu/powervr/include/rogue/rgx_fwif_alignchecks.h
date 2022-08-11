/*************************************************************************/ /*!
@File
@Title          RGX fw interface alignment checks
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Checks to avoid disalignment in RGX fw data structures
                shared with the host
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(RGX_FWIF_ALIGNCHECKS_H)
#define RGX_FWIF_ALIGNCHECKS_H

/* for the offsetof macro */
#if defined(__KERNEL__) && defined(LINUX)
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

/*!
 ******************************************************************************
 * Alignment UM/FW checks array
 *****************************************************************************/

#define RGXFW_ALIGN_CHECKS_UM_MAX 128U

#define RGXFW_ALIGN_CHECKS_INIT0								\
		sizeof(RGXFWIF_TRACEBUF),								\
		offsetof(RGXFWIF_TRACEBUF, ui32LogType),				\
		offsetof(RGXFWIF_TRACEBUF, sTraceBuf),					\
		offsetof(RGXFWIF_TRACEBUF, ui32TraceBufSizeInDWords),	\
		offsetof(RGXFWIF_TRACEBUF, ui32TracebufFlags),			\
																\
		sizeof(RGXFWIF_SYSDATA),								\
		offsetof(RGXFWIF_SYSDATA, ePowState),					\
		offsetof(RGXFWIF_SYSDATA, ui32HWPerfDropCount),			\
		offsetof(RGXFWIF_SYSDATA, ui32LastDropOrdinal),			\
		offsetof(RGXFWIF_SYSDATA, ui32FWFaults),				\
		offsetof(RGXFWIF_SYSDATA, ui32HWRStateFlags),			\
																\
		sizeof(RGXFWIF_OSDATA),									\
		offsetof(RGXFWIF_OSDATA, ui32HostSyncCheckMark),		\
		offsetof(RGXFWIF_OSDATA, ui32KCCBCmdsExecuted),			\
																	\
		sizeof(RGXFWIF_HWRINFOBUF),									\
		offsetof(RGXFWIF_HWRINFOBUF, aui32HwrDmLockedUpCount),		\
		offsetof(RGXFWIF_HWRINFOBUF, aui32HwrDmOverranCount),		\
		offsetof(RGXFWIF_HWRINFOBUF, aui32HwrDmRecoveredCount),		\
		offsetof(RGXFWIF_HWRINFOBUF, aui32HwrDmFalseDetectCount),	\
																	\
		/* RGXFWIF_CMDTA checks */						\
		sizeof(RGXFWIF_CMDTA),							\
		offsetof(RGXFWIF_CMDTA, sGeomRegs),				\
														\
		/* RGXFWIF_CMD3D checks */						\
		sizeof(RGXFWIF_CMD3D),							\
		offsetof(RGXFWIF_CMD3D, s3DRegs),				\
														\
		/* RGXFWIF_CMDTRANSFER checks */				\
		sizeof(RGXFWIF_CMDTRANSFER),					\
		offsetof(RGXFWIF_CMDTRANSFER, sTransRegs),		\
														\
														\
		/* RGXFWIF_CMD_COMPUTE checks */				\
		sizeof(RGXFWIF_CMD_COMPUTE),					\
		offsetof(RGXFWIF_CMD_COMPUTE, sCDMRegs),		\
														\
		/* RGXFWIF_FREELIST checks */					\
		sizeof(RGXFWIF_FREELIST),						\
		offsetof(RGXFWIF_FREELIST, psFreeListDevVAddr),	\
		offsetof(RGXFWIF_FREELIST, ui32MaxPages),		\
		offsetof(RGXFWIF_FREELIST, ui32CurrentPages),	\
														\
		/* RGXFWIF_HWRTDATA checks */						\
		sizeof(RGXFWIF_HWRTDATA),							\
		offsetof(RGXFWIF_HWRTDATA, psVHeapTableDevVAddr),	\
		offsetof(RGXFWIF_HWRTDATA, psPMMListDevVAddr),		\
		offsetof(RGXFWIF_HWRTDATA, apsFreeLists),			\
		offsetof(RGXFWIF_HWRTDATA, ui64VCECatBase),			\
		offsetof(RGXFWIF_HWRTDATA, eState),					\
															\
		/* RGXFWIF_HWRTDATA_COMMON checks */					\
		sizeof(RGXFWIF_HWRTDATA_COMMON),						\
		offsetof(RGXFWIF_HWRTDATA_COMMON, bTACachesNeedZeroing),\
																\
		/* RGXFWIF_HWPERF_CTL_BLK checks */					\
		sizeof(RGXFWIF_HWPERF_CTL_BLK),						\
		offsetof(RGXFWIF_HWPERF_CTL_BLK, aui64CounterCfg),	\
															\
		/* RGXFWIF_HWPERF_CTL checks */			\
		sizeof(RGXFWIF_HWPERF_CTL),				\
		offsetof(RGXFWIF_HWPERF_CTL, SelCntr)

#if defined(RGX_FEATURE_TLA)
#define RGXFW_ALIGN_CHECKS_INIT1                   \
		RGXFW_ALIGN_CHECKS_INIT0,                  \
		/* RGXFWIF_CMD2D checks */                 \
		sizeof(RGXFWIF_CMD2D),                     \
		offsetof(RGXFWIF_CMD2D, s2DRegs)
#else
#define RGXFW_ALIGN_CHECKS_INIT1		RGXFW_ALIGN_CHECKS_INIT0
#endif	/* RGX_FEATURE_TLA */


#if defined(RGX_FEATURE_FASTRENDER_DM)
#define RGXFW_ALIGN_CHECKS_INIT                    \
		RGXFW_ALIGN_CHECKS_INIT1,                  \
		/* RGXFWIF_CMDTDM checks */                \
		sizeof(RGXFWIF_CMDTDM),                    \
		offsetof(RGXFWIF_CMDTDM, sTDMRegs)
#else
#define RGXFW_ALIGN_CHECKS_INIT		RGXFW_ALIGN_CHECKS_INIT1
#endif /* ! RGX_FEATURE_FASTRENDER_DM */



/*!
 ******************************************************************************
 * Alignment KM checks array
 *****************************************************************************/

#define RGXFW_ALIGN_CHECKS_INIT_KM                                   \
		sizeof(RGXFWIF_SYSINIT),                                     \
		offsetof(RGXFWIF_SYSINIT, sFaultPhysAddr),                   \
		offsetof(RGXFWIF_SYSINIT, sPDSExecBase),                     \
		offsetof(RGXFWIF_SYSINIT, sUSCExecBase),                     \
		offsetof(RGXFWIF_SYSINIT, asSigBufCtl),                      \
		offsetof(RGXFWIF_SYSINIT, sTraceBufCtl),                     \
		offsetof(RGXFWIF_SYSINIT, sFwSysData),                       \
		sizeof(RGXFWIF_OSINIT),                                      \
		offsetof(RGXFWIF_OSINIT, psKernelCCBCtl),                    \
		offsetof(RGXFWIF_OSINIT, psKernelCCB),                       \
		offsetof(RGXFWIF_OSINIT, psFirmwareCCBCtl),                  \
		offsetof(RGXFWIF_OSINIT, psFirmwareCCB),                     \
		offsetof(RGXFWIF_OSINIT, sFwOsData),                         \
		offsetof(RGXFWIF_OSINIT, sRGXCompChecks),                    \
		                                                             \
		/* RGXFWIF_FWRENDERCONTEXT checks */                         \
		sizeof(RGXFWIF_FWRENDERCONTEXT),                             \
		offsetof(RGXFWIF_FWRENDERCONTEXT, sTAContext),               \
		offsetof(RGXFWIF_FWRENDERCONTEXT, s3DContext),               \
		                                                             \
		sizeof(RGXFWIF_FWCOMMONCONTEXT),                             \
		offsetof(RGXFWIF_FWCOMMONCONTEXT, psFWMemContext),           \
		offsetof(RGXFWIF_FWCOMMONCONTEXT, sRunNode),                 \
		offsetof(RGXFWIF_FWCOMMONCONTEXT, psCCB),                    \
		                                                             \
		sizeof(RGXFWIF_MMUCACHEDATA),                                \
		offsetof(RGXFWIF_MMUCACHEDATA, ui32Flags),                   \
		offsetof(RGXFWIF_MMUCACHEDATA, sMMUCacheSync),               \
		offsetof(RGXFWIF_MMUCACHEDATA, ui32MMUCacheSyncUpdateValue)

#endif /* RGX_FWIF_ALIGNCHECKS_H */

/******************************************************************************
 End of file (rgx_fwif_alignchecks.h)
******************************************************************************/
