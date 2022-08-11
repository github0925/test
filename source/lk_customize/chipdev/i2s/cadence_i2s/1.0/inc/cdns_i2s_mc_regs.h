//*****************************************************************************
//
// cdns_i2s_mc_regs.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __CDNS_I2S_MC_REGS_H__
#define __CDNS_I2S_MC_REGS_H__

typedef union {
    struct {
        volatile u32 i2s_en: 8;
        volatile u32 tr_cfg: 8;
        volatile u32 loopback: 4;
        volatile u32 sfr_rst: 1;
        volatile u32 t_ms: 1;
        volatile u32 r_ms: 1;
        volatile u32 tfifo_rst: 1;
        volatile u32 rfifo_rst: 1;
        volatile u32 tsync_rst: 1;
        volatile u32 rsync_rst: 1;
        volatile u32 tsync_loopback: 1;
        volatile u32 rsync_loopback: 1;
        volatile u32 : 3;
    };
    volatile u32 v;
} i2s_mc_i2s_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 tdata_underrun: 1;
        volatile u32 underrun_code: 3;
        volatile u32 rdata_overrun: 1;
        volatile u32 overrun_code: 3;
        volatile u32 tfifo_empty: 1;
        volatile u32 tfifo_aempty: 1;
        volatile u32 tfifo_full: 1;
        volatile u32 tfifo_afull: 1;
        volatile u32 rfifo_empty: 1;
        volatile u32 rfifo_aempty: 1;
        volatile u32 rfifo_full: 1;
        volatile u32 rfifo_afull: 1;
        volatile u32 : 16;
    };
    volatile u32 v;
} i2s_mc_i2s_intr_stat_reg_t;

typedef union {
    struct {
        volatile u32 tsrate: 11;
        volatile u32 tresolution: 5;
        volatile u32 rsrate: 11;
        volatile u32 rresolution: 5;
    };
    volatile u32 v;
} i2s_mc_srr_reg_t;

typedef union {
    struct {
        volatile u32 i2s_stb: 8;
        volatile u32 stb_ts: 1;
        volatile u32 stb_rs: 1;
        volatile u32 : 5;
        volatile u32 intreq_mask: 1;
        volatile u32 i2s_mask: 8;
        volatile u32 tfifo_empty_mask: 1;
        volatile u32 tfifo_aempty_mask: 1;
        volatile u32 tfifo_full_mask: 1;
        volatile u32 tfifo_afull_mask: 1;
        volatile u32 rfifo_empty_mask: 1;
        volatile u32 rfifo_aempty_mask: 1;
        volatile u32 rfifo_full_mask: 1;
        volatile u32 rfifo_afull_mask: 1;
    };
    volatile u32 v;
} i2s_mc_cid_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 tlevel: 10;
        volatile u32 : 22;
    };
    volatile u32 v;
} i2s_mc_tfifo_stat_reg_t;

typedef union {
    struct {
        volatile u32 rlevel: 10;
        volatile u32 : 22;
    };
    volatile u32 v;
} i2s_mc_rfifo_stat_reg_t;

typedef union {
    struct {
        volatile u32 taempty_threshold: 9;
        volatile u32 : 7;
        volatile u32 tafull_threshold: 9;
        volatile u32 : 7;
    };
    volatile u32 v;
} i2s_mc_tfifo_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 raempty_threshold: 9;
        volatile u32 : 7;
        volatile u32 rafull_threshold: 9;
        volatile u32 : 7;
    };
    volatile u32 v;
} i2s_mc_rfifo_ctrl_reg_t;

typedef union {
    struct {
        volatile u32 tran_dev_cfg: 6;
        volatile u32 rec_dev_cfg: 6;
        volatile u32 : 20;
    };
    volatile u32 v;
} i2s_mc_dev_conf_reg_t;

typedef union {
    struct {
        volatile u32 tx_empty: 1;
        volatile u32 tx_aempty: 1;
        volatile u32 tx_underrun: 1;
        volatile u32 : 1;
        volatile u32 rx_full: 1;
        volatile u32 rx_afull: 1;
        volatile u32 rx_overrun: 1;
        volatile u32 : 25;
    };
    volatile u32 v;
} i2s_mc_poll_stat_reg_t;

typedef struct {
    // i2s mc ctrl register entity
    volatile i2s_mc_i2s_ctrl_reg_t ctrl;
    volatile i2s_mc_i2s_intr_stat_reg_t intr_stat;
    volatile i2s_mc_srr_reg_t srr;
    volatile i2s_mc_cid_ctrl_reg_t cid_ctrl;
    volatile i2s_mc_tfifo_stat_reg_t tfifo_stat;
    volatile i2s_mc_rfifo_stat_reg_t rfifo_stat;
    volatile i2s_mc_tfifo_ctrl_reg_t tfifo_ctrl;
    volatile i2s_mc_rfifo_ctrl_reg_t rfifo_ctrl;
    volatile i2s_mc_dev_conf_reg_t dev_conf;
    volatile i2s_mc_poll_stat_reg_t poll_stat;
} i2s_mc_regs_t;

#endif
