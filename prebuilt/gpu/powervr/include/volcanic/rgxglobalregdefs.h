/*************************************************************************/ /*!
@Title          Information about the uses of USC global registers.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXGLOBALREGDEFS_H
#define RGXGLOBALREGDEFS_H

#if (RGX_FEATURE_USC_NUM_GLOBAL_SPECIAL_REGISTERS > 0)

typedef enum _RGX_USC_GLOBALREGS
{
	#define DCL(X)		X,
	#include "rgxglobalregdefs_internal.h"
	#undef DCL
} RGX_USC_GLOBALREGS, *PRGX_USC_GLOBALREGS;

/*
  Check enough registers are available.
*/
static_assert(RGX_USC_GLOBALREGS_COUNT <= RGX_FEATURE_USC_NUM_GLOBAL_SPECIAL_REGISTERS, "Not enough global registers available.");

/*
  Maps a global register ID to the config register for the firmware to read/write the global register's value.
*/
#define RGX_CR_USC_GN(DM,N)     (RGX_CR_RGX_CR_USC_GLOBAL_SPECIAL_REGISTER_ ## DM ## 0 + \
	(RGX_CR_RGX_CR_USC_GLOBAL_SPECIAL_REGISTER_ ## DM ## 1 - RGX_CR_RGX_CR_USC_GLOBAL_SPECIAL_REGISTER_ ## DM ## 0) * (N))

#endif /* (RGX_FEATURE_USC_NUM_GLOBAL_SPECIAL_REGISTERS > 0) */
#endif /* RGXGLOBALREGDEFS_H */
