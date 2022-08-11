/*
 * boardinfo_usr.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#ifndef __BOARD_INFO_USR_H__
#define __BOARD_INFO_USR_H__
#include "__regs_base.h"

#ifndef PACKED
#define PACKED  __attribute__ ((__packed__))
#endif /* PACKED */

#define HW_ID_MAGIC 0x5
enum sd_chipid_e {//5 bit
    CHIPID_UNKNOWN,
    CHIPID_X9E,
    CHIPID_X9M,
    CHIPID_X9H,
    CHIPID_X9P,
    CHIPID_G9S,
    CHIPID_G9X,
    CHIPID_G9E,
    CHIPID_V9L,
    CHIPID_V9T,
    CHIPID_V9F,
    CHIPID_D9A,
};

enum sd_board_type_e {//2 bit
    BOARD_TYPE_UNKNOWN,
    BOARD_TYPE_EVB,
    BOARD_TYPE_REF,
    BOARD_TYPE_MS,
};

enum sd_boardid_major_e {//3 bit
    BOARDID_MAJOR_UNKNOWN,
    BOARDID_MAJOR_A,
    BOARDID_MAJOR_G9A
};
enum sd_boardid_ms_major_e {//3 bit
    BOARDID_MAJOR_MPS = 1,
    BOARDID_MAJOR_TI_A01,
    BOARDID_MAJOR_TI_A02,
};

enum sd_boardid_ms_minor_e {//4 bit
    BOARDID_MINOR_UNKNOWN,
};
struct version1 {//24 bit
    uint32_t chipid: 5; //chip version
    uint32_t featurecode: 2; //feature code
    uint32_t speed_grade: 2; //
    uint32_t temp_grade: 2; //
    uint32_t pkg_type: 2; //
    uint32_t revision: 2; //
    uint32_t board_type: 2; //
    uint32_t board_id_major:3; //major, in ms board, this will be core boardid,
    uint32_t board_id_minor:4; //minor, in ms board, this will be base boardid,
} PACKED;

union version {
    struct version1 v1;
};

// on system for user
struct sd_hwid_usr {
    uint32_t magic: 4; // must be 0x5
    uint32_t ver: 4; // encode version
    union version v;
} PACKED;


enum part_e {
    PART_CHIPID,
    PART_FEATURE,
    PART_SPEED,
    PART_TEMP,
    PART_PKG,
    PART_REV,
    PART_BOARD_TYPE,
    PART_BOARD_ID_MAJ,
    PART_BOARD_ID_MIN,
};

int get_part_id(struct sd_hwid_usr *hwid, enum part_e part);
bool is_valid_hwid_usr(struct sd_hwid_usr *id);
#endif
