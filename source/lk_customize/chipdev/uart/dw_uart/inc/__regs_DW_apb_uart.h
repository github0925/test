#ifndef __REGS_DW_APB_UART_H
#define __REGS_DW_APB_UART_H

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

/* Receive Buffer Register.
Reset Value: 0x0
 This register can be accessed only when the DLAB bit (LCR[7]) is cleared.*/

/** @defgroup Register_Definition
  * @{
  */

typedef union {
	struct {
        uint32_t RBR8:8;    /*bit 0-7 Receive Buffer Register  Access: RO*/
        uint32_t RSVD_RBR24:24;    /*bit 8-31 RBR 31to8 Reserved bits and read as zero (0)  Access: RO*/
    };
    struct {
        uint32_t RBR9:9;    /*bit 0-8 Receive Buffer Register  Access: RO*/
        uint32_t RSVD_RBR23:23;    /*bit 9-31 RBR 31to9 Reserved bits and read as zero (0)  Access: RO*/
    };
    uint32_t v;
} RBR_Type;

typedef union {
    struct {
        uint32_t DLL:8;    /*bit 0-7 Divisor Latch (Low).  Access: RW*/
        uint32_t RSVD_DLL_31to8:24;    /*bit 8-31 DLL 31to8 Reserved bits and read as zero (0)  Access: RO*/
    };
    uint32_t v;
} DLL_Type;

typedef union {
    struct {
        uint32_t THR8:8;    /*bit 0-7 Transmit Holding Register.  Access: W*/
        uint32_t RSVD_THR_31to8:24;    /*bit 8-31 THR 31to8 Reserved bits and read as zero (0)  Access: RO*/
    };
	struct {
        uint32_t THR9:9;    /*bit 0-8 Transmit Holding Register.  Access: W*/
        uint32_t RSVD_THR_31to9:23;    /*bit 9-31 THR 31to9 Reserved bits and read as zero (0)  Access: RO*/
    };
    uint32_t v;
} THR_Type;

typedef union {
    struct {
        uint32_t DLH:8;    /*bit 0-7 Divisor Latch High.  Access: RW*/
        uint32_t RSVD_DLH:24;    /*bit 8-31 DLH 31to8 Reserved bits and read as zero  Access: RO*/
    };
    uint32_t v;
} DLH_Type;

/* ERBFI value description
    0x0: Disable Receive data Interrupt
    0x1: Enable Receive data Interrupt
*/
/* ETBEI value description
    0x0: Disable Transmit empty interrupt
    0x1: Enable Transmit empty interrupt
*/
/* ELSI value description
    0x0: Disable Receiver Line Status Interrupt
    0x1: Enable Receiver Line Status Interrupt
*/
/* EDSSI value description
    0x0: Disable Modem Status Interrupt
    0x1: Enable Modem Status Interrupt
*/
/* ELCOLR value description
    0x0: Disable ALC
    0x1: Enable ALC
*/
/* PTIME value description
    0x0: Disable Programmable THRE Interrupt Mode
    0x1: Enable Programmable THRE Interrupt Mode
*/

typedef union {
    struct {
        uint32_t ERBFI:1;    /*bit 0 Enable Received Data Available Interrupt  Access: RW*/
        uint32_t ETBEI:1;    /*bit 1 Enable Transmit Holding Register Empty Interrupt  Access: RW*/
        uint32_t ELSI:1;    /*bit 2 Enable Receiver Line Status Interrupt  Access: RW*/
        uint32_t EDSSI:1;    /*bit 3 Enable Modem Status Interrupt  Access: RW*/
        uint32_t ELCOLR:1;    /*bit 4 Interrupt Enable Register: ELCOLR, this bit controls the method for clearing the status in the LSR register  Access: RO*/
        uint32_t RSVD_IER_6TO5:2;    /*bit 5-6 IER 6to5 Reserved bits read as zero (0)  Access: RO*/
        uint32_t PTIME:1;    /*bit 7 Programmable THRE Interrupt Mode Enable  Access: RW*/
        uint32_t RSVD_IER_31TO8:24;    /*bit 8-31 IER 31to8 Reserved bits and read as zero (0)  Access: RO*/
    };
    uint32_t v;
} IER_Type;

typedef union {
    struct {
        uint32_t FIFOE:1;    /*bit 0  FIFO Enable.  Access: W*/
		uint32_t RFIFOR:1;    /*bit 1 RCVR FIFO Reset.  Access: W*/
		uint32_t XFIFOR:1;    /*bit 2 XMIT FIFO Reset.  Access: W*/
		uint32_t DMAM:1;    /*bit 1 DMA Mode.  Access: W*/
		uint32_t TET:2;    /*bit 4-5 TX Empty Trigger. Access: W*/
		uint32_t RT:2;    /*bit 6-7 RCVR Trigger  Access: W*/
        uint32_t RSVD_FCR_31to8:24;    /*bit 8-31 FCR 31to8 Reserved bits and read as 0.  Access: RO*/
    };
    uint32_t v;
} FCR_Type;

/* IID value description
    0x0: modem status
    0x1: no interrupt pending
    0x2: THR empty
    0x4: received data available
    0x6: receiver line status
    0x7: busy detect
    0xc: character timeout
*/
/* FIFOSE value description
    0x0: FIFOs are disabled
    0x3: FIFOs are enabled
*/

typedef union {
    struct {
        uint32_t IID:4;    /*bit 0-3 Interrupt ID (or IID)  Access: RO*/
        uint32_t RSVD_IIR_5TO4:2;    /*bit 4-5 IIR 5to4 Reserved bits read as 0  Access: RO*/
        uint32_t FIFOSE:2;    /*bit 6-7 FIFOs Enabled (or FIFOSE)  Access: RO*/
        uint32_t RSVD_IIR_31TO8:24;    /*bit 8-31 IIR 31to8 Reserved bits and read as 0  Access: RO*/
    };
    uint32_t v;
} IIR_Type;

typedef union {
    struct {
        uint32_t DLS:2;    /*bit 0-1 Data Length Select (or CLS as used in legacy)  Access: RW*/
        uint32_t STOP:1;    /*bit 2 Number of stop bits  Access: RW*/
        uint32_t PEN:1;    /*bit 3 Parity Enable  Access: RW*/
        uint32_t EPS:1;    /*bit 4 Even Parity Select  Access: RW*/
        uint32_t SP:1;    /*bit 5 Stick Parity  Access: RW*/
        uint32_t BC:1;    /*bit 6 Break Control Bit  Access: RW*/
        uint32_t DLAB:1;    /*bit 7 Divisor Latch Access Bit  Access: RW*/
        uint32_t RSVD_LCR_31TO8:24;    /*bit 8-31 LCR 31to8 Reserved bits and read as 0  Access: RO*/
    };
    uint32_t v;
} LCR_Type;

/* DLS value description
    0x0: 5 data bits per character
    0x1: 6 data bits per character
    0x2: 7 data bits per character
    0x3: 8 data bits per character
*/
/* STOP value description
    0x0: 1 stop bit
    0x1: 1.5 stop bits when DLS (LCR[1:0]) is zero, else 2 stop bit
*/
/* PEN value description
    0x0: disable parity
    0x1: enable parity
*/
/* EPS value description
    0x0: an odd parity is transmitted or checked
    0x1: an even parity is transmitted or checked
*/
/* SP value description
    0x0: Stick parity disabled
    0x1: Stick parity enabled
*/
/* BC value description
    0x0: Serial output is released for data transmission
    0x1: Serial output is forced to spacing state
*/
/* DLAB value description
    0x0: Divisor Latch register is writable only when UART Not BUSY
    0x1: Divisor Latch register is always readable and writable
*/
typedef union {
    struct {
        uint32_t DTR:1;    /*bit 0 Data Terminal Ready  Access: RW*/
        uint32_t RTS:1;    /*bit 1 Request to Send  Access: RW*/
        uint32_t OUT1:1;    /*bit 2 OUT1  Access: RW*/
        uint32_t OUT2:1;    /*bit 3 OUT2  Access: RW*/
        uint32_t LOOPBACK:1;    /*bit 4 LoopBack Bit  Access: RW*/
        uint32_t AFCE:1;    /*bit 5 Auto Flow Control Enable  Access: RW*/
        uint32_t SIRE:1;    /*bit 6 SIR Mode Enable  Access: RW*/
        uint32_t RSVD_MCR_31TO7:25;    /*bit 7-31 MCR 31to7 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} MCR_Type;

/* DTR value description
    0x0: dtr_n de-asserted (logic1)
    0x1: dtr_n asserted (logic 0)
*/
/* RTS value description
    0x0: Request to Send rts_n de-asserted (logic 1)
    0x1: Request to Send rts_n asserted (logic 0)
*/
/* OUT1 value description
    0x0: out1_n de-asserted (logic 1)
    0x1: out1_n asserted (logic 0)
*/
/* OUT2 value description
    0x0: out2_n de-asserted (logic 1)
    0x1: out2_n asserted (logic 0)
*/
/* LOOPBACK value description
    0x0: Loopback mode disabled
    0x1: Loopback mode enabled
*/
/* AFCE value description
    0x0: Auto Flow Control Mode disabled
    0x1: Auto Flow Control Mode enabled
*/
/* SIRE value description
    0x0: IrDA SIR Mode disabled
    0x1: IrDA SIR Mode enabled
*/
typedef union {
    struct {
        uint32_t DR:1;    /*bit 0 Data Ready bit  Access: RO*/
        uint32_t OE:1;    /*bit 1 Overrun error bit  Access: RO*/
        uint32_t PE:1;    /*bit 2 Parity Error bit  Access: RO*/
        uint32_t FE:1;    /*bit 3 Framing Error bit  Access: RO*/
        uint32_t BI:1;    /*bit 4 Break Interrupt bit  Access: RO*/
        uint32_t THRE:1;    /*bit 5 Transmit Holding Register Empty bit  Access: RO*/
        uint32_t TEMT:1;    /*bit 6 Transmitter Empty bit  Access: RO*/
        uint32_t RFE:1;    /*bit 7 Receiver FIFO Error bit  Access: RO*/
        uint32_t ADDR_RCVD:1;    /*bit 8 Address Received Bit  Access: RO*/
        uint32_t RSVD_LSR_31TO9:23;    /*bit 9-31 LSR 31to9 Reserved bits read as zero  Access: RO*/
    };
    uint32_t v;
} LSR_Type;

/* DR value description
    0x0: data not ready
    0x1: data ready
*/
/* OE value description
    0x0: no overrun error
    0x1: overrun error
*/
/* PE value description
    0x0: no parity error
    0x1: parity error
*/
/* FE value description
    0x0: no framing error
    0x1: framing error
*/
/* BI value description
    0x0: No break sequence detected
    0x1: Break sequence detected
*/
/* THRE value description
    0x0: THRE interrupt control is disabled
    0x1: THRE interrupt control is enabled
*/
/* TEMT value description
    0x0: Transmitter not empty
    0x1: Transmitter empty
*/
/* RFE value description
    0x0: No error in RX FIFO
    0x1: Error in RX FIFO
*/
typedef union {
    struct {
        uint32_t DCTS:1;    /*bit 0 Delta Clear to Send  Access: RO*/
        uint32_t DDSR:1;    /*bit 1 Delta Data Set Ready  Access: RO*/
        uint32_t TERI:1;    /*bit 2 Trailing Edge of Ring Indicator  Access: RO*/
        uint32_t DDCD:1;    /*bit 3 Delta Data Carrier Detect  Access: RO*/
        uint32_t CTS:1;    /*bit 4 Clear to Send  Access: RO*/
        uint32_t DSR:1;    /*bit 5 Data Set Ready  Access: RO*/
        uint32_t RI:1;    /*bit 6 Ring Indicator  Access: RO*/
        uint32_t DCD:1;    /*bit 7 Data Carrier Detect  Access: RO*/
        uint32_t RSVD_MSR_31TO8:24;    /*bit 8-31 MSR 31to8 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} MSR_Type;

/* DCTS value description
    0x0: no change on cts_n since last read of MSR
    0x1: change on cts_n since last read of MSR
*/
/* DDSR value description
    0x0: no change on dsr_n since last read of MSR
    0x1: change on dsr_n since last read of MSR
*/
/* TERI value description
    0x0: no change on ri_n since last read of MSR
    0x1: change on ri_n since last read of MSR
*/
/* DDCD value description
    0x0: No change on dcd_n since last read of MSR
    0x1: change on dcd_n since last read of MSR
*/
/* CTS value description
    0x0: cts_n input is de-asserted (logic 1)
    0x1: cts_n input is asserted (logic 0)
*/
/* DSR value description
    0x0: dsr_n input is de-asserted (logic 1)
    0x1: dsr_n input is asserted (logic 0)
*/
/* RI value description
    0x0: ri_n input is de-asserted (logic 1)
    0x1: ri_n input is asserted (logic 0)
*/
/* DCD value description
    0x0: dcd_n input is de-asserted (logic 1)
    0x1: dcd_n input is asserted (logic 0)
*/
typedef union {
    struct {
        uint32_t SCR:8;    /*bit 0-7 This register is for programmers to use as a temporary storage space  Access: RW*/
        uint32_t RSVD_SCR_31TO8:24;    /*bit 8-31 SCR 31to8 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SCR_Type;

typedef union {
    struct {
        uint32_t SRBRN:9;    /*bit 0-8 Shadow Receive Buffer Register n  Access: RO*/
        uint32_t RSVD_SRBRN:23;    /*bit 9-31 SRBR0 31 to SRBRN_REG_SIZE Reserved bits read read as 0  Access: RO*/
    };
    uint32_t v;
} SRBRX_Type;

typedef union {
    struct {
        uint32_t FAR:1;    /*bit 0 Writes will have no effect when FIFO_ACCESS == No, always readable  Access: RW*/
        uint32_t RSVD_FAR_31TO1:31;    /*bit 1-31 FAR 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} FAR_Type;

/* FAR value description
    0x0: FIFO access mode disabled
    0x1: FIFO access mode enabled
*/
typedef union {
    struct {
        uint32_t TFR:8;    /*bit 0-7 Transmit FIFO Read  Access: RO*/
        uint32_t RSVD_TFR_31TO8:24;    /*bit 8-31 TFR 31to8 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} TFR_Type;

typedef union {
    struct {
        uint32_t RFWD:8;    /*bit 0-7 Receive FIFO Write Data  Access: WO*/
        uint32_t RFPE:1;    /*bit 8 Receive FIFO Parity Error  Access: WO*/
        uint32_t RFFE:1;    /*bit 9 Receive FIFO Framing Error  Access: WO*/
        uint32_t RSVD_RFW_31TO10:22;    /*bit 10-31 RFW 31to10 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} RFW_Type;

/* RFPE value description
    0x0: Parity error disabled
    0x1: Parity error enabled
*/
/* RFFE value description
    0x0: Frame error disabled
    0x1: Frame error enabled
*/
typedef union {
    struct {
        uint32_t RSVD_BUSY:1;    /*bit 0 UART Busy  Access: RO*/
        uint32_t TFNF:1;    /*bit 1 Transmit FIFO Not Full  Access: RO*/
        uint32_t TFE:1;    /*bit 2 Transmit FIFO Empty  Access: RO*/
        uint32_t RFNE:1;    /*bit 3 Receive FIFO Not Empty  Access: RO*/
        uint32_t RFF:1;    /*bit 4 Receive FIFO Full  Access: RO*/
        uint32_t RSVD_USR_31TO5:27;    /*bit 5-31 USR 31to5 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} USR_Type;

/* RSVD_BUSY value description
    0x0: DW_apb_uart is idle or inactive
    0x1: DW_apb_uart is busy (actively transferring data)
*/
/* TFNF value description
    0x0: Transmit FIFO is full
    0x1: Transmit FIFO is not full
*/
/* TFE value description
    0x0: Transmit FIFO is not empty
    0x1: Transmit FIFO is empty
*/
/* RFNE value description
    0x0: Receive FIFO is empty
    0x1: Receive FIFO is not empty
*/
/* RFF value description
    0x0: Receive FIFO not full
    0x1: Receive FIFO full
*/
typedef union {
    struct {
        uint32_t TFL:6;    /*bit 0-5 Transmit FIFO Level  Access: RO*/
        uint32_t RSVD_TFL_31TOADDR_WIDTH:26;    /*bit 6-31 TFL 31 to ADDR_WIDTH Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} TFL_Type;

typedef union {
    struct {
        uint32_t RFL:6;    /*bit 0-5 Receive FIFO Level  Access: RO*/
        uint32_t RSVD_RFL_31TOADDR_WIDTH:26;    /*bit 6-31 RFL 31 to ADDR_WIDTH Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} RFL_Type;

typedef union {
    struct {
        uint32_t UR:1;    /*bit 0 UART Reset  Access: WO*/
        uint32_t RFR:1;    /*bit 1 RCVR FIFO Reset  Access: WO*/
        uint32_t XFR:1;    /*bit 2 XMIT FIFO Reset  Access: WO*/
        uint32_t RSVD_SRR_31TO3:29;    /*bit 3-31 SRR 31to3 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SRR_Type;

/* UR value description
    0x0: No Uart Reset
    0x1: Uart reset
*/
typedef union {
    struct {
        uint32_t SRTS:1;    /*bit 0 Shadow Request to Send  Access: RW*/
        uint32_t RSVD_SRTS_31TO1:31;    /*bit 1-31 SRTS 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SRTS_Type;

/* SRTS value description
    0x0: Shadow Request to Send uart_rts_n logic1
    0x1: Shadow Request to Send uart_rts_n logic0
*/
typedef union {
    struct {
        uint32_t SBCB:1;    /*bit 0 Shadow Break Control Bit  Access: RW*/
        uint32_t RSVD_SBCR_31TO1:31;    /*bit 1-31 SBCR 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SBCR_Type;

/* SBCB value description
    0x0: No spacing on serial output
    0x1: Serial output forced to the spacing
*/
typedef union {
    struct {
        uint32_t SDMAM:1;    /*bit 0 Shadow DMA Mode  Access: RW*/
        uint32_t RSVD_SDMAM_31TO1:31;    /*bit 1-31 SDMAM 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SDMAM_Type;

/* SDMAM value description
    0x0: Mode 0
    0x1: Mode 1
*/
typedef union {
    struct {
        uint32_t SFE:1;    /*bit 0 Shadow FIFO Enable  Access: RW*/
        uint32_t RSVD_SFE_31TO1:31;    /*bit 1-31 SFE 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SFE_Type;

/* SFE value description
    0x0: FIFOs are disabled
    0x1: FIFOs are enabled
*/
typedef union {
    struct {
        uint32_t SRT:2;    /*bit 0-1 Shadow RCVR Trigger  Access: RW*/
        uint32_t RSVD_SRT_31TO2:30;    /*bit 2-31 SRT 31to2 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} SRT_Type;

/* SRT value description
    0x0: 1 character in FIFO
    0x1: FIFO 1/4 full
    0x2: FIFO 1/2 full
    0x3: FIFO 2 less than full
*/
typedef union {
    struct {
        uint32_t STET:2;    /*bit 0-1 Shadow TX Empty Trigger  Access: RW*/
        uint32_t RSVD_STET_31TO2:30;    /*bit 2-31 STET 31to2 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} STET_Type;

/* STET value description
    0x0: FIFO empty
    0x1: 2 characters in FIFO
    0x2: FIFO 1/4 full
    0x3: FIFO 1/2 full
*/
typedef union {
    struct {
        uint32_t HTX:1;    /*bit 0 Halt TX  Access: RW*/
        uint32_t RSVD_HTX_31TO1:31;    /*bit 1-31 HTX 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} HTX_Type;

/* HTX value description
    0x0: Halt Transmission disabled
    0x1: Halt Transmission enabled
*/
typedef union {
    struct {
        uint32_t DMASA:1;    /*bit 0 DMA Software Acknowledge  Access: WO*/
        uint32_t RSVD_DMASA_31TO1:31;    /*bit 1-31 DMASA 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} DMASA_Type;

/* DMASA value description
    0x1: DMA software acknowledge
*/
typedef union {
    struct {
        uint32_t RS485_EN:1;    /*bit 0 RS485 Transfer Enable  Access: RW*/
        uint32_t RE_POL:1;    /*bit 1 Receiver Enable Polarity  Access: RW*/
        uint32_t DE_POL:1;    /*bit 2 Driver Enable Polarity  Access: RW*/
        uint32_t XFER_MODE:2;    /*bit 3-4 Transfer Mode  Access: RW*/
        uint32_t RSVD_TCR_31TO5:27;    /*bit 5-31 TCR 31to5 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} TCR_Type;

typedef union {
    struct {
        uint32_t DE_ENABLE:1;    /*bit 0 DE Enable control  Access: RW*/
        uint32_t RSVD_DE_EN_31TO1:31;    /*bit 1-31 DE_EN 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} DE_EN_Type;

typedef union {
    struct {
        uint32_t RE_ENABLE:1;    /*bit 0 RE Enable control  Access: RW*/
        uint32_t RSVD_RE_EN_31TO1:31;    /*bit 1-31 RE_EN 31to1 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} RE_EN_Type;

typedef union {
    struct {
        uint32_t DE_ASSERTION_TIME:8;    /*bit 0-7 Driver Enable assertion time  Access: RW*/
        uint32_t RSVD_DE_AT_15TO8:8;    /*bit 8-15 DET 15to8 Reserved bits read as 0  Access: RO*/
        uint32_t DE_DEASSERTION_TIME:8;    /*bit 16-23 Driver Enable de-assertion time  Access: RW*/
        uint32_t RSVD_DE_DEAT_31TO24:8;    /*bit 24-31 DET 31to24 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} DET_Type;

typedef union {
    struct {
        uint32_t DE_TO_RE:16;    /*bit 0-15 Driver Enable to Receiver Enable TurnAround time  Access: RW*/
        uint32_t RE_TO_DE:16;    /*bit 16-31 Receiver Enable to Driver Enable TurnAround time  Access: RW*/
    };
    uint32_t v;
} TAT_Type;

typedef union {
    struct {
        uint32_t DLF:4;    /*bit 0-3 Fractional part of divisor  Access: RW*/
        uint32_t RSVD_DLF:28;    /*bit 4-31 DLF 31 to DLF_SIZE Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} DLF_Type;

typedef union {
    struct {
        uint32_t RAR:8;    /*bit 0-7 This is an address matching register during receive mode  Access: RW*/
        uint32_t RSVD_RAR_31TO8:24;    /*bit 8-31 RAR 31to8 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} RAR_Type;

typedef union {
    struct {
        uint32_t TAR:8;    /*bit 0-7 This is an address matching register during transmit mode  Access: RW*/
        uint32_t RSVD_TAR_31TO8:24;    /*bit 8-31 TAR 31to8 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} TAR_Type;

typedef union {
    struct {
        uint32_t DLS_E:1;    /*bit 0 Extension for DLS  Access: RW*/
        uint32_t ADDR_MATCH:1;    /*bit 1 Address Match Mode  Access: RW*/
        uint32_t SEND_ADDR:1;    /*bit 2 Send address control bit  Access: RW*/
        uint32_t TRANSMIT_MODE:1;    /*bit 3 Transmit mode control bit  Access: RW*/
        uint32_t RSVD_LCR_EXT:28;    /*bit 4-31 LCR_EXT 31to4 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} LCR_EXT_Type;

typedef union {
    struct {
        uint32_t APB_DATA_WIDTH:2;    /*bit 0-1 Encoding of APB_DATA_WIDTH configuration parameter value  Access: RO*/
        uint32_t RSVD_CPR_3TO2:2;    /*bit 2-3 CPR 3to2 Reserved bits read as 0  Access: RO*/
        uint32_t AFCE_MODE:1;    /*bit 4 Encoding of AFCE_MODE configuration parameter value  Access: RO*/
        uint32_t THRE_MODE:1;    /*bit 5 Encoding of THRE_MODE configuration parameter value  Access: RO*/
        uint32_t SIR_MODE:1;    /*bit 6 Encoding of SIR_MODE configuration parameter value  Access: RO*/
        uint32_t SIR_LP_MODE:1;    /*bit 7 Encoding of SIR_LP_MODE configuration parameter value  Access: RO*/
        uint32_t ADDITIONAL_FEAT:1;    /*bit 8 Encoding of ADDITIONAL_FEATURES configuration parameter value  Access: RO*/
        uint32_t FIFO_ACCESS:1;    /*bit 9 Encoding of FIFO_ACCESS configuration parameter value  Access: RO*/
        uint32_t FIFO_STAT:1;    /*bit 10 Encoding of FIFO_STAT configuration parameter value  Access: RO*/
        uint32_t SHADOW:1;    /*bit 11 Encoding of SHADOW configuration parameter value  Access: RO*/
        uint32_t UART_ADD_ENCODED_PARAMS:1;    /*bit 12 Encoding of UART_ADD_ENCODED_PARAMS configuration parameter value  Access: RO*/
        uint32_t DMA_EXTRA:1;    /*bit 13 Encoding of DMA_EXTRA configuration parameter value  Access: RO*/
        uint32_t RSVD_CPR_15TO14:2;    /*bit 14-15 CPR 15to14 Reserved bits read as 0  Access: RO*/
        uint32_t FIFO_MODE:8;    /*bit 16-23 Encoding of FIFO_MODE configuration parameter value  Access: RO*/
        uint32_t RSVD_CPR_31TO24:8;    /*bit 24-31 CPR 31to24 Reserved bits read as 0  Access: RO*/
    };
    uint32_t v;
} CPR_Type;

/* APB_DATA_WIDTH value description
    0x0: APB data width is 8 bits
    0x1: APB data width is 16 bits
    0x2: APB data width is 32 bits
*/
/* AFCE_MODE value description
    0x0: AFCE mode disabled
    0x1: AFCE mode enabled
*/
/* THRE_MODE value description
    0x0: THRE mode disabled
    0x1: THRE mode enabled
*/
/* SIR_MODE value description
    0x0: SIR mode disabled
    0x1: SIR mode enabled
*/
/* SIR_LP_MODE value description
    0x0: SIR_LP mode disabled
    0x1: SIR_LP mode enabled
*/
/* ADDITIONAL_FEAT value description
    0x0: Additional features disabled
    0x1: Additional features enabled
*/
/* FIFO_ACCESS value description
    0x0: FIFO_ACCESS disabled
    0x1: FIFO ACCESS enabled
*/
/* FIFO_STAT value description
    0x0: FIFO_STAT disabled
    0x1: FIFO_STAT enabled
*/
/* SHADOW value description
    0x0: SHADOW disabled
    0x1: SHADOW enabled
*/
/* UART_ADD_ENCODED_PARAMS value description
    0x0: UART_ADD_ENCODED_PARAMS disabled
    0x1: UART_ADD_ENCODED_PARAMS enabled
*/
/* DMA_EXTRA value description
    0x0: DMA_EXTRA disabled
    0x1: DMA_EXTRA enabled
*/
/* FIFO_MODE value description
    0x0: FIFO mode is 0
    0x1: FIFO mode is 16
    0x2: FIFO mode is 32
    0x4: FIFO mode is 64
    0x8: FIFO mode is 128
*/
typedef union {
    struct {
        uint32_t UART_COMPONENT_VERSION;    /*bit 0-31 ASCII value for each number in the version, followed by *  Access: RO */
    };
    uint32_t v;
} UCV_Type;

typedef union {
    struct {
        uint32_t PERIPHERAL_ID;    /*bit 0-31 This register contains the peripherals identification code  Access: RO */
    };
    uint32_t v;
} CTR_Type;


/**
  * @}
  */

/** @brief Register_Mem_Controller
  * @{
  */

typedef struct {
	union {
    	__IO RBR_Type     RBR;    /*Offset: 0x0*/
		__IO DLL_Type     DLL;    /*Offset: 0x0*/
		__IO THR_Type     THR;    /*Offset: 0x0*/
	};
	union {
    	__IO DLH_Type     DLH;    /*Offset: 0x4*/
		__IO IER_Type     IER;    /*Offset: 0x4*/
	};
	union {
    	__IO FCR_Type     FCR;    /*Offset: 0x8*/
		__IO IIR_Type     IIR;    /*Offset: 0x8*/
	};
    __IO LCR_Type     LCR;    /*Offset: 0xc*/
    __IO MCR_Type     MCR;    /*Offset: 0x10*/
    __IO LSR_Type     LSR;    /*Offset: 0x14*/
    __IO MSR_Type     MSR;    /*Offset: 0x18*/
    __IO SCR_Type     SCR;    /*Offset: 0x1c*/
    __IO uint32_t     RESERVED0;    /*Offset: 0x20*/
    __IO uint32_t     RESERVED1;    /*Offset: 0x24*/
    __IO uint32_t     RESERVED2;    /*Offset: 0x28*/
    __IO uint32_t     RESERVED3;    /*Offset: 0x2c*/
    __IO SRBRX_Type     SRBR0;    /*Offset: 0x30*/
    __IO SRBRX_Type     SRBR1;    /*Offset: 0x34*/
    __IO SRBRX_Type     SRBR2;    /*Offset: 0x38*/
    __IO SRBRX_Type     SRBR3;    /*Offset: 0x3c*/
    __IO SRBRX_Type     SRBR4;    /*Offset: 0x40*/
    __IO SRBRX_Type     SRBR5;    /*Offset: 0x44*/
    __IO SRBRX_Type     SRBR6;    /*Offset: 0x48*/
    __IO SRBRX_Type     SRBR7;    /*Offset: 0x4c*/
    __IO SRBRX_Type     SRBR8;    /*Offset: 0x50*/
    __IO SRBRX_Type     SRBR9;    /*Offset: 0x54*/
    __IO SRBRX_Type     SRBR10;    /*Offset: 0x58*/
    __IO SRBRX_Type     SRBR11;    /*Offset: 0x5c*/
    __IO SRBRX_Type     SRBR12;    /*Offset: 0x60*/
    __IO SRBRX_Type     SRBR13;    /*Offset: 0x64*/
    __IO SRBRX_Type     SRBR14;    /*Offset: 0x68*/
    __IO SRBRX_Type     SRBR15;    /*Offset: 0x6c*/
    __IO FAR_Type     FAR;    /*Offset: 0x70*/
    __IO TFR_Type     TFR;    /*Offset: 0x74*/
    __IO RFW_Type     RFW;    /*Offset: 0x78*/
    __IO USR_Type     USR;    /*Offset: 0x7c*/
    __IO TFL_Type     TFL;    /*Offset: 0x80*/
    __IO RFL_Type     RFL;    /*Offset: 0x84*/
    __IO SRR_Type     SRR;    /*Offset: 0x88*/
    __IO SRTS_Type     SRTS;    /*Offset: 0x8c*/
    __IO SBCR_Type     SBCR;    /*Offset: 0x90*/
    __IO SDMAM_Type     SDMAM;    /*Offset: 0x94*/
    __IO SFE_Type     SFE;    /*Offset: 0x98*/
    __IO SRT_Type     SRT;    /*Offset: 0x9c*/
    __IO STET_Type     STET;    /*Offset: 0xa0*/
    __IO HTX_Type     HTX;    /*Offset: 0xa4*/
    __IO DMASA_Type     DMASA;    /*Offset: 0xa8*/
    __IO TCR_Type     TCR;    /*Offset: 0xac*/
    __IO DE_EN_Type     DE_EN;    /*Offset: 0xb0*/
    __IO RE_EN_Type     RE_EN;    /*Offset: 0xb4*/
    __IO DET_Type     DET;    /*Offset: 0xb8*/
    __IO TAT_Type     TAT;    /*Offset: 0xbc*/
    __IO DLF_Type     DLF;    /*Offset: 0xc0*/
    __IO RAR_Type     RAR;    /*Offset: 0xc4*/
    __IO TAR_Type     TAR;    /*Offset: 0xc8*/
    __IO LCR_EXT_Type     LCR_EXT;    /*Offset: 0xcc*/
    __IO uint32_t     RESERVED4;    /*Offset: 0xd0*/
    __IO uint32_t     RESERVED5;    /*Offset: 0xd4*/
    __IO uint32_t     RESERVED6;    /*Offset: 0xd8*/
    __IO uint32_t     RESERVED7;    /*Offset: 0xdc*/
    __IO uint32_t     RESERVED8;    /*Offset: 0xe0*/
    __IO uint32_t     RESERVED9;    /*Offset: 0xe4*/
    __IO uint32_t     RESERVED10;    /*Offset: 0xe8*/
    __IO uint32_t     RESERVED11;    /*Offset: 0xec*/
    __IO uint32_t     RESERVED12;    /*Offset: 0xf0*/
    __IO CPR_Type     CPR;    /*Offset: 0xf4*/
    __IO UCV_Type     UCV;    /*Offset: 0xf8*/
    __IO CTR_Type     CTR;    /*Offset: 0xfc*/
} DW_APB_UART_uart_TypeDef;


/**
  * @}
  */

/** @addtogroup Peripheral_declaration
  * @{
  */

typedef struct {
    DW_APB_UART_uart_TypeDef    DW_APB_UART_uart;
} DW_APB_UART_TypeDef;

/**
  * @}
  */

#endif