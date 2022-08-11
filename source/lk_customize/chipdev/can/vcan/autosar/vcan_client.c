
#include <kernel/event.h>
#include <kernel/thread.h>
#include <string.h>
#include "Can.h"
#ifdef SUPPORT_SDPE_RPC
#include "vcan_service.h"
#endif
#ifdef COM
#include "com_cbk.h"
#endif

#define VIRCANIF_DEBUG INFO

extern void CanIf_TxConfirmation( PduIdType canTxPduId );
extern void CanIf_RxIndication( uint16_t Hrh, Can_IdType CanId,
                                uint8_t CanDlc, const uint8_t *CanSduPtr );
extern void CanIf_ControllerBusOff( uint8_t Controller );
extern void CanIf_SetWakeupEvent( uint8_t Controller );

#if SUPPORT_BOARD_DIAG
void board_diag_can_rxind(uint16_t Hrh, Can_IdType CanId,
                          uint8_t CanDlc, const uint8_t *CanSduPtr);
#endif

#if SUPPORT_CAN_TEST
void can_rx_indication_test(uint16_t hrh, uint32_t can_id,
                            uint8_t dlc, uint8_t *sdu);
void can_tx_confirmation_test(uint16_t pdu_id);
#endif

#define CAN_CTRL_CONFIG_CNT 20

/**********************************************************************************
** Used for Autosar write to SDPE, Autosar is client.
**********************************************************************************/
void Can_Init(const Can_ConfigType *Config)
{
#if (VIRCANIF_DEBUG <= LK_DEBUGLEVEL)
    uint32_t i, j;

    printf("\n%s()\n", __func__);

    for (i = 0; i < Config->baudRateCfgCount; i++) {
        printf("\n");
        printf("BaudRateCfg %d:\n", i);
        printf("NomiBitTiming - preDivider: %d  rJumpwidth: %d  propSeg: %d  phaseSeg1: %d  phaseSeg2: %d\n",
               Config->baudRateCfg[i].nomianlBitTimingCfg.preDivider,
               Config->baudRateCfg[i].nomianlBitTimingCfg.rJumpwidth,
               Config->baudRateCfg[i].nomianlBitTimingCfg.propSeg,
               Config->baudRateCfg[i].nomianlBitTimingCfg.phaseSeg1,
               Config->baudRateCfg[i].nomianlBitTimingCfg.phaseSeg2);
        printf("DataBitTiming - preDivider: %d  rJumpwidth: %d  propSeg: %d  phaseSeg1: %d  phaseSeg2: %d\n",
               Config->baudRateCfg[i].dataBitTimingCfg.preDivider,
               Config->baudRateCfg[i].dataBitTimingCfg.rJumpwidth,
               Config->baudRateCfg[i].dataBitTimingCfg.propSeg,
               Config->baudRateCfg[i].dataBitTimingCfg.phaseSeg1,
               Config->baudRateCfg[i].dataBitTimingCfg.phaseSeg2);
        printf("\n");
    }

    for (i = 0; i < Config->controllerCount; i++) {
        printf("\n");
        printf("Can_Config %d:\n", i);
        printf("ControllerId: %d  baseAddr: 0x%X  clkSrc: %d  maxMbNum: %d  enableLoopBack: %d\n",
               Config->ctrllerCfg[i].controllerId,
               (uint32_t)Config->ctrllerCfg[i].baseAddr,
               (uint8_t)Config->ctrllerCfg[i].flexcanCfg.clkSrc,
               Config->ctrllerCfg[i].flexcanCfg.maxMbNum,
               Config->ctrllerCfg[i].flexcanCfg.enableLoopBack);
        printf("enableListenOnly: %d  enableSelfWakeup: %d  enableIndividMask: %d  enableCANFD: %d\n",
               Config->ctrllerCfg[i].flexcanCfg.enableListenOnly,
               Config->ctrllerCfg[i].flexcanCfg.enableSelfWakeup,
               Config->ctrllerCfg[i].flexcanCfg.enableIndividMask,
               Config->ctrllerCfg[i].flexcanCfg.enableCANFD);
        printf("nomiBitTiming preDivider: %d rJumpwidth: %d, propSeg: %d, phaseSeg1: %d phaseSeg2: %d\n",
               Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.preDivider,
               Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.rJumpwidth,
               Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.propSeg,
               Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.phaseSeg1,
               Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.phaseSeg2);
        printf("dataBitTiming preDivider: %d rJumpwidth: %d, propSeg: %d, phaseSeg1: %d phaseSeg2: %d\n",
               Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.preDivider,
               Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.rJumpwidth,
               Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.propSeg,
               Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.phaseSeg1,
               Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.phaseSeg2);
        printf("enableISOCANFD: %d enableBRS: %d, enableTDC: %d, TDCOffset: %d r0_mb_data_size: %d r1_mb_data_size: %d\n",
               Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableISOCANFD,
               Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableBRS,
               Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableTDC,
               Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.TDCOffset,
               (uint8_t)Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r0_mb_data_size,
               (uint8_t)Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r1_mb_data_size);
        printf("\n");
    }

    for (i = 0; i < Config->rxCount; i++) {
        printf("\n");
        printf("rxMBCfg %d:\n", i);
        printf("hwObjId: %d  controllerId: %d  mbId: %d  frameId: 0x%X\n",
               Config->rxMBCfg[i].hwObjId,
               Config->rxMBCfg[i].controllerId,
               Config->rxMBCfg[i].mbId,
               Config->rxMBCfg[i].rx.frameId);
        printf("rxIDFilterMask: 0x%X  frameType: %d  frameFormat: %d\n",
               Config->rxMBCfg[i].rx.rxIDFilterMask,
               Config->rxMBCfg[i].rx.frameType,
               Config->rxMBCfg[i].rx.frameFormat);
        printf("\n");
    }

    for (i = 0; i < Config->txCount; i++) {
        printf("\n");
        printf("txMBCfg %d:\n", i);
        printf("hwObjId: %d  controllerId: %d  mbId: %d  paddingVal: 0x%X\n",
               Config->txMBCfg[i].hwObjId,
               Config->txMBCfg[i].controllerId,
               Config->txMBCfg[i].mbId,
               Config->txMBCfg[i].tx.paddingVal);
        printf("\n");
    }

    for (i = 0; i < Config->rxFifoCount; i++) {
        printf("\n");
        printf("rxFIFOCfg %d:\n", i);
        printf("hwObjId: %d  controllerId: %d  rxFifoIdFilterMask: 0x%X\n",
               Config->rxFIFOCfg[i].hwObjId,
               Config->rxFIFOCfg[i].controllerId,
               *(Config->rxFIFOCfg[i].rxFifoIdFilterMask));
        printf("idFilterType: %d  priority: %d  idFilterNum: %d\n",
               (uint8_t)Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterType,
               (uint8_t)Config->rxFIFOCfg[i].flexcanRxFIFOCfg.priority,
               Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterNum);

        for (j = 0; j < Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterNum; j++) {
            printf("idFilterTable %d: 0x%X\n", j,
                   Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterTable[j]);
        }

        printf("\n");
    }

#endif

    virCan_Init(Config);
}

void Can_DeInit(void)
{
    dprintf(VIRCANIF_DEBUG, "\n%s()\n", __func__);
    virCan_DeInit();
}

Std_ReturnType Can_SetBaudrate(uint8_t Controller,
                               uint16_t BaudRateConfigID)
{
    Std_ReturnType RetVal;

    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d, BaudRateConfigID: %d\n",
            __func__, Controller, BaudRateConfigID);
    RetVal = virCan_SetBaudrate(Controller, BaudRateConfigID);
    dprintf(VIRCANIF_DEBUG, "%s() Result: %d\n", __func__, RetVal);
    return RetVal;
}

Std_ReturnType Can_SetControllerMode(uint8_t Controller,
                                     Can_ControllerStateType Transition)
{
    Std_ReturnType RetVal;

    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d, Transition: %d\n",
            __func__, Controller, Transition);
    RetVal = virCan_SetControllerMode(Controller, (uint8_t)Transition);
    dprintf(VIRCANIF_DEBUG, "%s() Result: %d\n", __func__, RetVal);
    return RetVal;
}

void Can_DisableControllerInterrupts(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    virCan_DisableControllerInterrupts(Controller);
}

void Can_EnableControllerInterrupts(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    virCan_EnableControllerInterrupts(Controller);
}

void Can_CheckWakeup(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    virCan_CheckWakeup(Controller);
}

Std_ReturnType Can_GetControllerErrorState(uint8_t Controller,
        Can_ErrorStateType *ErrorStatePtr)
{
    Std_ReturnType RetVal;

    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    RetVal = virCan_GetControllerErrorState(Controller,
                                            (uint8_t *)ErrorStatePtr);
    dprintf(VIRCANIF_DEBUG, "%s() Result: %d, ErrorState: %d\n", __func__,
            RetVal, *ErrorStatePtr);
    return RetVal;
}

Std_ReturnType Can_GetControllerMode(uint8_t Controller,
                                     Can_ControllerStateType *ControllerModePtr)
{
    Std_ReturnType RetVal;

    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    RetVal = virCan_GetControllerMode(Controller,
                                      (uint8_t *)ControllerModePtr);
    dprintf(VIRCANIF_DEBUG, "%s() Result: %d, ControllerMode: %d\n", __func__,
            RetVal, *ControllerModePtr);
    return RetVal;
}

Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType *PduInfo)
{
    Std_ReturnType RetVal;
    Can_PduType virPduInfo;

    virPduInfo.swPduHandle = PduInfo->swPduHandle;
    virPduInfo.id = PduInfo->id;
    virPduInfo.length = PduInfo->length;
    virPduInfo.sdu = PduInfo->sdu;

    dprintf(INFO,
            "\n%s() Hth: %d, swPduHandle: %d, id: 0x%X, length: %d\n",
            __func__, Hth, PduInfo->swPduHandle, PduInfo->id,
            PduInfo->length);
    RetVal = virCan_Write(Hth, &virPduInfo);
    dprintf(VIRCANIF_DEBUG, "%s() Result: %d\n", __func__, RetVal);
    return RetVal;
}

void Can_MainFunction_Write(void)
{
    vircan_MainFunction_Write();
}

void Can_MainFunction_Read(void)
{
    vircan_MainFunction_Read();
}

void Can_MainFunction_BusOff(void)
{
}

void Can_MainFunction_Error(void)
{
}

void Can_MainFunction_Wakeup(void)
{
}

/**********************************************************************************
** Used for SDPE write to Autosar, Autosar is server.
**********************************************************************************/
void virCan_ControllerBusOff(uint8_t Controller)
{
    dprintf(CRITICAL, "\n%s() Controller: %d\n", __func__, Controller);
    //CanIf_ControllerBusOff(Controller);
}

void virCan_ControllerWakeUp(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
}

void virCan_SetWakeupEvent(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Controller: %d\n", __func__, Controller);
    //CanIf_SetWakeupEvent(Controller);
}

void virCan_RxIndication(uint16_t Hrh, uint32_t CanId, uint8_t CanDlc,
                         uint8_t *CanSduPtr)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() Hrh: %d, CanId: 0x%X, CanDlc: %d\n",
            __func__, Hrh, CanId, CanDlc);

#ifdef COM
    uint16_t bus_id = Hrh;
    com_rx_frame(0, bus_id, CanId, CanSduPtr);
#endif

#if SUPPORT_BOARD_DIAG
    board_diag_can_rxind(Hrh, CanId, CanDlc, CanSduPtr);
#endif

#if SUPPORT_CAN_TEST
    can_rx_indication_test(Hrh, CanId, CanDlc, CanSduPtr);
#endif
}

void virCan_TxConfirmation(uint16_t canTxPduId)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() canTxPduId: %d\n", __func__, canTxPduId);
    //CanIf_TxConfirmation(canTxPduId);
#if SUPPORT_CAN_TEST
    can_tx_confirmation_test(canTxPduId);
#endif
}
