/*
 * pll_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: LIN HAL.
 *
 * Revision History:
 * -----------------
 */
#ifndef _LIN_HAL_H
#define _LIN_HAL_H

#include "uart_hal.h"
#include "Lin_Cfg.h"
#include "Lin_GeneralTypes.h"

#ifdef SUPPORT_3RD_ERPC
/* Generated LIN driver interfaces. */
#include "../gen/lin_drv.h"
#endif

#ifndef DBGV
#define DBGV 4
#endif

#ifndef WARN
#define WARN 1
#endif

#ifndef E_OK
#define E_OK     0u
#endif
#ifndef E_NOT_OK
#define E_NOT_OK 1u
#endif
#define NULL_PTR ((void *)0)

/* Error codes */
#define LIN_E_NO_ERROR         0xFFu //No error.
#define LIN_E_UNINIT           0x00u //API service used without module initialization.
#define LIN_E_INVALID_CHANNEL  0x02u //API service used with an invalid or inactive channel parameter.
#define LIN_E_INVALID_POINTER  0x03u //API service called with invalid configuration pointer.
#define LIN_E_STATE_TRANSITION 0x04u //Invalid state transition for the current state.
#define LIN_E_PARAM_POINTER    0x05u //API service called with a NULL pointer.
#define LIN_E_PARAM_VALUE      0x06u //API service called with invalid parameter value.

#ifndef SUPPORT_3RD_ERPC

// Aliases data types declarations
typedef struct Lin_PduType Lin_PduType;
typedef struct Lin_ControllerConfigType Lin_ControllerConfigType;
typedef struct Lin_ConfigType Lin_ConfigType;

// Structures/unions data types declarations
struct Lin_PduType
{
    uint8_t Pid;
    uint8_t Cs;
    uint8_t Drc;
    uint8_t Dl;
    uint8_t * SduPtr;
};

struct Lin_ControllerConfigType
{
    uint32_t hrdChannel;
    uint32_t sclk;
    uint32_t baud;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t loopback_enable;
    uint8_t fifo_enable;
    uint8_t rx_trigger;
    uint8_t tx_trigger;
};

struct Lin_ConfigType
{
    uint32_t Count;
    Lin_ControllerConfigType * Config;
};

#endif

void Lin_Init(const Lin_ConfigType *Config);
Std_ReturnType Lin_SendFrame(uint8 Channel,
                             const Lin_PduType *PduInfoPtr);
Std_ReturnType Lin_CheckWakeup(uint8 Channel);
Std_ReturnType Lin_GoToSleep(uint8 Channel);
Std_ReturnType Lin_GoToSleepInternal(uint8 Channel);
Std_ReturnType Lin_Wakeup(uint8 Channel);
Std_ReturnType Lin_WakeupInternal(uint8 Channel);
Lin_StatusType Lin_GetStatus(uint8 Channel, uint8 **Lin_SduPtr);

#endif /* _LIN_HAL_H */

