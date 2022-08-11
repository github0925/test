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

#ifndef SDRV_SLOTS_PARSE_H
#define SDRV_SLOTS_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system_cfg.h"
#include "storage_device.h"
#include "ab_partition_parser.h"
#include "partition_parser.h"


/*
 * Setup partition devices array, and return the specific one.
 *
 * Since the system may have several boot devices, the parameter storage_id_e
 * indicate which the user want to programe.
 * invoker will get the instance of partition device by the passed parameter,
 * with partition device, user can get part name, part offset, attribute, etc.
 * for interface of partition devices, see partition_parser.h
 *
 * Return 0 if successfully, otherwise -1;
 **/

int switch_storage_dev(partition_device_t **ptdev, storage_id_e dev);

/*
 * Count available slots the system support currently.
 *
 * For instance, a system with a single set of partitions would return
 * 1, a system with A/B would return 2, A/B/C -> 3...
 **/

unsigned get_number_slots(partition_device_t *ptdev);

/*
 * The slot num the system boot from, the value letting the system know
 * whether the current slot is A or B. The meaning of A and B is
 * left up to the implementer. It is assumed that if the current slot
 * is A, then the block devices underlying B can be accessed directly
 * without any risk of corruption.
 *
 * Return the active and succful boot slot num.
 **/
unsigned get_current_slot(partition_device_t *ptdev);

/*
 * Marks the current slot as having booted successfully.
 * this shall be invoked once the system boot up successfully.
 * if the successful attribute not get marked after several reboot try.
 * the system will try rollback to old software.
 *
 * Return 0 on success, -1 on error.
 **/
int mark_boot_successful(partition_device_t *ptdev);

/*
 * Marks the slot passed in parameter as the active slot.
 * this shall be invoked after all inactive parts get updated,
 * and get confirmed by user, boot from the updated part after reboot.
 *
 * Return 0 on success, -1 on error.
 **/
int set_active_boot_slot(partition_device_t *ptdev, unsigned slot);

/*
 * Marks the slot passed in parameter as an unbootable.
 * This can be used while updating the contents of the slot's partitions,
 * so that the system will not attempt to boot a known bad set up.
 *
 * Returns 0 on success, -1 on error.
 **/
int set_slot_as_unbootable(partition_device_t *ptdev, unsigned slot);

/*
 * Returns if the slot passed in parameter is bootable.
 * Note that slots can be made unbootable by both the bootloader and
 * by the OS using setSlotAsUnbootable.
 *
 * Returns 1 if the slot is bootable, 0 if it's not, and -1 on error.
 **/
int is_slot_bootable(partition_device_t *ptdev, unsigned slot);

/*
 * Returns the string suffix used by partitions that correspond to
 * the slot number passed in parameter. The returned string is expected
 * to be statically allocated and not need to be freed.
 *
 * Returns NULL if slot does not match an existing slot.
 **/
const char *get_suffix(partition_device_t *ptdev, unsigned slot);

/*
 * Returns if the slot passed in parameter has been marked as successful
 * using markBootSuccessful.
 *
 * Returns 1 if the slot has been marked as successful, 0 if it's not the case,
 * and -1 on error.
 **/

int is_slot_marked_successful(partition_device_t *ptdev, unsigned slot);

/*
 * This interface is for no all parts update in one slot feature.
 * Since the interface set_active_boot_slot, set_slot_as_unbootable will mark all
 * parts attribute in a slot, but user may want to update several parts instead
 * of all parts. User can invoke this interface to update single part attribute.
 *
 * Get part name from partition_device_t.
 *
 * Return 0 on success, otherwise return -1;
 **/

int setAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                            char *partName, part_attr_t attr);

/*
 * Same with set_part_attr, clean  single part attribute.
 *
 * Return 0 on success, otherwise return -1;
 **/
int clearAttributeByPartition(partition_device_t *ptdev, unsigned slot,
                              char *partName,  part_attr_t attr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // SDRV_SLOTS_PARSE_H
