/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

/*

OTA flow:
---------------|----------------------------------------------------------------------------------------------------
               |      SLOT       |       ATTR      |          DIL         |                linux
---------------|----------------------------------------------------------------------------------------------------
first bootup   |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |        0        |           0          | +active +successful +retry(3) +bootable
               |      B (no data)|        0        |           0          |                    0
---------------|----------------------------------------------------------------------------------------------------
second bootup  |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |   A + S + R + B |      A + S + R + B   |          A + S + R + B
               |      B (no data)|        0        |            0         |               0
---------------|----------------------------------------------------------------------------------------------------
update         |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |   A + S + R + B |      A + S + R + B   |          A + S + R + B      - A
               |      B (NEW)    |         0       |            0         |               0             + A + B + R(3)
---------------|----------------------------------------------------------------------------------------------------
reboot         |
---------------|----------------------------------------------------------------------------------------------------
               |      A          |   S + R + B     |       S + R + B      |          S + R + B
     >>>       |      B (NEW)    |   A + B + R(3)  |       A + B + R(2)   |          A + B + R(2)       + S
---------------|----------------------------------------------------------------------------------------------------
normal boot    |
---------------|----------------------------------------------------------------------------------------------------
               |      A (data)   |   S + R + B     |       S + R + B      |          S + R + B
     >>>       |      B (data)   | A + S + B + R(2)|     A + S + B + R(2) |          A + S + B + R(2)
---------------|----------------------------------------------------------------------------------------------------


roll back:
---------------|----------------------------------------------------------------------------------------------------
               |      SLOT       |         ATTR        |          DIL         |                linux
---------------|----------------------------------------------------------------------------------------------------
first bootup   |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |          0          |           0          | +active +successful +retry(3) +bootable
               |      B (no data)|          0          |           0          |                    0
---------------|----------------------------------------------------------------------------------------------------
second bootup  |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |     A + S + R + B   |      A + S + R + B   |      A + S + R + B
               |      B (no data)|           0         |            0         |               0
---------------|----------------------------------------------------------------------------------------------------
update         |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A          |     A + S + R + B   |      A + S + R + B   |      A + S + R + B       - A
               |      B (NEW)    |           0         |            0         |               0          + A + B + R(3)
---------------|----------------------------------------------------------------------------------------------------
reboot         |
---------------|----------------------------------------------------------------------------------------------------
               |      A          |      S + R + B      |       S + R + B      |      S + R + B
     >>>       |      B (NEW)    |      A + B + R(3)   |       A + B + R(2)   |      A + B + R(2)
---------------|----------------------------------------------------------------------------------------------------
reboot         |
---------------|----------------------------------------------------------------------------------------------------
               |      A (data)   |      S + R + B      |       S + R + B      |      S + R + B
     >>>       |      B (NEW)    |      A + B + R(2)   |       A + B + R(1)   |      A + B + R(1)
---------------|----------------------------------------------------------------------------------------------------
reboot         |
---------------|----------------------------------------------------------------------------------------------------
               |      A (data)   |      S + R + B      |       S + R + B      |      S + R + B
     >>>       |      B (NEW)    |      A + B + R(1)   |       A + B + R(0)   |      A + B + R(0)
---------------|----------------------------------------------------------------------------------------------------
reboot         |
---------------|----------------------------------------------------------------------------------------------------
     >>>       |      A (data)   |      S + R + B      |    S + R + B + A     |      A + S + R + B
               |      B (NEW)    |      A + B + R(0)   | A + B + R(0) - A - B |            0
---------------|----------------------------------------------------------------------------------------------------

*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "slots_parse.h"

#define LBA_SIZE 512
#define MAX_CALLOC_SIZE (256*1024)
#define USE_RW

int OTA_programming(partition_device_t *ptdev);

int main(int argc, char *argv[])
{
    int ret, slot, inactive_slot;
    partition_device_t *ptdev = NULL;
    const char *suffix;

    /* 1. FOR EMMC */
    if ((1 == argc) || (!strcmp(argv[1], "emmc"))) {
        ret = switch_storage_dev(&ptdev, DEV_EMMC0);
    }
    else if (!strcmp(argv[1], "ospi")) {
        ret = switch_storage_dev(&ptdev, DEV_SPI_NOR0);
    }
    else {
        PRINTF_INFO("Usage: %s <option>\n", argv[0]);
        PRINTF_INFO("%s               : emmc OTA\n", argv[0]);
        PRINTF_INFO("%s ospi          : ospi OTA\n", argv[0]);
        return -1;
    }

    if (0 != ret) {
        PRINTF_CRITICAL("failed to setup ptdev\n");
        return -1;
    }

    /* dump current part */
    PRINTF_INFO("first bootup dump ptdev.\n");
    ptdev_dump(ptdev);
    ptdev_attr_dump(ptdev);
    /*  first bootup attribute */
    /*
        active  bootable  success  retry  name
          N       Y         N        0    bootloader_a
          N       Y         N        0    bootloader_b
          N       Y         N        0    rootfs_a
          N       Y         N        0    rootfs_b
          N       Y         N        0    dtb_a
          N       Y         N        0    dtb_b
          N       Y         N        0    kernel_a
          N       Y         N        0    kernel_b
          N       Y         N        0    userdata
    */

    /* All slots are zeroed, mark SLOT 0, active & bootable */
    ret = get_number_slots(ptdev);
    PRINTF_INFO("slots count %d\n", ret);
    slot = get_current_slot(ptdev);
    PRINTF_INFO("corrent slot %d\n", slot);

    if (slot == INVALID) {
        PRINTF_INFO("bad slot %d\n", slot);
        return -1;
    }

    suffix = get_suffix(ptdev, slot);
    PRINTF_INFO("slot suffix %s\n", suffix);

    inactive_slot = get_inverse_slot(ptdev, slot);

    /* mark bootup successfully if necessary */
    ret = is_slot_marked_successful(ptdev, slot);

    if (ret != 1) {
        mark_boot_successful(ptdev);
    }

    /* normal bootup attribute */
    /*
         active  bootable  success  retry  name
            Y       Y         Y        3    bootloader_a
            N       Y         N        0    bootloader_b
            Y       Y         Y        3    rootfs_a
            N       Y         N        0    rootfs_b
            Y       Y         Y        3    dtb_a
            N       Y         N        0    dtb_b
            Y       Y         Y        3    kernel_a
            N       Y         N        0    kernel_b
            Y       Y         Y        3    userdata
    */
    ptdev_attr_dump(ptdev);

    /* OTA update */
    PRINTF_INFO("OTA updating ... \n");
    ret = OTA_programming(ptdev);

    if (ret < 0) {
        PRINTF_INFO("OTA_programming error\n");
        return -1;
    }

    /* successful */
    ptdev_clean_slot_attr(ptdev, inactive_slot, ATTR_SUCCESSFUL);
    /* retry */
    ptdev_mark_slot_attr(ptdev, inactive_slot, ATTR_RETRY);

    /* active & bootable */
    set_active_boot_slot(ptdev, inactive_slot);

    /* OTA finished, pending reboot */
    /*
        active  bootable  success  retry  name
          N       Y         Y        3    bootloader_a
          Y       Y         N        3    bootloader_b
          N       Y         Y        3    rootfs_a
          Y       Y         N        3    rootfs_b
          N       Y         Y        3    dtb_a
          Y       Y         N        3    dtb_b
          N       Y         Y        3    kernel_a
          Y       Y         N        3    kernel_b
          Y       Y         Y        3    userdata
    */
    ptdev_attr_dump(ptdev);
    sync();
    PRINTF_INFO("OTA finish, try reboot to take effect \n");


    /* OTA successfully, after rebooting attr shall be: */
    /*
        active  bootable  success  retry  name
          N       Y         Y        3    bootloader_a
          Y       Y         Y        3    bootloader_b
          N       Y         Y        3    rootfs_a
          Y       Y         Y        3    rootfs_b
          N       Y         Y        3    dtb_a
          Y       Y         Y        3    dtb_b
          N       Y         Y        3    kernel_a
          Y       Y         Y        3    kernel_b
          Y       Y         Y        3    userdata
    */

    ptdev_destroy(ptdev);

    return 0;
}

/* Just duplicate all parts, shall be implemented by vendor */
int OTA_programming(partition_device_t *ptdev)
{
    int slot_active[64] = {0};
    int slot_inactive[64] = {0};
    int active_slot = 0;
    int inactive_slot = 0;
    int len = 0;
    int ret = -1;
    unsigned long long in = 0;
    unsigned long long out = 0;
    unsigned long long size = 0;
    unsigned long long left = 0;
    unsigned long long cnt = 0;
    char *pname = NULL;
    const char *suffix = NULL;
    int index = 0;
    unsigned char *data;
    unsigned partition_count = ptdev->count;
    struct partition_entry *partition_entries = ptdev->partition_entries;
    storage_device_t *storage = ptdev->storage;
    unsigned long long gpt_offset = ptdev->gpt_offset;

    active_slot = get_current_slot(ptdev);
    suffix = get_suffix(ptdev, active_slot);

    for (int i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        len = strlen(pname);

        if (len < 4)
            continue; /* too few, ignore */

        for (int j = i + 1; j < partition_count; j++) {
            if ((!strncmp(pname, (char *)partition_entries[j].name, len - 2)) &&
                    (len == strlen((char *)partition_entries[j].name))) {
                if (strstr(pname, suffix)) {
                    slot_active[index] = i;
                    slot_inactive[index] = j;
                }
                else {
                    slot_active[index] = j;
                    slot_inactive[index] = i;
                }

                index++;
            }
        }
    }

    for (int i = 0; i < index; i++) {
        active_slot = slot_active[i];
        inactive_slot = slot_inactive[i];
        in = (partition_entries[active_slot].first_lba) * LBA_SIZE;
        out = (partition_entries[inactive_slot].first_lba) * LBA_SIZE;
        size = (partition_entries[active_slot].last_lba -
                partition_entries[active_slot].first_lba + 1) * LBA_SIZE;

        in += gpt_offset;
        out += gpt_offset;

        PRINTF_INFO("i=%d\n", i);
        PRINTF_INFO("active_slot=%s\n", partition_entries[active_slot].name);
        PRINTF_INFO("inactive_slot=%s\n", partition_entries[inactive_slot].name);
        PRINTF_INFO("in=%p out=%p size=%lld\n", (void *)in, (void *)out, size);

#ifdef USE_RW
        cnt =  (size / MAX_CALLOC_SIZE);
        left = (size % MAX_CALLOC_SIZE);
        data = (unsigned char *)calloc(1, (cnt != 0 ? (MAX_CALLOC_SIZE) : size));

        if (!data) {
            PRINTF_INFO("OTA_programming calloc failed\n");
            return -1;
        }

        for (unsigned long long n = 0; n < cnt; n++) {
            ret = storage->read(storage, in, data, MAX_CALLOC_SIZE);

            if (ret < 0) {
                PRINTF_INFO("OTA_programming storage->read failed\n");
                free(data);
                return -1;
            }

            storage->write(storage, out, data, MAX_CALLOC_SIZE);

            if (ret < 0) {
                PRINTF_INFO("OTA_programming storage->write failed\n");
                free(data);
                return -1;
            }

            in  += MAX_CALLOC_SIZE;
            out += MAX_CALLOC_SIZE;
        }

        if (0 != left) {
            ret = storage->read(storage, in, data, left);

            if (ret < 0) {
                PRINTF_INFO("OTA_programming storage->read failed\n");
                free(data);
                return -1;
            }

            ret = storage->write(storage, out, data, left);

            if (ret < 0) {
                PRINTF_INFO("OTA_programming storage->write failed\n");
                free(data);
                return -1;
            }
        }


        free(data);

#else
        /* only copy data, not for download data
          in and out must align to 4k
        */
        ret = storage->copy(storage, in, out, size);

        if (ret < 0) {
            PRINTF_INFO("OTA_programming storage->copy failed\n");
            return -1;
        }

#endif

    }

    return 0;
}
