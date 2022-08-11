/*
 * boardinfo.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include "debug.h"

#include "boardinfo_hwid_usr.h"

#define GET_PART_ID(part, v) \
    switch(part)    \
        {   \
            case PART_CHIPID:   \
                return v->chipid;   \
            case PART_FEATURE:  \
                return v->featurecode;  \
            case PART_SPEED:    \
                return v->speed_grade;  \
            case PART_TEMP: \
                return v->temp_grade;   \
            case PART_PKG:  \
                return v->pkg_type; \
            case PART_REV:  \
                return v->revision; \
            case PART_BOARD_TYPE:   \
                return v->board_type;   \
            case PART_BOARD_ID_MAJ: \
                return v->board_id_major;   \
            case PART_BOARD_ID_MIN: \
                return v->board_id_minor;   \
            default:    \
                goto err;   \
        }

bool is_valid_hwid_usr(struct sd_hwid_usr *id)
{
    return id->magic == HW_ID_MAGIC;
}

int get_part_id(struct sd_hwid_usr *hwid, enum part_e part)
{
    if (!hwid ||hwid->magic != HW_ID_MAGIC)
    {
        return 0;
    }

    if (hwid->ver == 1) {
        struct version1 *v = &(hwid->v.v1);
        GET_PART_ID(part, v);
    }

err:
    return 0;
}

