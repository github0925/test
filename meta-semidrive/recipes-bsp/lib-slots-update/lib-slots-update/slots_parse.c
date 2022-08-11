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

#ifdef __cplusplus
extern "C" {
#endif

#include "system_cfg.h"
#include "ab_partition_parser.h"
#include "slots_parse.h"

static partition_device_t *ptdev_list[DEV_MAX];

unsigned get_number_slots(partition_device_t *ptdev)
{
    if (ptdev_multislot_is_supported(ptdev))
        return AB_SUPPORTED_SLOTS;

    return 1;
}

#ifdef SUPPORT_PART_UPDATE_ISOLATED
unsigned get_current_slot(partition_device_t *ptdev)
{
    /* logical slot, not physics */
    if (ptdev_multislot_is_supported(ptdev))
        return SLOT_A;

    return INVALID;
}
#else
unsigned get_current_slot(partition_device_t *ptdev)
{
    signed active_slot = ptdev_find_active_slot(ptdev);

    if (ptdev->attributes_updated) {
        ptdev_attributes_update(ptdev);
        ptdev->attributes_updated = false;
    }

    return active_slot;
}
#endif

int mark_boot_successful(partition_device_t *ptdev)
{
    signed active_slot = ptdev_find_active_slot(ptdev);
    ptdev_mark_slot_attr_noupdate(ptdev, active_slot, ATTR_SUCCESSFUL);

    if (ptdev->attributes_updated) {
        ptdev_attributes_update(ptdev);
        ptdev->attributes_updated = false;
    }

    return 0;
}

int set_active_boot_slot(partition_device_t *ptdev, unsigned slot)
{
    ptdev_mark_slot_attr_noupdate(ptdev, slot, ATTR_ACTIVE);
    ptdev_mark_slot_attr_noupdate(ptdev, slot, ATTR_RETRY);
    ptdev_clean_slot_attr_noupdate(ptdev, slot, ATTR_UNBOOTABLE);

    if (ptdev->attributes_updated) {
        ptdev_attributes_update(ptdev);
        ptdev->attributes_updated = false;
    }

    if (get_current_slot(ptdev) == slot)
        return 0;

    return -1;

}

int set_slot_as_unbootable(partition_device_t *ptdev, unsigned slot)
{
    ptdev_mark_slot_attr(ptdev, slot, ATTR_UNBOOTABLE);
    return 0;
}

int is_slot_bootable(partition_device_t *ptdev, unsigned slot)
{
    int ret = ptdev_find_boot_slot(ptdev);

    if (ret == AB_SUPPORTED_SLOTS)
        return 1;
    else
        return (ret == slot) ? 1 : 0;
}

const char *get_suffix(partition_device_t *ptdev, unsigned slot)
{
    if (slot < AB_SUPPORTED_SLOTS)
        return suffix_slot[slot];

    return NULL;
}

int is_slot_marked_successful(partition_device_t *ptdev, unsigned slot)
{
    int ret = ptdev_find_successfull_slot(ptdev);

    if (ret == AB_SUPPORTED_SLOTS)
        return 1;
    else
        return (ret == slot) ? 1 : 0;
}

int switch_storage_dev(partition_device_t **ptdev, storage_id_e dev)
{
    storage_device_t *storage;
    *ptdev = NULL;

    if (!ptdev_list[dev]) {
        storage = setup_storage_dev(dev);

        if (!storage) {
            PRINTF_CRITICAL("failed to setup storage dev\n");
            return -1;
        }

        ptdev_list[dev] = ptdev_setup(storage, storage->gpt_offset);

        if (!ptdev_list[dev]) {
            PRINTF_CRITICAL("failed to setup ptdev\n");
            return -1;
        }

        ptdev_read_table(ptdev_list[dev]);
    }

    *ptdev = ptdev_list[dev];
    return 0;
}

#ifdef SUPPORT_PART_UPDATE_ISOLATED
int setAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                            char *partName, part_attr_t attr)
{
    /* TODO */
    return 0;
}

int clearAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                              char *partName, part_attr_t attr)
{
    /* TODO */
    return 0;
}

#else

int setAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                            char *partName, part_attr_t attr)
{
    return ptdev_mark_part_attr(ptdev, slot, partName, attr);
}

int clearAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                              char *partName, part_attr_t attr)
{
    return ptdev_clean_part_attr(ptdev, slot, partName, attr);
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */