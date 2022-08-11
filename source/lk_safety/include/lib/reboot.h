/*
 * reboot.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _REBOOT_H_
#define _REBOOT_H_

#pragma pack(push)
#pragma pack(4)
typedef union reboot_args {
    struct {
        uint32_t reason : 4;
        uint32_t source : 4;
        uint32_t para   : 24;
    } args;

    uint32_t val;
} reboot_args_t;
#pragma pack(pop)

#endif
