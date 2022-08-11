/*************************************************************************/ /*!
@Title          Information about the uses of USC global registers.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXGLOBALREGDEFS_H
#define RGXGLOBALREGDEFS_H

typedef enum _RGX_USC_GLOBALREGS
{
	#define DCL(X)		X,
	#include "rgxglobalregdefs_internal.h"
	#undef DCL
} RGX_USC_GLOBALREGS, *PRGX_USC_GLOBALREGS;

#if defined(RGX_USCINST_REGBANK_SIZE_GLOBAL)

/*
  Check enough registers are available.
*/
static_assert(RGX_USC_GLOBALREGS_COUNT <= RGX_USCINST_REGBANK_SIZE_GLOBAL, "Not enough global registers available.");

#endif /* defined(RGX_USCINST_REGBANK_SIZE_GLOBAL) */

/*
  Maps a global register ID to the config register for the firmware to read/write the global register's value.
*/
#define RGX_CR_USC_GN(N) (RGX_CR_USC_G0 + ((RGX_CR_USC_G1 - RGX_CR_USC_G0) * (N)))

#endif /* RGXGLOBALREGDEFS_H */
