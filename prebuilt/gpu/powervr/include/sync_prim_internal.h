/*************************************************************************/ /*!
@File
@Title          Services internal synchronisation typedef header
@Description    Defines synchronisation types that are used internally
                only
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef SYNC_INTERNAL_H
#define SYNC_INTERNAL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <powervr/mem_types.h>

/* These are included here as the typedefs are required
 * internally.
 */

typedef struct SYNC_PRIM_CONTEXT *PSYNC_PRIM_CONTEXT;
typedef struct PVRSRV_CLIENT_SYNC_PRIM
{
	volatile uint32_t __iomem *pui32LinAddr;	/*!< User pointer to the primitive */
} PVRSRV_CLIENT_SYNC_PRIM;

/*!
 * Bundled information for a sync prim operation
 *
 *   Structure: #PVRSRV_CLIENT_SYNC_PRIM_OP
 *   Typedef: ::PVRSRV_CLIENT_SYNC_PRIM_OP
 */
typedef struct PVRSRV_CLIENT_SYNC_PRIM_OP
{
	#define PVRSRV_CLIENT_SYNC_PRIM_OP_CHECK	(1U << 0)
	#define PVRSRV_CLIENT_SYNC_PRIM_OP_UPDATE	(1U << 1)
	#define PVRSRV_CLIENT_SYNC_PRIM_OP_UNFENCED_UPDATE (PVRSRV_CLIENT_SYNC_PRIM_OP_UPDATE | (1U<<2))
	uint32_t                    ui32Flags;       /*!< Operation flags: PVRSRV_CLIENT_SYNC_PRIM_OP_XXX */
	PVRSRV_CLIENT_SYNC_PRIM    *psSync;          /*!< Pointer to the client sync primitive */
	uint32_t                    ui32FenceValue;  /*!< The Fence value (only used if PVRSRV_CLIENT_SYNC_PRIM_OP_CHECK is set) */
	uint32_t                    ui32UpdateValue; /*!< The Update value (only used if PVRSRV_CLIENT_SYNC_PRIM_OP_UPDATE is set) */
} PVRSRV_CLIENT_SYNC_PRIM_OP;

#if defined(__cplusplus)
}
#endif
#endif /* SYNC_INTERNAL_H */
