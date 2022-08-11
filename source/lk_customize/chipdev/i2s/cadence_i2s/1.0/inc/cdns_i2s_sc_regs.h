//*****************************************************************************
//
// cdns_i2s_sc_regs.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __CDNS_I2S_SC_REGS_H__
#define __CDNS_I2S_SC_REGS_H__

typedef union {
    struct {
        volatile u32 i2s_en: 1;  // 1--enalbe 0--reset, cfg sfr bits stay
        volatile u32 dir_cfg: 1;  // 1--transmitter, 0--receiver
        volatile u32 ms_cfg: 1;  // 1--master, 0--slave
        volatile u32 sfr_rst: 1;  // active low--sfr block reset
        volatile u32 fifo_rst: 1;  // active low--fifo pointer reset to 0(fd:tfifo)
        // audio chan time slot width under i2s master or dsp/tdm m/s mode
        volatile u32 chn_width: 3;
        // non-tdm mode:bit 8:0--dsp;1--stardard audio i/f spec format. tdm mode: 0-dsp,other-ws changes after given nunber of channels.
        volatile u32 ws_mode: 4;
        // 1-right audio channel 0-left audio chan is active
        volatile u32 mono_mode: 1;
        // (non-tdm&&stard i/f)1--mono mode is active 0--both left & right.
        volatile u32 audio_mode: 1;
        // 1-- serial data updated on falling edge, 0-- rising edge.
        volatile u32 sck_polar: 1;
        volatile u32 ws_polar: 1;
        volatile u32 data_ws_del: 5;
        volatile u32 data_align: 1;
        volatile u32 data_order: 1;
        volatile u32 host_data_align: 1;
        volatile u32 i2s_stb: 1;
        volatile u32 intreq: 1;
        volatile u32 i2s_mask: 1;
        volatile u32 fifo_empty_mask: 1;
        volatile u32 fifo_aempty_mask: 1;
        volatile u32 fifo_full_mask: 1;
        volatile u32 fifo_afull_mask: 1;
        volatile u32 lr_pack: 1;
    };
    volatile u32 v;
} i2s_sc_i2s_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 full_duplex: 1;
        volatile u32 i2s_ftx_en: 1;
        volatile u32 i2s_frx_en: 1;
        volatile u32 : 1;
        volatile u32 fifo_rst: 1;
        volatile u32 : 21;
        volatile u32 ri2s_mask: 1;
        volatile u32 rfifo_empty_mask: 1;
        volatile u32 rfifo_aempty_mask: 1;
        volatile u32 rfifo_full_mask: 1;
        volatile u32 rfifo_afull_mask: 1;
        volatile u32 : 1;
    };
    volatile u32 v;
} i2s_sc_i2s_ctrl_fdx_reg_t;

typedef union {
    struct {
        volatile u32 resolution: 5;
        volatile u32 : 27;
    };
    volatile u32 v;
} i2s_sc_i2s_sres_reg_t;

typedef union {
    struct {
        volatile u32 resolution: 5;
        volatile u32 : 27;
    };
    volatile u32 v;
} i2s_sc_i2s_sres_fdr_reg_t;

typedef union {
    struct {
        volatile u32 srate: 20;
        volatile u32 : 12;
    };
    volatile u32 v;
} i2s_sc_i2s_srate_reg_t;

typedef union {
    struct {
        volatile u32 tdata_underrun: 1;
        volatile u32 rdata_overrun: 1;
        volatile u32 fifo_empty: 1;
        volatile u32 fifo_aempty: 1;
        volatile u32 fifo_full: 1;
        volatile u32 fifo_afull: 1;
        volatile u32 : 10;
        volatile u32 rfifo_empty: 1;
        volatile u32 rfifo_aempty: 1;
        volatile u32 rfifo_full: 1;
        volatile u32 rfifo_afull: 1;
        volatile u32 : 12;
    };
    volatile u32 v;
} i2s_sc_i2s_stat_reg_t;

typedef union {
    struct {
        volatile u32 level: 8;
        volatile u32 : 24;
    };
    volatile u32 v;
} i2s_sc_fifo_level_reg_t;

typedef union {
    struct {
        volatile u32 threshold: 7;
        volatile u32 : 25;
    };
    volatile u32 v;
} i2s_sc_fifo_aempty_reg_t;

typedef union {
    struct {
        volatile u32 threshold: 7;
        volatile u32 : 25;
    };
    volatile u32 v;
} i2s_sc_fifo_afull_reg_t;

typedef union {
    struct {
        volatile u32 level: 8;
        volatile u32 : 24;
    };
    volatile u32 v;
} i2s_sc_fifo_level_fdr_reg_t;

typedef union {
    struct {
        volatile u32 threshold: 7;
        volatile u32 : 25;
    };
    volatile u32 v;
} i2s_sc_fifo_aempty_fdr_reg_t;

typedef union {
    struct {
        volatile u32 threshold: 7;
        volatile u32 : 25;
    };
    volatile u32 v;
} i2s_sc_fifo_afull_fdr_reg_t;

typedef union {
    struct {
        volatile u32 tdm_en: 1;
        volatile u32 chn_no: 4;
        volatile u32 : 11;
        volatile u32 ch_en: 16;
    };
    volatile u32 v;
} i2s_sc_tdm_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 ch_rxen: 16;
        volatile u32 ch_txen: 16;
    };
    volatile u32 v;
} i2s_sc_tdm_fd_dir_reg_t;

typedef struct {
    // i2s sc register entity
    volatile i2s_sc_i2s_ctrl_reg_t ctrl;
    volatile i2s_sc_i2s_ctrl_fdx_reg_t ctrl_fdx;
    volatile i2s_sc_i2s_sres_reg_t sres;
    volatile i2s_sc_i2s_sres_fdr_reg_t sres_fdr;
    volatile i2s_sc_i2s_srate_reg_t srate;
    volatile i2s_sc_i2s_stat_reg_t stat;
    volatile i2s_sc_fifo_level_reg_t fifo_level;
    volatile i2s_sc_fifo_aempty_reg_t fifo_aempty;
    volatile i2s_sc_fifo_afull_reg_t fifo_afull;
    volatile i2s_sc_fifo_level_fdr_reg_t fifo_level_fdr;
    volatile i2s_sc_fifo_aempty_fdr_reg_t fifo_aempty_fdr;
    volatile i2s_sc_fifo_afull_fdr_reg_t fifo_afull_fdr;
    volatile i2s_sc_tdm_ctrl_reg_t tdm_ctrl;
    volatile i2s_sc_tdm_fd_dir_reg_t tdm_fd_dir;
} i2s_sc_regs_t;

#endif
