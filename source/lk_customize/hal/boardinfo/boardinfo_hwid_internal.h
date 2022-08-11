/*
 * boardinfo_hwid_internal.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#ifndef __BOARD_INFO_INTERNAL_H__
#define __BOARD_INFO_INTERNAL_H__

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
                printf("not recognize the  part\n");    \
                goto err;   \
        }

#endif
