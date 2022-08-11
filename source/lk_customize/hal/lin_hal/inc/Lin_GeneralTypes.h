/*
 * Lin_GeneralTypes.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _LIN_GENERALTYPES_H
#define _LIN_GENERALTYPES_H

#include <stdint.h>

/* platform type */
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
/* AUTOSAR standard */
typedef uint8 Std_ReturnType;
typedef uint8_t Lin_FramePidType;
typedef uint8_t Lin_FrameDlType;

typedef enum Lin_FrameCsModelTypeTag {
    LIN_ENHANCED_CS = 0, /* Enhanced checksum */
    LIN_CLASSIC_CS  = 1  /* Classic checksum */
} Lin_FrameCsModelType;

typedef enum Lin_FrameResponseTypeTag {
    LIN_MASTER_RESPONSE = 0, /* Response is generated from this (master) node */
    LIN_SLAVE_RESPONSE  = 1, /* Response is generated from a remote slave node */
    LIN_SLAVE_TO_SLAVE  = 2  /* Response is generated from one slave to another slave */
} Lin_FrameResponseType;

#define LIN_UNINIT              (0x01U)
#define LIN_INIT                (0x02U)

typedef enum Lin_ChStatusTag {
    LIN_CH_SLEEP_PENDING,
    LIN_CH_SLEEP_STATE,
    LIN_CH_OPERATIONAL
} Lin_ChStatusType;


#endif /* _LIN_GENERALTYPES_H */
