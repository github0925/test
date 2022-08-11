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

#include "boardinfo_hwid_usr.h"
#include <reg.h>
#include "lib/reg.h"
#include "string.h"
#include "boardinfo_hwid_internal.h"

struct sd_hwid_usr g_hwid_usr = {0};
static struct sd_hwid_usr g_hwid_usr_dummy = {
    .magic = HW_ID_MAGIC,
    .ver = 1,
    .v.v1 = {
        .chipid = 0,
        .featurecode = 0,
        .speed_grade = 0,
        .temp_grade = 0,
        .pkg_type = 0,
        .revision = 0,
        .board_type = BOARD_TYPE_UNKNOWN,
        .board_id_major = BOARDID_MAJOR_UNKNOWN,
        .board_id_minor = 0,
    },
};

static bool is_valid_hwid_usr(struct sd_hwid_usr *id)
{
    return id->magic == HW_ID_MAGIC;
}
void clear_local_hwid(void)
{
    g_hwid_usr.magic = 0;
}

static bool init_hwid_from_system(struct sd_hwid_usr *id)
{
    struct sd_hwid_usr *usr;
    int count = 10000;
    uint32_t v;

    do {
        v = readl(_ioaddr(RSTGEN_HWID_REG));
        usr = (struct sd_hwid_usr *)&v;
        spin(100);
    } while (usr->magic != HW_ID_MAGIC && count-- > 0);

    if (usr->magic != HW_ID_MAGIC) {
#if !SUPPORT_FAST_BOOT
        printf("get hwid timeout, will use the dummy one?\n");
#endif
        *usr = g_hwid_usr_dummy;
    }

    //get hwid from rstgen reg
    *id = *usr;
    return true;
}

struct sd_hwid_usr get_fullhw_id(void)
{
    if (!is_valid_hwid_usr(&g_hwid_usr))
        init_hwid_from_system(&g_hwid_usr);

    return g_hwid_usr;
}

int get_part_id(enum part_e part)
{
    if (!is_valid_hwid_usr(&g_hwid_usr))
        init_hwid_from_system(&g_hwid_usr);

    if (g_hwid_usr.magic != HW_ID_MAGIC)
        panic("magic num is not right, %d\n", g_hwid_usr.magic);

    if (g_hwid_usr.ver == 1) {
        struct version1 *v = &(g_hwid_usr.v.v1);
        GET_PART_ID(part, v);
    }

err:
    printf("code is too old to parse the id\n");
    return 0;
}

void dump_all_part_id(void)
{
    enum part_e i;
    printf("sizeof g_hwid %d\n", (int)sizeof(g_hwid_usr));

    for (i = PART_CHIPID; i <= PART_BOARD_ID_MIN; i++)
        printf("part %d:%d\n", i, get_part_id(i));
}
char *get_hwid_friendly_name(char *hwid, int len)
{
    enum sd_board_type_e type = BOARD_TYPE_UNKNOWN;

    //chipid
    switch (get_part_id(PART_CHIPID)) {
        case CHIPID_X9H:
            strcpy(hwid, "X9H-");
            break;

        case CHIPID_X9M:
            strcpy(hwid, "X9M-");
            break;

        case CHIPID_X9E:
            strcpy(hwid, "X9E-");
            break;

        case CHIPID_X9P:
            strcpy(hwid, "X9P-");
            break;

        case CHIPID_X9U:
            strcpy(hwid, "X9U-");
            break;

        case CHIPID_G9S:
        case CHIPID_G9X:
        case CHIPID_G9E:
            strcpy(hwid, "G9-");
            break;

        case CHIPID_V9L:
        case CHIPID_V9T:
        case CHIPID_V9F:
            strcpy(hwid, "V9-");
            break;

        case CHIPID_D9A:
            strcpy(hwid, "D9-");
            break;

        default:
            strcpy(hwid, "UNKNOWN-");
            break;
    }

    //board type
    type = get_part_id(PART_BOARD_TYPE);

    switch (type) {
        case BOARD_TYPE_EVB:
            strcat(hwid, "EVB-");
            break;

        case BOARD_TYPE_REF:
            strcat(hwid, "REF-");
            break;

        case BOARD_TYPE_MS:
            strcat(hwid, "MS_");
            break;

        default:
            strcat(hwid, "UNKNOWN-");
            break;
    }

    //boardid
    if (type == BOARD_TYPE_MS) {
        int minor = 0;
        enum sd_boardid_ms_major_e ms_maj = get_part_id(PART_BOARD_ID_MAJ);

        switch (ms_maj) {
            case BOARDID_MAJOR_MPS:
                strcat(hwid, "MPS-");
                break;

            case BOARDID_MAJOR_TI_A01:
                strcat(hwid, "TI-");
                break;

            case BOARDID_MAJOR_TI_A02:
                strcat(hwid, "A02-");
                break;

            default:
                strcat(hwid, "UNKNOWN-");
                break;
        }

        minor = get_part_id(PART_BOARD_ID_MIN);

        switch (minor) {
            case BOARDID_MINOR_UNKNOWN:
                strcat(hwid, "UNKNOWN");
                break;

            default:
                sprintf(hwid, "%sBS%02d", hwid, minor);
                break;
        }
    }
    else {
        switch (get_part_id(PART_BOARD_ID_MAJ)) {
            case BOARDID_MAJOR_UNKNOWN:
                strcat(hwid, "UNKNOWN");
                break;

            case BOARDID_MAJOR_G9A:
            case BOARDID_MAJOR_A:
                sprintf(hwid, "%sA%02d", hwid, get_part_id(PART_BOARD_ID_MIN));
                break;

            default:
                strcat(hwid, "UNKNOWN");
                break;
        }
    }

    return hwid;
}

