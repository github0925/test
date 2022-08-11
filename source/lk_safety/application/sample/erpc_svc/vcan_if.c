
#include <kernel/event.h>
#include <kernel/thread.h>
#include <string.h>
#include "virt_com.h"
#include "Std_Types.h"
//#include "Can.h"

#define VIRCANIF_DEBUG ALWAYS
#define CONFIG_SUPPORT_CAN_DRV (0)
static bool vircom_init_success = false;

#if CONFIG_SUPPORT_CAN_DRV
extern void CanIf_TxConfirmation( PduIdType canTxPduId );
extern void CanIf_RxIndication( uint16_t Hrh, Can_IdType CanId, uint8_t CanDlc, const uint8_t *CanSduPtr );
extern void CanIf_ControllerBusOff( uint8_t Controller );
extern void CanIf_SetWakeupEvent( uint8_t Controller );
#else
typedef char Can_ConfigType;
#define E_NOT_OK (0xff)


#endif

static void vircan_config_init(void)
{

}

void virCanIf_Init(void)
{
    vircan_config_init();
    vircom_init_success = vircom_init();
    if(!vircom_init_success) {
        dprintf(VIRCANIF_DEBUG, "%s() vircom init fail\n", __func__);
    }
}

/**********************************************************************************
** Used for Autosar write to SDPE, Autosar is client.
**********************************************************************************/
void Can_Init(const Can_ConfigType* Config)
{
    virCan_ConfigType virConfig;

    if(!vircom_init_success) {
        return;
    }

#if 0
    virConfig.controllerCount = Config->controllerCount;
    virConfig.rxFifoCount = Config->rxFifoCount;
    virConfig.baudRateCfgCount = Config->baudRateCfgCount;
    virConfig.rxCount = Config->rxCount;
    virConfig.txCount = Config->txCount;
    virConfig.ctrllerCfg = (virCan_ControllerConfig *)Config->ctrllerCfg;
    virConfig.rxMBCfg = (virCan_RxMBConfig *)Config->rxMBCfg;
    virConfig.txMBCfg = (virCan_TxMBConfig *)Config->txMBCfg;
    virConfig.rxFIFOCfg = (virCan_RxFIFOConfig *)Config->rxFIFOCfg;
    virConfig.baudRateCfg = (virCan_BaudRateConfig *)Config->baudRateCfg;

    uint32_t i, j;
    for(i = 0; i < Config->baudRateCfgCount; i++)
    {
        dprintf(VIRCANIF_DEBUG, "baudRateCfg %d:\n", i);
        dprintf(VIRCANIF_DEBUG, "%d %d %d %d %d %d %d %d %d %d \n", 
                                Config->baudRateCfg[i].bitTimingCfg.arbitrPreDivider,
                                Config->baudRateCfg[i].bitTimingCfg.dataPreDivider,
                                Config->baudRateCfg[i].bitTimingCfg.arbitrRJumpwidth,
                                Config->baudRateCfg[i].bitTimingCfg.arbitrPhaseSeg1,
                                Config->baudRateCfg[i].bitTimingCfg.arbitrPhaseSeg2,
                                Config->baudRateCfg[i].bitTimingCfg.arbitrPropSeg,
                                Config->baudRateCfg[i].bitTimingCfg.dataRJumpwidth,
                                Config->baudRateCfg[i].bitTimingCfg.dataPhaseSeg1,
                                Config->baudRateCfg[i].bitTimingCfg.dataPhaseSeg2,
                                Config->baudRateCfg[i].bitTimingCfg.dataPropSeg);
    }
    
    for(i = 0; i < Config->controllerCount; i++)
    {
        dprintf(VIRCANIF_DEBUG, "ctrllerCfg %d:\n", i);
        dprintf(VIRCANIF_DEBUG, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
                                Config->ctrllerCfg[i].controllerId,
                                (uint32_t)Config->ctrllerCfg[i].baseAddr,
                                Config->ctrllerCfg[i].clkFreq,
                                Config->ctrllerCfg[i].flexcanCfg.baudRate.arbitrBaudRate,           
                                Config->ctrllerCfg[i].flexcanCfg.baudRate.dataBaudRate,
                                Config->ctrllerCfg[i].flexcanCfg.clkSrc,
                                Config->ctrllerCfg[i].flexcanCfg.maxMbNum,
                                Config->ctrllerCfg[i].flexcanCfg.enableLoopBack,
                                Config->ctrllerCfg[i].flexcanCfg.enableSelfWakeup,
                                Config->ctrllerCfg[i].flexcanCfg.enableIndividMask,
                                Config->ctrllerCfg[i].flexcanCfg.enableDoze,
                                Config->ctrllerCfg[i].flexcanCfg.enableCANFD,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableBRS,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableTDC,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.TDCOffset,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r0_mb_data_size,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r1_mb_data_size,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r2_mb_data_size,
                                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r3_mb_data_size);
    }

    for(i = 0; i < Config->rxCount; i++)
    {
        dprintf(VIRCANIF_DEBUG, "rxMBCfg %d:\n", i);
        dprintf(VIRCANIF_DEBUG, "%d %d %d %d %d %d %d \n", 
                                Config->rxMBCfg[i].hwObjId,
                                Config->rxMBCfg[i].controllerId,
                                Config->rxMBCfg[i].mbId,
                                Config->rxMBCfg[i].rx.frameId,
                                Config->rxMBCfg[i].rx.rxIDFilterMask,
                                Config->rxMBCfg[i].rx.frameType,
                                Config->rxMBCfg[i].rx.frameFormat);
    }

    for(i = 0; i < Config->txCount; i++)
    {
        dprintf(VIRCANIF_DEBUG, "txMBCfg %d:\n", i);
        dprintf(VIRCANIF_DEBUG, "%d %d %d %d %d %d \n", 
                                Config->txMBCfg[i].hwObjId,
                                Config->txMBCfg[i].controllerId,
                                Config->txMBCfg[i].mbId,
                                Config->txMBCfg[i].tx.paddingVal,
                                Config->txMBCfg[i].tx.isMBIdle,
                                (uint32_t)Config->txMBCfg[i].tx.txLock);
    }

    for(i = 0; i < Config->rxFifoCount; i++)
    {
        dprintf(VIRCANIF_DEBUG, "rxFIFOCfg %d:\n", i);
        dprintf(VIRCANIF_DEBUG, "%d %d %d %d %d %d \n", 
                                Config->rxFIFOCfg[i].hwObjId,
                                Config->rxFIFOCfg[i].controllerId,
                                *(Config->rxFIFOCfg[i].rxFifoIdFilterMask),
                                Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterType,
                                Config->rxFIFOCfg[i].flexcanRxFIFOCfg.priority,
                                Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterNum);

        for(j = 0; j < Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterNum; j++)
        {
            Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterTable[j] =
                            Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterTable[j];

            dprintf(VIRCANIF_DEBUG, "idFilterTable %d: %d\n", j,
                                    Config->rxFIFOCfg[i].flexcanRxFIFOCfg.idFilterTable[j]);
        }
    }
#endif

    virCan_Init((const virCan_ConfigType*)&virConfig);
}

void Can_DeInit(void)
{
    if(!vircom_init_success) {
        return;
    }
    dprintf(VIRCANIF_DEBUG, "%s()\n", __func__);
    virCan_DeInit();
}

Std_ReturnType Can_SetBaudrate(uint8_t Controller, uint16_t BaudRateConfigID)
{
    if(!vircom_init_success) {
        return E_NOT_OK;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d, BaudRateConfigID: %d\n", __func__, Controller, BaudRateConfigID);
    return virCan_SetBaudrate(Controller, BaudRateConfigID);
}
#if 0
Std_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ControllerStateType Transition)
{
    if(!vircom_init_success) {
        return E_NOT_OK;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d, Transition: %d\n", __func__, Controller, Transition);
    return virCan_SetControllerMode(Controller, (uint8_t)Transition);
}

void Can_DisableControllerInterrupts(uint8_t Controller)
{
    if(!vircom_init_success) {
        return;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
    virCan_DisableControllerInterrupts(Controller);
}

void Can_EnableControllerInterrupts(uint8_t Controller)
{
    if(!vircom_init_success) {
        return;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
    virCan_EnableControllerInterrupts(Controller);
}

void Can_CheckWakeup(uint8_t Controller)
{
    if(!vircom_init_success) {
        return;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
    virCan_CheckWakeup(Controller);
}

Std_ReturnType Can_GetControllerErrorState(uint8_t Controller, Can_ErrorStateType * ErrorStatePtr)
{
    if(!vircom_init_success) {
        return E_NOT_OK;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
    return virCan_GetControllerErrorState(Controller, (uint8_t *)ErrorStatePtr);
}

Std_ReturnType Can_GetControllerMode(uint8_t Controller, Can_ControllerStateType * ControllerModePtr)
{
    if(!vircom_init_success) {
        return E_NOT_OK;
    }
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
    return virCan_GetControllerMode(Controller, (uint8_t *)ControllerModePtr);
}

Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo)
{
    virCan_PduType virPduInfo;

    if(!vircom_init_success) {
        return E_NOT_OK;
    }

    virPduInfo.swPduHandle = PduInfo->swPduHandle;
    virPduInfo.id = PduInfo->id;
    virPduInfo.length = PduInfo->length;
    virPduInfo.sdu = PduInfo->sdu;
    dprintf(VIRCANIF_DEBUG, "%s() Hth: %d, swPduHandle: %d, id: %d, length: %d, sdu: ", 
                            __func__, Hth, PduInfo->swPduHandle, PduInfo->id, PduInfo->length);
    for(uint32_t i = 0; i < PduInfo->length; i++)
    {
        dprintf(VIRCANIF_DEBUG, "%d, ", PduInfo->sdu[i]);
    }
    dprintf(VIRCANIF_DEBUG, "\n");
    return virCan_Write(Hth, &virPduInfo);
}

void Can_MainFunction_Write(void)
{
}

void Can_MainFunction_Read(void)
{
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
#endif

/**********************************************************************************
** Used for SDPE write to Autosar, Autosar is server.
**********************************************************************************/
void virCan_ControllerBusOff(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
//    CanIf_ControllerBusOff(Controller);
}

void virCan_SetWakeupEvent(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "%s() Controller: %d\n", __func__, Controller);
//    CanIf_SetWakeupEvent(Controller);
}

void virCan_RxIndication(uint16_t Hrh, uint32_t CanId, uint8_t CanDlc, uint8_t * CanSduPtr)
{
    dprintf(VIRCANIF_DEBUG, "%s() Hrh: %d, CanId: %d, CanDlc: %d, Sdu: \n", 
                            __func__, Hrh, CanId, CanDlc);
    for(uint32_t i = 0; i < CanDlc; i++)
    {
        dprintf(VIRCANIF_DEBUG, "%d, ", CanSduPtr[i]);
    }
    dprintf(VIRCANIF_DEBUG, "\n");
//    CanIf_RxIndication(Hrh, CanId, CanDlc, CanSduPtr);
}

void virCan_TxConfirmation(uint16_t canTxPduId)
{
    dprintf(VIRCANIF_DEBUG, "%s() canTxPduId: %d\n", __func__, canTxPduId);
//    CanIf_TxConfirmation(canTxPduId);
}


