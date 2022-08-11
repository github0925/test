/*
 * Lin.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: LIN driver.
 *
 * Revision History:
 * -----------------
 * 011, 11/23/2019 Xidong Du    Implement this file.
 */
#include <assert.h>
#include <bits.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "Lin.h"

/* LIN Baud rates. */
#define BREAK_BAUDRATE(ch)          Lin_gConfig->Config[ch].baud/2
#define NORMAL_BAUDRATE(ch)         Lin_gConfig->Config[ch].baud

#define MAX_TIMEOUT(ch, length)  (10*length+44+1)*1400/Lin_gConfig->Config[ch].baud

typedef struct Lin_ifc_PduTypeTag {
    Lin_PduType     SrcPudInfo;
    uint8           parityID;
    uint8           checkSum;
    uint8           sduBuffer[LIN_FRAME_DATA_LENGTH];
} Lin_ifc_PduType;

typedef struct Lin_channel {
    Lin_ifc_PduType     pdu;
    Lin_ChStatusType    status;
    Lin_StatusType      SubStatus;
    Lin_ifc_StatusType  FrameStatus;
    void                *handle;
    bool                MasterResponse;
    uint8               MasterLength;
    bool                SlaveResponse;
    uint8               SlaveLength;
    uint8               RxBuffer[LIN_FRAME_DATA_LENGTH];
    mutex_t             mutex;
    event_t             event;
    bool                RXIRQStatus;
} Lin_ifc_ChannelType;

static volatile Lin_ifc_ChannelType Lin_gChannels[LIN_IFC_CHN_MAX];
static uint8 Lin_gLinDrvStatus = LIN_UNINIT;
static const Lin_ConfigType *Lin_gConfig;

static void Lin_Interrupt(uint8 Channel, uint8 rxData);

/* TODO - We can use one single ISR for all UART channels, if the UART
 * HAL callback has a "private" pointer.
 */
static int Lin_ISR_Channel_1(uint8 Data);
static int Lin_ISR_Channel_2(uint8 Data);
static int Lin_ISR_Channel_3(uint8 Data);
static int Lin_ISR_Channel_4(uint8 Data);

static hal_uart_int_callback lin_CallBack[LIN_IFC_CHN_MAX] = {
    Lin_ISR_Channel_1,
    Lin_ISR_Channel_2,
    Lin_ISR_Channel_3,
    Lin_ISR_Channel_4,
};

static int Lin_ISR_Channel_1(uint8 Data)
{
    volatile Lin_ifc_ChannelType *chn;

    chn = &Lin_gChannels[LIN_IFC_SCI1];

    if (chn->RXIRQStatus == 0x01) {
        Lin_Interrupt(LIN_IFC_SCI1, Data);
    }

    return 0;
}

static int Lin_ISR_Channel_2(uint8 Data)
{
    volatile Lin_ifc_ChannelType *chn;

    chn = &Lin_gChannels[LIN_IFC_SCI2];

    if (chn->RXIRQStatus == 0x01) {
        Lin_Interrupt(LIN_IFC_SCI2, Data);
    }

    return 0;
}

static int Lin_ISR_Channel_3(uint8 Data)
{
    volatile Lin_ifc_ChannelType *chn;

    chn = &Lin_gChannels[LIN_IFC_SCI3];

    if (chn->RXIRQStatus == 0x01) {
        Lin_Interrupt(LIN_IFC_SCI3, Data);
    }

    return 0;
}

static int Lin_ISR_Channel_4(uint8 Data)
{
    volatile Lin_ifc_ChannelType *chn;

    chn = &Lin_gChannels[LIN_IFC_SCI4];

    if (chn->RXIRQStatus == 0x01) {
        Lin_Interrupt(LIN_IFC_SCI4, Data);
    }

    return 0;
}

/******************************************************************************
 ** \brief Check if the LIN channel is valid.
 **
 ** \param [in]   Channel          hardware uart channel
 ** \return       True if the channel is valid
 *****************************************************************************/
static bool Lin_ChannelValid(uint8 Channel)
{
    return Channel < Lin_gConfig->Count &&
           Lin_gChannels[Channel].handle != NULL;
}

/******************************************************************************
 ** \brief change uart baudrate
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   baudrate         hardware baudrate
 ** \param [out]  NA
 ** \return       NA
 *****************************************************************************/
static void Lin_SetBaudrate(uint8 Channel, uint32 baudrate)
{
    ASSERT(Lin_ChannelValid(Channel));
    hal_uart_baudrate_set(Lin_gChannels[Channel].handle,
                          Lin_gConfig->Config[Channel].sclk, baudrate);
}

/******************************************************************************
 ** \brief send one Byte data
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   Data             data need to be transimit
 ** \param [out]  NA
 ** \return       NA
 *****************************************************************************/
static void Lin_SendOneByte(uint8 Channel, uint8 Data)
{
    ASSERT(Lin_ChannelValid(Channel));
    hal_uart_putc(Lin_gChannels[Channel].handle, Data);
}

/******************************************************************************
 ** \brief caculate the parity ID
 **
 ** \param [in]   id               the src id get from ldf file
 ** \param [out]  NA
 ** \return       uint8            result of parity ID
 *****************************************************************************/
static uint8 Lin_CalcParity(uint8 id)
{
    uint8 p0 = BIT_SET(id, 0) ^ BIT_SET(id, 1)
             ^ BIT_SET(id, 2) ^ BIT_SET(id, 4);
    uint8 p1 = !(BIT_SET(id, 1) ^ BIT_SET(id, 3)
             ^ BIT_SET(id, 4) ^ BIT_SET(id, 5));

    return (id & BIT_MASK(6)) | (p0 << 6) | (p1 << 7);
}

/******************************************************************************
 ** \brief caculate the Enhanced/Classic checksum
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   Cs               checksum model
 ** \param [in]   buffer           point to the rx data buffer
 ** \param [in]   cnt              rx data length
 ** \return       NA
 *****************************************************************************/
static uint8 Lin_CheckSum(uint8 Channel, Lin_FrameCsModelType Cs,
                          volatile uint8 *buffer, uint8 cnt)
{
    uint16 carry = 0x00;
    uint16 checkSum = LIN_CLASSIC_CS == Cs ? 0 :
                      Lin_gChannels[Channel].pdu.parityID;

    /* 0x3c/0x7d classic checksum */
    if((Lin_gChannels[Channel].pdu.parityID == 0x3c) || \
            (Lin_gChannels[Channel].pdu.parityID == 0x7d)) {
        checkSum = 0;
    }

    for (int i = 0; i < cnt; i++, buffer++) {
        checkSum += *buffer;
    }

    carry = checkSum >> 8;
    checkSum = (checkSum & 0xFF) + carry;

    return (uint8)(~checkSum);  //reverse
}

/******************************************************************************
 ** \brief master send parity id and data
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   rxData           data of rx fifo
 ** \return       NA
 *****************************************************************************/
static void Lin_MasterResponse(uint8 Channel, uint8 rxData)
{
    uint8 checksum = 0u;
    volatile Lin_PduType *srcPdu;
    volatile Lin_ifc_PduType *pdu;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));

    srcPdu = &Lin_gChannels[Channel].pdu.SrcPudInfo;
    pdu = &Lin_gChannels[Channel].pdu;
    chn = &Lin_gChannels[Channel];

    /* verify the first byte is parityID and send data */
    if (chn->MasterResponse == 1) {
        /* verify the first byte is parityID */
        if (pdu->parityID != rxData) {
            chn->FrameStatus = LIN_ID_ERROR;
        }

        /* send frame & add checksum */
        hal_uart_transmit(chn->handle, (char *)pdu->sduBuffer, srcPdu->Dl + 1,
                          true);
        chn->MasterResponse = 0;

    }
    /* receive and verify master send data here */
    else {
        /* check frame */
        chn->RxBuffer[chn->MasterLength] = rxData;

        /* received all data include checksum */
        if (chn->MasterLength == srcPdu->Dl) {
            checksum = Lin_CheckSum(Channel, srcPdu->Cs,
                                    chn->RxBuffer, chn->MasterLength);

            if (checksum == chn->RxBuffer[chn->MasterLength]) {
                chn->FrameStatus = LIN_IDLE;
            }
            else {
                chn->FrameStatus = LIN_MASTER_CHECKSUM_ERROR;
            }

            chn->MasterLength = 0;
            /* ready send next frame */
            chn->MasterResponse = 1;
            chn->RXIRQStatus = 0x00;
        }
        else {
            chn->MasterLength++;
        }
    }
}

/******************************************************************************
 ** \brief slave send data
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   rxData           data of rx fifo
 ** \return       NA
 *****************************************************************************/
static void Lin_SlaveResponse(uint8 Channel, uint8 rxData)
{
    uint8 checksum = 0u;
    volatile Lin_PduType *srcPdu;
    volatile Lin_ifc_PduType *pdu;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));

    srcPdu = &Lin_gChannels[Channel].pdu.SrcPudInfo;
    pdu = &Lin_gChannels[Channel].pdu;
    chn = &Lin_gChannels[Channel];

    /* verify the first byte is parityID */
    if (chn->SlaveResponse == 1u) {
        if (pdu->parityID != rxData) {
            chn->FrameStatus = LIN_ID_ERROR;
        }
        chn->SlaveResponse = 0u;
    }
    /* receive slave respone data here */
    else {
        chn->RxBuffer[chn->SlaveLength] = rxData;

        /* received all data include checksum */
        if (chn->SlaveLength == srcPdu->Dl) {
            checksum = Lin_CheckSum(Channel, srcPdu->Cs, chn->RxBuffer,
                                    chn->SlaveLength);

            /* last byte is checksum */
            if (checksum == chn->RxBuffer[chn->SlaveLength]) {
                /* TODO: */
                chn->FrameStatus = LIN_IDLE;
            }
            else {
                chn->FrameStatus = LIN_SLAVE_CHECKSUM_ERROR;
            }

            chn->SlaveLength = 0u;
            /* ready reveive next frame */
            chn->SlaveResponse = 1u;
            chn->RXIRQStatus = 0x00;
        }
        else {
            chn->SlaveLength++;
        }
    }
}

/******************************************************************************
 ** \brief transfer lin config type to uart_hal type
 **
 ** \param [in]   Channel         hardware uart channel
 ** \param [in]  hal_uart_cfg_t   pointer to the uart hal layer config type
 ** \return       NA
 *****************************************************************************/
static void Lin_LinCfgToUartCfg(uint8 Channel, hal_uart_cfg_t *uart_cfg,
                                const Lin_ConfigType *lin_cfg)
{
    if ((lin_cfg == NULL) || (Channel >= lin_cfg->Count)) {
        return;
    }

    uart_cfg->port_cfg.sclk = lin_cfg->Config[Channel].sclk;
    uart_cfg->port_cfg.baud = lin_cfg->Config[Channel].baud;
    uart_cfg->port_cfg.data_bits = lin_cfg->Config[Channel].data_bits;
    uart_cfg->port_cfg.stop_bits = lin_cfg->Config[Channel].stop_bits;
    uart_cfg->port_cfg.parity = lin_cfg->Config[Channel].parity;
    uart_cfg->port_cfg.loopback_enable =
        lin_cfg->Config[Channel].loopback_enable;
    uart_cfg->fifo_cfg.fifo_enable = lin_cfg->Config[Channel].fifo_enable;
    uart_cfg->fifo_cfg.rx_trigger = lin_cfg->Config[Channel].rx_trigger;
    uart_cfg->fifo_cfg.tx_trigger = lin_cfg->Config[Channel].tx_trigger;

    /* lin drive don't need 485/9bit/DMA */
#ifdef UART_DRV_SUPPORT_RS485
    uart_cfg->rs485_cfg.rs485_enable = false;
#endif
#ifdef UART_DRV_SUPPORT_9BITS
    uart_cfg->nine_bits_cfg.nine_bits_enable = false;
#endif
#ifdef UART_DRV_SUPPORT_DMA
    uart_cfg->dma_cfg.dma_enable = false;
#endif
}

/******************************************************************************
 ** \brief base lin_pbcfg.c init uart channel and register cbk func
 **
 ** \param [in]   Config           not used
 ** \return       NA
 *****************************************************************************/
static void Lin_IFC_WakeUp(uint8 Channel)
{
    Lin_SetBaudrate(Channel, BREAK_BAUDRATE(Channel));
    /* break time = 1/9600*8 = 833us */
    Lin_SendOneByte(Channel, BREAK);
}

static Lin_StatusType Lin_IFC_GetStatus(uint8 Channel)
{
    Lin_StatusType RetTmp = LIN_NOT_OK;
    volatile Lin_ifc_ChannelType *chn;
    chn = &Lin_gChannels[Channel];

    if (LIN_SYNC_ERROR == chn->FrameStatus
            || LIN_ID_ERROR == chn->FrameStatus) {
        RetTmp = LIN_TX_HEADER_ERROR;
    }
    else {
        if (LIN_MASTER_RESPONSE == chn->pdu.SrcPudInfo.Drc) {
            if (LIN_MASTER_CHECKSUM_ERROR == chn->FrameStatus) {
                RetTmp = LIN_TX_ERROR;
            }
            else if (LIN_IDLE == chn->FrameStatus) {
                RetTmp = LIN_TX_OK;
            }
            else {
                RetTmp = LIN_TX_BUSY;
            }
        }
        else if (LIN_SLAVE_RESPONSE == chn->pdu.SrcPudInfo.Drc) {
            if (LIN_SLAVE_CHECKSUM_ERROR == chn->FrameStatus) {
                RetTmp = LIN_RX_ERROR;
            }
            else if (LIN_IDLE == chn->FrameStatus) {
                RetTmp = LIN_RX_OK;
            }
            else if ((LIN_SLAVE_RESPONE_DATA == chn->FrameStatus)
                     && (0 == chn->SlaveLength)) {
                RetTmp = LIN_RX_NO_RESPONSE;
            }
            else {
                RetTmp = LIN_RX_BUSY;
            }
        }
    }

    return RetTmp;
}

/******************************************************************************
 ** \brief cancel pre trans
 **
 ** \param [in]   Config           not used
 ** \return       NA
 *****************************************************************************/
static void Lin_CancelPreTrans(volatile Lin_ifc_ChannelType *chn)
{
    if (chn->FrameStatus == LIN_SLAVE_RESPONE_DATA) {
        chn->SlaveLength = 0;
        chn->SlaveResponse = 1u;
        chn->RXIRQStatus = 0;
    }
    else if (chn->FrameStatus == LIN_MASTER_RESPONE_DATA) {
        chn->MasterLength = 0;
        chn->MasterResponse = 1u;
        chn->RXIRQStatus = 0;
    }

    chn->FrameStatus = LIN_IDLE;
}

/******************************************************************************
 ** \brief base lin_pbcfg.c init uart channel and register cbk func
 **
 ** \param [in]   Config           not used
 ** \return       NA
 *****************************************************************************/
void Lin_Init(const Lin_ConfigType *Config)
{
    uint8 Index = 0u;
    hal_uart_cfg_t uartCfg;
    volatile Lin_ifc_ChannelType *chn;
    const Lin_ConfigType *pConfigPtr = NULL_PTR;

    if (LIN_UNINIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_STATE_TRANSITION */
    }

    if (NULL_PTR == Config) {
        /* TODO:DET ERROR REPORT LIN_E_INVALID_POINTER */
        return;
    }
    else {
        pConfigPtr = Config;
    }

    for (Index = 0u; Index < Config->Count; Index++) {
        chn = &Lin_gChannels[Index];

        if (pConfigPtr->Config[Index].hrdChannel == 0) {
            dprintf(INFO, "LIN channel %d not configured. Ignore\n", Index);
            continue;
        }

        /* Configure UART for this LIN channel. */
        hal_uart_creat_handle((void **)&chn->handle,
                              pConfigPtr->Config[Index].hrdChannel);
        ASSERT(NULL != chn->handle);

        Lin_LinCfgToUartCfg(Index, &uartCfg, pConfigPtr);
        hal_uart_init(chn->handle, &uartCfg);
        hal_uart_int_cbk_register(chn->handle, LIN_RX_INT,
                                  lin_CallBack[Index]);

        chn->status = LIN_CH_SLEEP_STATE;
        chn->SubStatus = LIN_CH_SLEEP;
        chn->FrameStatus = LIN_NOT_READY;

        chn->MasterResponse = 1u;
        chn->SlaveResponse = 1u;

        mutex_init((void *)&chn->mutex);
        event_init((void *)&chn->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    }

    Lin_gLinDrvStatus = LIN_INIT;

    /* FIXME - the Config parameter MUST point to static storage. */
    Lin_gConfig = Config;
}

/******************************************************************************
 ** \brief send lin frame
 **
 ** \param [in]   Channel         hardware uart channel
 ** \param [in]   Lin_PduType     pointer to the origin Pudinfo
 ** \return       NA
 *****************************************************************************/
Std_ReturnType Lin_SendFrame(uint8 Channel, const Lin_PduType *PduInfoPtr)
{
    uint8 i;
    Std_ReturnType RetTmp = E_OK;
    volatile Lin_PduType *srcPdu;
    volatile Lin_ifc_PduType *pdu;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));

    srcPdu = &Lin_gChannels[Channel].pdu.SrcPudInfo;
    pdu = &Lin_gChannels[Channel].pdu;
    chn = &Lin_gChannels[Channel];

    if (chn->status != LIN_CH_OPERATIONAL) {
        dprintf(CRITICAL, "LIN channel %d status error:%d\n",
                Channel, chn->status);
        return E_NOT_OK;
    }

    mutex_acquire((void *)&chn->mutex);

    /* walkaround Lin_Init uart register, will trigger a RX ISR */
    if (chn->RXIRQStatus == 0x01) {
        Lin_CancelPreTrans(chn);
    }
    chn->RXIRQStatus = 0x01;
    /* TODO:ERROR check */

    if (LIN_IDLE == chn->FrameStatus ) {
        srcPdu->Pid = PduInfoPtr->Pid;
        srcPdu->Cs = PduInfoPtr->Cs;
        srcPdu->Drc = PduInfoPtr->Drc;
        srcPdu->Dl = PduInfoPtr->Dl;

        for (i = 0; i < srcPdu->Dl; i++) {
            pdu->sduBuffer[i] = PduInfoPtr->SduPtr[i];
        }

        /* calculate parity ID */
        pdu->parityID = Lin_CalcParity(PduInfoPtr->Pid);

        /* only master response caculate checksum */
        if (LIN_MASTER_RESPONSE == srcPdu->Drc) {
            /* calculate parity checksum */
            pdu->checkSum = Lin_CheckSum(Channel, srcPdu->Cs, PduInfoPtr->SduPtr,
                                         PduInfoPtr->Dl);
            /* add checksum compose one frame */
            pdu->sduBuffer[PduInfoPtr->Dl] = pdu->checkSum;
        }

        chn->FrameStatus = LIN_SEND_SYNC;
        Lin_SetBaudrate(Channel, BREAK_BAUDRATE(Channel));
        Lin_SendOneByte(Channel, BREAK);
        event_wait_timeout((void *)&chn->event, MAX_TIMEOUT(Channel,PduInfoPtr->Dl));
    }

    mutex_release((void *)&chn->mutex);
    return RetTmp;
}

/******************************************************************************
 ** \brief Maintenance state machine
 **
 ** \param [in]   Channel          hardware uart channel
 ** \param [in]   rxData           data of rx fifo
 ** \return       NA
 *****************************************************************************/
void Lin_Interrupt(uint8 Channel, uint8 rxData)
{
    volatile Lin_PduType *srcPdu;
    volatile Lin_ifc_PduType *pdu;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));

    srcPdu = &Lin_gChannels[Channel].pdu.SrcPudInfo;
    pdu = &Lin_gChannels[Channel].pdu;
    chn = &Lin_gChannels[Channel];

    dprintf(DBGV, "%s: channel %d, rxdata 0x%x, state %d\n", __func__, Channel, rxData, chn->FrameStatus);

    switch (chn->FrameStatus) {
        case LIN_SEND_SYNC:
            Lin_SetBaudrate(Channel, NORMAL_BAUDRATE(Channel));
            Lin_SendOneByte(Channel, SYNC);
            chn->FrameStatus = LIN_SEND_ID;
            break;

        case LIN_SEND_ID:
            if (SYNC != rxData) {
                chn->FrameStatus = LIN_SYNC_ERROR;
            }
            else {
                Lin_SendOneByte(Channel, pdu->parityID);

                if (LIN_MASTER_RESPONSE == srcPdu->Drc) {
                    chn->FrameStatus = LIN_MASTER_RESPONE_DATA;
                }
                else if (LIN_SLAVE_RESPONSE == srcPdu->Drc) {
                    chn->FrameStatus = LIN_SLAVE_RESPONE_DATA;
                }
                else {
                    /* TODO:slave to slave */
                }
            }
            break;

        case LIN_MASTER_RESPONE_DATA:
            Lin_MasterResponse(Channel, rxData);
            break;

        case LIN_SLAVE_RESPONE_DATA:
            Lin_SlaveResponse(Channel, rxData);
            break;

        default:
            break;
    }

    event_signal((void *)&chn->event, false);
}

Std_ReturnType Lin_CheckWakeup(uint8 Channel)
{
    return 0;
}

Std_ReturnType Lin_GoToSleep(uint8 Channel)
{
    /* The service instructs the driver to transmit a go-to-sleep-command on the
    addressed LIN channel. */
    Std_ReturnType RetTmp = (uint8)E_NOT_OK;
    Lin_PduType lin_PduInfo;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));
    chn = &Lin_gChannels[Channel];

    uint8 lindata[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    lin_PduInfo.Pid = 0x3C;
    lin_PduInfo.Cs = LIN_CLASSIC_CS;
    lin_PduInfo.Drc = LIN_MASTER_RESPONSE;
    lin_PduInfo.Dl = 8;
    lin_PduInfo.SduPtr = lindata;

    if (LIN_INIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_UNINIT */
        RetTmp = (uint8)E_NOT_OK;
    }
    else {
        if (Channel >= LIN_IFC_CHN_MAX) {
            /* TODO:DET ERROR REPORT LIN_E_INVALID_CHANNEL */
            RetTmp = (uint8)E_NOT_OK;
        }
        else {
            if (LIN_CH_SLEEP_STATE == chn->status) {
                RetTmp = (uint8)E_OK;
            }
            else {
                /* 1.enable the wake-up detection */
                /* 2.set the LIN hardware unit to reduced power operation mode */
                /* 3.terminate on going frame transmission of prior transmission requests */
                Lin_SendFrame(Channel, &lin_PduInfo);

                /* TODO:disable RX isr, enable RX input edge detect,enter low power */
                chn->status = LIN_CH_SLEEP_PENDING;
                RetTmp = (uint8)E_OK;
            }
        }
    }

    return RetTmp;
}

Std_ReturnType Lin_GoToSleepInternal(uint8 Channel)
{
    Std_ReturnType RetTmp = (uint8)E_NOT_OK;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));
    chn = &Lin_gChannels[Channel];

    if (LIN_INIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_UNINIT */
        RetTmp = (uint8)E_NOT_OK;
    }
    else {
        if (Channel >= LIN_IFC_CHN_MAX) {
            /* TODO:DET ERROR REPORT LIN_E_INVALID_CHANNEL */
            RetTmp = (uint8)E_NOT_OK;
        }
        else {
            if (LIN_CH_SLEEP_STATE == chn->status) {
                RetTmp = (uint8)E_OK;
            }
            else {
                /* 1.enable the wake-up */
                /* 2.set the LIN hardware unit to reduced power operation mode */

                /* TODO:disable RX isr, enable RX input edge detect,enter low power */
                chn->status = LIN_CH_SLEEP_STATE;
                chn->SubStatus = LIN_CH_SLEEP;
                chn->FrameStatus = LIN_NOT_READY;
                RetTmp = (uint8)E_OK;
            }
        }
    }

    return RetTmp;
}

Std_ReturnType Lin_Wakeup(uint8 Channel)
{
    Std_ReturnType RetTmp = (uint8)E_NOT_OK;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));
    chn = &Lin_gChannels[Channel];

    if (LIN_INIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_UNINIT */
        RetTmp = (uint8)E_NOT_OK;
    }
    else {
        if (Channel >= LIN_IFC_CHN_MAX) {
            /* TODO:DET ERROR REPORT LIN_E_INVALID_CHANNEL */
            RetTmp = (uint8)E_NOT_OK;
        }
        else {
            if (LIN_CH_SLEEP_STATE != chn->status) {
                /* TODO:DET ERROR REPORT LIN_E_STATE_TRANSITION */
                RetTmp = (uint8)E_NOT_OK;
            }
            else {
                /* Generates a wake up pulse and sets the channel state to
                LIN_CH_OPERATIONAL. */
                Lin_IFC_WakeUp(Channel);
                /* Update LIN channel status to LIN_CH_OPERATIONAL */
                chn->status = LIN_CH_OPERATIONAL;
                chn->SubStatus = LIN_OPERATIONAL;
                chn->FrameStatus = LIN_IDLE;
                RetTmp = (uint8)E_OK;
            }
        }
    }

    return RetTmp;
}

Std_ReturnType Lin_WakeupInternal(uint8 Channel)
{
    /* The function Lin_WakeupInternal sets the addressed LIN
    channel to state LIN_CH_OPERATIONAL without generating a wake up pulse */
    Std_ReturnType RetTmp = (uint8)E_NOT_OK;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));
    chn = &Lin_gChannels[Channel];

    if (LIN_INIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_UNINIT */
        RetTmp = (uint8)E_NOT_OK;
    }
    else {
        if (Channel >= LIN_IFC_CHN_MAX) {
            /* TODO:DET ERROR REPORT LIN_E_INVALID_CHANNEL */
            RetTmp = (uint8)E_NOT_OK;
        }
        else {
            if (LIN_CH_SLEEP_STATE != chn->status) {
                /* TODO:DET ERROR REPORT LIN_E_STATE_TRANSITION */
                RetTmp = (uint8)E_NOT_OK;
            }
            else {
                /* Update LIN channel status to LIN_CH_OPERATIONAL */
                chn->status = LIN_CH_OPERATIONAL;
                chn->SubStatus = LIN_OPERATIONAL;
                chn->FrameStatus = LIN_IDLE;
                RetTmp = (uint8)E_OK;
            }
        }
    }

    return RetTmp;
}

Lin_StatusType Lin_GetStatus(uint8 Channel, uint8 **Lin_SduPtr)
{
    Lin_StatusType RetTmp = (uint8)LIN_NOT_OK;
    uint8 ChannelState;
    volatile Lin_ifc_ChannelType *chn;

    ASSERT(Lin_ChannelValid(Channel));
    chn = &Lin_gChannels[Channel];

    if (LIN_INIT != Lin_gLinDrvStatus) {
        /* TODO:DET ERROR REPORT LIN_E_UNINIT */
        RetTmp = (uint8)LIN_NOT_OK;
    }
    else {
        if (Channel >= LIN_IFC_CHN_MAX) {
            /* TODO:DET ERROR REPORT LIN_E_INVALID_CHANNEL */
            RetTmp = (uint8)LIN_NOT_OK;
        }
        else {
            if (NULL_PTR == Lin_SduPtr) {
                /* TODO:DET ERROR REPORT LIN_E_PARAM_POINTER */
                RetTmp = (uint8)LIN_NOT_OK;
            }
            else {
                ChannelState = chn->status;

                /* LIN channel state */
                switch (ChannelState) {
                    case LIN_CH_SLEEP_STATE:
                        RetTmp = LIN_CH_SLEEP;
                        break;

                    case LIN_CH_SLEEP_PENDING:
                        chn->status = LIN_CH_SLEEP_STATE;
                        chn->SubStatus = LIN_CH_SLEEP;
                        chn->FrameStatus = LIN_NOT_READY;
                        RetTmp = chn->SubStatus;
                        break;

                    case LIN_CH_OPERATIONAL:
                        *Lin_SduPtr = (void *)chn->RxBuffer;
                        RetTmp = Lin_IFC_GetStatus(Channel);
                        break;

                    default :
                        break;
                }
            }
        }
    }

    return RetTmp;
}
