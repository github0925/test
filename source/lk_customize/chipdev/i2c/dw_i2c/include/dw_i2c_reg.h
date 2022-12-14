/*
 * dw_i2c_reg.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: DWC I2C register info.
 *
 * Revision History:
 * -----------------
 */
#ifndef DW_I2C_REG_H
#define DW_I2C_REG_H

/*
 * IP Ref Info     : REG_DW_I2C
 * RTL version     :
 * */

/* 0x00: IC_CON */
#define IC_CON_SMBUS_PERSISTENT_SLV_ADDR_EN_FIELD_OFFSET 19
#define IC_CON_SMBUS_PERSISTENT_SLV_ADDR_EN_FIELD_SIZE 1
#define IC_CON_SMBUS_ARP_EN_FIELD_OFFSET 18
#define IC_CON_SMBUS_ARP_EN_FIELD_SIZE 1
#define IC_CON_SMBUS_SLAVE_QUICK_EN_FIELD_OFFSET 17
#define IC_CON_SMBUS_SLAVE_QUICK_EN_FIELD_SIZE 1
#define IC_CON_BUS_CLEAR_FEATURE_CTRL_FIELD_OFFSET 11
#define IC_CON_BUS_CLEAR_FEATURE_CTRL_FIELD_SIZE 1
#define IC_CON_STOP_DET_IF_MASTER_ACTIVE_FIELD_OFFSET 10
#define IC_CON_STOP_DET_IF_MASTER_ACTIVE_FIELD_SIZE 1
#define IC_CON_RX_FIFO_FULL_HLD_CTRL_FIELD_OFFSET 9
#define IC_CON_RX_FIFO_FULL_HLD_CTRL_FIELD_SIZE 1
#define IC_CON_TX_EMPTY_CTRL_FIELD_OFFSET 8
#define IC_CON_TX_EMPTY_CTRL_FIELD_SIZE 1
#define IC_CON_STOP_DET_IFADDRESSED_FIELD_OFFSET 7
#define IC_CON_STOP_DET_IFADDRESSED_FIELD_SIZE 1
#define IC_CON_IC_SLAVE_DISABLE_FIELD_OFFSET 6
#define IC_CON_IC_SLAVE_DISABLE_FIELD_SIZE 1
#define IC_CON_IC_RESTART_EN_FIELD_OFFSET 5
#define IC_CON_IC_RESTART_EN_FIELD_SIZE 1
#define IC_CON_IC_10BITADDR_MASTER_FIELD_OFFSET 4
#define IC_CON_IC_10BITADDR_MASTER_FIELD_SIZE 1
#define IC_CON_IC_10BITADDR_SLAVE_FIELD_OFFSET 3
#define IC_CON_IC_10BITADDR_SLAVE_FIELD_SIZE 1
#define IC_CON_SPEED_FIELD_OFFSET 1
#define IC_CON_SPEED_FIELD_SIZE 2
#define IC_CON_MASTER_MODE_FIELD_OFFSET 0
#define IC_CON_MASTER_MODE_FIELD_SIZE 1

/* 0x04: IC_TAR */
#define IC_TAR_SMBUS_QUICK_CMD_FIELD_OFFSET 16
#define IC_TAR_SMBUS_QUICK_CMD_FIELD_SIZE 1
#define IC_TAR_SPECIAL_FIELD_OFFSET 11
#define IC_TAR_SPECIAL_FIELD_SIZE 1
#define IC_TAR_GC_OR_START_FIELD_OFFSET 10
#define IC_TAR_GC_OR_START_FIELD_SIZE 1
#define IC_TAR_IC_TAR_FIELD_OFFSET 0
#define IC_TAR_IC_TAR_FIELD_SIZE 10

/* 0x08: IC_SAR */
#define IC_SAR_IC_SAR_FIELD_OFFSET 0
#define IC_SAR_IC_SAR_FIELD_SIZE 10

/* 0x10: IC_DATA_CMD */
#define IC_DATA_CMD_FIRST_DATA_BYTE_FIELD_OFFSET 11
#define IC_DATA_CMD_FIRST_DATA_BYTE_FIELD_SIZE 1
#define IC_DATA_CMD_RESTART_FIELD_OFFSET 10
#define IC_DATA_CMD_RESTART_FIELD_SIZE 1
#define IC_DATA_CMD_STOP_FIELD_OFFSET 9
#define IC_DATA_CMD_STOP_FIELD_SIZE 1
#define IC_DATA_CMD_CMD_FIELD_OFFSET 8
#define IC_DATA_CMD_CMD_FIELD_SIZE 1
#define IC_DATA_CMD_DAT_FIELD_OFFSET 0
#define IC_DATA_CMD_DAT_FIELD_SIZE 8

/* 0x14: IC_SS_SCL_HCNT */
#define IC_SS_SCL_HCNT_IC_SS_SCL_HCNT_FIELD_OFFSET 0
#define IC_SS_SCL_HCNT_IC_SS_SCL_HCNT_FIELD_SIZE 16

/* 0x18: IC_SS_SCL_LCNT */
#define IC_SS_SCL_LCNT_IC_SS_SCL_LCNT_FIELD_OFFSET 0
#define IC_SS_SCL_LCNT_IC_SS_SCL_LCNT_FIELD_SIZE 16

/* 0x1c: IC_FS_SCL_HCNT */
#define IC_FS_SCL_HCNT_IC_FS_SCL_HCNT_FIELD_OFFSET 0
#define IC_FS_SCL_HCNT_IC_FS_SCL_HCNT_FIELD_SIZE 16

/* 0x20: IC_FS_SCL_LCNT */
#define IC_FS_SCL_LCNT_IC_FS_SCL_LCNT_FIELD_OFFSET 0
#define IC_FS_SCL_LCNT_IC_FS_SCL_LCNT_FIELD_SIZE 16

/* 0x24: IC_HS_SCL_HCNT */
#define IC_HS_SCL_HCNT_IC_HS_SCL_HCNT_FIELD_OFFSET 0
#define IC_HS_SCL_HCNT_IC_HS_SCL_HCNT_FIELD_SIZE 16

/* 0x28: IC_HS_SCL_LCNT */
#define IC_HS_SCL_LCNT_IC_HS_SCL_LCNT_FIELD_OFFSET 0
#define IC_HS_SCL_LCNT_IC_HS_SCL_LCNT_FIELD_SIZE 16

/* 0x2c: IC_INTR_STAT */
#define IC_INTR_STAT_R_SCL_STUCK_AT_LOW_FIELD_OFFSET 14
#define IC_INTR_STAT_R_SCL_STUCK_AT_LOW_FIELD_SIZE 1
#define IC_INTR_STAT_R_MASTER_ON_HOLD_FIELD_OFFSET 13
#define IC_INTR_STAT_R_MASTER_ON_HOLD_FIELD_SIZE 1
#define IC_INTR_STAT_R_RESTART_DET_FIELD_OFFSET 12
#define IC_INTR_STAT_R_RESTART_DET_FIELD_SIZE 1
#define IC_INTR_STAT_R_GEN_CALL_FIELD_OFFSET 11
#define IC_INTR_STAT_R_GEN_CALL_FIELD_SIZE 1
#define IC_INTR_STAT_R_START_DET_FIELD_OFFSET 10
#define IC_INTR_STAT_R_START_DET_FIELD_SIZE 1
#define IC_INTR_STAT_R_STOP_DET_FIELD_OFFSET 9
#define IC_INTR_STAT_R_STOP_DET_FIELD_SIZE 1
#define IC_INTR_STAT_R_ACTIVITY_FIELD_OFFSET 8
#define IC_INTR_STAT_R_ACTIVITY_FIELD_SIZE 1
#define IC_INTR_STAT_R_RX_DONE_FIELD_OFFSET 7
#define IC_INTR_STAT_R_RX_DONE_FIELD_SIZE 1
#define IC_INTR_STAT_R_TX_ABRT_FIELD_OFFSET 6
#define IC_INTR_STAT_R_TX_ABRT_FIELD_SIZE 1
#define IC_INTR_STAT_R_RD_REQ_FIELD_OFFSET 5
#define IC_INTR_STAT_R_RD_REQ_FIELD_SIZE 1
#define IC_INTR_STAT_R_TX_EMPTY_FIELD_OFFSET 4
#define IC_INTR_STAT_R_TX_EMPTY_FIELD_SIZE 1
#define IC_INTR_STAT_R_TX_OVER_FIELD_OFFSET 3
#define IC_INTR_STAT_R_TX_OVER_FIELD_SIZE 1
#define IC_INTR_STAT_R_RX_FULL_FIELD_OFFSET 2
#define IC_INTR_STAT_R_RX_FULL_FIELD_SIZE 1
#define IC_INTR_STAT_R_RX_OVER_FIELD_OFFSET 1
#define IC_INTR_STAT_R_RX_OVER_FIELD_SIZE 1
#define IC_INTR_STAT_R_RX_UNDER_FIELD_OFFSET 0
#define IC_INTR_STAT_R_RX_UNDER_FIELD_SIZE 1

/* 0x30: IC_INTR_MASK */
#define IC_INTR_MASK_M_SCL_STUCK_AT_LOW_FIELD_OFFSET 14
#define IC_INTR_MASK_M_SCL_STUCK_AT_LOW_FIELD_SIZE 1
#define IC_INTR_MASK_M_MASTER_ON_HOLD_READ_ONLY_FIELD_OFFSET 13
#define IC_INTR_MASK_M_MASTER_ON_HOLD_READ_ONLY_FIELD_SIZE 1
#define IC_INTR_MASK_M_RESTART_DET_FIELD_OFFSET 12
#define IC_INTR_MASK_M_RESTART_DET_FIELD_SIZE 1
#define IC_INTR_MASK_M_GEN_CALL_FIELD_OFFSET 11
#define IC_INTR_MASK_M_GEN_CALL_FIELD_SIZE 1
#define IC_INTR_MASK_M_START_DET_FIELD_OFFSET 10
#define IC_INTR_MASK_M_START_DET_FIELD_SIZE 1
#define IC_INTR_MASK_M_STOP_DET_FIELD_OFFSET 9
#define IC_INTR_MASK_M_STOP_DET_FIELD_SIZE 1
#define IC_INTR_MASK_M_ACTIVITY_FIELD_OFFSET 8
#define IC_INTR_MASK_M_ACTIVITY_FIELD_SIZE 1
#define IC_INTR_MASK_M_RX_DONE_FIELD_OFFSET 7
#define IC_INTR_MASK_M_RX_DONE_FIELD_SIZE 1
#define IC_INTR_MASK_M_TX_ABRT_FIELD_OFFSET 6
#define IC_INTR_MASK_M_TX_ABRT_FIELD_SIZE 1
#define IC_INTR_MASK_M_RD_REQ_FIELD_OFFSET 5
#define IC_INTR_MASK_M_RD_REQ_FIELD_SIZE 1
#define IC_INTR_MASK_M_TX_EMPTY_FIELD_OFFSET 4
#define IC_INTR_MASK_M_TX_EMPTY_FIELD_SIZE 1
#define IC_INTR_MASK_M_TX_OVER_FIELD_OFFSET 3
#define IC_INTR_MASK_M_TX_OVER_FIELD_SIZE 1
#define IC_INTR_MASK_M_RX_FULL_FIELD_OFFSET 2
#define IC_INTR_MASK_M_RX_FULL_FIELD_SIZE 1
#define IC_INTR_MASK_M_RX_OVER_FIELD_OFFSET 1
#define IC_INTR_MASK_M_RX_OVER_FIELD_SIZE 1
#define IC_INTR_MASK_M_RX_UNDER_FIELD_OFFSET 0
#define IC_INTR_MASK_M_RX_UNDER_FIELD_SIZE 1

#define DW_IC_INTR_RX_UNDER (1<<IC_INTR_MASK_M_RX_UNDER_FIELD_OFFSET)
#define DW_IC_INTR_RX_OVER  (1<<IC_INTR_MASK_M_RX_OVER_FIELD_OFFSET)
#define DW_IC_INTR_RX_FULL  (1<<IC_INTR_MASK_M_RX_FULL_FIELD_OFFSET)
#define DW_IC_INTR_TX_OVER  (1<<IC_INTR_MASK_M_TX_OVER_FIELD_OFFSET)
#define DW_IC_INTR_TX_EMPTY (1<<IC_INTR_MASK_M_TX_EMPTY_FIELD_OFFSET)
#define DW_IC_INTR_RD_REQ   (1<<IC_INTR_MASK_M_RD_REQ_FIELD_OFFSET)
#define DW_IC_INTR_TX_ABRT  (1<<IC_INTR_MASK_M_TX_ABRT_FIELD_OFFSET)
#define DW_IC_INTR_RX_DONE  (1<<IC_INTR_MASK_M_RX_DONE_FIELD_OFFSET)
#define DW_IC_INTR_ACTIVITY (1<<IC_INTR_MASK_M_ACTIVITY_FIELD_OFFSET)
#define DW_IC_INTR_STOP_DET (1<<IC_INTR_MASK_M_STOP_DET_FIELD_OFFSET)
#define DW_IC_INTR_START_DET    (1<<IC_INTR_MASK_M_START_DET_FIELD_OFFSET)
#define DW_IC_INTR_GEN_CALL (1<<IC_INTR_MASK_M_GEN_CALL_FIELD_OFFSET)
#define DW_IC_INTR_RESTART_DET  (1<<IC_INTR_MASK_M_RESTART_DET_FIELD_OFFSET)

#define DW_IC_INTR_DEFAULT_MASK     (DW_IC_INTR_RX_FULL | \
                     DW_IC_INTR_TX_ABRT | \
                     DW_IC_INTR_STOP_DET)
#define DW_IC_INTR_MASTER_MASK      (DW_IC_INTR_DEFAULT_MASK | \
                     DW_IC_INTR_TX_EMPTY)

/* 0x34: IC_RAW_INTR_STAT */
#define IC_RAW_INTR_STAT_SCL_STUCK_AT_LOW_FIELD_OFFSET 14
#define IC_RAW_INTR_STAT_SCL_STUCK_AT_LOW_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_MASTER_ON_HOLD_FIELD_OFFSET 13
#define IC_RAW_INTR_STAT_MASTER_ON_HOLD_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RESTART_DET_FIELD_OFFSET 12
#define IC_RAW_INTR_STAT_RESTART_DET_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_GEN_CALL_FIELD_OFFSET 11
#define IC_RAW_INTR_STAT_GEN_CALL_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_START_DET_FIELD_OFFSET 10
#define IC_RAW_INTR_STAT_START_DET_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_STOP_DET_FIELD_OFFSET 9
#define IC_RAW_INTR_STAT_STOP_DET_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_ACTIVITY_FIELD_OFFSET 8
#define IC_RAW_INTR_STAT_ACTIVITY_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RX_DONE_FIELD_OFFSET 7
#define IC_RAW_INTR_STAT_RX_DONE_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_TX_ABRT_FIELD_OFFSET 6
#define IC_RAW_INTR_STAT_TX_ABRT_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RD_REQ_FIELD_OFFSET 5
#define IC_RAW_INTR_STAT_RD_REQ_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_TX_EMPTY_FIELD_OFFSET 4
#define IC_RAW_INTR_STAT_TX_EMPTY_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_TX_OVER_FIELD_OFFSET 3
#define IC_RAW_INTR_STAT_TX_OVER_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RX_FULL_FIELD_OFFSET 2
#define IC_RAW_INTR_STAT_RX_FULL_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RX_OVER_FIELD_OFFSET 1
#define IC_RAW_INTR_STAT_RX_OVER_FIELD_SIZE 1
#define IC_RAW_INTR_STAT_RX_UNDER_FIELD_OFFSET 0
#define IC_RAW_INTR_STAT_RX_UNDER_FIELD_SIZE 1

/* 0x38: IC_RX_TL */
#define IC_RX_TL_RX_TL_FIELD_OFFSET 0
#define IC_RX_TL_RX_TL_FIELD_SIZE 8

/* 0x3c: IC_TX_TL */
#define IC_TX_TL_TX_TL_FIELD_OFFSET 0
#define IC_TX_TL_TX_TL_FIELD_SIZE 8

/* 0x40: IC_CLR_INTR */
#define IC_CLR_INTR_CLR_INTR_FIELD_OFFSET 0
#define IC_CLR_INTR_CLR_INTR_FIELD_SIZE 1

/* 0x44: IC_CLR_RX_UNDER */
#define IC_CLR_RX_UNDER_CLR_RX_UNDER_FIELD_OFFSET 0
#define IC_CLR_RX_UNDER_CLR_RX_UNDER_FIELD_SIZE 1

/* 0x48: IC_CLR_RX_OVER */
#define IC_CLR_RX_OVER_CLR_RX_OVER_FIELD_OFFSET 0
#define IC_CLR_RX_OVER_CLR_RX_OVER_FIELD_SIZE 1

/* 0x4c: IC_CLR_TX_OVER */
#define IC_CLR_TX_OVER_CLR_TX_OVER_FIELD_OFFSET 0
#define IC_CLR_TX_OVER_CLR_TX_OVER_FIELD_SIZE 1

/* 0x50: IC_CLR_RD_REQ */
#define IC_CLR_RD_REQ_CLR_RD_REQ_FIELD_OFFSET 0
#define IC_CLR_RD_REQ_CLR_RD_REQ_FIELD_SIZE 1

/* 0x54: IC_CLR_TX_ABRT */
#define IC_CLR_TX_ABRT_CLR_TX_ABRT_FIELD_OFFSET 0
#define IC_CLR_TX_ABRT_CLR_TX_ABRT_FIELD_SIZE 1

/* 0x58: IC_CLR_RX_DONE */
#define IC_CLR_RX_DONE_CLR_RX_DONE_FIELD_OFFSET 0
#define IC_CLR_RX_DONE_CLR_RX_DONE_FIELD_SIZE 1

/* 0x5c: IC_CLR_ACTIVITY */
#define IC_CLR_ACTIVITY_CLR_ACTIVITY_FIELD_OFFSET 0
#define IC_CLR_ACTIVITY_CLR_ACTIVITY_FIELD_SIZE 1

/* 0x60: IC_CLR_STOP_DET */
#define IC_CLR_STOP_DET_CLR_STOP_DET_FIELD_OFFSET 0
#define IC_CLR_STOP_DET_CLR_STOP_DET_FIELD_SIZE 1

/* 0x64: IC_CLR_START_DET */
#define IC_CLR_START_DET_CLR_START_DET_FIELD_OFFSET 0
#define IC_CLR_START_DET_CLR_START_DET_FIELD_SIZE 1

/* 0x68: IC_CLR_GEN_CALL */
#define IC_CLR_GEN_CALL_CLR_GEN_CALL_FIELD_OFFSET 0
#define IC_CLR_GEN_CALL_CLR_GEN_CALL_FIELD_SIZE 1

/* 0x6c: IC_ENABLE */
#define IC_ENABLE_SMBUS_CLK_RESET_FIELD_OFFSET 16
#define IC_ENABLE_SMBUS_CLK_RESET_FIELD_SIZE 1
#define IC_ENABLE_SDA_STUCK_RECOVERY_ENABLE_FIELD_OFFSET 3
#define IC_ENABLE_SDA_STUCK_RECOVERY_ENABLE_FIELD_SIZE 1
#define IC_ENABLE_TX_CMD_BLOCK_FIELD_OFFSET 2
#define IC_ENABLE_TX_CMD_BLOCK_FIELD_SIZE 1
#define IC_ENABLE_ABORT_FIELD_OFFSET 1
#define IC_ENABLE_ABORT_FIELD_SIZE 1
#define IC_ENABLE_ENABLE_FIELD_OFFSET 0
#define IC_ENABLE_ENABLE_FIELD_SIZE 1

/* 0x70: IC_STATUS */
#define IC_STATUS_SMBUS_SLAVE_ADDR_RESOLVED_FIELD_OFFSET 18
#define IC_STATUS_SMBUS_SLAVE_ADDR_RESOLVED_FIELD_SIZE 1
#define IC_STATUS_SMBUS_SLAVE_ADDR_VALID_FIELD_OFFSET 17
#define IC_STATUS_SMBUS_SLAVE_ADDR_VALID_FIELD_SIZE 1
#define IC_STATUS_SMBUS_QUICK_CMD_BIT_FIELD_OFFSET 16
#define IC_STATUS_SMBUS_QUICK_CMD_BIT_FIELD_SIZE 1
#define IC_STATUS_SDA_STUCK_NOT_RECOVERED_FIELD_OFFSET 11
#define IC_STATUS_SDA_STUCK_NOT_RECOVERED_FIELD_SIZE 1
#define IC_STATUS_SLV_ACTIVITY_FIELD_OFFSET 6
#define IC_STATUS_SLV_ACTIVITY_FIELD_SIZE 1
#define IC_STATUS_MST_ACTIVITY_FIELD_OFFSET 5
#define IC_STATUS_MST_ACTIVITY_FIELD_SIZE 1
#define IC_STATUS_RFF_FIELD_OFFSET 4
#define IC_STATUS_RFF_FIELD_SIZE 1
#define IC_STATUS_RFNE_FIELD_OFFSET 3
#define IC_STATUS_RFNE_FIELD_SIZE 1
#define IC_STATUS_TFE_FIELD_OFFSET 2
#define IC_STATUS_TFE_FIELD_SIZE 1
#define IC_STATUS_TFNF_FIELD_OFFSET 1
#define IC_STATUS_TFNF_FIELD_SIZE 1
#define IC_STATUS_ACTIVITY_FIELD_OFFSET 0
#define IC_STATUS_ACTIVITY_FIELD_SIZE 1

/* 0x74: IC_TXFLR */
#define IC_TXFLR_TXFLR_FIELD_OFFSET 0
#define IC_TXFLR_TXFLR_FIELD_SIZE 5

/* 0x78: IC_RXFLR */
#define IC_RXFLR_RXFLR_FIELD_OFFSET 0
#define IC_RXFLR_RXFLR_FIELD_SIZE 5

/* 0x7c: IC_SDA_HOLD */
#define IC_SDA_HOLD_IC_SDA_RX_HOLD_FIELD_OFFSET 16
#define IC_SDA_HOLD_IC_SDA_RX_HOLD_FIELD_SIZE 8
#define IC_SDA_HOLD_IC_SDA_TX_HOLD_FIELD_OFFSET 0
#define IC_SDA_HOLD_IC_SDA_TX_HOLD_FIELD_SIZE 16

/* 0x80: IC_TX_ABRT_SOURCE */
#define IC_TX_ABRT_SOURCE_TX_FLUSH_CNT_FIELD_OFFSET 23
#define IC_TX_ABRT_SOURCE_TX_FLUSH_CNT_FIELD_SIZE 9
#define IC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_FIELD_OFFSET 17
#define IC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_USER_ABRT_FIELD_OFFSET 16
#define IC_TX_ABRT_SOURCE_ABRT_USER_ABRT_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_FIELD_OFFSET 15
#define IC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_FIELD_OFFSET 14
#define IC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_FIELD_OFFSET 13
#define IC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ARB_LOST_FIELD_OFFSET 12
#define IC_TX_ABRT_SOURCE_ARB_LOST_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_FIELD_OFFSET 11
#define IC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_FIELD_OFFSET 10
#define IC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_FIELD_OFFSET 9
#define IC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_FIELD_OFFSET 8
#define IC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_FIELD_OFFSET 7
#define IC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_FIELD_OFFSET 6
#define IC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_GCALL_READ_FIELD_OFFSET 5
#define IC_TX_ABRT_SOURCE_ABRT_GCALL_READ_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_FIELD_OFFSET 4
#define IC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_FIELD_OFFSET 3
#define IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_FIELD_OFFSET 2
#define IC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_FIELD_OFFSET 1
#define IC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_FIELD_SIZE 1
#define IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_FIELD_OFFSET 0
#define IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_FIELD_SIZE 1

/* 0x88: IC_DMA_CR */
#define IC_DMA_CR_TDMAE_FIELD_OFFSET 1
#define IC_DMA_CR_TDMAE_FIELD_SIZE 1
#define IC_DMA_CR_RDMAE_FIELD_OFFSET 0
#define IC_DMA_CR_RDMAE_FIELD_SIZE 1

/* 0x8c: IC_DMA_TDLR */
#define IC_DMA_TDLR_DMATDL_FIELD_OFFSET 0
#define IC_DMA_TDLR_DMATDL_FIELD_SIZE 4

/* 0x90: IC_DMA_RDLR */
#define IC_DMA_RDLR_DMARDL_FIELD_OFFSET 0
#define IC_DMA_RDLR_DMARDL_FIELD_SIZE 4

/* 0x94: IC_SDA_SETUP */
#define IC_SDA_SETUP_SDA_SETUP_FIELD_OFFSET 0
#define IC_SDA_SETUP_SDA_SETUP_FIELD_SIZE 8

/* 0x98: IC_ACK_GENERAL_CALL */
#define IC_ACK_GENERAL_CALL_ACK_GEN_CALL_FIELD_OFFSET 0
#define IC_ACK_GENERAL_CALL_ACK_GEN_CALL_FIELD_SIZE 1

/* 0x9c: IC_ENABLE_STATUS */
#define IC_ENABLE_STATUS_SLV_RX_DATA_LOST_FIELD_OFFSET 2
#define IC_ENABLE_STATUS_SLV_RX_DATA_LOST_FIELD_SIZE 1
#define IC_ENABLE_STATUS_SLV_DISABLED_WHILE_BUSY_FIELD_OFFSET 1
#define IC_ENABLE_STATUS_SLV_DISABLED_WHILE_BUSY_FIELD_SIZE 1
#define IC_ENABLE_STATUS_IC_EN_FIELD_OFFSET 0
#define IC_ENABLE_STATUS_IC_EN_FIELD_SIZE 1

/* 0xa0: IC_FS_SPKLEN */
#define IC_FS_SPKLEN_IC_FS_SPKLEN_FIELD_OFFSET 0
#define IC_FS_SPKLEN_IC_FS_SPKLEN_FIELD_SIZE 8

/* 0xa8: IC_CLR_RESTART_DET */
#define IC_CLR_RESTART_DET_CLR_RESTART_DET_FIELD_OFFSET 0
#define IC_CLR_RESTART_DET_CLR_RESTART_DET_FIELD_SIZE 1

/* 0xac: IC_SCL_STUCK_AT_LOW_TIMEOUT */
#define IC_SCL_STUCK_AT_LOW_TIMEOUT_IC_SCL_STUCK_LOW_TIMEOUT_FIELD_OFFSET 0
#define IC_SCL_STUCK_AT_LOW_TIMEOUT_IC_SCL_STUCK_LOW_TIMEOUT_FIELD_SIZE 32

/* 0xb0: IC_SDA_STUCK_AT_LOW_TIMEOUT */
#define IC_SDA_STUCK_AT_LOW_TIMEOUT_IC_SDA_STUCK_LOW_TIMEOUT_FIELD_OFFSET 0
#define IC_SDA_STUCK_AT_LOW_TIMEOUT_IC_SDA_STUCK_LOW_TIMEOUT_FIELD_SIZE 32

/* 0xb4: IC_CLR_SCL_STUCK_DET */
#define IC_CLR_SCL_STUCK_DET_CLR_SCL_STUCK_DET_FIELD_OFFSET 0
#define IC_CLR_SCL_STUCK_DET_CLR_SCL_STUCK_DET_FIELD_SIZE 1

/* 0xbc: IC_SMBUS_CLK_LOW_SEXT */
#define IC_SMBUS_CLK_LOW_SEXT_SMBUS_CLK_LOW_SEXT_TIMEOUT_FIELD_OFFSET 0
#define IC_SMBUS_CLK_LOW_SEXT_SMBUS_CLK_LOW_SEXT_TIMEOUT_FIELD_SIZE 32

/* 0xc0: IC_SMBUS_CLK_LOW_MEXT */
#define IC_SMBUS_CLK_LOW_MEXT_SMBUS_CLK_LOW_MEXT_TIMEOUT_FIELD_OFFSET 0
#define IC_SMBUS_CLK_LOW_MEXT_SMBUS_CLK_LOW_MEXT_TIMEOUT_FIELD_SIZE 32

/* 0xc4: IC_SMBUS_THIGH_MAX_IDLE_COUNT */
#define IC_SMBUS_THIGH_MAX_IDLE_COUNT_SMBUS_THIGH_MAX_BUS_IDLE_CNT_FIELD_OFFSET 0
#define IC_SMBUS_THIGH_MAX_IDLE_COUNT_SMBUS_THIGH_MAX_BUS_IDLE_CNT_FIELD_SIZE 16

/* 0xc8: IC_SMBUS_INTR_STAT */
#define IC_SMBUS_INTR_STAT_R_SLV_RX_PEC_NACK_FIELD_OFFSET 8
#define IC_SMBUS_INTR_STAT_R_SLV_RX_PEC_NACK_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_ARP_ASSGN_ADDR_CMD_DET_FIELD_OFFSET 7
#define IC_SMBUS_INTR_STAT_R_ARP_ASSGN_ADDR_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_ARP_GET_UDID_CMD_DET_FIELD_OFFSET 6
#define IC_SMBUS_INTR_STAT_R_ARP_GET_UDID_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_ARP_RST_CMD_DET_FIELD_OFFSET 5
#define IC_SMBUS_INTR_STAT_R_ARP_RST_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_ARP_PREPARE_CMD_DET_FIELD_OFFSET 4
#define IC_SMBUS_INTR_STAT_R_ARP_PREPARE_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_HOST_NOTIFY_MST_DET_FIELD_OFFSET 3
#define IC_SMBUS_INTR_STAT_R_HOST_NOTIFY_MST_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_QUICK_CMD_DET_FIELD_OFFSET 2
#define IC_SMBUS_INTR_STAT_R_QUICK_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_MST_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 1
#define IC_SMBUS_INTR_STAT_R_MST_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1
#define IC_SMBUS_INTR_STAT_R_SLV_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 0
#define IC_SMBUS_INTR_STAT_R_SLV_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1

/* 0xcc: IC_SMBUS_INTR_MASK */
#define IC_SMBUS_INTR_MASK_M_SLV_RX_PEC_NACK_FIELD_OFFSET 8
#define IC_SMBUS_INTR_MASK_M_SLV_RX_PEC_NACK_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_ARP_ASSGN_ADDR_CMD_DET_FIELD_OFFSET 7
#define IC_SMBUS_INTR_MASK_M_ARP_ASSGN_ADDR_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_ARP_GET_UDID_CMD_DET_FIELD_OFFSET 6
#define IC_SMBUS_INTR_MASK_M_ARP_GET_UDID_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_ARP_RST_CMD_DET_FIELD_OFFSET 5
#define IC_SMBUS_INTR_MASK_M_ARP_RST_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_ARP_PREPARE_CMD_DET_FIELD_OFFSET 4
#define IC_SMBUS_INTR_MASK_M_ARP_PREPARE_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_HOST_NOTIFY_MST_DET_FIELD_OFFSET 3
#define IC_SMBUS_INTR_MASK_M_HOST_NOTIFY_MST_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_QUICK_CMD_DET_FIELD_OFFSET 2
#define IC_SMBUS_INTR_MASK_M_QUICK_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_MST_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 1
#define IC_SMBUS_INTR_MASK_M_MST_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1
#define IC_SMBUS_INTR_MASK_M_SLV_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 0
#define IC_SMBUS_INTR_MASK_M_SLV_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1

/* 0xd0: IC_SMBUS_RAW_INTR_STAT */
#define IC_SMBUS_RAW_INTR_STAT_SLV_RX_PEC_NACK_FIELD_OFFSET 8
#define IC_SMBUS_RAW_INTR_STAT_SLV_RX_PEC_NACK_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_ARP_ASSGN_ADDR_CMD_DET_FIELD_OFFSET 7
#define IC_SMBUS_RAW_INTR_STAT_ARP_ASSGN_ADDR_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_ARP_GET_UDID_CMD_DET_FIELD_OFFSET 6
#define IC_SMBUS_RAW_INTR_STAT_ARP_GET_UDID_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_ARP_RST_CMD_DET_FIELD_OFFSET 5
#define IC_SMBUS_RAW_INTR_STAT_ARP_RST_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_ARP_PREPARE_CMD_DET_FIELD_OFFSET 4
#define IC_SMBUS_RAW_INTR_STAT_ARP_PREPARE_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_HOST_NTFY_MST_DET_FIELD_OFFSET 3
#define IC_SMBUS_RAW_INTR_STAT_HOST_NTFY_MST_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_QUICK_CMD_DET_FIELD_OFFSET 2
#define IC_SMBUS_RAW_INTR_STAT_QUICK_CMD_DET_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_MST_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 1
#define IC_SMBUS_RAW_INTR_STAT_MST_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1
#define IC_SMBUS_RAW_INTR_STAT_SLV_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 0
#define IC_SMBUS_RAW_INTR_STAT_SLV_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1

/* 0xd4: IC_CLR_SMBUS_INTR */
#define IC_CLR_SMBUS_INTR_CLR_SLV_RX_PEC_NACK_FIELD_OFFSET 8
#define IC_CLR_SMBUS_INTR_CLR_SLV_RX_PEC_NACK_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_ARP_ASSGN_ADDR_CMD_DET_FIELD_OFFSET 7
#define IC_CLR_SMBUS_INTR_CLR_ARP_ASSGN_ADDR_CMD_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_ARP_GET_UDID_CMD_DET_FIELD_OFFSET 6
#define IC_CLR_SMBUS_INTR_CLR_ARP_GET_UDID_CMD_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_ARP_RST_CMD_DET_FIELD_OFFSET 5
#define IC_CLR_SMBUS_INTR_CLR_ARP_RST_CMD_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_ARP_PREPARE_CMD_DET_FIELD_OFFSET 4
#define IC_CLR_SMBUS_INTR_CLR_ARP_PREPARE_CMD_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_HOST_NOTIFY_MST_DET_FIELD_OFFSET 3
#define IC_CLR_SMBUS_INTR_CLR_HOST_NOTIFY_MST_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_QUICK_CMD_DET_FIELD_OFFSET 2
#define IC_CLR_SMBUS_INTR_CLR_QUICK_CMD_DET_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_MST_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 1
#define IC_CLR_SMBUS_INTR_CLR_MST_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1
#define IC_CLR_SMBUS_INTR_CLR_SLV_CLOCK_EXTND_TIMEOUT_FIELD_OFFSET 0
#define IC_CLR_SMBUS_INTR_CLR_SLV_CLOCK_EXTND_TIMEOUT_FIELD_SIZE 1

/* 0xdc: IC_SMBUS_UDID */
#define IC_SMBUS_UDID_LSB_SMBUS_UDID_LSB_FIELD_OFFSET 0
#define IC_SMBUS_UDID_LSB_SMBUS_UDID_LSB_FIELD_SIZE 32

/* 0xf4: IC_COMP_PARAM_1 */
#define IC_COMP_PARAM_1_TX_BUFFER_DEPTH_FIELD_OFFSET 16
#define IC_COMP_PARAM_1_TX_BUFFER_DEPTH_FIELD_SIZE 8
#define IC_COMP_PARAM_1_RX_BUFFER_DEPTH_FIELD_OFFSET 8
#define IC_COMP_PARAM_1_RX_BUFFER_DEPTH_FIELD_SIZE 8
#define IC_COMP_PARAM_1_ADD_ENCODED_PARAMS_FIELD_OFFSET 7
#define IC_COMP_PARAM_1_ADD_ENCODED_PARAMS_FIELD_SIZE 1
#define IC_COMP_PARAM_1_HAS_DMA_FIELD_OFFSET 6
#define IC_COMP_PARAM_1_HAS_DMA_FIELD_SIZE 1
#define IC_COMP_PARAM_1_INTR_IO_FIELD_OFFSET 5
#define IC_COMP_PARAM_1_INTR_IO_FIELD_SIZE 1
#define IC_COMP_PARAM_1_HC_COUNT_VALUES_FIELD_OFFSET 4
#define IC_COMP_PARAM_1_HC_COUNT_VALUES_FIELD_SIZE 1
#define IC_COMP_PARAM_1_MAX_SPEED_MODE_FIELD_OFFSET 2
#define IC_COMP_PARAM_1_MAX_SPEED_MODE_FIELD_SIZE 2
#define IC_COMP_PARAM_1_APB_DATA_WIDTH_FIELD_OFFSET 0
#define IC_COMP_PARAM_1_APB_DATA_WIDTH_FIELD_SIZE 2

/* 0xf8: IC_COMP_VERSION */
#define IC_COMP_VERSION_IC_COMP_VERSION_FIELD_OFFSET 0
#define IC_COMP_VERSION_IC_COMP_VERSION_FIELD_SIZE 32

/* 0xfc: IC_COMP_TYPE */
#define IC_COMP_TYPE_IC_COMP_TYPE_FIELD_OFFSET 0
#define IC_COMP_TYPE_IC_COMP_TYPE_FIELD_SIZE 32


#endif
