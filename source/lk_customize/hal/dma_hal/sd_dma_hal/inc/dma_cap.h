//*****************************************************************************
//
// dma_cap.h - Prototypes for the dma hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DMA_CAP_H__
#define __DMA_CAP_H__
#include "dma_hal.h"
#include <bits.h>
/*  This header file is for dev layer*/
typedef enum {

    DMA_MEM_CAP = BIT(0xffffffff, 0),
    DMA_PERI_CAP1 = BIT(0xffffffff, 1),
    DMA_PERI_CAP2 = BIT(0xffffffff, 2),
    /*    reserve for future dma controller's capabilities */
    DMA_PERI_CAP3 = BIT(0xffffffff, 3),
    DMA_PERI_CAP4 = BIT(0xffffffff, 4),
    DMA_PERI_CAP5 = BIT(0xffffffff, 5),
    DMA_PERI_CAP6 = BIT(0xffffffff, 6),
    DMA_PERI_CAP7 = BIT(0xffffffff, 7),
    DMA_PERI_CAP8 = BIT(0xffffffff, 8),
} DMA_XFER_CAP;

#endif
