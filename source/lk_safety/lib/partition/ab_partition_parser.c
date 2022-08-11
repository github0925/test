/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <ab_partition_parser.h>
#include <partition_parser.h>
#include <arch.h>
#if defined(MMC_SDHCI_SUPPORT) || defined(UFS_SUPPORT)
#include <ufs.h>
#endif
#include <debug.h>
#include <lib/cksum.h>

//#define AB_DEBUG

/* Slot suffix */
const char *suffix_slot[] = {"_a",
                             "_b"
                            };
const char *suffix_delimiter = "_";

static int
boot_slot_index[AB_SUPPORTED_SLOTS]; /* store index for boot parition */

static void reboot_device(int reason)
{
    printf("reboot reason:%d\n", reason);
}

static size_t memscpy(void *dest, size_t dst_size, const void *src,
                      size_t src_size)
{
    size_t copy_size = MIN(dst_size, src_size);
    memcpy(dest, src, copy_size);
    return copy_size;
}

static void strrev(unsigned char *str)
{
    int i;
    int j;
    unsigned char a;
    unsigned len = strlen((const char *)str);

    for (i = 0, j = len - 1; i < j; i++, j--) {
        a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

int itoa(int num, unsigned char *str, int len, int base)
{
    int sum = num;
    int i = 0;
    int digit;

    if (len == 0)
        return -1;

    do {
        digit = sum % base;

        if (digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;

        sum /= base;
    }
    while (sum && (i < (len - 1)));

    if (i == (len - 1) && sum)
        return -1;

    str[i] = '\0';
    strrev(str);
    return 0;
}

/* local functions. */
static void attributes_update(partition_device_t *part_dev);
static void mark_all_partitions_active(partition_device_t *part_dev,
                                       unsigned slot);
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot);

static void
swap_guid(partition_device_t *part_dev, int new_slot);
/*
    Function: To read slot attribute of
        of the partition_entry
*/
inline bool slot_is_active(struct partition_entry *partition_entries,
                           unsigned index)
{
    return !!(partition_entries[index].attribute_flag & PART_ATT_ACTIVE_VAL);
}

inline bool slot_is_sucessful(struct partition_entry *partition_entries,
                              unsigned index)
{
    return !!(partition_entries[index].attribute_flag &
              PART_ATT_SUCCESSFUL_VAL);
}

inline unsigned slot_retry_count(struct partition_entry *partition_entries,
                                 unsigned index)
{
    return ((partition_entries[index].attribute_flag
             & PART_ATT_MAX_RETRY_COUNT_VAL) >> PART_ATT_MAX_RETRY_CNT_BIT);
}

inline unsigned slot_priority(struct partition_entry *partition_entries,
                              unsigned index)
{
    return ((partition_entries[index].attribute_flag
             & PART_ATT_PRIORITY_VAL) >> PART_ATT_PRIORITY_BIT);
}

inline bool slot_is_bootable(struct partition_entry *partition_entries,
                             unsigned index)
{
    return !((partition_entries[index].attribute_flag &
              PART_ATT_UNBOOTABLE_VAL) >> PART_ATT_UNBOOTABLE_BIT);
}

inline uint8_t get_slot_attribute(struct partition_entry *partition_entries,
                                  unsigned index)
{
    return ((partition_entries[index].attribute_flag >> PART_ATT_PRIORITY_BIT) &
            0xFF);
}

int get_inverse_slot(partition_device_t *part_dev, int slot)
{
    if (part_dev->multislot_support)
        return (slot == SLOT_A) ? SLOT_B : SLOT_A;

    return INVALID;
}

static void
decrease_slot_retrycnt(partition_device_t *part_dev, unsigned slot)
{
    int i, j;
    char *pname = NULL;
    char *suffix_str = NULL;
    uint64_t boot_retry_count;
    struct partition_entry *partition_entries;
    int partition_count = 0;

    if (NULL == part_dev) {
        dprintf(CRITICAL,  "Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        dprintf(CRITICAL, "mark_slot_attr:slot =%d unknown\n", slot);
        return;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        dprintf(CRITICAL, "Transversing partition %s\n", pname);
#endif

        /* 1. Find partition, if it is A/B enabled. */
        for ( j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                boot_retry_count = slot_retry_count(partition_entries, i);

                if ((boot_retry_count > 0) && (boot_retry_count < MAX_RETRY_COUNT)) {
                    partition_entries[i].attribute_flag &= ~PART_ATT_MAX_RETRY_COUNT_VAL;
                    partition_entries[i].attribute_flag |= ((boot_retry_count - 1) <<
                                                            PART_ATT_MAX_RETRY_CNT_BIT);
                }
            }
        }
    }

    part_dev->attributes_updated = true;
}

void
ptdev_deactivate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        dprintf(CRITICAL, "ERROR: slot= %d  wrong slot number\n", slot);
        return;
    }

    count = part_dev->count;
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* 1. Find partition, if it is A/B enabled. */
        for ( int j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* Set Unbootable bit */
                SET_BIT(partition_entries[i].attribute_flag,
                        PART_ATT_UNBOOTABLE_BIT);

                /* Clear Sucess bit and Active bits */
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Clear Max retry count and priority value */
                partition_entries[i].attribute_flag &= (~PART_ATT_PRIORITY_VAL &
                                                        ~PART_ATT_MAX_RETRY_COUNT_VAL);
            }
        }
    }

    return;
}

/*
Useage : Active a new slot. use after fisrt start up or after OTA update
*/
void
ptdev_activate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        dprintf(CRITICAL, "ERROR: slot= %d  wrong slot number\n", slot);
        return;
    }

    count = part_dev->count;
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* 1. Find partition, if it is A/B enabled. */
        for ( int j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* CLR Unbootable bit and Sucess bit*/
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_UNBOOTABLE_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);

                /* Set Active bits */
                SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Set Max retry count and priority value */
                partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                        PART_ATT_MAX_RETRY_COUNT_VAL);
            }
        }
        else {
            CLR_BIT(partition_entries[i].attribute_flag,
                    PART_ATT_UNBOOTABLE_BIT);
            CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
            SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
            partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                    PART_ATT_MAX_RETRY_COUNT_VAL);
        }
    }

    return;
}

/*
    Function scan boot partition to find SLOT_A/SLOT_B suffix.
    If found than make multislot_boot flag true and
    scans another partition.
*/
bool ptdev_scan_for_multislot(partition_device_t *part_dev)
{
    int j, len, count;
    char *tmp1, *tmp2;
    int partition_count;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return 0;
    }

    partition_count = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);

    /* Intialize all slot specific variables */
    part_dev->multislot_support = false;
    part_dev->active_slot = INVALID;
    part_dev->attributes_updated = false;

    if (partition_count > NUM_PARTITIONS) {
        dprintf(CRITICAL, "ERROR: partition_count more than supported.\n");
        return part_dev->multislot_support;
    }

    int scan_nr = partition_count > MAX_NR_SCAN_FOR_SLOT ? MAX_NR_SCAN_FOR_SLOT
                  : partition_count;

    for (int m = 0; m < scan_nr; m++) {
        tmp1 = (char *)partition_entries[m].name;
        len = strlen(tmp1);

        for (int x = m + 1; x < scan_nr; x++) {
            if (!strncmp((const char *)tmp1, (char *)partition_entries[x].name,
                         len - 2)) {
                tmp1 = tmp1 + len - 2;
                tmp2 = (char *)(partition_entries[x].name + len - 2);
                count = 0;

                for (j = 0; j < AB_SUPPORTED_SLOTS; j++) {
                    if (!strcmp(tmp1, suffix_slot[j]) || !strcmp(tmp2, suffix_slot[j]))
                        count++;
                }

                /* Break out of loop if all slot index are found*/
                if (count == AB_SUPPORTED_SLOTS) {
                    part_dev->multislot_support = true;
                    boot_slot_index[0] = m;
                    boot_slot_index[1] = x;
                    break;
                }
            }
        }

        if (part_dev->multislot_support)
            break;
    }

    return part_dev->multislot_support;
}

/*
    Function: To reset partition attributes
    This function reset partition_priority, retry_count
    and clear successful and bootable bits.
*/
void ptdev_reset_attributes(partition_device_t *part_dev, unsigned index)
{
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    partition_entries[index].attribute_flag |= (PART_ATT_PRIORITY_VAL |
            PART_ATT_MAX_RETRY_COUNT_VAL);

    partition_entries[index].attribute_flag &= ((~PART_ATT_SUCCESSFUL_VAL) &
            (~PART_ATT_UNBOOTABLE_VAL));

    if (!part_dev->attributes_updated)
        part_dev->attributes_updated = true;

    /* Make attributes persistant */
    ptdev_mark_active_slot(part_dev, part_dev->active_slot);
}

/*
    Function: Switch active partitions.
*/
void ptdev_switch_slots(partition_device_t *part_dev, int old_slot,
                        int new_slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    if ( (old_slot != SLOT_A) && (old_slot != SLOT_B)) {
        dprintf(CRITICAL, "ERROR: old_slot= %d  wrong slot number\n", old_slot);
        return;
    }

    if ( (new_slot != SLOT_A) && (new_slot != SLOT_B)) {
        dprintf(CRITICAL, "ERROR: new_slot= %d  wrong slot number\n", new_slot);
        return;
    }

    if (new_slot == old_slot) {
        return;
    }

    count = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* 1. Find partition, if it is A/B enabled. */
        for ( int j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            /*old active slot */
            if (!strcmp(suffix_str, SUFFIX_SLOT(old_slot))) {
                /* Set Unbootable bit */
                SET_BIT(partition_entries[i].attribute_flag,
                        PART_ATT_UNBOOTABLE_BIT);
                /* Clear Sucess bit and Active bits */
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Clear Max retry count and priority value */
                partition_entries[i].attribute_flag &= (~PART_ATT_PRIORITY_VAL &
                                                        ~PART_ATT_MAX_RETRY_COUNT_VAL);
            }
            /*new active slot */
            else if (!strcmp(suffix_str, SUFFIX_SLOT(new_slot))) {
                /* CLR Unbootable bit*/
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_UNBOOTABLE_BIT);
                //CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);

                /* Set Active bits */
                SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Set Max retry count and priority value */
                partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                        PART_ATT_MAX_RETRY_COUNT_VAL);
            }
        }
        else {
            CLR_BIT(partition_entries[i].attribute_flag,
                    PART_ATT_UNBOOTABLE_BIT);
            //CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
            SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
            partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                    PART_ATT_MAX_RETRY_COUNT_VAL);
        }
    }

    swap_guid(part_dev, new_slot);
    part_dev->active_slot = new_slot;
    part_dev->attributes_updated = true;
    return;
}
/*
    This function returns the most priority and active slot,
    also you need to update the global state seperately.

*/
int ptdev_find_active_slot(partition_device_t *part_dev)
{
    unsigned current_priority;
    int i, count = 0;
    bool current_active_bit;
    unsigned boot_priority;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        goto out;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
#ifdef AB_DEBUG
    dprintf(INFO, "ptdev_find_active_slot() called\n");
#endif

    /* Return current active slot if already found */
    if (part_dev->active_slot != INVALID)
        goto out;

    for (boot_priority = (MAX_PRIORITY - 1);
            boot_priority >= 0; boot_priority--) {
        /* Search valid boot slot with highest priority */
        for (i = 0; i < AB_SUPPORTED_SLOTS; i++) {
            current_priority = slot_priority(partition_entries, boot_slot_index[i]);
            current_active_bit = slot_is_active(partition_entries, boot_slot_index[i]);

            /* Count number of slots with all attributes as zero */
            if ( !current_priority &&
                    !current_active_bit) {
                count ++;
                continue;
            }

#ifdef AB_DEBUG
            dprintf(INFO, "Slot:Priority:Active:Bootable %s:%d:%d:%d \n",
                    partition_entries[boot_slot_index[i]].name,
                    current_priority,
                    current_active_bit);
#endif

            if (boot_priority == current_priority) {
                if (current_active_bit) {
#ifdef AB_DEBUG
                    dprintf(INFO, "Slot (%s) is Valid High Priority Slot\n", SUFFIX_SLOT(i));
#endif
                    part_dev->active_slot = i;
                    goto out;
                }
            }
        }

        /* All slots are zeroed, this is first bootup */
        /* Marking and trying SLOT 0 as default */
        if (count == AB_SUPPORTED_SLOTS) {
            /* Update the priority of the boot slot */
            ptdev_activate_slot(part_dev, SLOT_A);

            part_dev->active_slot = SLOT_A;

            /* This is required to mark all bits as active,
            for fresh boot post fresh flash */
            part_dev->attributes_updated = true;
            goto out;
        }
    }

out:
    return part_dev->active_slot;
}

void ptdev_roll_back_check(partition_device_t *part_dev)
{
    int active_slot = INVALID;
    int other_slot;
    uint64_t boot_retry_count = 0;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev!\n");
        goto out;
    }

    active_slot = ptdev_find_active_slot(part_dev);
    other_slot = get_inverse_slot(part_dev, active_slot);

    if ((active_slot != SLOT_A) && (active_slot != SLOT_B)) {
        dprintf(CRITICAL, "an INVALID active_slot slot!\n");
        goto check_update;
    }

    if ((other_slot != SLOT_A) && (other_slot != SLOT_B)) {
        dprintf(CRITICAL, "an INVALID inactive_slot slot!\n");
        goto check_update;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    /*Do not need fallback, if a active slot's partition flag is set to sucessful and bootable*/
    if ( slot_is_bootable(partition_entries, boot_slot_index[active_slot]) &&
            slot_is_sucessful(partition_entries, boot_slot_index[active_slot])) {
        dprintf(ALWAYS, "Rollback skip! Active slot%s is Successful and Bootable\n",
                SUFFIX_SLOT(active_slot));
        goto check_update;
    }

    /*Do not need fallback, if a inactive slot's partition flag is not set to sucessful and bootable */
    if ( (!slot_is_bootable(partition_entries, boot_slot_index[other_slot])) ||
            (!slot_is_sucessful(partition_entries, boot_slot_index[other_slot]))) {
        dprintf(ALWAYS,
                "Rollback skip! Inactive slot%s is not Successful or not Bootable\n",
                SUFFIX_SLOT(other_slot));
        goto check_update;
    }

    boot_retry_count = slot_retry_count(partition_entries,
                                        boot_slot_index[active_slot]);

    if ((boot_retry_count != 0) && (boot_retry_count < MAX_RETRY_COUNT)) {
        /* Decrement retry count, boot from active slot */
        dprintf(ALWAYS,
                "Rollback decrement Retrycount from %llu to %llu, boot from slot%s\n",
                boot_retry_count, (boot_retry_count - 1), SUFFIX_SLOT(active_slot));
        decrease_slot_retrycnt(part_dev, active_slot);
    }
    else {
        dprintf(ALWAYS, "Rollback from slot%s to slot%s!\n", SUFFIX_SLOT(active_slot),
                SUFFIX_SLOT(other_slot));
        ptdev_switch_slots(part_dev, active_slot, other_slot);

    }

check_update:

    if (part_dev->attributes_updated) {
        attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

out:
    return;
}

/*Priority :
    Active + Bootable + Successful
    Active + Bootable ---------------->check Retry_count>0
    Bootable + Successful
    Bootable
*/

int ptdev_find_boot_slot(partition_device_t *part_dev)
{
    int boot_slot = INVALID;

    if (part_dev == NULL) {
        dprintf(CRITICAL, "Invalide partition dev!\n");
        return INVALID;
    }

    boot_slot = ptdev_find_active_slot(part_dev);

    if (boot_slot == INVALID) {
        dprintf(CRITICAL, "can't find a bootable slot!\n");
        return INVALID;
    }

    return boot_slot;
}

static
void guid_update(struct partition_entry *partition_entries,
                 unsigned old_index,
                 unsigned new_index)
{
    unsigned char tmp_guid[PARTITION_TYPE_GUID_SIZE];

#ifdef AB_DEBUG
    dprintf(INFO, "Swapping GUID (%s) --> (%s) \n",
            partition_entries[old_index].name,
            partition_entries[new_index].name);
#endif
    memcpy(tmp_guid, partition_entries[old_index].type_guid,
           PARTITION_TYPE_GUID_SIZE);
    memcpy(partition_entries[old_index].type_guid,
           partition_entries[new_index].type_guid,
           PARTITION_TYPE_GUID_SIZE);
    memcpy(partition_entries[new_index].type_guid, tmp_guid,
           PARTITION_TYPE_GUID_SIZE);
    return;
}

/*
    Function to swap guids of slots
*/
static void
swap_guid(partition_device_t *part_dev, int new_slot)
{
    unsigned i, j, tmp_strlen;
    unsigned partition_cnt;
    struct partition_entry *partition_entries;
    const char *ptr_pname, *ptr_suffix;
    int old_slot;

    if (NULL == part_dev) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    if ((new_slot != SLOT_A) && (new_slot != SLOT_B)) {
        dprintf(CRITICAL, "ERROR: slot= %d  wrong slot number\n", new_slot);
        return;
    }

    if ((part_dev->active_slot != SLOT_B) && (part_dev->active_slot != SLOT_A)) {
        dprintf(CRITICAL, "first set Active, do not need swap guid\n");
        return;
    }

    partition_cnt = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);
    old_slot = part_dev->active_slot;

    if ( old_slot == new_slot)
        return;

    for (i = 0; i < partition_cnt; i++) {
        ptr_pname = (const char *)partition_entries[i].name;

        /* Search for suffix in partition name */
        if ((ptr_suffix = strstr(ptr_pname, SUFFIX_SLOT(new_slot)))) {
            for (j = i + 1; j < partition_cnt; j++) {
                tmp_strlen = strlen(ptr_pname) - strlen(SUFFIX_SLOT(new_slot));

                if (!strncmp((const char *)partition_entries[j].name, ptr_pname,
                             tmp_strlen) &&
                        strstr((const char *)partition_entries[j].name, SUFFIX_SLOT(old_slot)))
                    guid_update(partition_entries, j, i);
            }
        }
        else if ((ptr_suffix = strstr(ptr_pname, SUFFIX_SLOT(old_slot)))) {
            for (j = i + 1; j < partition_cnt; j++) {
                tmp_strlen = strlen(ptr_pname) - strlen(SUFFIX_SLOT(old_slot));

                if (!strncmp((const char *)partition_entries[j].name, ptr_pname,
                             tmp_strlen) &&
                        strstr((const char *)partition_entries[j].name, SUFFIX_SLOT(new_slot)))
                    guid_update(partition_entries, i, j);
            }
        }
    }
}

/*
Function: To set active bit of all partitions of actve slot.
    also, unset active bits of all other slot
*/
void
mark_all_partitions_active(partition_device_t *part_dev, unsigned slot)
{
    int i, j;
    char *pname = NULL;
    char *suffix_str = NULL;
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        dprintf(INFO, "Transversing partition %s\n", pname);
#endif

        /* 1. Find partition, if it is A/B enabled. */
        for ( j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot)))
                /* 2a. Mark matching partition as active. */
                partition_entries[i].attribute_flag |= PART_ATT_ACTIVE_VAL;
            else
                /* 2b. Unset active bit for all other partitions. */
                partition_entries[i].attribute_flag &= ~PART_ATT_ACTIVE_VAL;
        }
        else {
            /* 3. If it isn't A/B enabled, Mark partition as active as well */
            partition_entries[i].attribute_flag |= PART_ATT_ACTIVE_VAL;
        }
    }

    part_dev->attributes_updated = true;
}

/*
    Function: Mark the slot to be active and also conditionally
    update the slot parameters if there is a change.
*/
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot)
{
    if (!part_dev) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        goto out;
    }

    if (part_dev->active_slot == slot)
        goto out;

    if (slot != INVALID) {
        dprintf(INFO, "Marking (%s) as active\n", SUFFIX_SLOT(slot));

        /* 1. Swap GUID's to new slot */
        swap_guid(part_dev, slot);

        /* 2. Set Active bit for all partitions of active slot */
        mark_all_partitions_active(part_dev, slot);
    }

    part_dev->active_slot = slot;
out:

    if (part_dev->attributes_updated) {
        attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

    return;
}

/* Function to find if multislot is supported */
bool ptdev_multislot_is_supported(partition_device_t *part_dev)
{
    if (!part_dev) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return false;
    }

    return part_dev->multislot_support;
}

/*
    Function to populate partition meta used
    for fastboot get var info publication.

    Input partition_entries, partition_count and
    buffer to fill information.

*/
int ptdev_fill_partition_meta(partition_device_t *part_dev,
                              char has_slot_pname[][MAX_GET_VAR_NAME_SIZE],
                              char has_slot_reply[][MAX_RSP_SIZE],
                              int array_size)
{
    int i, j, tmp;
    int count = 0;
    char *pname = NULL;
    int pname_size;
    struct partition_entry *partition_entries;
    int partition_count;
    char *suffix_str;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return 0;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        pname_size = strlen(pname);
        suffix_str = NULL;
#ifdef AB_DEBUG
        dprintf(INFO, "Transversing partition %s\n", pname);
#endif

        /* 1. Find partition, if it is A/B enabled. */
        for ( j = 0; j < AB_SUPPORTED_SLOTS; j++) {
            suffix_str = strstr(pname, SUFFIX_SLOT(j));

            if (suffix_str)
                break;
        }

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(SLOT_A))) {
                /* 2. put the partition name in array */
                tmp = pname_size - strlen(suffix_str);
                strlcpy(has_slot_pname[count], pname, tmp + 1);
                strlcpy(has_slot_reply[count], " Yes", MAX_RSP_SIZE);
                count++;
            }
        }
        else {
            strlcpy(has_slot_pname[count], pname, MAX_GET_VAR_NAME_SIZE);
            strlcpy(has_slot_reply[count], " No", MAX_RSP_SIZE);
            count++;
        }

        /* Avoid over population of array provided */
        if (count >= array_size) {
            dprintf(CRITICAL, "ERROR: Not able to parse all partitions\n");
            return count;
        }
    }

#ifdef AB_DEBUG

    for (i = 0; i < count; i++)
        dprintf(INFO, "has-slot:%s:%s\n", has_slot_pname[i], has_slot_reply[i]);

#endif
    return count;
}

/*
    Function to populate the slot meta used
    for fastboot get var info publication.
*/
void ptdev_fill_slot_meta(partition_device_t *part_dev,
                          struct ab_slot_info *slot_info)
{
    int i, current_slot_index;
    struct partition_entry *ptn_entries;
    char buff[3];

    if ((!part_dev) || (!part_dev->partition_entries)) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    if (NULL == slot_info) {
        dprintf(CRITICAL, "slot_info is NULL\n");
        return;
    }

    ptn_entries = ptdev_get_partition_entries(part_dev);

    /* Update slot info */
    for (i = 0; i < AB_SUPPORTED_SLOTS; i++) {
        current_slot_index = boot_slot_index[i];
        strlcpy(slot_info[i].slot_is_unbootable_rsp,
                slot_is_bootable(ptn_entries, current_slot_index) ? "No" : "Yes",
                MAX_RSP_SIZE);
        strlcpy(slot_info[i].slot_is_active_rsp,
                slot_is_active(ptn_entries, current_slot_index) ? "Yes" : "No",
                MAX_RSP_SIZE);
        strlcpy(slot_info[i].slot_is_succesful_rsp,
                slot_is_sucessful(ptn_entries, current_slot_index) ? "Yes" : "No",
                MAX_RSP_SIZE);
        itoa(slot_retry_count(ptn_entries, current_slot_index),
             (unsigned char *)buff, 2, 10);
        strlcpy(slot_info[i].slot_retry_count_rsp, buff, MAX_RSP_SIZE);
    }
}

/*
    Function to read and update the attributes of
    GPT
*/
static int
update_gpt(partition_device_t *part_dev, uint64_t gpt_start_addr,
           uint64_t gpt_hdr_offset,
           uint64_t gpt_entries_offset)
{
    uint8_t *buffer = NULL;
    uint8_t *gpt_entries_ptr, *gpt_hdr_ptr, *tmp = NULL;
    struct partition_entry *partition_entries;
    uint32_t partition_count;
    unsigned max_partition_count = 0;
    unsigned partition_entry_size = 0;
    storage_device_t *storage;
    uint32_t block_size;
    uint32_t crc_val = 0;
    int ret = 0;
    uint64_t offset;
    uint64_t max_gpt_size_bytes;

    if (NULL == part_dev) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        goto out;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);
    storage = part_dev->storage;
    block_size = storage->get_block_size(storage);
    offset = part_dev->gpt_offset;
    max_gpt_size_bytes =
        (PARTITION_ENTRY_SIZE * NUM_PARTITIONS + GPT_HEADER_BLOCKS * block_size);

    /* Get Current LUN for UFS target */

    buffer = memalign(block_size, ROUNDUP(max_gpt_size_bytes, block_size));

    if (gpt_hdr_offset) {
        /* Primary GPT shall with offset, no necessery for Secondary GPT */
        offset = 0;
    }

    if (!buffer) {
        dprintf(CRITICAL, "update_gpt: Failed at memory allocation\n");
        goto out;
    }

    ret = storage->read(storage, gpt_start_addr + offset,
                        (uint8_t *)buffer,
                        max_gpt_size_bytes);

    if (ret) {
        dprintf(CRITICAL, "Failed to read GPT\n");
        goto out;
    }

    /* 0. Intialise ptrs for header and entries */
    gpt_entries_ptr = buffer + gpt_entries_offset * block_size;
    gpt_hdr_ptr = buffer + gpt_hdr_offset * block_size;

    /* Update attributes_flag of partition entry */
    tmp = gpt_entries_ptr;

    for (unsigned i = 0; i < partition_count; i++) {
        /* Update the partition attributes */
        PUT_LONG_LONG(&tmp[ATTRIBUTE_FLAG_OFFSET],
                      partition_entries[i].attribute_flag);
        memscpy(tmp, PARTITION_TYPE_GUID_SIZE, partition_entries[i].type_guid,
                PARTITION_TYPE_GUID_SIZE);

        /* point to the next partition entry */
        tmp += PARTITION_ENTRY_SIZE;
    }

    /* Calculate and update CRC of partition entries array */
    max_partition_count =
        GET_LWORD_FROM_BYTE(&gpt_hdr_ptr[PARTITION_COUNT_OFFSET]);
    partition_entry_size =
        GET_LWORD_FROM_BYTE(&gpt_hdr_ptr[PENTRY_SIZE_OFFSET]);

    /* Check for partition entry size */
    if (partition_entry_size != PARTITION_ENTRY_SIZE) {
        dprintf(CRITICAL, "Invalid parition entry size\n");
        goto out;
    }

    /* Check for maximum partition size */
    if ((max_partition_count) > (MIN_PARTITION_ARRAY_SIZE /
                                 (partition_entry_size))) {
        dprintf(CRITICAL, "Invalid maximum partition count\n");
        goto out;
    }

    crc_val  = crc32(0U, gpt_entries_ptr, ((max_partition_count) *
                                           (partition_entry_size)));
    PUT_LONG(&gpt_hdr_ptr[PARTITION_CRC_OFFSET], crc_val);

    /* Write CRC to 0 before we calculate the crc of the GPT header */
    crc_val = 0;
    PUT_LONG(&gpt_hdr_ptr[HEADER_CRC_OFFSET], crc_val);
    crc_val  = crc32(0U, gpt_hdr_ptr, GPT_HEADER_SIZE);
    PUT_LONG(&gpt_hdr_ptr[HEADER_CRC_OFFSET], crc_val);

    /* write to mmc */
    ret = storage->write(storage, gpt_start_addr + offset,
                         (uint8_t *)buffer, max_gpt_size_bytes);

    if (ret) {
        dprintf(CRITICAL, "Failed to write gpt\n");
        goto out;
    }

out:

    if (buffer)
        free(buffer);

    return ret;
}

/**
    Function to update the backup and primary gpt
    partition.
**/
static void attributes_update(partition_device_t *part_dev)
{
    uint64_t offset;
    uint64_t gpt_entries_offset, gpt_hdr_offset;
    uint64_t gpt_start_addr;
    int ret;
    storage_device_t *storage;
    uint32_t block_size;
    unsigned max_entries_size_bytes;
    unsigned max_entries_blocks;
    unsigned max_gpt_blocks;

    if (NULL == part_dev) {
        dprintf(CRITICAL, "Invalide partition dev\n");
        return;
    }

    storage = part_dev->storage;
    block_size = storage->get_block_size(storage);
    max_entries_size_bytes = PARTITION_ENTRY_SIZE * NUM_PARTITIONS;
    max_entries_blocks = max_entries_size_bytes / block_size;
    max_gpt_blocks = GPT_HEADER_BLOCKS + max_entries_blocks;

    /* Update Primary GPT */
    offset = 0x01;  /*  offset is 0x1 for primary GPT */
    gpt_start_addr = offset * block_size;
    /* Take gpt_start_addr as start and calculate offset from that in block sz*/
    gpt_hdr_offset = 0; /* For primary partition offset is zero */
    gpt_entries_offset = GPT_HEADER_BLOCKS;

    ret = update_gpt(part_dev, gpt_start_addr, gpt_hdr_offset,
                     gpt_entries_offset);

    if (ret) {
        dprintf(CRITICAL, "Failed to update Primary GPT\n");
        return;
    }

    /* Update Secondary GPT */
    offset = ((storage->get_capacity(storage) / block_size) - max_gpt_blocks);
    gpt_start_addr = offset * block_size;
    gpt_hdr_offset = max_entries_blocks;
    gpt_entries_offset = 0; /* For secondary GPT entries offset is zero */

    ret = update_gpt(part_dev, gpt_start_addr, gpt_hdr_offset,
                     gpt_entries_offset);

    if (ret) {
        dprintf(CRITICAL, "Failed to update Secondary GPT\n");
        return;
    }

    return;
}
