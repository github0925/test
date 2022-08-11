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

#include "stdio.h"
#include "debug.h"

#include "boardinfo_common.h"
#include "boardinfo_hwid_hw.h"
#include "fuse_ctrl.h"
#include <reg.h>
#include "lib/reg.h"
#include "string.h"
#include "boardinfo_hwid_internal.h"
struct sd_hwid_stor g_hwid_stor = {0};
#define PRODUCT_ID_INDEX 10
//system api
//static bool program storage
bool sys_program_hwid(struct sd_hwid_stor *hwid)
{
    uint32_t v;
    memcpy(&v, hwid, 4);
    //program ospi
    //program eeprom
    boardinfo_write(STOR_ITEM_HWID, &v, 4);
    //program emmc rpmb
    return true;
}
static int get_part_id_from_hwid(struct sd_hwid_stor *hwid,
                                 enum part_e part)
{
    if (hwid->magic != HW_ID_MAGIC)
        goto err;

    if (hwid->ver == 1) {
        struct version1 *v = &(hwid->v.v1);
        GET_PART_ID(part, v);
    }

err:
    return -1;
}

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
            hwid->v.v1.chipid = CHIPID_X9U;
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
//get from ospi/eeprom/emmc rpmb
static bool sys_recognize_hwid_from_hw(struct sd_hwid_stor *hwid_stor)
{
    struct sd_hwid_stor hwid = {0};
    uint32_t product_id;
    uint8_t major, minor, rev;
    //only board id will get from stor. others part will get from efuse.
    //read prt num from efuse
    product_id = fuse_read(PRODUCT_ID_INDEX);
#if !SUPPORT_FAST_BOOT
    printf("product id 0x%x\n", product_id);
#endif
    minor = product_id & 0xff;
    major = (product_id >> 8) & 0xff;
    rev = (product_id >> 16) & 0xff;
    //found board id from ospi
    //found board id from eeprom
    boardinfo_read(STOR_ITEM_HWID, &hwid, 4);

    if (get_part_id_from_hwid(&hwid, PART_BOARD_TYPE) == BOARD_TYPE_MS) {
        struct sd_hwid_stor hwid_part2 = {0};
        boardinfo_read_part2(STOR_ITEM_HWID, &hwid_part2, 4);

        if (hwid_part2.magic == HW_ID_MAGIC && hwid_part2.ver == 1) {
            hwid.v.v1.board_id_minor = hwid_part2.v.v1.board_id_minor;
        }
    }

    //found board id from emmc rpmb
    if (hwid.magic != HW_ID_MAGIC) { //force set the unknown version
        memset(&hwid, 0, sizeof(struct sd_hwid_stor));
        hwid.magic = HW_ID_MAGIC;
        hwid.ver = 1;
    }

    translate_product_id_to_hwid(&hwid, minor, major, rev);
    *hwid_stor = hwid;
    return true;
}
static void converthwid_to_usr(struct sd_hwid_stor *stor,
                               struct sd_hwid_usr *usr)
{
    struct sd_hwid_usr *s = (struct sd_hwid_usr *)stor;
    *usr = *s;
}
//save hwid to rstgen reg/dcf propery
static void save_hwid_to_system_for_others(struct sd_hwid_usr *hwid)
{
    uint32_t *v = (uint32_t *)hwid;
    writel(*v, _ioaddr(RSTGEN_HWID_REG));
#if !SUPPORT_FAST_BOOT
    printf("get hwid 0x%x\n", *v);
#endif
}

void init_hwid(void)
{
    struct sd_hwid_stor hwid_stor;
    struct sd_hwid_usr hwid_usr;
    sys_recognize_hwid_from_hw(&hwid_stor);
    converthwid_to_usr(&hwid_stor, &hwid_usr);
    save_hwid_to_system_for_others(&hwid_usr);
}


