/*
 * Lin_Cfg.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _LIN_CFG_H
#define _LIN_CFG_H

#define LIN_PDU_LENGTH          8   /* data size */
#define LIN_FRAME_DATA_LENGTH   9   /* data(8) + checksum(1) */

/* Break & Sync fields */
#define BREAK                   0x00u
#define SYNC                    0x55u

/* UART clock 84M */
#define UART_SCLK           (84000 * 1000)
#define UART_BAUD           5250000

#define LIN_RX_INT          UART_HAL_RX_CHAR_INT_SRC



/* LIN Channels */
typedef enum {
    LIN_IFC_SCI1,
    LIN_IFC_SCI2,
    LIN_IFC_SCI3,
    LIN_IFC_SCI4,
    LIN_IFC_CHN_MAX,
} Lin_ifc_Channel;

typedef enum Lin_ifc_StatusTypeTag {
    LIN_SEND_BREAK              = 0x01u,
    LIN_SEND_SYNC               = 0x02u,
    LIN_SEND_ID                 = 0x03u,
    LIN_SYNC_ERROR              = 0x04u,
    LIN_ID_ERROR                = 0x05u,
    LIN_MASTER_RESPONE_DATA     = 0x06u,
    LIN_SLAVE_RESPONE_DATA      = 0x07u,
    LIN_BUS_SLEEP               = 0x08u,
    LIN_IDLE                    = 0x09u,
    LIN_MASTER_CHECKSUM_ERROR   = 0x0Au,
    LIN_SLAVE_CHECKSUM_ERROR    = 0x0Bu,
    LIN_NOT_READY               = 0x0Cu
} Lin_ifc_StatusType;

typedef enum {
    /** LIN frame operation return value.
     *  Development or production error occurred */
    LIN_NOT_OK,

    /** LIN frame operation return value.
     *  Successful transmission. */
    LIN_TX_OK,

    /** LIN frame operation return value.
     *  Ongoing transmission (Header or Response). */
    LIN_TX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous header transmission such as:
     *  - Mismatch between sent and read back data
     *  - Identifier parity error or
     *  - Physical bus error */
    LIN_TX_HEADER_ERROR,

    /** LIN frame operation return value.
     *  Erroneous response transmission such as:
     *  - Mismatch between sent and read back data
     *  - Physical bus error */
    LIN_TX_ERROR,

    /** LIN frame operation return value.
     *  Reception of correct response. */
    LIN_RX_OK,

    /** LIN frame operation return value. Ongoing reception: at
     *  least one response byte has been received, but the
     *  checksum byte has not been received. */
    LIN_RX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous response reception such as:
     *  - Framing error
     *  - Overrun error
     *  - Checksum error or
     *  - Short response */
    LIN_RX_ERROR,

    /** LIN frame operation return value.
     *  No response byte has been received so far. */
    LIN_RX_NO_RESPONSE,

    /** LIN channel state return value.
     *  Normal operation; the related LIN channel is ready to
     *  transmit next header. No data from previous frame
     *  available (e.g. after initialization) */
    LIN_OPERATIONAL,

    /** LIN channel state return value.
     *  Sleep mode operation; in this mode wake-up detection
     *  from slave nodes is enabled. */
    LIN_CH_SLEEP
} Lin_StatusType;

#endif /* _LIN_CFG_H */
