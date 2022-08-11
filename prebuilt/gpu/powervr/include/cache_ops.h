/*************************************************************************/ /*!
@File
@Title          Services cache management header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Defines for cache management which are visible internally
                and externally
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef CACHE_OPS_H
#define CACHE_OPS_H
#include "img_types.h"

#define CACHE_BATCH_MAX (8)
typedef IMG_UINT32 PVRSRV_CACHE_OP;				/*!< Type represents cache maintenance operation */
#define PVRSRV_CACHE_OP_NONE				0x0	/*!< No operation */
#define PVRSRV_CACHE_OP_CLEAN				0x1	/*!< Flush w/o invalidate */
#define PVRSRV_CACHE_OP_INVALIDATE			0x2	/*!< Invalidate w/o flush */
#define PVRSRV_CACHE_OP_FLUSH				0x3	/*!< Flush w/ invalidate */

#endif	/* CACHE_OPS_H */
