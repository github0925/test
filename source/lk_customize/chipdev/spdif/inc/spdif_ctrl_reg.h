//*****************************************************************************
//
// spdif_ctrl_reg.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPDIF_CRTL_REG_H__
#define __SPDIF_CRTL_REG_H__

typedef union {
    struct {
        volatile u32 tsamplerate: 8;
        volatile u32 sfr_en: 1;
        volatile u32 spdif_en: 1;
        volatile u32 fifo_en: 1;
        volatile u32 clk_en: 1;
        volatile u32 tr_mode: 1;
        volatile u32 parity_check: 1;
        volatile u32 parity_gen: 1;
        volatile u32 validity_check: 1;
        volatile u32 channel_mode: 1;
        volatile u32 duplicate: 1;
        volatile u32 setpreambb: 1;
        volatile u32 use_fifo_if: 1;
        volatile u32 : 1;
        volatile u32 parity_mask: 1;
        volatile u32 underr_mask: 1;
        volatile u32 ovrerr_mask: 1;
        volatile u32 empty_mask: 1;
        volatile u32 aempty_mask: 1;
        volatile u32 full_mask: 1;
        volatile u32 afull_mask: 1;
        volatile u32 syncerr_mask: 1;
        volatile u32 lock_mask: 1;
        volatile u32 begin_mask: 1;
        volatile u32 intreq_mask: 1;
    };
    volatile u32 v;
} spdif_crtl;

typedef union {
    struct {
        volatile u32 rsamplerate: 8;
        volatile u32 preambledel: 13;
        volatile u32 parityo: 1;
        volatile u32 tdata_underr: 1;
        volatile u32 rdata_ovrerr: 1;
        volatile u32 fifo_empty: 1;
        volatile u32 fifo_aempty: 1;
        volatile u32 fifo_full: 1;
        volatile u32 fifo_afull: 1;
        volatile u32 syncerr: 1;
        volatile u32 lock: 1;
        volatile u32 block_begin: 1;
    };
    volatile u32 v;
} spdif_int_reg;

typedef union {
    struct {
        volatile u32 aempty_threshold: 6;
        volatile u32 : 10;
        volatile u32 afull_threshold: 6;
        volatile u32 : 10;
    };
    volatile u32 v;
} spdif_fifo_ctrl;

typedef union {
    struct {
        volatile u32 fifo_level: 7;
        volatile u32 : 14;
        volatile u32 parity_flag: 1;
        volatile u32 underr_flag: 1;
        volatile u32 ovrerr_flag: 1;
        volatile u32 empty_flag: 1;
        volatile u32 aempty_flag: 1;
        volatile u32 full_flag: 1;
        volatile u32 afull_flag: 1;
        volatile u32 syncerr_flag: 1;
        volatile u32 lock_flag: 1;
        volatile u32 begin_flag: 1;
        volatile u32 right_left: 1;
    };
    volatile u32 v;
} spdif_fifo_stat_reg;

typedef struct {
    spdif_crtl ctrl;
    spdif_int_reg interrput;
    spdif_fifo_ctrl fifo_ctrl;
    spdif_fifo_stat_reg fifo_status;
} spdif_reg;

#endif
