/*
* dw_dmac.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* dw dma controller head
*
* Revision History:
* -----------------
* 0.1, 3/29/2019 yishao init version
*/
#ifndef __DW_DMAC_H
#define __DW_DMAC_H

#include <bits.h>
#include "dw_dmac_mux.h"

#define DMA_MIN_CHANNEL (0)
#define DMA_MAX_CHANNEL (7)

#define DMA_MIN_INDEX (0)
#define DMA_MAX_INDEX (7)

#define DMA_HW_HANDSHAKE (0)
#define DMA_SW_HANDSHAKE (1)

#define DW_DMAC_CHANNEL_NUMB (8)
#define DW_DMAC_MIN_CHANNEL (0)
#define DW_DMAC_MAX_CHANNEL (7)

#define DW_DMAC_MAX_NUMB (7)

#define DW_DMAC_MAX_BLK_SIZE (0x10000) /* Semidrive version size */
#define DW_DMAC_LLI_LAST_OFFSET (0x4000000000000000)
typedef enum
{
    DMA_CHAN_1 = 1,
    DMA_CHAN_2 = 2,
    DMA_CHAN_3 = 3,
    DMA_CHAN_4 = 4,
    DMA_CHAN_5 = 5,
    DMA_CHAN_6 = 6,
    DMA_CHAN_7 = 7,
} DMA_CHANNEL;

typedef enum
{
    DMA_TRANSFER_TYPE_CONTINUOUS = 0, /*both src and dst set continuous result single type*/
    DMA_TRANSFER_TYPE_RELOAD,
    DMA_TRANSFER_TYPE_SHADOW,
    DMA_TRANSFER_TYPE_LLI,
} DMA_TRANSFER_TYPE;

typedef enum
{
    DMA_TRANSFER_ADDR_INCREMENT = 0, /*both src and dst set continuous result single type*/
    DMA_TRANSFER_ADDR_NOCHANGE = 1,
} DMA_TRANSFER_ADDRESS;

typedef enum
{
    DMA_TRANSFER_DIR_MEM2MEM_DMAC = 0,
    DMA_TRANSFER_DIR_MEM2PER_DMAC = 1,
    DMA_TRANSFER_DIR_PER2MEM_DMAC = 2,
    DMA_TRANSFER_DIR_PER2PER_DMAC = 3,
    DMA_TRANSFER_DIR_PER2MEM_SRC = 4,
    DMA_TRANSFER_DIR_PER2PER_SRC = 5,
    DMA_TRANSFER_DIR_MEM2PER_DST = 6,
    DMA_TRANSFER_DIR_PER2PER_DST = 7,
} DMA_TRANSFER_DIRECTION;

typedef enum
{
    DMA_TRANSFER_WIDTH_8BITS = 0,   /* 1 */
    DMA_TRANSFER_WIDTH_16BITS = 1,  /* 2 */
    DMA_TRANSFER_WIDTH_32BITS = 2,  /* 4 */
    DMA_TRANSFER_WIDTH_64BITS = 3,  /* 8 */
    DMA_TRANSFER_WIDTH_128BITS = 4, /* 16 */
    DMA_TRANSFER_WIDTH_256BITS = 5, /* 32 */
    DMA_TRANSFER_WIDTH_512BITS = 6, /* 64 */
    DMA_TRANSFER_WIDTH_MAX = DMA_TRANSFER_WIDTH_512BITS
} DMA_TRANSFER_WIDTH;

/* Number of data items, each of width CHx_CTL.SRC_TR_WIDTH, to be read from the source every time a source burst transaction request is made from the corresponding hardware or software handshaking interface.
 */
#if IS_64BIT
#define GEN_MASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))
#else
#define GEN_MASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))
#endif
/* Int status */
typedef enum
{
    DMA_BLOCK_TR_DONE = BIT(0xffffffff, 0),
    DMA_DMA_TR_DONE = BIT(0xffffffff, 1),
    DMA_SRC_COMP = BIT(0xffffffff, 3),
    DMA_DST_COMP = BIT(0xffffffff, 4),
    DMA_SRC_DEC_ERR = BIT(0xffffffff, 5),
    DMA_DST_DEC_ERR = BIT(0xffffffff, 6),
    DMA_SRC_SLV_ERR = BIT(0xffffffff, 7),
    DMA_DST_SLV_ERR = BIT(0xffffffff, 8),
    DMA_LLI_RD_DEC_ERR = BIT(0xffffffff, 9),
    DMA_LLI_WR_DEC_ERR = BIT(0xffffffff, 10),
    DMA_LLI_RD_SLV_ERR = BIT(0xffffffff, 11),
    DMA_LLI_WR_SLV_ERR = BIT(0xffffffff, 12),
    DMA_INVALID_ERR = BIT(0xffffffff, 13),
    DMA_MULTIBLKTYPE_ERR = BIT(0xffffffff, 14),
    DMA_DEC_ERR = BIT(0xffffffff, 16),
    DMA_WR2RO_ERR = BIT(0xffffffff, 17),
    DMA_RD2RWO_ERR = BIT(0xffffffff, 18),
    DMA_WR_ON_CHEN_ERR = BIT(0xffffffff, 19),
    DMA_SHADOW_WR_VALID_ERR = BIT(0xffffffff, 20),
    DMA_WR_ONHOLD_ERR = BIT(0xffffffff, 21),
    DMA_LOCK_CLEARED = BIT(0xffffffff, 27),
    DMA_SRC_SUSPENDED = BIT(0xffffffff, 28),
    DMA_SUSPENDED = BIT(0xffffffff, 29),
    DMA_DISABLED = BIT(0xffffffff, 30),
    DMA_ABORTED = BIT(0xffffffff, 31),
    DMA_ALL_ERR = (GEN_MASK(21, 16) | GEN_MASK(14, 5)),
    DMA_ALL = GEN_MASK(31, 0)
} DMA_INT_STATUS;
//
/* 0x10 DMAC_CFGREG */
typedef union {
    struct
    {
        volatile u32 dmac_en : 1; /* This bit is used to enable the DW_axi_dmac. */
        volatile u32 int_en : 1;  /* This bit is used to globally enable the interrupt generation. */
        volatile u32 rsvd : 30;
        volatile u32 rsvd1 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_cfg_t;

/* 0x18 This is DW_axi_dmac Channel Enable Register. */
typedef union {
    struct
    {
        volatile u32 ch1_en : 1;       /* enable the DW_axi_dmac channel 1. */
        volatile u32 ch2_en : 1;       /* enable the DW_axi_dmac channel 2. */
        volatile u32 ch3_en : 1;       /* enable the DW_axi_dmac channel 3. */
        volatile u32 ch4_en : 1;       /* enable the DW_axi_dmac channel 4. */
        volatile u32 ch5_en : 1;       /* enable the DW_axi_dmac channel 5. */
        volatile u32 ch6_en : 1;       /* enable the DW_axi_dmac channel 6. */
        volatile u32 ch7_en : 1;       /* enable the DW_axi_dmac channel 7. */
        volatile u32 ch8_en : 1;       /* enable the DW_axi_dmac channel 8. */
        volatile u32 ch1_en_we : 1;    /* enable the DW_axi_dmac channel 1 Write Enable bit. */
        volatile u32 ch2_en_we : 1;    /* enable the DW_axi_dmac channel 2 Write Enable bit. */
        volatile u32 ch3_en_we : 1;    /* enable the DW_axi_dmac channel 3 Write Enable bit. */
        volatile u32 ch4_en_we : 1;    /* enable the DW_axi_dmac channel 4 Write Enable bit. */
        volatile u32 ch5_en_we : 1;    /* enable the DW_axi_dmac channel 5 Write Enable bit. */
        volatile u32 ch6_en_we : 1;    /* enable the DW_axi_dmac channel 6 Write Enable bit. */
        volatile u32 ch7_en_we : 1;    /* enable the DW_axi_dmac channel 7 Write Enable bit. */
        volatile u32 ch8_en_we : 1;    /* enable the DW_axi_dmac channel 8 Write Enable bit. */
        volatile u32 ch1_susp : 1;     /* the DW_axi_dmac channel 1 Suspend Request. */
        volatile u32 ch2_susp : 1;     /* the DW_axi_dmac channel 2 Suspend Request. */
        volatile u32 ch3_susp : 1;     /* the DW_axi_dmac channel 3 Suspend Request. */
        volatile u32 ch4_susp : 1;     /* the DW_axi_dmac channel 4 Suspend Request. */
        volatile u32 ch5_susp : 1;     /* the DW_axi_dmac channel 5 Suspend Request. */
        volatile u32 ch6_susp : 1;     /* the DW_axi_dmac channel 6 Suspend Request. */
        volatile u32 ch7_susp : 1;     /* the DW_axi_dmac channel 7 Suspend Request. */
        volatile u32 ch8_susp : 1;     /* the DW_axi_dmac channel 8 Suspend Request. */
        volatile u32 ch1_susp_we : 1;  /* the DW_axi_dmac channel 1 Suspend Request Write Enable bit. */
        volatile u32 ch2_susp_we : 1;  /* the DW_axi_dmac channel 2 Suspend Request Write Enable bit. */
        volatile u32 ch3_susp_we : 1;  /* the DW_axi_dmac channel 3 Suspend Request Write Enable bit. */
        volatile u32 ch4_susp_we : 1;  /* the DW_axi_dmac channel 4 Suspend Request Write Enable bit. */
        volatile u32 ch5_susp_we : 1;  /* the DW_axi_dmac channel 5 Suspend Request Write Enable bit. */
        volatile u32 ch6_susp_we : 1;  /* the DW_axi_dmac channel 6 Suspend Request Write Enable bit. */
        volatile u32 ch7_susp_we : 1;  /* the DW_axi_dmac channel 7 Suspend Request Write Enable bit. */
        volatile u32 ch8_susp_we : 1;  /* the DW_axi_dmac channel 8 Suspend Request Write Enable bit. */
        volatile u32 ch1_abort : 1;    /* The DW_axi_dmac channel 1 abort request. */
        volatile u32 ch2_abort : 1;    /* The DW_axi_dmac channel 2 abort request. */
        volatile u32 ch3_abort : 1;    /* The DW_axi_dmac channel 3 abort request. */
        volatile u32 ch4_abort : 1;    /* The DW_axi_dmac channel 4 abort request. */
        volatile u32 ch5_abort : 1;    /* The DW_axi_dmac channel 5 abort request. */
        volatile u32 ch6_abort : 1;    /* The DW_axi_dmac channel 6 abort request. */
        volatile u32 ch7_abort : 1;    /* The DW_axi_dmac channel 7 abort request. */
        volatile u32 ch8_abort : 1;    /* The DW_axi_dmac channel 8 abort request. */
        volatile u32 ch1_abort_we : 1; /* The DW_axi_dmac channel 1 abort request Write Enable bit. */
        volatile u32 ch2_abort_we : 1; /* The DW_axi_dmac channel 2 abort request Write Enable bit. */
        volatile u32 ch3_abort_we : 1; /* The DW_axi_dmac channel 3 abort request Write Enable bit. */
        volatile u32 ch4_abort_we : 1; /* The DW_axi_dmac channel 4 abort request Write Enable bit. */
        volatile u32 ch5_abort_we : 1; /* The DW_axi_dmac channel 5 abort request Write Enable bit. */
        volatile u32 ch6_abort_we : 1; /* The DW_axi_dmac channel 6 abort request Write Enable bit. */
        volatile u32 ch7_abort_we : 1; /* The DW_axi_dmac channel 7 abort request Write Enable bit. */
        volatile u32 ch8_abort_we : 1; /* The DW_axi_dmac channel 8 abort request Write Enable bit. */
        volatile u32 rvsd_48 : 16;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_ch_en_t;

/* 0x30 DMAC Interrupt Status Register captures the combined channel interrupt for each channel and Combined common register block interrupt. */
typedef union {
    struct
    {
        volatile u32 ch1_int_stat : 1;      /* Channel 1 Interrupt Status Bit. */
        volatile u32 ch2_int_stat : 1;      /* Channel 2 Interrupt Status Bit. */
        volatile u32 ch3_int_stat : 1;      /* Channel 3 Interrupt Status Bit. */
        volatile u32 ch4_int_stat : 1;      /* Channel 4 Interrupt Status Bit. */
        volatile u32 ch5_int_stat : 1;      /* Channel 5 Interrupt Status Bit. */
        volatile u32 ch6_int_stat : 1;      /* Channel 6 Interrupt Status Bit. */
        volatile u32 ch7_int_stat : 1;      /* Channel 7 Interrupt Status Bit. */
        volatile u32 ch8_int_stat : 1;      /* Channel 8 Interrupt Status Bit. */
        volatile u32 rsvd_8 : 8;            /* Channel 1 Interrupt Status Bit. */
        volatile u32 comm_reg_int_Stat : 1; /* Common Register Interrupt Status Bit. */
        volatile u32 rsvd_17 : 15;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_int_stat_t;

/* 0x38 Writing 1 to specific field clears the corresponding field in DMAC Common register Interrupt Status Register (DMAC_CommonReg_IntStatusReg). */
typedef union {
    struct
    {
        volatile u32 slvif_dec_err : 1;       /* Slave Interface Common Register Decode Error Interrupt clear Bit. */
        volatile u32 slvif_wr2ro_err : 1;     /* Slave Interface Common Register Write to Read only Error Interrupt clear Bit. */
        volatile u32 slvif_rd2wo_err : 1;     /* Slave Interface Common Register Read to Write only Error Interrupt clear Bit. */
        volatile u32 slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On Hold Error Interrupt clear Bit. */
        volatile u32 rsvd_4 : 4;
        volatile u32 slvif_undef_dec_err : 1; /* Slave Interface Undefined register Decode Error Interrupt clear Bit. */
        volatile u32 rsvd_9 : 23;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_comm_int_clr_t;

/* 0x40 Writing 1 to specific field enables the corresponding interrupt status generation in DMAC Common register Interrupt Status Register (DMAC_CommonReg_IntStatusReg). */
typedef union {
    struct
    {
        volatile u32 slvif_dec_err : 1;       /* Slave Interface Common Register Decode Error Interrupt enable Bit. */
        volatile u32 slvif_wr2ro_err : 1;     /* Slave Interface Common Register Write to Read only Error Interrupt enable Bit. */
        volatile u32 slvif_rd2wo_err : 1;     /* Slave Interface Common Register Read to Write only Error Interrupt enable Bit. */
        volatile u32 slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On Hold Error Interrupt enable Bit. */
        volatile u32 rsvd_4 : 4;
        volatile u32 slvif_undef_dec_err : 1; /* Slave Interface Undefined register Decode Error Interrupt enable Bit. */
        volatile u32 rsvd_9 : 23;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_comm_int_stat_en_t;

/* 0x48 Writing 1 to specific field will propagate the corresponding interrupt status in DMAC Common register Interrupt Status Register (DMAC_CommonReg_IntStatusReg) to generate an port level interrupt. */
typedef union {
    struct
    {
        volatile u32 slvif_dec_err : 1;       /* Slave Interface Common Register Decode Error Interrupt clear Bit. */
        volatile u32 slvif_wr2ro_err : 1;     /* Slave Interface Common Register Write to Read only Error Interrupt clear Bit. */
        volatile u32 slvif_rd2wo_err : 1;     /* Slave Interface Common Register Read to Write only Error Interrupt clear Bit. */
        volatile u32 slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On Hold Error Interrupt clear Bit. */
        volatile u32 rsvd_4 : 4;
        volatile u32 slvif_undef_dec_err : 1; /* Slave Interface Undefined register Decode Error Interrupt clear Bit. */
        volatile u32 rsvd_9 : 23;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_comm_int_sig_en_t;

/* 0x50 This Register captures Slave interface access errors. */
typedef union {
    struct
    {
        volatile u32 slvif_dec_err : 1;       /* Slave Interface Common Register Decode Error Interrupt clear Bit. */
        volatile u32 slvif_wr2ro_err : 1;     /* Slave Interface Common Register Write to Read only Error Interrupt clear Bit. */
        volatile u32 slvif_rd2wo_err : 1;     /* Slave Interface Common Register Read to Write only Error Interrupt clear Bit. */
        volatile u32 slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On Hold Error Interrupt clear Bit. */
        volatile u32 rsvd_4 : 4;
        volatile u32 slvif_undef_dec_err : 1; /* Slave Interface Undefined register Decode Error Interrupt clear Bit. */
        volatile u32 rsvd_9 : 23;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_comm_int_stat_t;

/* 0x58 This register is used to initiate the Software Reset to DW_axi_dmac. */
typedef union {
    struct
    {
        volatile u32 reset : 1; /* DMAC Reset Request bit Software writes 1 to this bit */
        volatile u32 rsvd : 31;
        volatile u32 rsvd2 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };

} dmac_reset_t;

/* 0x110 + x(x-1)*0x100 */
typedef union {
    struct
    {
        volatile u32 blk_ts : 22; /* Block Transfer Size = BLOCK_TS+1 */
        volatile u32 rsvd : 10;
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_blk_ts_t;

/* ctl structure 0x118 + x(x-1)*0x100 */
typedef union {
    struct
    {
        volatile u32 sms : 1;                     /* default to 0 */
        volatile u32 rsvd_1 : 1;                  /* reserved chan x0 1 */
        volatile u32 dms : 1;                     /* default to 0 */
        volatile u32 rsvd_3 : 1;                  /* reserved chan x0 3 */
        volatile u32 sinc : 1;                    /*  Source Address Increment. */
        volatile u32 rsvd_5 : 1;                  /* reserved chan x0 5 */
        volatile u32 dinc : 1;                    /*  Destination Address Increment. */
        volatile u32 rsvd_7 : 1;                  /* reserved chan x0 7 */
        volatile u32 src_tr_width : 3;            /* Source Transfer Width. */
        volatile u32 dst_tr_width : 3;            /* Destination Transfer Width */
        volatile u32 src_msize : 4;               /* Source Burst Transaction Length. */
        volatile u32 dst_msize : 4;               /* Destination Burst Transaction Length. */
        volatile u32 ar_cache : 4;                /* default 0 AXI 'ar_cache' signal */
        volatile u32 aw_cache : 4;                /* default 0 AXI 'aw_cache' signal */
        volatile u32 nonposted_last_write_en : 1; /*  0 1 : no transfer 0 :transfer */
        volatile u32 rsvd_31 : 1;                 /* reserved chan x0 7 */

        volatile u32 ar_prot : 3;          /* AXI 'ar_prot' signal */
        volatile u32 aw_prot : 3;          /* AXI 'aw_prot' signal */
        volatile u32 arlen_en : 1;         /* Source Burst Length Enable */
        volatile u32 arlen : 8;            /* Source Burst Length */
        volatile u32 awlen_en : 1;         /* Destination Burst Length Enable */
        volatile u32 awlen : 8;            /* Destination Burst Length */
        volatile u32 src_stat_en : 1;      /* Source Status Enable */
        volatile u32 dst_stat_en : 1;      /* Destination Status Enable */
        volatile u32 ioc_blk_tr : 1;       /* Interrupt On completion of Block Transfer 1 enable interrupt 0 disable */
        volatile u32 rsvd_59 : 3;          /* reserved chan x0 59-61 */
        volatile u32 shadow_lli_last : 1;  /* Last Shadow Register/Linked List Item. 0: Not last Shadow Register/LLI 1:Last Shadow Register/LLI */
        volatile u32 shadow_lli_valid : 1; /* 0: Shadow Register content/LLI is invalid.1: Last Shadow Register/LLI is valid. */
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_ctl_t;

/* 0x120 + x(x-1)*0x100 */
typedef union {
    struct
    {
        volatile u32 src_multblk_type : 2; /* Source Multi Block Transfer Type. */
        volatile u32 dst_multblk_type : 2; /* Destination Multi Block Transfer Type. */
        volatile u32 rsvd_4 : 28;

        volatile u32 tt_fc : 3;        /* Transfer Type and Flow Control. */
        volatile u32 hs_sel_src : 1;   /* Source Software or Hardware Handshaking Select. */
        volatile u32 hs_sel_dst : 1;   /* Destination Software or Hardware Handshaking Select. */
        volatile u32 src_hwhs_pol : 1; /* Source Hardware Handshaking Interface Polarity. */
        volatile u32 dst_hwhs_pol : 1; /* Destination Hardware Handshaking Interface Polarity. */
        volatile u32 src_per : 4;
        volatile u32 rsvd_42 : 1;
        volatile u32 dst_per : 4;
        volatile u32 rsvd_48 : 1;
        volatile u32 ch_pri : 3;      /* Channel Priority 0-7 */
        volatile u32 lock_ch : 1;     /* Channel Lock bit */
        volatile u32 lock_ch_l : 2;   /* Channel Lock Level */
        volatile u32 src_osr_lmt : 4; /* Source Outstanding Request Limit */
        volatile u32 dst_osr_lmt : 4; /* Channel Lock Level */
        volatile u32 rsvd_63 : 1;     /* DMAC Channel_x Transfer Configuration Register */
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_cfg_t;
/* 0x128 + x(x-1)*0x100 */
typedef union {
    struct
    {
        volatile u64 lms : 1; /* DMAX_CH(x)_LMS == 2) ? "read-write" : "read-only"} */
        volatile u64 rsvd : 5;
        volatile u64 loc : 58; /* Starting Address Memory of LLI block */
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_llp_t;

/* 0x130 + x(x-1)*0x100 Channel_x Status Register contains fields that indicate the status of DMA transfers for Channelx. */
typedef union {
    struct
    {
        volatile u32 cmp_blk_tr : 22; /* Completed Block Transfer Size. */
        volatile u32 rsvd : 10;
        volatile u32 data_in_fifo : 15; /* Data Left in FIFO. */
        volatile u32 rsvd_47 : 17;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_stat_t;

/* 0x180 Writing 1 to specific field enables the corresponding interrupt status generation in Channelx Interrupt Status Register(CH1_IntStatusReg). */
typedef union {
    struct
    {
        /* data */
        volatile u32 block_tr_done : 1; /* Block Transfer Done Interrupt Status Enable. */
        volatile u32 dma_tr_done : 1;   /* DMA Transfer Done Interrupt Status Enable. */
        volatile u32 rvsd_2 : 1;
        volatile u32 src_tr_comp : 1;             /* Source Transaction Completed Status Enable. */
        volatile u32 dst_tr_comp : 1;             /* Destination Transaction Completed Status Enable. */
        volatile u32 src_dec_err : 1;             /* Source Decode Error Status Enable. */
        volatile u32 dst_dec_err : 1;             /* Destination Decode Error Status Enable. */
        volatile u32 src_slv_err : 1;             /* Source Slave Error Status Enable. */
        volatile u32 dst_slv_err : 1;             /* Destination Slave Error Status Enable. */
        volatile u32 lli_rd_dec_err : 1;          /* LLI Read Decode Error Status Enable. */
        volatile u32 lli_wr_dec_err : 1;          /* LLI WRITE Decode Error Status Enable. */
        volatile u32 lli_rd_slv_err : 1;          /* LLI Read Slave Error Status Enable. */
        volatile u32 lli_wr_slv_err : 1;          /* LLI WRITE Slave Error Status Enable. */
        volatile u32 shadow_lli_invalid_err : 1;  /* Shadow register or LLI Invalid Error Status Enable. */
        volatile u32 slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type Error Signal Enable. */
        volatile u32 rsvd_15 : 1;
        volatile u32 slvif_dec_err : 1;             /* Slave Interface Decode Error Signal Enable. */
        volatile u32 slvif_wr2ro_err : 1;           /* Slave Interface Write to Read Only Error Signal Enable. */
        volatile u32 slvif_rd2wo_err : 1;           /* Slave Interface Read to write Only Error Signal Enable. */
        volatile u32 slvif_wr_on_chan_err : 1;      /* Slave Interface Write On Channel Enabled Error Signal Enable. */
        volatile u32 slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid Error Signal Enable. */
        volatile u32 slvif_wr_onhold_err : 1;       /* Slave Interface Write On Hold Error Signal Enable. */
        volatile u32 rsvd_22 : 5;
        volatile u32 ch_lock_cleared : 1;  /* Channel Lock Cleared Signal Enable. */
        volatile u32 ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile u32 ch_suspended : 1;     /* Channel Suspended Signal Enable. */
        volatile u32 ch_disabled : 1;      /* Channel Disabled Signal Enable. */
        volatile u32 ch_aborted : 1;       /* Channel Aborted Signal Enable. */
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_int_stat_en_t;

typedef union {
    struct
    {
        /* data */
        volatile u32 block_tr_done : 1; /* Block Transfer Done Interrupt Status Enable. */
        volatile u32 dma_tr_done : 1;   /* DMA Transfer Done Interrupt Status Enable. */
        volatile u32 rvsd_2 : 1;
        volatile u32 src_tr_comp : 1;             /* Source Transaction Completed Status Enable. */
        volatile u32 dst_tr_comp : 1;             /* Destination Transaction Completed Status Enable. */
        volatile u32 src_dec_err : 1;             /* Source Decode Error Status Enable. */
        volatile u32 dst_dec_err : 1;             /* Destination Decode Error Status Enable. */
        volatile u32 src_slv_err : 1;             /* Source Slave Error Status Enable. */
        volatile u32 dst_slv_err : 1;             /* Destination Slave Error Status Enable. */
        volatile u32 lli_rd_dec_err : 1;          /* LLI Read Decode Error Status Enable. */
        volatile u32 lli_wr_dec_err : 1;          /* LLI WRITE Decode Error Status Enable. */
        volatile u32 lli_rd_slv_err : 1;          /* LLI Read Slave Error Status Enable. */
        volatile u32 lli_wr_slv_err : 1;          /* LLI WRITE Slave Error Status Enable. */
        volatile u32 shadow_lli_invalid_err : 1;  /* Shadow register or LLI Invalid Error Status Enable. */
        volatile u32 slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type Error Signal Enable. */
        volatile u32 rsvd_15 : 1;
        volatile u32 slvif_dec_err : 1;             /* Slave Interface Decode Error Signal Enable. */
        volatile u32 slvif_wr2ro_err : 1;           /* Slave Interface Write to Read Only Error Signal Enable. */
        volatile u32 slvif_rd2wo_err : 1;           /* Slave Interface Read to write Only Error Signal Enable. */
        volatile u32 slvif_wr_on_chan_err : 1;      /* Slave Interface Write On Channel Enabled Error Signal Enable. */
        volatile u32 slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid Error Signal Enable. */
        volatile u32 slvif_wr_onhold_err : 1;       /* Slave Interface Write On Hold Error Signal Enable. */
        volatile u32 rsvd_22 : 5;
        volatile u32 ch_lock_cleared : 1;  /* Channel Lock Cleared Signal Enable. */
        volatile u32 ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile u32 ch_suspended : 1;     /* Channel Suspended Signal Enable. */
        volatile u32 ch_disabled : 1;      /* Channel Disabled Signal Enable. */
        volatile u32 ch_aborted : 1;       /* Channel Aborted Signal Enable. */
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };

} ch_int_stat_t;

/* 0x190 This register contains fields that are used to enable the generation of port level interrupt at the channel level. */
typedef union {
    struct
    {
        /* data */
        volatile u32 block_tr_done : 1; /* Block Transfer Done Interrupt Status Enable. */
        volatile u32 dma_tr_done : 1;   /* DMA Transfer Done Interrupt Status Enable. */
        volatile u32 rvsd_2 : 1;
        volatile u32 src_tr_comp : 1;             /* Source Transaction Completed Status Enable. */
        volatile u32 dst_tr_comp : 1;             /* Destination Transaction Completed Status Enable. */
        volatile u32 src_dec_err : 1;             /* Source Decode Error Status Enable. */
        volatile u32 dst_dec_err : 1;             /* Destination Decode Error Status Enable. */
        volatile u32 src_slv_err : 1;             /* Source Slave Error Status Enable. */
        volatile u32 dst_slv_err : 1;             /* Destination Slave Error Status Enable. */
        volatile u32 lli_rd_dec_err : 1;          /* LLI Read Decode Error Status Enable. */
        volatile u32 lli_wr_dec_err : 1;          /* LLI WRITE Decode Error Status Enable. */
        volatile u32 lli_rd_slv_err : 1;          /* LLI Read Slave Error Status Enable. */
        volatile u32 lli_wr_slv_err : 1;          /* LLI WRITE Slave Error Status Enable. */
        volatile u32 shadow_lli_invalid_err : 1;  /* Shadow register or LLI Invalid Error Status Enable. */
        volatile u32 slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type Error Signal Enable. */
        volatile u32 rsvd_15 : 1;
        volatile u32 slvif_dec_err : 1;             /* Slave Interface Decode Error Signal Enable. */
        volatile u32 slvif_wr2ro_err : 1;           /* Slave Interface Write to Read Only Error Signal Enable. */
        volatile u32 slvif_rd2wo_err : 1;           /* Slave Interface Read to write Only Error Signal Enable. */
        volatile u32 slvif_wr_on_chan_err : 1;      /* Slave Interface Write On Channel Enabled Error Signal Enable. */
        volatile u32 slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid Error Signal Enable. */
        volatile u32 slvif_wr_onhold_err : 1;       /* Slave Interface Write On Hold Error Signal Enable. */
        volatile u32 rsvd_22 : 5;
        volatile u32 ch_lock_cleared : 1;  /* Channel Lock Cleared Signal Enable. */
        volatile u32 ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile u32 ch_suspended : 1;     /* Channel Suspended Signal Enable. */
        volatile u32 ch_disabled : 1;      /* Channel Disabled Signal Enable. */
        volatile u32 ch_aborted : 1;       /* Channel Aborted Signal Enable. */
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} dmac_ch_int_sig_en_t;

/* 0x198 */
typedef union {
    struct
    {
        /* data */
        volatile u32 block_tr_done : 1; /* Block Transfer Done Interrupt Status Enable. */
        volatile u32 dma_tr_done : 1;   /* DMA Transfer Done Interrupt Status Enable. */
        volatile u32 rvsd_2 : 1;
        volatile u32 src_tr_comp : 1;             /* Source Transaction Completed Status Enable. */
        volatile u32 dst_tr_comp : 1;             /* Destination Transaction Completed Status Enable. */
        volatile u32 src_dec_err : 1;             /* Source Decode Error Status Enable. */
        volatile u32 dst_dec_err : 1;             /* Destination Decode Error Status Enable. */
        volatile u32 src_slv_err : 1;             /* Source Slave Error Status Enable. */
        volatile u32 dst_slv_err : 1;             /* Destination Slave Error Status Enable. */
        volatile u32 lli_rd_dec_err : 1;          /* LLI Read Decode Error Status Enable. */
        volatile u32 lli_wr_dec_err : 1;          /* LLI WRITE Decode Error Status Enable. */
        volatile u32 lli_rd_slv_err : 1;          /* LLI Read Slave Error Status Enable. */
        volatile u32 lli_wr_slv_err : 1;          /* LLI WRITE Slave Error Status Enable. */
        volatile u32 shadow_lli_invalid_err : 1;  /* Shadow register or LLI Invalid Error Status Enable. */
        volatile u32 slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type Error Signal Enable. */
        volatile u32 rsvd_15 : 1;
        volatile u32 slvif_dec_err : 1;             /* Slave Interface Decode Error Signal Enable. */
        volatile u32 slvif_wr2ro_err : 1;           /* Slave Interface Write to Read Only Error Signal Enable. */
        volatile u32 slvif_rd2wo_err : 1;           /* Slave Interface Read to write Only Error Signal Enable. */
        volatile u32 slvif_wr_on_chan_err : 1;      /* Slave Interface Write On Channel Enabled Error Signal Enable. */
        volatile u32 slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid Error Signal Enable. */
        volatile u32 slvif_wr_onhold_err : 1;       /* Slave Interface Write On Hold Error Signal Enable. */
        volatile u32 rsvd_22 : 5;
        volatile u32 ch_lock_cleared : 1;  /* Channel Lock Cleared Signal Enable. */
        volatile u32 ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile u32 ch_suspended : 1;     /* Channel Suspended Signal Enable. */
        volatile u32 ch_disabled : 1;      /* Channel Disabled Signal Enable. */
        volatile u32 ch_aborted : 1;       /* Channel Aborted Signal Enable. */
        volatile u32 rsvd_32 : 32;
    };
    struct
    {
        volatile u32 vl;
        volatile u32 vh;
    };
} ch_int_clr_t;

/* dmac channel */
typedef struct dmac_ch
{
    volatile u32 sar_l; // Source Address Register      0x100 + x(x-1)*0x100
    volatile u32 sar_h;
    volatile u32 dar_l; // Destination Address Register 0x108 + x(x-1)*0x100
    volatile u32 dar_h;
    volatile ch_blk_ts_t blk_ts;    /* block-size. 0x110 + x(x-1)*0x100 */
    volatile ch_ctl_t ctl;          /* Control Register   0x118 + x(x-1)*0x100 */
    volatile ch_cfg_t cfg;          /* Configuration Register */
    volatile ch_llp_t llp;          /*  Linked List Pointer register. */
    volatile ch_stat_t status;      /* Channelx Status Register 0x130 */
    volatile u64 swhssrc;           /* Channelx Software handshake Source Register. */
    volatile u64 swhsdst;           /* Channelx Software handshake Destination Register. */
    volatile u64 blk_tr_resume_req; /* Channelx Block Transfer Resume Request Register. */
    volatile u64 axi_id;            /* Channelx AXI ID Register. */
    volatile u64 axi_qos;           /* Channelx AXI QOS Register. */
    volatile u64 sstat;
    volatile u64 dstat;
    volatile u64 sstatar;                     /* Channelx Source Status Fetch Register. */
    volatile u64 dstatar;                     /* Channelx Destination Status Fetch Register. */
    volatile ch_int_stat_en_t int_stat_en;    /* CHx_INTSTATUS_ENABLEREG */
    volatile ch_int_stat_t int_stat;          /* Channelx Interrupt Status Register captures the Channelx specific interrupts */
    volatile dmac_ch_int_sig_en_t int_sig_en; /* This register contains fields that are used to enable the generation of port level interrupt at the channel level. */
    volatile ch_int_clr_t int_clr;            /* Writing 1 to specific field will clear the corresponding field in Channelx Interrupt Status Register */
    volatile u64 rsvd[12];
} dmac_ch_t;

/* dw dma controller structure */
typedef struct
{
    volatile u64 dmac_id;                                   /* 0x0 */
    volatile u32 dmac_comp_ver;                             /* 0x8 Component Version Register. */
    volatile u32 rsvd_0;                                    /* 0xc */
    volatile dmac_cfg_t dmac_cfg;                           /* 0x10 */
    volatile dmac_ch_en_t dmac_ch_en;                       /* 0x18 */
    volatile u64 dmac_ch_susp;                              /* 0x20 This is DW_axi_dmac Channel Suspend Register. */
    volatile u64 dmac_ch_abort;                             /* 0x28 Channel Abort Register. */
    volatile dmac_int_stat_t dmac_int_stat;                 /* 0x30 DMAC Interrupt Status Register */
    volatile dmac_comm_int_clr_t dmac_comm_int_clr;         /* 0x38 DMAC_COMMONREG_INTCLEARREG */
    volatile dmac_comm_int_stat_en_t dmac_comm_int_stat_en; /* 0x40 DMAC_COMMONREG_INTSTATUS_ENABLEREG */
    volatile dmac_comm_int_sig_en_t dmac_comm_int_sig_en;   /* 0x48 MAC_COMMONREG_INTSIGNAL_ENABLEREG */
    volatile dmac_comm_int_stat_t dmac_comm_int_stat;       /* 0x50 DMAC_COMMONREG_INTSTATUSREG */
    volatile dmac_reset_t dmac_reset;                       /* 0x58 DMAC_RESETREG */
    volatile u64 dmac_low_pm_cfg;                           /* 0x60 DMAC_LOWPOWER_CFGREG */
    volatile u64 rsvd_2[19];                                /* 0x68 */
    volatile dmac_ch_t ch[DW_DMAC_CHANNEL_NUMB];
} dw_dmac_t;
/**
 * @description:
 * Dmac int handle structure call back function for irq.
 */
typedef struct dmac_irq_cbk
{
    void (*dmac_irq_evt_handle)(u32 status, u32 param, void *context);
    void *context;
    enum handler_return (*dmac_irq_err_handle)(u32 err);
    enum handler_return (*dmac_irq_dma_tr_done_handle)(void);
    enum handler_return (*dmac_irq_blk_tr_done_handle)(void);
    enum handler_return (*dmac_irq_src_tr_comp_handle)(void);
    enum handler_return (*dmac_irq_dst_tr_comp_handle)(void);
    enum handler_return (*dmac_irq_ch_lock_clr_handle)(void);      /* Channel Lock Cleared. */
    enum handler_return (*dmac_irq_ch_src_suspended_handle)(void); /* Channel Source Suspended. */
    enum handler_return (*dmac_irq_ch_suspended_handle)(void);     /* Channel Suspended. */
    enum handler_return (*dmac_irq_ch_disabled_handle)(void);      /* Channel Disabled. */
    enum handler_return (*dmac_irq_ch_aborted_handle)(void);       /* Channel Aborted. */
} dmac_irq_cbk_t;

typedef struct dmac_coeff
{
    u32 dmac_index;          /* choose dma 1~8 */
    u32 chan_id;             /* choose channel 1~8 */
    u64 src_addr;            /* src addr ,64bit for dma */
    u64 dst_addr;            /* dst addr ,64bit for dma */
    u32 block_transfer_size; /* block transfer size  : the number of data (length is set by src_tr_width) in a DMA block transfer (1~n) */
    u32 src_transfer_type;   /* src transter type:single;lli;shadow;reload;continuous */
    u32 dst_transfer_type;   /* dst transfer type:single;lli;shadow;reload;continuous */
    u32 src_msize;           /* src burst transaction length :number of item writtem to dst for each handshake request */
    u32 dst_msize;           /* dst burst transaction length :number of item writtem to dst for each handshake request */
    u32 src_tr_width;        /* dst transfer width : map to axi burst awsize */
    u32 dst_tr_width;        /* src transfer width : map to axi burst arsize */
    u32 dma_transfer_type;   /* 1:mem to peripheral;2:peripheral to mem;dma is controller */
    u32 channel_pri;         /* channel priority :0~31 */
    u32 hs_number;           /* choose handshake interface :0~143 */
    u32 arlen;               /* axi read length */
    u32 awlen;               /* axi write length */
    u32 arlen_en;            /* axi read length eb */
    u32 awlen_en;            /* axi write length eb */
    u32 sinc;                /* 0:increase,1:nochange  ---now chip not surpport nochange mode,fix later */
    u32 dinc;                /* 0:increase,1:nochange */
    u32 hs_sel_src;          /* 0:hard-hs,1:soft-hs */
    u32 hs_sel_dst;          /* 0:hard-hs,1:soft-hs */
    u32 dmac_int_en;         /* 1: dmac enable interrupt enable 0:disable interrupt */
    u32 dmac_int_mask;       /* only use in dmac_irq_evt_handle functions. */
    /* LLI related functions */
    u64 llp; /* first linked list pointer. */

    dmac_irq_cbk_t irq_callback; /* dmac irq call back function. */
} dmac_coeff_t;

/* init and set dmac base address. */
void dw_init_dmac(int instance, void *dmac_base, void *dma_mux_addr);
/* config dmac */
bool dw_dmac_cfg(dmac_coeff_t cfg);
/* enable channel */
bool dw_dmac_ch_enable(dmac_coeff_t cfg);
/* disable channel */
bool dw_dmac_ch_disable(dmac_coeff_t cfg);
/* is dw dmac channel enabled. */
bool dw_dmac_is_ch_enabled(dmac_coeff_t cfg);
/* wait dma done */
bool dw_dmac_tr_done(dmac_coeff_t cfg);
/* dmac irq handle function for driver app */
enum handler_return dw_dmac_irq_handle(int instance, int channel, ch_int_stat_t status);
/* print dmac information */
bool dw_get_dmac_status(void);
/* print channel information */
bool dw_dmac_ch_stat(dmac_coeff_t cfg);
/* dmac common register configure. */
bool dw_dmac_reg_cfg(dmac_coeff_t cfg);
/* convert peri addr to dma side */
addr_t dw_dmac_peri_addr_convert(addr_t addr);
/* dmac channel configure. */
bool dw_dmac_ch_reg_cfg(dmac_coeff_t cfg);
/* dmac config mux */
bool dw_dmac_mux_cfg(dmac_coeff_t cfg, DMR_REQ_PERI port, DMR_MUX_DIRECT direct);
/* print dmac reset */
bool dw_dmac_reset(int instance);
/* dmac config mux auto */
/* bool dw_dmac_mux_cfg(dmac_coeff_t cfg); */
/* enable irq */
bool dw_dmac_enable_irq(int instance);
/* disable irq */
bool dw_dmac_disable_irq(int instance);
/* get dw_dmac_get_ch_int_stat */
ch_int_stat_t dw_dmac_get_ch_int_stat(int instance, int chan);
/* get dw_dmac_get_int_stat(int instance); return channel id. */
bool dw_dmac_ch_has_int(int instance, int chan);
/* clear dw_dmac_clr_int_stat */
void dw_dmac_clr_int_stat(int instance, int chan, ch_int_stat_t status);
/* Clear dw chan_ stat */
void dw_clear_chan_stat(dmac_coeff_t cfg);
/* get src address. */
addr_t dw_get_transferred_src_addr(dmac_coeff_t cfg);
/* get dst address. */
addr_t dw_get_transferred_dst_addr(dmac_coeff_t cfg);

#endif