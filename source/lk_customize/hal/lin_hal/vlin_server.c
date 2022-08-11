/*
 * vlin_server.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Virtual LIN Server driver handles remote function calls, by
 * calling LIN hardware driver.
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>

#include "Lin.h"

#ifdef SDPE
extern Std_ReturnType
virLinIf_SendFrame(uint8 Channel, const Lin_PduType *PduInfoPtr);
extern Lin_StatusType
virLinIf_GetStatus(uint8 Channel, uint8 **Lin_SduPtr);
extern Std_ReturnType
virLinIf_GoToSleep(uint8 Channel);
extern Std_ReturnType
virLinIf_GoToSleepInternal(uint8 Channel);
extern Std_ReturnType
virLinIf_Wakeup(uint8 Channel);
extern Std_ReturnType
virLinIf_WakeupInternal(uint8 Channel);
extern Std_ReturnType
virLinIf_CheckWakeup(uint8 Channel);
#endif

static Lin_ConfigType virLin_Config;
static Lin_ControllerConfigType controller_config[LIN_IFC_CHN_MAX];

void virLin_Init(const Lin_ConfigType *Config)
{
    virLin_Config.Count = Config->Count;
    virLin_Config.Config = controller_config;

    for (uint8_t i = 0; i < virLin_Config.Count; i++) {
        controller_config[i].hrdChannel = Config->Config[i].hrdChannel;
        controller_config[i].sclk = Config->Config[i].sclk;
        controller_config[i].baud = Config->Config[i].baud;
        controller_config[i].data_bits = Config->Config[i].data_bits;
        controller_config[i].stop_bits = Config->Config[i].stop_bits;
        controller_config[i].parity = Config->Config[i].parity;
        controller_config[i].loopback_enable =
                Config->Config[i].loopback_enable;
        controller_config[i].fifo_enable = Config->Config[i].fifo_enable;
        controller_config[i].rx_trigger = Config->Config[i].rx_trigger;
        controller_config[i].tx_trigger = Config->Config[i].tx_trigger;

        dprintf(INFO, "%s() hrdChannel: 0x%x\n", __func__,
                virLin_Config.Config[i].hrdChannel);
        dprintf(INFO,
                "sclk: %d  baud: %d  data_bits: %d  stop_bits: %d  parity: %d\n",
                virLin_Config.Config[i].sclk,
                virLin_Config.Config[i].baud,
                virLin_Config.Config[i].data_bits,
                virLin_Config.Config[i].stop_bits,
                virLin_Config.Config[i].parity);
        dprintf(INFO,
                "loopback_enable: %d  fifo_enable: %d  rx_trigger: %d  tx_trigger: %d\n",
                virLin_Config.Config[i].loopback_enable,
                virLin_Config.Config[i].fifo_enable,
                virLin_Config.Config[i].rx_trigger,
                virLin_Config.Config[i].tx_trigger);
    }

    Lin_Init((const Lin_ConfigType *)&virLin_Config);
}

uint8_t virLin_SendFrame(uint8_t Channel, const Lin_PduType *PduInfoPtr)
{
    dprintf(INFO,
            "%s() channel:%d, Pid: %d, Cs: %d, Drc: %d, Dl: %d, Sdu: ",
            __func__, Channel, PduInfoPtr->Pid, PduInfoPtr->Cs, PduInfoPtr->Drc,
            PduInfoPtr->Dl);

    for (uint32_t i = 0; i < PduInfoPtr->Dl; i++) {
        uint8_t *Ptr = PduInfoPtr->SduPtr;
        dprintf(INFO, "%d ", Ptr[i]);
    }

    dprintf(INFO, "\n");

#ifdef SDPE
    /* When SDPE is enabled, the VLIN frame is actually scheduled
     * by SDPE LIN scheduler.
     */
    return virLinIf_SendFrame(Channel, PduInfoPtr);
#else
    return Lin_SendFrame(Channel, PduInfoPtr);
#endif
}

uint8_t virLin_CheckWakeup(uint8_t Channel)
{
    uint8_t ret = 0;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_CheckWakeup(Channel);
#else
    ret = Lin_CheckWakeup(Channel);
#endif
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);
    return ret;
}

uint8_t virLin_GoToSleep(uint8_t Channel)
{
    uint8_t ret = 0;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_GoToSleep(Channel);
#else
    ret = Lin_GoToSleep(Channel);
#endif
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

uint8_t virLin_GoToSleepInternal(uint8_t Channel)
{
    uint8_t ret = 0;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_GoToSleepInternal(Channel);
#else
    ret = Lin_GoToSleepInternal(Channel);
#endif
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);
    return ret;
}

uint8_t virLin_Wakeup(uint8_t Channel)
{
    uint8_t ret = 0;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_Wakeup(Channel);
#else
    ret = Lin_Wakeup(Channel);
#endif
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);
    return ret;
}

uint8_t virLin_WakeupInternal(uint8_t Channel)
{
    uint8_t ret = 0;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_WakeupInternal(Channel);
#else
    ret = Lin_WakeupInternal(Channel);
#endif
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

uint8_t virLin_GetStatus(uint8_t Channel, uint8_t *Lin_Sdu)
{
    uint8_t ret = 0;
    uint8_t *SduPtr;

    dprintf(INFO, "%s() channel: %d\n", __func__, Channel);
#ifdef SDPE
    ret = virLinIf_GetStatus(Channel, &SduPtr);
#else
    ret = Lin_GetStatus(Channel, &SduPtr);
#endif
    dprintf(INFO, "%s() ret: %d, sdu: ", __func__, ret);

    for (uint8_t i = 0; i < 8; i++) {
        Lin_Sdu[i] = SduPtr[i];
        dprintf(INFO, "%d ", Lin_Sdu[i]);
    }

    dprintf(INFO, "\n");

    return ret;
}
