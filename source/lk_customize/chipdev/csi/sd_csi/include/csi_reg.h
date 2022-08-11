/*
* csi_reg.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* csi reg
*
* Revision History:
* -----------------
* 0.1, 12/21/2018 init version
*/

#pragma once

#include "sd_csi_reg.h"
//#include "__regs_ap_csi.h"
#include <irq_v.h>
//#include "__regs_base.h"

#define REG(x)                      (x)

//#define WITH_ON_ZONE 1
//#define FPGA_CSI1_BASE 0x0
//#define FPGA_CSI1_INT 0x0

#if WITH_CSI_DUMMY_REG
#define CSI_BASE_ADDR(x)            (x)
#define CSI_IRQ(x)                  (x)
//#else
//#if WITH_ON_ZONE
//#define CSI_BASE_ADDR(x)            (APB_CSI1_BASE + (x) * 0x10000)
//#define CSI_IRQ(x)                  (IRQ_GIC4_CSI1_INTERRUPT_NUM + (x))
//#else
//#define CSI_BASE_ADDR(x)            (FPGA_CSI0_BASE + (x) * 0x20000)
//#define CSI_IRQ(x)                  (FPGA_CSI0_INT + (x))
//#endif
#endif



#define IMG_JUMP    0x80
#define IMG_COUNT   4

#define CSI_IMG_RGBY_BADDR_H_(i)    \
    (REG(CSI_IMG_RGBY_BADDR_H) + IMG_JUMP * (i))

#define CSI_IMG_RGBY_BADDR_L_(i)    \
    (REG(CSI_IMG_RGBY_BADDR_L) + IMG_JUMP * (i))

#define CSI_IMG_U_BADDR_H_(i)       \
    (REG(CSI_IMG_U_BADDR_H) + IMG_JUMP * (i))

#define CSI_IMG_U_BADDR_L_(i)       \
    (REG(CSI_IMG_U_BADDR_L) + IMG_JUMP * (i))

#define CSI_IMG_V_BADDR_H_(i)       \
    (REG(CSI_IMG_V_BADDR_H) + IMG_JUMP * (i))

#define CSI_IMG_V_BADDR_L_(i)       \
    (REG(CSI_IMG_V_BADDR_L) + IMG_JUMP * (i))


#define CSI_IMG_RGBY_STRIDE_(i)     \
    (REG(CSI_IMG_RGBY_STRIDE) + IMG_JUMP * (i))

#define CSI_IMG_U_STRIDE_(i)        \
    (REG(CSI_IMG_U_STRIDE) + IMG_JUMP * (i))

#define CSI_IMG_V_STRIDE_(i)        \
    (REG(CSI_IMG_V_STRIDE) + IMG_JUMP * (i))


#define CSI_IMG_SIZE_(i)            \
    (REG(CSI_IMG_SIZE) + IMG_JUMP * (i))


#define CSI_IMG_IPI_CTRL_(i)        \
    (REG(CSI_IMG_IPI_CTRL) + IMG_JUMP * (i))


#define CSI_INTERFACE_MIPI_CSI      0x00
#define CSI_INTERFACE_GATE_MODE2    0x02
#define CSI_INTERFACE_NON_GATE_MODE 0x03
#define CSI_INTERFACE_OTH_PARALLEL  0x04
#define CSI_INTERFACE_BT656         0x05
#define CSI_INTERFACE_BT1120        0x06

#define CSI_INTERFACE_DC_PARALLEL   0x11
#define CSI_INTERFACE_DC_PARALLEL2  0x12
#define CSI_INTERFACE_OTH_PARALLEL2  0x13

#define CSI_IMG_PIXEL_MASK0_(i)     \
    (REG(CSI_IMG_IF_PIXEL_MASK0) + IMG_JUMP * (i))

#define CSI_IMG_PIXEL_MASK1_(i)     \
    (REG(CSI_IMG_IF_PIXEL_MASK1) + IMG_JUMP * (i))

#define CSI_IMG_CHN_CTRL_(i)        \
    (REG(CSI_IMG_CHN_CTRL) + IMG_JUMP * (i))

#define CSI_IMG_CHN_SPLIT0_(i)      \
     (REG(CSI_IMG_CHN_SPLIT0) + IMG_JUMP * (i))

#define CSI_IMG_CHN_SPLIT1_(i)      \
    (REG(CSI_IMG_CHN_SPLIT1) + IMG_JUMP * (i))

#define CSI_IMG_CHN_SPLIT2_(i)      \
    (REG(CSI_IMG_CHN_SPLIT2) + IMG_JUMP * (i))

#define SPLIT_VALID_ALL         0x0
#define SPLIT_VALID_U           0x01
#define SPLIT_VALID_V           0x02
#define SPLIT_VALID_UV          0x03

#define SPLIT_CHN_MAP_0_15      0x0
#define SPLIT_CHN_MAP_16_31     0x01
#define SPLIT_CHN_MAP_32_47     0x02
#define SPLIT_CHN_MAP_48_63     0x03
#define SPLIT_CHN_MAP_NONE      0x07
#define SPLIT_CHN_MAP_MASK      0x07

#define SPLIT_CHN0_SHIFT        0
#define SPLIT_CHN1_SHIFT        3
#define SPLIT_CHN2_SHIFT        6
#define SPLIT_CHN3_SHIFT        9

#define CSI_IMG_CHN_CROP0_(i)       \
    (REG(CSI_IMG_CHN_CROP0) + IMG_JUMP * (i))

#define CSI_IMG_CHN_CROP1_(i)       \
    (REG(CSI_IMG_CHN_CROP1) + IMG_JUMP * (i))

#define CSI_IMG_CHN_CROP2_(i)       \
    (REG(CSI_IMG_CHN_CROP2) + IMG_JUMP * (i))

#define CSI_IMG_CHN_PACK0_(i)           \
    (REG(CSI_IMG_CHN_PACK0) + IMG_JUMP * (i))

#define CSI_IMG_CHN_PACK1_(i)           \
    (REG(CSI_IMG_CHN_PACK1) + IMG_JUMP * (i))

#define CSI_IMG_CHN_PACK2_(i)           \
    (REG(CSI_IMG_CHN_PACK2) + IMG_JUMP * (i))


#define COLOR_DEPTH_1BIT        1
#define COLOR_DEPTH_2BIT        2
#define COLOR_DEPTH_4BIT        4
#define COLOR_DEPTH_5BIT        5
#define COLOR_DEPTH_6BIT        6
#define COLOR_DEPTH_8BIT        8
#define COLOR_DEPTH_10BIT       10
#define COLOR_DEPTH_16BIT       16

#define CHN_JUMP    0x20
#define CHN_COUNT   8
#define CHN_PER_IMG (CHN_COUNT / IMG_COUNT)

#define CSI_WDMA_CHN_DFIFO_(i)          \
    (REG(CSI_WDMA_CHN_DFIFO) + CHN_JUMP * (i))

#define CSI_WDMA_CHN_CFIFO_(i)      \
    (REG(CSI_WDMA_CHN_CFIFO) + CHN_JUMP * (i))

#define CSI_WDMA_CHN_AXI_CTRL0_(i)      \
    (REG(CSI_WDMA_CHN_AXI_CTRL0) + CHN_JUMP * (i))


#define CSI_WDMA_CHN_AXI_CTRL1_(i)  \
    (REG(CSI_WDMA_CHN_AXI_CTRL1) + CHN_JUMP * (i))


#define CSI_WDMA_CHN_AXI_CTRL2_(i)  \
    (REG(CSI_WDMA_CHN_AXI_CTRL2) + CHN_JUMP * (i))



#define CSI_WDMA_CHN_STATE_(i)      \
    (REG(CSI_WDMA_CHN_STATE) + CHN_JUMP * (i))


#define MAP_JUMP 0x04
#define MAP_COUNT   12

#define CSI_PIXEL_MAP_(i)           \
    (REG(CSI_PIXEL_MAP) + MAP_JUMP * (i))
#define CSI_PIXEL2_MAP_(i)          \
    (REG(CSI_PIXEL2_MAP) + MAP_JUMP * (i))

