//*****************************************************************************
//
// dw_ssi_spi.h - Driver for the spi chipdev Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************


#ifndef __DW_SSI_SPI_H__
#define __DW_SSI_SPI_H__

#define DW_SSI_CTRLR0           0x0
#define DW_SSI_CTRLR1           0x4
#define DW_SSI_SSIENR           0x8
#define DW_SSI_MWCR             0xC
#define DW_SSI_SER              0x10
#define DW_SSI_BAUDR            0x14
#define DW_SSI_TXFTLR           0x18
#define DW_SSI_RXFTLR           0x1C
#define DW_SSI_TXFLR            0x20
#define DW_SSI_RXFLR            0x24
#define DW_SSI_SR               0x28
#define DW_SSI_IMR              0x2C
#define DW_SSI_ISR              0x30
#define DW_SSI_RISR             0x34
#define DW_SSI_TXOICR           0x38
#define DW_SSI_RXOICR           0x3C
#define DW_SSI_RXUICR           0x40
#define DW_SSI_MSTICR           0x44
#define DW_SSI_ICR              0x48
#define DW_SSI_DMACR            0x4C
#define DW_SSI_DMATDLR          0x50
#define DW_SSI_DMARDLR          0x54
#define DW_SSI_IDR              0x58
#define DW_SSI_VER_ID           0x5C
#define DW_SSI_DR               0x60
#define DW_SSI_RX_SAMPLE_DLY    0xF0
#define DW_SSI_SPI_CTRLR0       0xF4
#define DW_SSI_TXD_DRIVE_EDGE   0xF8

#undef BIT
#define BIT(nr) (1U << (nr))

#endif
