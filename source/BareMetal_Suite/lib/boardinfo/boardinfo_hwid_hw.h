/*
 * boardinfo_hw.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#ifndef __BOARD_INFO_HW_H__
#define __BOARD_INFO_HW_H__
#include "boardinfo_hwid_usr.h"

#ifndef PACKED
#define PACKED  __attribute__ ((__packed__))
#endif /* PACKED */

struct sd_hwid_stor {
    uint32_t magic: 4; // must be 0x5
    uint32_t ver: 4; // encode version
    union version v;
};

bool sys_recognize_hwid_from_hw(struct sd_hwid_stor *hwid,
                                struct sd_hwid_usr *usr);
void converthwid_to_usr(struct sd_hwid_stor *stor,
                        struct sd_hwid_usr *usr);
#endif
