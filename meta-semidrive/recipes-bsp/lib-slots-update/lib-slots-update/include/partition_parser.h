/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PARTITION_PARSER_H
#define __PARTITION_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include "storage_device.h"
#define INVALID_PTN               -1

#define PARTITION_TYPE_MBR         0
#define PARTITION_TYPE_GPT         1
#define PARTITION_TYPE_GPT_BACKUP  2

#define GPT_HEADER_SIZE 92
#define GPT_HEADER_BLOCKS 1
#define GPT_LBA 1
#define PARTITION_ENTRY_SIZE 128

/* GPT Signature should be 0x5452415020494645 */
#define GPT_SIGNATURE_1 0x54524150
#define GPT_SIGNATURE_2 0x20494645

#define MMC_MBR_SIGNATURE_BYTE_0  0x55
#define MMC_MBR_SIGNATURE_BYTE_1  0xAA

/* GPT Offsets */
#define PROTECTIVE_MBR_SIZE       BLOCK_SIZE
#define HEADER_SIZE_OFFSET        12
#define HEADER_CRC_OFFSET         16
#define PRIMARY_HEADER_OFFSET     24
#define BACKUP_HEADER_OFFSET      32
#define FIRST_USABLE_LBA_OFFSET   40
#define LAST_USABLE_LBA_OFFSET    48
#define PARTITION_ENTRIES_OFFSET  72
#define PARTITION_COUNT_OFFSET    80
#define PENTRY_SIZE_OFFSET        84
#define PARTITION_CRC_OFFSET      88

#define MIN_PARTITION_ARRAY_SIZE  0x4000

#define PARTITION_ENTRY_LAST_LBA  40

#define UNIQUE_GUID_OFFSET        16
#define FIRST_LBA_OFFSET          32
#define LAST_LBA_OFFSET           40
#define ATTRIBUTE_FLAG_OFFSET     48
#define PARTITION_NAME_OFFSET     56

#define MAX_GPT_NAME_SIZE          72
#define PARTITION_TYPE_GUID_SIZE   16
#define UNIQUE_PARTITION_GUID_SIZE 16
#define NUM_PARTITIONS             128
#define PART_ATT_READONLY_OFFSET   60

#define MAX_PRIORITY 4
#define MAX_RETRY_COUNT 4

#define PART_ATT_PRIORITY_BIT      48
#define PART_ATT_ACTIVE_BIT        50
#define PART_ATT_MAX_RETRY_CNT_BIT 51
#define PART_ATT_SUB_PARTITION_BIT 53
#define PART_ATT_SUCCESS_BIT       54
#define PART_ATT_UNBOOTABLE_BIT    55

#define PART_ATT_PRIORITY_VAL ((uint64_t)(MAX_PRIORITY-1) << PART_ATT_PRIORITY_BIT)
#define PART_ATT_ACTIVE_VAL ((uint64_t)0x1 << PART_ATT_ACTIVE_BIT)
#define PART_ATT_MAX_RETRY_COUNT_VAL ((uint64_t)(MAX_RETRY_COUNT-1) << PART_ATT_MAX_RETRY_CNT_BIT)
#define PART_ATT_SUCCESSFUL_VAL ((uint64_t)0x1 << PART_ATT_SUCCESS_BIT)
#define PART_ATT_UNBOOTABLE_VAL ((uint64_t)0x1 << PART_ATT_UNBOOTABLE_BIT)
#define PART_ATT_READONLY_VAL ((uint64_t)0x1 << PART_ATT_READONLY_OFFSET)


/* Some useful define used to access the MBR/EBR table */
#define BLOCK_SIZE                0x200
#define TABLE_ENTRY_0             0x1BE
#define TABLE_ENTRY_1             0x1CE
#define TABLE_ENTRY_2             0x1DE
#define TABLE_ENTRY_3             0x1EE
#define TABLE_SIGNATURE           0x1FE
#define TABLE_ENTRY_SIZE          0x010

#define OFFSET_STATUS             0x00
#define OFFSET_TYPE               0x04
#define OFFSET_FIRST_SEC          0x08
#define OFFSET_SIZE               0x0C
#define COPYBUFF_SIZE             (1024 * 16)
#define BINARY_IN_TABLE_SIZE      (16 * 512)
#define MAX_FILE_ENTRIES          20

#define MBR_EBR_TYPE              0x05
#define MBR_MODEM_TYPE            0x06
#define MBR_MODEM_TYPE2           0x0C
#define MBR_SBL1_TYPE             0x4D
#define MBR_SBL2_TYPE             0x51
#define MBR_SBL3_TYPE             0x45
#define MBR_RPM_TYPE              0x47
#define MBR_TZ_TYPE               0x46
#define MBR_MODEM_ST1_TYPE        0x4A
#define MBR_MODEM_ST2_TYPE        0x4B
#define MBR_EFS2_TYPE             0x4E

#define MBR_ABOOT_TYPE            0x4C
#define MBR_BOOT_TYPE             0x48
#define MBR_SYSTEM_TYPE           0x82
#define MBR_USERDATA_TYPE         0x83
#define MBR_RECOVERY_TYPE         0x60
#define MBR_MISC_TYPE             0x63
#define MBR_PROTECTED_TYPE        0xEE
#define MBR_SSD_TYPE              0x5D

#define GET_LWORD_FROM_BYTE(x)    ((unsigned)*(x) | \
        ((unsigned)*(x+1) << 8) | \
        ((unsigned)*(x+2) << 16) | \
        ((unsigned)*(x+3) << 24))

#define GET_LLWORD_FROM_BYTE(x)    ((unsigned long long)*(x) | \
        ((unsigned long long)*(x+1) << 8) | \
        ((unsigned long long)*(x+2) << 16) | \
        ((unsigned long long)*(x+3) << 24) | \
        ((unsigned long long)*(x+4) << 32) | \
        ((unsigned long long)*(x+5) << 40) | \
        ((unsigned long long)*(x+6) << 48) | \
        ((unsigned long long)*(x+7) << 56))

#define GET_LONG(x)    ((uint32_t)*(x) | \
            ((uint32_t)*(x+1) << 8) | \
            ((uint32_t)*(x+2) << 16) | \
            ((uint32_t)*(x+3) << 24))

#define PUT_LONG(x, y)   *(x) = y & 0xff;     \
    *(x+1) = (y >> 8) & 0xff;     \
    *(x+2) = (y >> 16) & 0xff;    \
    *(x+3) = (y >> 24) & 0xff;

#define PUT_LONG_LONG(x,y)    *(x) =(y) & 0xff; \
     *((x)+1) = (((y) >> 8) & 0xff);    \
     *((x)+2) = (((y) >> 16) & 0xff);   \
     *((x)+3) = (((y) >> 24) & 0xff);   \
     *((x)+4) = (((y) >> 32) & 0xff);   \
     *((x)+5) = (((y) >> 40) & 0xff);   \
     *((x)+6) = (((y) >> 48) & 0xff);   \
     *((x)+7) = (((y) >> 56) & 0xff);

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))

struct partition_entry {
    unsigned char type_guid[PARTITION_TYPE_GUID_SIZE];
    unsigned char unique_partition_guid[UNIQUE_PARTITION_GUID_SIZE];
    unsigned long long first_lba;
    unsigned long long last_lba;
    unsigned long long attribute_flag;
    unsigned char name[MAX_GPT_NAME_SIZE];
    //TMP no need
    unsigned dtype;
    unsigned long long size;
};

/* Partition info requested by app layer */
struct partition_info {
    uint64_t offset;
    uint64_t size;
};

typedef struct partition_device {
    storage_device_t *storage;
    uint64_t gpt_offset;
    struct partition_entry *partition_entries;
    unsigned count;
    signed active_slot;    /* to store current active slot */
    bool attributes_updated;    /* to store if we need to update partition table */
    bool multislot_support;    /* to store if multislot support is present */
    bool gpt_partitions_exist;
} partition_device_t;

/*
 * Name is for querying avaliable storage devices, which shall be implemented by platform.
 * gpt_start is for GPT table offset, some devices like OSPI, the GPT head is not located
 * at the second block.
 *
 * Return initialized partition_device
 */
partition_device_t *ptdev_setup(storage_device_t *storage,
                                uint64_t gpt_offset);

/*
 * Release partition entries and ptdev
 */
unsigned int ptdev_destroy(partition_device_t *ptdev);

/*
 * Query the index by partition name, the index is to indicate partition entries array
 */
unsigned int ptdev_get_index(partition_device_t *part_dev,
                             const char *name);

/*
 * Get size of the partition
 * Return 0 if the name is invalid
 */
unsigned long long ptdev_get_size(partition_device_t *part_dev,
                                  const char *name);

/*
 * Get offset of the partition
 * Return 0 if the name is invalid
 */
unsigned long long ptdev_get_offset(partition_device_t *part_dev,
                                    const char *name);

/*
 * Read the storage device, fill the partition entries array, scan and find active slot
 * Invoke after ptdev_set_abslot_decider, or else take for no a/b slot support
 */
int ptdev_read_table(partition_device_t *part_dev);

/*
 * Overwrite the partition table, if name is NULL, overwrite the globe partition table, or else
 * overwrite the sub-partition table.
 */
int ptdev_write_table(partition_device_t *part_dev, const char *name,
                      unsigned size, unsigned char *partition, bool last_part_extend);

/* Return the partition offset & size to app layer
 * Size and offset is zero if the name is invalid
 */
struct partition_info ptdev_get_info(partition_device_t *part_dev,
                                     const char *name);

/* For Debugging
 * Print all parsed partitions
 */
void ptdev_dump(partition_device_t *part_dev);

void ptdev_attr_dump(partition_device_t *ptdev);


/* Read only attribute for partition */
bool partition_is_readonly(partition_device_t *part_dev, const char *name);

/* Get Partition Count */
unsigned ptdev_get_partition_count(partition_device_t *part_dev);

/* Read Partition entried list pointer, indicate the entry by index from ptdev_get_index */
struct partition_entry *ptdev_get_partition_entries(partition_device_t
        *part_dev);

/* check the attribute of part entry */
bool part_is_active(const struct partition_entry *entry);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PARTITION_PARSER_H */