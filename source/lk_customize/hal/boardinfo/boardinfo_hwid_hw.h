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

//on the storage
struct sd_hwid_stor {
    uint32_t magic: 4; // must be 0x5
    uint32_t ver: 4; // encode version
    union version v;
};

//system api, and only safety & sec will call this
void init_hwid(void);
bool sys_program_hwid(struct sd_hwid_stor *hwid);

#endif
