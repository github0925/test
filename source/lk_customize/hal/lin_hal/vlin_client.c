/*
 * vlin_client.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Virtual LIN Client driver implements standard LIN driver
 * functions, by forwarding the calls to remote Virtual LIN server.
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>
#include <string.h>

#include "Lin.h"
#ifdef SUPPORT_SDPE_RPC
#include "vlin_service.h"
#endif

void Lin_Init(const Lin_ConfigType *Config)
{
    virLin_Init(Config);
}

Std_ReturnType Lin_SendFrame(uint8_t Channel,
                             const Lin_PduType *PduInfoPtr)
{
    Std_ReturnType ret;

    dprintf(INFO,
            "%s() channel:%d, Pid: %d, Cs: %d, Drc: %d, Dl: %d, Sdu: \n",
            __func__, Channel, PduInfoPtr->Pid,
            PduInfoPtr->Cs, PduInfoPtr->Drc, PduInfoPtr->Dl);

    for (uint32_t i = 0; i < PduInfoPtr->Dl; i++) {
        uint8_t *Ptr = PduInfoPtr->SduPtr;
        dprintf(INFO, "0x%x ", Ptr[i]);
    }

    dprintf(INFO, "\n");

    ret = virLin_SendFrame(Channel, PduInfoPtr);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Std_ReturnType Lin_CheckWakeup(uint8_t Channel)
{
    Std_ReturnType ret;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_CheckWakeup(Channel);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Std_ReturnType Lin_GoToSleep(uint8_t Channel)
{
    Std_ReturnType ret;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_GoToSleep(Channel);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Std_ReturnType Lin_GoToSleepInternal(uint8_t Channel)
{
    Std_ReturnType ret;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_GoToSleepInternal(Channel);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Std_ReturnType Lin_Wakeup(uint8_t Channel)
{
    Std_ReturnType ret;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_Wakeup(Channel);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Std_ReturnType Lin_WakeupInternal(uint8_t Channel)
{
    Std_ReturnType ret;

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_WakeupInternal(Channel);
    dprintf(INFO, "%s() ret: %d\n", __func__, ret);

    return ret;
}

Lin_StatusType Lin_GetStatus(uint8_t Channel, uint8_t **Lin_SduPtr)
{
    Lin_StatusType ret;
    static uint8_t SduPtr[LIN_IFC_CHN_MAX][LIN_FRAME_DATA_LENGTH];

    dprintf(INFO, "%s() Channel: %d\n", __func__, Channel);

    ret = virLin_GetStatus(Channel, SduPtr[Channel]);
    *Lin_SduPtr = SduPtr[Channel];

    dprintf(INFO, "%s() ret: %d, sdu: ", __func__, ret);

    for (uint8_t i = 0; i < 8; i++) {
        dprintf(INFO, "0x%x ", SduPtr[Channel][i]);
    }

    dprintf(INFO, "\n");

    return ret;
}
