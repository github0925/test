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
#include <types_def.h>
#include <debug.h>
#include <ab_partition_parser.h>
#include <partition_parser.h>
#include <arch.h>
#if defined(MMC_SDHCI_SUPPORT) || defined(UFS_SUPPORT)
#include <ufs.h>
#endif
//#include <debug.h>
#include "cksum.h"
#include "partition_mem.h"

//#define AB_DEBUG

/* Slot suffix */
const char *suffix_slot[] = {"_a",
                             "_b"
                            };
const char *suffix_delimiter = "_";

static int
boot_slot_index[AB_SUPPORTED_SLOTS]; /* store index for boot parition */

size_t strlcpy(char *dest, const char *src, size_t size)
{
    char *dst_pos = dest;

    if (!dest || !src)
        goto out;

    while(  *src != 0 && dst_pos < (dest + size - 1 ))
    {
        *dst_pos = *src;
        src++;
        dst_pos++;
    }

    *dst_pos = '\0';
out:
    return (size_t)(dst_pos - dest);
}

char * strstr(const char* src, const char* str)
{
    uint32_t src_len;
    uint32_t str_len;
    uint32_t i = 0;
#if 0
    if ((src == NULL) || (str == NULL))
        return NULL;
#endif
    src_len = strlen(src);
    str_len = strlen(str);

    if (str_len > src_len)
        return NULL;

    for ( i = 0; i <= src_len - str_len; i++)
    {
        if (!memcmp(src + i, str, str_len))
            return (char *)((addr_t)src + i);
    }
    return NULL;
}

static void reboot_device(int reason)
{
    printf("reboot reason:%d\n", reason);
}

#if 0
static size_t memscpy(void *dest, size_t dst_size, const void *src,
                      size_t src_size)
{
    size_t copy_size = MIN(dst_size, src_size);
    memcpy(dest, src, copy_size);
    return copy_size;
}
#endif

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
static void mark_all_partitions_active(partition_device_t *part_dev,
                                       unsigned slot);
#if 0
static void attributes_update(partition_device_t *part_dev);
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot);
#endif
/*
    Function: To read slot attribute of
        of the partition_entry
*/
static bool slot_is_active(struct partition_entry *partition_entries,
                           unsigned index)
{
    return !!(partition_entries[index].attribute_flag & PART_ATT_ACTIVE_VAL);
}

static bool slot_is_sucessful(struct partition_entry *partition_entries,
                              unsigned index)
{
    return !!(partition_entries[index].attribute_flag &
              PART_ATT_SUCCESSFUL_VAL);
}

static unsigned slot_retry_count(struct partition_entry *partition_entries,
                                 unsigned index)
{
    return ((partition_entries[index].attribute_flag
             & PART_ATT_MAX_RETRY_COUNT_VAL) >> PART_ATT_MAX_RETRY_CNT_BIT);
}

static unsigned slot_priority(struct partition_entry *partition_entries,
                              unsigned index)
{
    return ((partition_entries[index].attribute_flag
             & PART_ATT_PRIORITY_VAL) >> PART_ATT_PRIORITY_BIT);
}

static bool slot_is_bootable(struct partition_entry *partition_entries,
                             unsigned index)
{
    return !(partition_entries[index].attribute_flag &
             PART_ATT_UNBOOTABLE_VAL);
}

void
ptdev_deactivate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int slt_index = boot_slot_index[slot];

    /* Set Unbootable bit */
    SET_BIT(partition_entries[slt_index].attribute_flag,
            PART_ATT_UNBOOTABLE_BIT);

    /* Clear Sucess bit and Active bits */
    CLR_BIT(partition_entries[slt_index].attribute_flag, PART_ATT_SUCCESS_BIT);
    CLR_BIT(partition_entries[slt_index].attribute_flag, PART_ATT_ACTIVE_BIT);

    /* Clear Max retry count and priority value */
    partition_entries[slt_index].attribute_flag &= (~PART_ATT_PRIORITY_VAL &
            ~PART_ATT_MAX_RETRY_COUNT_VAL);

    return;
}

void
ptdev_activate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int slt_index = boot_slot_index[slot];

    /* CLR Unbootable bit and Sucess bit*/
    CLR_BIT(partition_entries[slt_index].attribute_flag,
            PART_ATT_UNBOOTABLE_BIT);
    CLR_BIT(partition_entries[slt_index].attribute_flag, PART_ATT_SUCCESS_BIT);

    /* Set Active bits */
    SET_BIT(partition_entries[slt_index].attribute_flag, PART_ATT_ACTIVE_BIT);

    /* Set Max retry count and priority value */
    partition_entries[slt_index].attribute_flag |= (PART_ATT_PRIORITY_VAL |
            PART_ATT_MAX_RETRY_COUNT_VAL);

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
    int partition_count = ptdev_get_partition_count(part_dev);
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);

    /* Intialize all slot specific variables */
    part_dev->multislot_support = false;
    part_dev->active_slot = INVALID;
    part_dev->attributes_updated = false;

    if (partition_count > NUM_PARTITIONS) {
        DBG( "ERROR: partition_count more than supported.\n");
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

#if 0
/*
    Function: To reset partition attributes
    This function reset partition_priority, retry_count
    and clear successful and bootable bits.
*/
void ptdev_reset_attributes(partition_device_t *part_dev, unsigned index)
{
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);

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
#ifdef AB_DEBUG
    DBG( "Switching slots %s to %s\n",
            SUFFIX_SLOT(old_slot), SUFFIX_SLOT(new_slot));
#endif
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int old_slot_index = boot_slot_index[old_slot];
    int new_slot_index = boot_slot_index[new_slot];

    /* Mark current slot inactive, keeping all other attributes intact */
    partition_entries[old_slot_index].attribute_flag &= ~PART_ATT_ACTIVE_VAL;

    /* Mark new slot active */
    partition_entries[new_slot_index].attribute_flag |=
        ((PART_ATT_PRIORITY_VAL |
          PART_ATT_ACTIVE_VAL |
          PART_ATT_MAX_RETRY_COUNT_VAL));
    partition_entries[new_slot_index].attribute_flag &=
        (~PART_ATT_SUCCESSFUL_VAL
         & ~PART_ATT_UNBOOTABLE_VAL);

    if (!part_dev->attributes_updated)
        part_dev->attributes_updated = true;

    /* Update active slot and gpt table */
    ptdev_mark_active_slot(part_dev, new_slot);
    return;
}
#endif
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
    struct partition_entry *partition_entries = ptdev_get_partition_entries(
                part_dev);

#ifdef AB_DEBUG
    DBG( "ptdev_find_active_slot() called\n");
#endif

    /* Return current active slot if already found */
    if (part_dev->active_slot != INVALID)
        goto out;

    for (boot_priority = MAX_PRIORITY;
            boot_priority > 0; boot_priority--) {
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
            DBG( "Slot:Priority:Active:Bootable %s:%d:%d:%d \n",
                    partition_entries[boot_slot_index[i]].name,
                    current_priority,
                    current_active_bit);
#endif

            if (boot_priority == current_priority) {
                if (current_active_bit) {
#ifdef AB_DEBUG
                    DBG( "Slot (%s) is Valid High Priority Slot\n", SUFFIX_SLOT(i));
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
            mark_all_partitions_active(part_dev, SLOT_A);
            goto out;
        }
    }

out:
    return part_dev->active_slot;
}

static int
next_active_bootable_slot(struct partition_entry *ptn_entry)
{
    int i, slt_index;

    for (i = 0; i < AB_SUPPORTED_SLOTS; i++) {
        slt_index = boot_slot_index[i];

        if (slot_is_bootable(ptn_entry, slt_index))
            return i;
    }

    DBG( "ERROR: Unable to find any bootable slot");
    return INVALID;
}

int ptdev_find_boot_slot(partition_device_t *part_dev)
{
    int boot_slot, next_slot;
    int slt_index;
    uint64_t boot_retry_count;
    struct partition_entry *partition_entries = ptdev_get_partition_entries(
                part_dev);

    boot_retry_count = 0;
    boot_slot = ptdev_find_active_slot(part_dev);

    if (boot_slot == INVALID)
        goto out;

    slt_index = boot_slot_index[boot_slot];

    /*  Boot if partition flag is set to sucessful */
    if (partition_entries[slt_index].attribute_flag & PART_ATT_SUCCESSFUL_VAL)
        goto out;

    boot_retry_count = slot_retry_count(partition_entries, slt_index);

#ifdef AB_DEBUG
    DBG( "Boot Partition:RetryCount %s:%lld\n",
            partition_entries[slt_index].name,
            boot_retry_count);
#endif

    if (!boot_retry_count) {
        /* Mark slot invalid and unbootable */
        ptdev_deactivate_slot(part_dev, boot_slot);

        next_slot = next_active_bootable_slot(partition_entries);

        if (next_slot != INVALID) {
            ptdev_switch_slots(part_dev, boot_slot, next_slot);
            reboot_device(0);
        }
        else {
            boot_slot = INVALID;
        }
    }
    else {
        /* Do normal boot */
        /* Decrement retry count */
        partition_entries[slt_index].attribute_flag =
            (partition_entries[slt_index].attribute_flag
             & ~PART_ATT_MAX_RETRY_COUNT_VAL)
            | ((boot_retry_count - 1) << PART_ATT_MAX_RETRY_CNT_BIT);

        if (!part_dev->attributes_updated)
            part_dev->attributes_updated = true;

        goto out;
    }

out:
#ifdef AB_DEBUG
    DBG( "Booting SLOT %d\n", boot_slot);
#endif
    return boot_slot;
}

#if 0
static
void guid_update(struct partition_entry *partition_entries,
                 unsigned old_index,
                 unsigned new_index)
{
    unsigned char tmp_guid[PARTITION_TYPE_GUID_SIZE];

#ifdef AB_DEBUG
    DBG( "Swapping GUID (%s) --> (%s) \n",
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
    unsigned partition_cnt = ptdev_get_partition_count(part_dev);
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    const char *ptr_pname, *ptr_suffix;

    int old_slot = part_dev->active_slot;

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
#endif
/*
Function: To set active bit of all partitions of actve slot.
    also, unset active bits of all other slot
*/
static void
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
        DBG( "Transversing partition %s\n", pname);
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
    }

    part_dev->attributes_updated = true;
}

#if 0
/*
    Function: Mark the slot to be active and also conditionally
    update the slot parameters if there is a change.
*/
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot)
{
    if (part_dev->active_slot == slot)
        goto out;

    if (slot != INVALID) {
        DBG( "Marking (%s) as active\n", SUFFIX_SLOT(slot));

        /* 1. Swap GUID's to new slot */
        swap_guid(part_dev, slot);

        /* 2. Set Active bit for all partitions of active slot */
        mark_all_partitions_active(part_dev, slot);
    }

    part_dev->active_slot = slot;
out:

    if (part_dev->attributes_updated)
        attributes_update(part_dev);

    return;
}
#endif

/* Function to find if multislot is supported */
bool ptdev_multislot_is_supported(partition_device_t *part_dev)
{
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
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int partition_count = ptdev_get_partition_count(part_dev);
    char *suffix_str;

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        pname_size = strlen(pname);
        suffix_str = NULL;
#ifdef AB_DEBUG
        DBG( "Transversing partition %s\n", pname);
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
                memset(has_slot_pname[count], 0x0, MAX_GET_VAR_NAME_SIZE);
                strncpy(has_slot_pname[count], pname, tmp + 1);
                strncpy(has_slot_reply[count], " Yes", MAX_RSP_SIZE);
                count++;
            }
        }
        else {
            memset(has_slot_pname[count], 0x0, MAX_GET_VAR_NAME_SIZE);
            strncpy(has_slot_pname[count], pname, MAX_GET_VAR_NAME_SIZE);
            strncpy(has_slot_reply[count], " No", MAX_RSP_SIZE);
            count++;
        }

        /* Avoid over population of array provided */
        if (count >= array_size) {
            DBG( "ERROR: Not able to parse all partitions\n");
            return count;
        }
    }

#ifdef AB_DEBUG

    for (i = 0; i < count; i++)
        DBG( "has-slot:%s:%s\n", has_slot_pname[i], has_slot_reply[i]);

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
    struct partition_entry *ptn_entries = ptdev_get_partition_entries(
            part_dev);
    char buff[3];

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
#if 0
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
    struct partition_entry *partition_entries = ptdev_get_partition_entries(
                part_dev);
    uint32_t partition_count = ptdev_get_partition_count(part_dev);
    unsigned max_partition_count = 0;
    unsigned partition_entry_size = 0;
    storage_device_t *storage = part_dev->storage;
    uint32_t block_size = storage->get_block_size(storage);
    uint32_t crc_val = 0;
    int ret = 0;
    uint64_t max_gpt_size_bytes =
        (PARTITION_ENTRY_SIZE * NUM_PARTITIONS + GPT_HEADER_BLOCKS * block_size);

    /* Get Current LUN for UFS target */
#if 0
    buffer = PTDEV_MEMALIGN(block_size, ROUNDUP(max_gpt_size_bytes, block_size));

    if (!buffer) {
        DBG( "update_gpt: Failed at memory allocation\n");
        goto out;
    }
#endif
    ret = storage->read(storage, gpt_start_addr + part_dev->gpt_offset,
                        &buffer, max_gpt_size_bytes);

    if (ret) {
        DBG( "Failed to read GPT\n");
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
        DBG( "Invalid parition entry size\n");
        goto out;
    }

    /* Check for maximum partition size */
    if ((max_partition_count) > (MIN_PARTITION_ARRAY_SIZE /
                                 (partition_entry_size))) {
        DBG( "Invalid maximum partition count\n");
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
    ret = storage->write(storage, gpt_start_addr + part_dev->gpt_offset,
                         (uint8_t *)buffer, max_gpt_size_bytes);

    if (ret) {
        DBG( "Failed to write gpt\n");
        goto out;
    }

out:
#if 0
    if (buffer)
        PTDEV_FREE(buffer);
#endif
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
    storage_device_t *storage = part_dev->storage;
    uint32_t block_size = storage->get_block_size(storage);
    unsigned max_entries_size_bytes = PARTITION_ENTRY_SIZE * NUM_PARTITIONS;
    unsigned max_entries_blocks = max_entries_size_bytes / block_size;
    unsigned max_gpt_blocks = GPT_HEADER_BLOCKS + max_entries_blocks;

    /* Update Primary GPT */
    offset = 0x01;  /*  offset is 0x1 for primary GPT */
    gpt_start_addr = offset * block_size;
    /* Take gpt_start_addr as start and calculate offset from that in block sz*/
    gpt_hdr_offset = 0; /* For primary partition offset is zero */
    gpt_entries_offset = GPT_HEADER_BLOCKS;

    ret = update_gpt(part_dev, gpt_start_addr, gpt_hdr_offset,
                     gpt_entries_offset);

    if (ret) {
        DBG( "Failed to update Primary GPT\n");
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
        DBG( "Failed to update Secondary GPT\n");
        return;
    }

    return;
}
#endif
