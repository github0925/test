/*
 * boardinfo_common.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#ifndef __BOARD_INFO_COMMON_H__
#define __BOARD_INFO_COMMON_H__

enum stor_item_e {
    STOR_ITEM_HWID,
};

int boardinfo_write(enum stor_item_e item, const void *data, int len);
int boardinfo_read(enum stor_item_e item, void *data, int len);
int boardinfo_write_part2(enum stor_item_e item, const void *data,
                          int len);
int boardinfo_read_part2(enum stor_item_e item, void *data, int len);
int boardinfo_get_serialno(uint32_t *uuid);
#endif

