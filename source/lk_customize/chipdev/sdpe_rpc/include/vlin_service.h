/*
 * vlin_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _VLIN_SERVICE_H
#define _VLIN_SERVICE_H

#include "Lin.h"

/* VLIN SERVICE */

int vlin_service_init(void);
int virLin_Init(const Lin_ConfigType *config);
int virLin_CheckWakeup(uint8_t channel);
int virLin_GoToSleep(uint8_t channel);
int virLin_GoToSleepInternal(uint8_t channel);
int virLin_Wakeup(uint8_t channel);
int virLin_WakeupInternal(uint8_t channel);
int virLin_GetStatus(uint8_t channel, uint8_t *lin_sdu);
int virLin_SendFrame(uint8_t channel, const Lin_PduType *pdu);

#endif
