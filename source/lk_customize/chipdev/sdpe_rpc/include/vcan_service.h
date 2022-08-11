/*
 * vcan_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _VCAN_SERVICE_H
#define _VCAN_SERVICE_H

#include "Can.h"

/* VCAN SERVICE */

int vcan_service_init(void);
void virCan_Init(const Can_ConfigType *config);
void virCan_DeInit(void);
int virCan_SetBaudrate(uint8_t controller, uint16_t baudrate);
int virCan_SetControllerMode(uint8_t controller, uint8_t mode);
int virCan_GetControllerMode(uint8_t controller, uint8_t *mode);
int virCan_EnableControllerInterrupts(uint8_t controller);
int virCan_DisableControllerInterrupts(uint8_t controller);
int virCan_CheckWakeup(uint8_t controller);
int virCan_GetControllerErrorState(uint8_t controller, uint8_t *state);
int virCan_Write(uint16_t hth, const Can_PduType *pdu);
void virCan_ControllerBusOff(uint8_t controller);
void virCan_SetWakeupEvent(uint8_t controller);
void virCan_RxIndication(uint16_t hrh, uint32_t id, uint8_t dlc,
                         uint8_t *pdu);
void virCan_TxConfirmation(uint16_t pdu_id);
void vircan_MainFunction_Read(void);
void vircan_MainFunction_Write(void);
void virCan_ControllerWakeUp(uint8_t controller);

#endif
