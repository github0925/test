/**************************************************************************/ /*!
@File
@Title          RGX memory allocation flags
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGX_MEMALLOCFLAGS_H
#define RGX_MEMALLOCFLAGS_H

#define PMMETA_PROTECT          (1U << 0)      /* Memory that only the PM and Meta can access */
#define FIRMWARE_CACHED         (1U << 1)      /* Memory that is cached in META/MIPS */

#endif
