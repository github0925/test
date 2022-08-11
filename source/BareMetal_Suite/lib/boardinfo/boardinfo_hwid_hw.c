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
#include "boardinfo_hwid_hw.h"
#include "fuse_ctrl.h"
#include "string.h"

#define PRODUCT_ID_INDEX 10

static void translate_product_id_to_hwid(struct sd_hwid_stor *hwid,
        uint8_t minor, uint8_t major, uint8_t rev)
{
    //uint16_t chipid = (major<<8 | minor);
    switch (major) {
        case 0x90:
            hwid->v.v1.chipid = CHIPID_G9X;
            break;

        case 0x91:
            hwid->v.v1.chipid = CHIPID_V9F;
            break;

        case 0x92:
            hwid->v.v1.chipid = CHIPID_X9E;
            break;

        case 0x93:
            hwid->v.v1.chipid = CHIPID_D9A;
            break;

        case 0x94:
            hwid->v.v1.chipid = CHIPID_X9M;
            break;

        case 0x96:
            hwid->v.v1.chipid = CHIPID_X9H;
            break;

        case 0x97:
            hwid->v.v1.chipid = CHIPID_V9T;
            break;

        case 0x98:
            hwid->v.v1.chipid = CHIPID_X9P;
            break;

        default:
            hwid->v.v1.chipid = CHIPID_UNKNOWN;
            break;
    }

    switch (rev) {
        case 0x01:
            hwid->v.v1.revision = 1;
            break;

        default:
            hwid->v.v1.revision = 0;
            break;
    }

    return;
}

bool sys_recognize_hwid_from_hw(struct sd_hwid_stor *hwid, struct sd_hwid_usr *usr)
{
    uint32_t product_id;
    uint8_t major, minor, rev;

    /* only board id will get from stor. others part will get from efuse.
    *  read prt num from efuse
    */

    product_id = fuse_read(PRODUCT_ID_INDEX);
    minor = product_id & 0xff;
    major = (product_id >> 8) & 0xff;
    rev = (product_id >> 16) & 0xff;

    if (!hwid)
        return false;

    if (hwid->magic != HW_ID_MAGIC) { //force set the unknown version
        memset(hwid, 0, sizeof(struct sd_hwid_stor));
        hwid->magic = HW_ID_MAGIC;
        hwid->ver = 1;
    }

    translate_product_id_to_hwid(hwid, minor, major, rev);

    if (usr)
        converthwid_to_usr(hwid, usr);

    return true;
}

void converthwid_to_usr(struct sd_hwid_stor *stor,
                               struct sd_hwid_usr *usr)
{
    struct sd_hwid_usr *s = (struct sd_hwid_usr *)stor;
    *usr = *s;
}
