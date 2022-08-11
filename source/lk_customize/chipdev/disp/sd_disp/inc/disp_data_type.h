/*
* disp_main.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __DISP_DATA_TYPE_H__
#define __DISP_DATA_TYPE_H__

typedef enum {
    SWAP_A_RGB          = 0x0,
    SWAP_A_RBG          = 0x1,
    SWAP_A_GBR          = 0x2,
    SWAP_A_GRB          = 0x3,
    SWAP_A_BGR          = 0x4,
    SWAP_A_BRG          = 0x5,
    SWAP_B_ARG          = 0x8,
    SWAP_B_AGR          = 0x9,
    SWAP_B_RGA          = 0xA,
    SWAP_B_RAG          = 0xB,
    SWAP_B_GRA          = 0xC,
    SWAP_B_GAR          = 0xD
} COMP_SWAP_MODE;

typedef enum {
    UV_YUV444_RGB       = 0x0,
    UV_YUV422           = 0x1,
    UV_YUV440           = 0x2,
    UV_YUV420           = 0x3
} DATA_UV_MODE;

typedef enum {
    LINEAR_MODE             = 0x0,
    RLE_COMPR_MODE          = 0x1,
    GPU_RAW_TILE_MODE       = 0x2,
    GPU_CPS_TILE_MODE       = 0x3,
    VPU_RAW_TILE_MODE       = 0x4,
    VPU_CPS_TILE_MODE       = 0x5,
    VPU_RAW_TILE_988_MODE   = 0x6,
} DATA_MODE;

typedef enum {
    FMT_INTERLEAVED     = 0x0,
    FMT_MONOTONIC       = 0x1,
    FMT_SEMI_PLANAR     = 0x2,
    FMT_PLANAR          = 0x3,
} FRM_BUF_STR_FMT;

typedef enum {
    ROT_DEFAULT         = 0x0,
    ROT_ROT             = 0x1,
    ROT_VFLIP           = 0x2,
    ROT_HFLIP           = 0x4,
} ROT_TYPE;

#define COLORFMT_PACK(VIDEO, UV_SWAP, UV_MODE, A_BITS, Y_BITS, U_BITS, V_BITS, COMP_SWAP, DATA_MODE, BUF_FMT, PLANE) \
    ((VIDEO            << 31) | \
     (UV_SWAP          << 30) | \
     (UV_MODE          << 28) | \
     (A_BITS           << 24) | \
     (Y_BITS           << 20) | \
     (U_BITS           << 16) | \
     (V_BITS           << 12) | \
     (COMP_SWAP        << 8)  | \
     (DATA_MODE        << 4)  | \
     (BUF_FMT          << 2)  | \
     (PLANE            << 0))

#define COLOR_GET_IS_YUV(color)      ((0x80000000 & color) >> 31)
#define COLOR_GET_IS_UV_SWAP(color)  ((0x40000000 & color) >> 30)
#define COLOR_GET_UV_MODE(color)     ((0x30000000 & color) >> 28)
#define COLOR_GET_A_BITS_NUM(color)  ((0x0F000000 & color) >> 24)
#define COLOR_GET_Y_BITS_NUM(color)  ((0x00F00000 & color) >> 20)
#define COLOR_GET_U_BITS_NUM(color)  ((0x000F0000 & color) >> 16)
#define COLOR_GET_V_BITS_NUM(color)  ((0x0000F000 & color) >> 12)
#define COLOR_GET_COMP_SWAP(color)   ((0x00000F00 & color) >> 8)
#define COLOR_GET_DATA_MODE(color)   ((0x000000F0 & color) >> 4)
#define COLOR_GET_BUF_FMT(color)     ((0x0000000C & color) >> 2)
#define COLOR_GET_PLANE_COUNT(color) ((0x0F000003 & color) >> 0)

typedef enum {
    /*16 bits*/
    COLOR_RGB565   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0, 5, 6, 5, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGR565   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0, 5, 6, 5, SWAP_A_BGR, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_ARGB4444   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 4, 4, 4, 4, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGRA4444   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 4, 4, 4, 4, SWAP_B_GRA, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_ARGB1555   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 1, 5, 5, 5, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGRA1555   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 1, 5, 5, 5, SWAP_B_GRA, LINEAR_MODE, FMT_INTERLEAVED, 1),
    /*18 bits*/
    COLOR_RGB666   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0 ,6, 6, 6, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGR666   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0 ,6, 6, 6, SWAP_A_BGR, LINEAR_MODE, FMT_INTERLEAVED, 1),
    /*24 bits*/
    COLOR_RGB888   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0 ,8, 8, 8, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGR888   = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 0 ,8, 8, 8, SWAP_A_BGR, LINEAR_MODE, FMT_INTERLEAVED, 1),
    /*32 bits*/
    COLOR_ARGB8888  = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 8, 8, 8, 8, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGRA8888  = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 8, 8, 8, 8, SWAP_B_RGA, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_ABGR8888  = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 8, 8, 8, 8, SWAP_A_BGR, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_ARGB2101010  = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 2, 10, 10, 10, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_BGRA2101010  = COLORFMT_PACK(0, 0, UV_YUV444_RGB, 2, 10, 10, 10, SWAP_B_RGA, LINEAR_MODE, FMT_INTERLEAVED, 1),

    /*YUV*/
    COLOR_NV21  = COLORFMT_PACK(1, 0, UV_YUV420, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_SEMI_PLANAR, 2),
    COLOR_NV12  = COLORFMT_PACK(1, 1, UV_YUV420, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_SEMI_PLANAR, 2),
    COLOR_YUV420P  = COLORFMT_PACK(1, 0, UV_YUV420, 0, 8, 8, 8, SWAP_A_RGB, LINEAR_MODE, FMT_PLANAR, 3),
    COLOR_YUYV  = COLORFMT_PACK(1, 0, UV_YUV422, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_YUVY  = COLORFMT_PACK(1, 0, UV_YUV422, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_INTERLEAVED, 1),
    COLOR_NV61  = COLORFMT_PACK(1, 0, UV_YUV422, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_SEMI_PLANAR, 2),
    COLOR_NV16  = COLORFMT_PACK(1, 1, UV_YUV422, 0, 8, 8, 0, SWAP_A_RGB, LINEAR_MODE, FMT_SEMI_PLANAR, 2),
    COLOR_YUV422P  = COLORFMT_PACK(1, 0, UV_YUV422, 0, 8, 8, 8, SWAP_A_RGB, LINEAR_MODE, FMT_PLANAR, 3),
} DISP_COLOR_ENUM;

inline unsigned char COLOR_GET_SWAP(int fmt)
{
	switch (fmt) {
	case COLOR_YUYV:
		return 0x1;
	default:
		return 0x0;
	}
}

#endif //__DISP_DATA_TYPE_H__
