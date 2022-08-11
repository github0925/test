/*
 * board_cfg.h.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:crc.h
 *
 * Revision History:
 * -----------------
 */

#ifndef __CRC_H
#define __CRC_H_

#include "board_start.h"

extern const uint8_t crchitab[];
extern const uint8_t crclotab[];
extern uint16_t crcchks(uint8_t *chkmsg, uint8_t crclen);

#endif
