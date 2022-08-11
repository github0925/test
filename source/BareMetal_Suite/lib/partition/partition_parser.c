/* Copyright (c) 2011-2018, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation. nor the names of its
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

#include <stdlib.h>
#include <string.h>
#include <partition_parser.h>
#include <ab_partition_parser.h>
#include <debug.h>
#include <assert.h>
#include <cksum.h>
#include "libavb.h"
#include "partition_mem.h"
#include "helper.h"

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(x)  \
    do{\
        if (!(x)) \
            DBG("assert %s %d:%d", __func__, __LINE__);\
    }while(0)

#define ENTRIES_NUM_NOT_AFTER (32)

#define round_boundary(value, boundary)     \
    ((__typeof__(value))((boundary) - 1))

#define round_up(value, boundary)       \
    ((((value) - 1) | round_boundary(value, boundary)) + 1)

#ifdef EMMC_BOOT
#define BOOT_DEV MMC
#else
#define BOOT_DEV OSPI
#endif

#ifndef BOOT_SECTOR_IDX
#define BOOT_SECTOR_IDX 2
#endif

static uint32_t ptdev_read_gpt(partition_device_t *part_dev,
                               struct partition_entry *parent_entry, uint32_t block_size);
static uint32_t ptdev_read_mbr(partition_device_t *part_dev,
                               uint32_t block_size);
static void mbr_fill_name(struct partition_entry *partition_ent,
                          uint32_t type);
static uint32_t ptdev_verify_mbr_signature(uint32_t size,
        const uint8_t *buffer);

static uint32_t ptdev_parse_gpt_header(partition_device_t *part_dev,
                                       uint8_t *buffer,
                                       uint64_t *first_usable_lba,
                                       uint32_t *partition_entry_size,
                                       uint32_t *header_size,
                                       uint32_t *max_partition_count,
                                       struct partition_entry *parent_entry,
                                       bool secondary_gpt,
                                       uint64_t *partition_entries_offset,
                                       uint32_t *crc_entries_array,
                                       bool check_lba);

static const char *partition_separator = "$";

static bool ptdev_sub_part_exist(struct partition_entry *partition_entries,
                                 unsigned index);

unsigned ptdev_get_partition_count(partition_device_t *part_dev)
{
    return part_dev->count;
}

struct partition_entry *ptdev_get_partition_entries(partition_device_t
        *part_dev)
{
    return part_dev->partition_entries;
}

int ptdev_read_table(partition_device_t *part_dev)
{
    uint32_t ret;
    uint32_t block_size;
    uint32_t partition_count;
    struct partition_entry   *partition_entries;
    storage_device_t *storage = part_dev->storage;
    static struct partition_entry s_entries[ENTRIES_NUM_NOT_AFTER];
    block_size = storage->get_block_size(storage);
    /* Before getting partition count, set count to 0 and set entries pointer to NULL */
    part_dev->count = 0;
    memset(s_entries, 0x0, sizeof(s_entries));
    part_dev->partition_entries =
        &s_entries[0];//(struct partition_entry *) PTDEV_CALLOC(
    //   partition_count, sizeof(struct partition_entry));
    /* Read MBR of the card */
    ret = ptdev_read_mbr(part_dev, block_size);

    if (ret) {
        WARN("Boot: MBR read failed!\n");
        return 1;
    }

    /* Read GPT of the card if exist */
    if (part_dev->gpt_partitions_exist) {
        ret = ptdev_read_gpt(part_dev, NULL, block_size);

        if (ret) {
            WARN("Boot: GPT read failed!\n");
            return 1;
        }
    }

    partition_entries = part_dev->partition_entries;
    partition_count = part_dev->count;

    for (unsigned i = 0; i < partition_count; i++) {
        if (ptdev_sub_part_exist(partition_entries, i)) {
            DBG("%s may have sub partitions\n", partition_entries[i].name);

            if (ptdev_read_gpt(part_dev, &partition_entries[i], block_size)) {
                WARN("read extral partition table failed\n");
            }
        }
    }

    /* Scan of multislot support */
    ptdev_scan_for_multislot(part_dev);
    return 0;
}

/*
 * Read MBR from MMC card and fill partition table.
 */
static uint32_t ptdev_read_mbr(partition_device_t *part_dev,
                               uint32_t block_size)
{
    uint8_t *buffer = NULL;
    uint32_t dtype;
    uint32_t dfirstsec;
    uint32_t EBR_first_sec;
    uint32_t EBR_current_sec;
    int ret = 0;
    int idx, i;
    storage_device_t *storage = part_dev->storage;
    unsigned partition_count = part_dev->count;
    struct partition_entry *partition_entries = part_dev->partition_entries;
    struct partition_entry *current_entry;
    struct partition_entry pt_entry;
    uint8_t local_buf[DEFAULT_BLK_SZ] __ALIGNED(DEFAULT_BLK_SZ);
#if 0
    buffer = block_data_buf;//PTDEV_MEMALIGN(block_size, block_size);

    if (!buffer) {
        DBG(
            "Error allocating memory while reading partition table\n");
        ret = -1;
        goto end;
    }

#endif

    /* Print out the MBR first */
    if (storage->read_ptr) {
        buffer = storage->read_ptr(storage, part_dev->gpt_offset, block_size);
    }

    if (!buffer) {
        buffer = local_buf;
        storage->read(storage, part_dev->gpt_offset, buffer, block_size);
    }

    /* Check to see if signature exists */
    ret = ptdev_verify_mbr_signature(block_size, buffer);

    if (ret) {
        goto end;
    }

    /*
     * Process each of the four partitions in the MBR by reading the table
     * information into our mbr table.
     */
    idx = TABLE_ENTRY_0;

    for (i = 0; i < 4; i++) {
        /* Type 0xEE indicates end of MBR and GPT partitions exist */
        dtype = buffer[idx + i * TABLE_ENTRY_SIZE + OFFSET_TYPE];

        if (dtype == MBR_PROTECTED_TYPE) {
            part_dev->gpt_partitions_exist = true;
            goto end;
        }

        current_entry = partition_entries ? &partition_entries[partition_count] :
                        &pt_entry;
        current_entry->dtype = dtype;
        current_entry->attribute_flag =
            buffer[idx + i * TABLE_ENTRY_SIZE + OFFSET_STATUS];
        current_entry->first_lba =
            GET_LWORD_FROM_BYTE(&buffer[idx +
                                            i * TABLE_ENTRY_SIZE +
                                            OFFSET_FIRST_SEC]);
        current_entry->size =
            GET_LWORD_FROM_BYTE(&buffer[idx +
                                            i * TABLE_ENTRY_SIZE +
                                            OFFSET_SIZE]);
        dfirstsec = current_entry->first_lba;
        mbr_fill_name(current_entry, current_entry->dtype);
        partition_count++;

        if (partition_count == NUM_PARTITIONS)
            goto end;
    }

    part_dev->count = partition_count;

    /* See if the last partition is EBR, if not, parsing is done */
    if (dtype != MBR_EBR_TYPE) {
        goto end;
    }

    EBR_first_sec = dfirstsec;
    EBR_current_sec = dfirstsec;
    buffer = NULL;

    if (storage->read_ptr) {
        buffer = storage->read_ptr(storage,
                                   (EBR_first_sec * block_size) + part_dev->gpt_offset,
                                   block_size);
    }

    if (!buffer) {
        buffer = local_buf;
        storage->read(storage,
                      (EBR_first_sec * block_size) + part_dev->gpt_offset,
                      buffer, block_size);
    }

    /* Loop to parse the EBR */
    for (i = 0;; i++) {
        ret = ptdev_verify_mbr_signature(block_size, buffer);

        if (ret) {
            ret = 0;
            break;
        }

        current_entry = partition_entries ? &partition_entries[partition_count] :
                        &pt_entry;
        current_entry->attribute_flag =
            buffer[TABLE_ENTRY_0 + OFFSET_STATUS];
        current_entry->dtype =
            buffer[TABLE_ENTRY_0 + OFFSET_TYPE];
        current_entry->first_lba =
            GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 +
                                                      OFFSET_FIRST_SEC]) +
            EBR_current_sec;
        current_entry->size =
            GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 + OFFSET_SIZE]);
        mbr_fill_name(current_entry, current_entry->dtype);
        partition_count++;

        if (partition_count == NUM_PARTITIONS)
            goto end;

        dfirstsec =
            GET_LWORD_FROM_BYTE(&buffer
                                [TABLE_ENTRY_1 + OFFSET_FIRST_SEC]);

        if (dfirstsec == 0) {
            /* Getting to the end of the EBR tables */
            break;
        }

        /* More EBR to follow - read in the next EBR sector */
        DBG("Reading EBR block from 0x%X\n", EBR_first_sec
            + dfirstsec);
        buffer = NULL;

        if (storage->read_ptr) {
            buffer = storage->read_ptr(storage,
                                       ((EBR_first_sec + dfirstsec) * block_size) +
                                       part_dev->gpt_offset, block_size);
        }

        if (!buffer) {
            buffer = local_buf;
            storage->read(storage, ((EBR_first_sec + dfirstsec) * block_size)
                          + part_dev->gpt_offset, buffer, block_size);
        }

        EBR_current_sec = EBR_first_sec + dfirstsec;
    }

end:
#if 0

    if (buffer)
        PTDEV_FREE(buffer);

#endif
    return ret;
}

/*
 * Read GPT from MMC and fill partition table
 */
static uint32_t ptdev_read_gpt(partition_device_t *part_dev,
                               struct partition_entry *parent_entry,
                               uint32_t block_size)
{
    int ret = 0;
    uint32_t crc_val = 0;
    uint32_t crc_entries = 0;
    uint32_t header_size;
    bool valid_entry_finish = false;
    uint64_t first_usable_lba;
    uint32_t max_partition_count = 0;
    uint32_t partition_entry_size;
    uint32_t i = 0; /* Counter for each block */
    uint32_t j = 0; /* Counter for each entry in a block */
    uint32_t n = 0; /* Counter for UTF-16 -> 8 conversion */
    unsigned char UTF16_name[MAX_GPT_NAME_SIZE];
    /* LBA of first partition -- 1 Block after Protected MBR + 1 for PT */
    static uint8_t data[DEFAULT_BLK_SZ];
    uint8_t *pbuf = NULL;
    uint32_t part_entry_cnt = block_size / PARTITION_ENTRY_SIZE;
    unsigned long long offset;
    unsigned long long lba_offset;
    uint64_t partition_entries_offset = 0;
    struct partition_entry *partition_entries = part_dev->partition_entries;
    struct partition_entry *current_entry;
    struct partition_entry pt_entry;
    unsigned partition_count = part_dev->count;
    storage_device_t *storage = part_dev->storage;
    uint8_t local_buf[DEFAULT_BLK_SZ] __ALIGNED(DEFAULT_BLK_SZ);

    /* Get the density of the boot device */

    if (!parent_entry) {
        //device_capacity = storage->get_capacity(storage);
        offset = 0;
        lba_offset = 0;
    }
    else {
        offset = parent_entry->first_lba * block_size;
        //device_capacity = (parent_entry->last_lba -
        //                   parent_entry->first_lba + 1) * block_size;
        lba_offset = parent_entry->first_lba;
        DBG("name %s, first %llu, last %llu\n",
            parent_entry->name, parent_entry->first_lba,
            parent_entry->last_lba);
    }

#if 0
    data = block_data_buf;//(uint8_t *)PTDEV_MEMALIGN(block_size, block_size);

    if (!data) {
        DBG("Failed to Allocate memory to read partition table\n");
        ret = -1;
        goto end;
    }

    data_org_ptr = data;
#endif
    /* Print out the GPT first */
    pbuf = NULL;

    if (storage->read_ptr) {
        pbuf = storage->read_ptr(storage,
                                 block_size + offset + part_dev->gpt_offset,
                                 block_size);
    }

    if (!pbuf) {
        pbuf = local_buf;
        storage->read(storage, block_size + offset + part_dev->gpt_offset, pbuf,
                      block_size);
    }

    memcpy(&data[0], pbuf, block_size);
    ret = ptdev_parse_gpt_header(part_dev, data, &first_usable_lba,
                                 &partition_entry_size, &header_size,
                                 &max_partition_count, parent_entry, false,
                                 &partition_entries_offset,
                                 &crc_entries, true);

    if (ret) {
        WARN("GPT: Primary signatures invalid\n");
        goto end;
    }

#if 0

    if (ret) {
        DBG("GPT: (WARNING) Primary signature invalid\n");
        /* Check the backup gpt */
        /* Get size of MMC */
        card_size_sec = (device_capacity) / block_size;
        ASSERT(card_size_sec > 0);
        backup_header_lba = card_size_sec - 1;
        ret = storage->read(storage, (backup_header_lba * block_size) +
                            part_dev->gpt_offset, &data,
                            block_size);

        if (ret) {
            DBG("GPT: Could not read backup gpt from boot_device\n");
            goto end;
        }

        ret = ptdev_parse_gpt_header(part_dev, data, &first_usable_lba,
                                     &partition_entry_size,
                                     &header_size,
                                     &max_partition_count, parent_entry, true,
                                     &partition_entries_offset,
                                     &crc_entries, true);

        if (ret) {
            DBG("GPT: Primary and backup signatures invalid\n");
            goto end;
        }
    }

#endif
#if 0
    entries_buffer = entries_buf;//PTDEV_MEMALIGN(block_size, block_size);

    if (!entries_buffer) {
        DBG("GPT: Allocate memory fail\n");
        ret = 1;
        goto end;
    }

#endif
    crc_val = 0;

    /* Read GPT Entries */
    for (i = 0;
            i < (round_up(max_partition_count, part_entry_cnt)) / part_entry_cnt;
            i++) {
        ASSERT(partition_count < NUM_PARTITIONS);
        pbuf = NULL;

        if (storage->read_ptr) {
            pbuf = storage->read_ptr(storage,
                                     partition_entries_offset + (i * block_size),
                                     block_size);
        }

        if (!pbuf) {
            pbuf = local_buf;
            storage->read(storage, partition_entries_offset + (i * block_size), pbuf,
                          block_size);
        }

        memcpy(&data[0], pbuf, block_size);
        crc_val = crc32(crc_val, data, block_size);

        for (j = 0; j < part_entry_cnt && !valid_entry_finish; j++) {
            /*
             * If partition_entries is NULL,
             * it means that the caller only wants
             * to get partition entries count
             */
            current_entry = &pt_entry;
            memcpy(&(current_entry->type_guid),
                   &data[(j * partition_entry_size)],
                   PARTITION_TYPE_GUID_SIZE);

            if (current_entry->type_guid[0] == 0x00
                    && current_entry->type_guid[1] == 0x00) {
                /*
                 * Here, the last valid partition has got,
                 * but needs to read remain data for crc32 of partition
                 * entries array
                 */
                //i = ROUNDUP(max_partition_count, part_entry_cnt);
                valid_entry_finish = true;
                break;
            }

            if (partition_entries && partition_count < ENTRIES_NUM_NOT_AFTER) {
                current_entry = &partition_entries[partition_count];
                memcpy(&(current_entry->type_guid), &(pt_entry.type_guid),
                       PARTITION_TYPE_GUID_SIZE);
            }

            memcpy(&(current_entry->unique_partition_guid),
                   &data[(j * partition_entry_size) + UNIQUE_GUID_OFFSET],
                   UNIQUE_PARTITION_GUID_SIZE);
            current_entry->first_lba =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) + FIRST_LBA_OFFSET])
                + lba_offset;
            current_entry->last_lba =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) + LAST_LBA_OFFSET])
                + lba_offset;
            /* If partition entry LBA is not valid, skip this entry
                and parse next entry */
#if 0

            if ((current_entry->first_lba) < first_usable_lba
                    || (current_entry->last_lba) >
                    ((offset + device_capacity) / block_size -
                     (blocks_for_entries + GPT_HEADER_BLOCKS + 1))
                    || current_entry->first_lba >
                    current_entry->last_lba) {
                DBG("Partition entry(%d), lba not valid\n", j);
                continue;
            }

#endif
            current_entry->size = current_entry->last_lba - current_entry->first_lba +
                                  1;
            current_entry->attribute_flag =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) +
                                                                      ATTRIBUTE_FLAG_OFFSET]);
            memcpy(UTF16_name, &data[(j * partition_entry_size) +
                                                                PARTITION_NAME_OFFSET],
                   MAX_GPT_NAME_SIZE);

            /*
             * Currently partition names in *.xml are UTF-8 and lowercase
             * Only supporting english for now so removing 2nd byte of UTF-16
             */
            for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++) {
                current_entry->name[n] = UTF16_name[n * 2];
            }

            if (parent_entry) {
                unsigned char *parent_name = parent_entry->name;
                char name[MAX_GPT_NAME_SIZE * 2 + 1] = {0};
                sprintf(name, "%s%s%s", parent_name, partition_separator,
                        current_entry->name);

                if (strlen(name) >= MAX_GPT_NAME_SIZE)
                    WARN("parent or sub partiton name is too long.\n");

                strcpy((char *)current_entry->name,
                       (const char *)&name);
            }

            partition_count++;

            if (!partition_entries
                    && current_entry->attribute_flag & PART_ATT_SUB_PARTITION_VAL) {
                ptdev_read_gpt(part_dev, current_entry, block_size);
            }
        }
    }

    if (crc_val != crc_entries) {
        WARN("Partition entires crc mismatch crc_val= 0x%08x with crc_val_org= 0x%08x\n",
             crc_val, crc_entries);
        ret = 1;
        goto end;
    }

    part_dev->count = partition_count;
end:
#if 0

    if (data_org_ptr)
        PTDEV_FREE(data_org_ptr);

    if (entries_buffer) {
        PTDEV_FREE(entries_buffer);
        entries_buffer = NULL;
    }

#endif
    return ret;
}

#if 0
static uint32_t write_mbr_in_blocks(partition_device_t *part_dev,
                                    uint32_t size, uint8_t *mbrImage, uint32_t block_size)
{
    uint32_t dtype;
    uint32_t dfirstsec;
    uint32_t ebrSectorOffset;
    unsigned char *ebrImage;
    unsigned char *lastAddress;
    int idx, i;
    uint32_t ret;
    storage_device_t *storage = part_dev->storage;
    /* Write the first block */
    ret = storage->write(storage, part_dev->gpt_offset, (uint8_t *)mbrImage,
                         block_size);

    if (ret) {
        DBG("Failed to write mbr partition\n");
        goto end;
    }

    DBG("write of first MBR block ok\n");
    /*
       Loop through the MBR table to see if there is an EBR.
       If found, then figure out where to write the first EBR
     */
    idx = TABLE_ENTRY_0;

    for (i = 0; i < 4; i++) {
        dtype = mbrImage[idx + i * TABLE_ENTRY_SIZE + OFFSET_TYPE];

        if (MBR_EBR_TYPE == dtype) {
            DBG("EBR found.\n");
            break;
        }
    }

    if (MBR_EBR_TYPE != dtype) {
        DBG("No EBR in this image\n");
        goto end;
    }

    /* EBR exists.  Write each EBR block to boot_device */
    ebrImage = mbrImage + block_size;
    ebrSectorOffset =
        GET_LWORD_FROM_BYTE(&mbrImage
                            [idx + i * TABLE_ENTRY_SIZE +
                                 OFFSET_FIRST_SEC]);
    dfirstsec = 0;
    DBG("first EBR to be written at sector 0x%X\n", dfirstsec);
    lastAddress = mbrImage + size;

    while (ebrImage < lastAddress) {
        DBG("writing to 0x%X\n",
            (ebrSectorOffset + dfirstsec) * block_size);
        ret =
            storage->write(storage, (ebrSectorOffset + dfirstsec) * block_size +
                           part_dev->gpt_offset,
                           (uint8_t *)ebrImage, block_size);

        if (ret) {
            DBG("Failed to write EBR block to sector 0x%X\n",
                dfirstsec);
            goto end;
        }

        dfirstsec =
            GET_LWORD_FROM_BYTE(&ebrImage
                                [TABLE_ENTRY_1 + OFFSET_FIRST_SEC]);
        ebrImage += block_size;
    }

    DBG("MBR written to boot device successfully\n");
end:
    return ret;
}
#endif
/*
 * A8h reflected is 15h, i.e. 10101000 <--> 00010101
*/
int reflect(int data, int len)
{
    int ref = 0;

    for (int i = 0; i < len; i++) {
        if (data & 0x1) {
            ref |= (1 << ((len - 1) - i));
        }

        data = (data >> 1);
    }

    return ref;
}

#if 0
/*
 * Write the GPT Partition Entry Array to the MMC.
 */
static uint32_t
write_gpt_partition_array(partition_device_t *part_dev, uint8_t *header,
                          unsigned char *partition_array_start,
                          uint32_t array_size,
                          uint32_t block_size,
                          uint64_t parent_lba)
{
    uint32_t ret = 1;
    unsigned long long partition_entry_lba;
    unsigned long long partition_entry_array_start_location;
    storage_device_t *storage = part_dev->storage;
    partition_entry_lba =
        GET_LLWORD_FROM_BYTE(&header[PARTITION_ENTRIES_OFFSET]) + parent_lba;
    partition_entry_array_start_location = partition_entry_lba * block_size;
    ret = storage->write(storage, partition_entry_array_start_location +
                         part_dev->gpt_offset,
                         (uint8_t *)partition_array_start, array_size);

    if (ret) {
        DBG("GPT: FAILED to write the partition entry array\n");
        goto end;
    }

end:
    return ret;
}

static void
patch_gpt(partition_device_t *part_dev, uint8_t *gptImage,
          uint64_t density, uint32_t array_size,
          uint32_t max_part_count, uint32_t part_entry_size, uint32_t block_size,
          struct partition_entry *parent_entry, bool last_part_extend)
{
    unsigned char *primary_gpt_header;
    unsigned char *secondary_gpt_header;
    unsigned long long *last_partition_entry;
    uint32_t offset;
    unsigned char *partition_entry_array_start;
    unsigned long long card_size_sec;
    int total_part = 0;
    uint32_t crc_value = 0;
    uint32_t last_part_offset;
    unsigned ptn_entries_blocks = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) /
                                  block_size;
    /* Get size of MMC */
    card_size_sec = (density) / block_size;

    /* Working around cap at 4GB */
    if (card_size_sec == 0) {
        card_size_sec = 4 * 1024 * 1024 * 2 - 1;
    }

    /* Patching primary header */
    primary_gpt_header = (gptImage + block_size);
    PUT_LONG_LONG(primary_gpt_header + BACKUP_HEADER_OFFSET,
                  ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(primary_gpt_header + LAST_USABLE_LBA_OFFSET,
                  ((long long)(card_size_sec -
                               (ptn_entries_blocks + GPT_HEADER_BLOCKS + 1))));
    /* Patching backup GPT */
    offset = (2 * array_size);
    secondary_gpt_header = offset + block_size + primary_gpt_header;
    PUT_LONG_LONG(secondary_gpt_header + PRIMARY_HEADER_OFFSET,
                  ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(secondary_gpt_header + LAST_USABLE_LBA_OFFSET,
                  ((long long)(card_size_sec -
                               (ptn_entries_blocks + GPT_HEADER_BLOCKS + 1))));
    PUT_LONG_LONG(secondary_gpt_header + PARTITION_ENTRIES_OFFSET,
                  ((long long)(card_size_sec -
                               (ptn_entries_blocks + GPT_HEADER_BLOCKS))));
    /* Find last partition */
    last_partition_entry = (unsigned long long *)
                           (primary_gpt_header + block_size + total_part * PARTITION_ENTRY_SIZE);

    //need check 128 bit for GUID
    while (*last_partition_entry != 0 ||
            *(last_partition_entry + 1) != 0) {
        total_part++;
        last_partition_entry = (unsigned long long *)
                               (primary_gpt_header + block_size + total_part * PARTITION_ENTRY_SIZE);
    }

    /* Patching last partition */
    if (last_part_extend) {
        last_part_offset =
            (total_part - 1) * PARTITION_ENTRY_SIZE + PARTITION_ENTRY_LAST_LBA;
        PUT_LONG_LONG(primary_gpt_header + block_size + last_part_offset,
                      (long long)(card_size_sec -
                                  (ptn_entries_blocks + GPT_HEADER_BLOCKS + 1)));
        PUT_LONG_LONG(primary_gpt_header + block_size + last_part_offset +
                      array_size, (long long)(card_size_sec -
                                              (ptn_entries_blocks + GPT_HEADER_BLOCKS + 1)));
    }

    /* Updating CRC of the Partition entry array in both headers */
    partition_entry_array_start = primary_gpt_header + block_size;
    crc_value = crc32(0, partition_entry_array_start,
                      max_part_count * part_entry_size);
    PUT_LONG(primary_gpt_header + PARTITION_CRC_OFFSET, crc_value);
    crc_value = crc32(0, partition_entry_array_start + array_size,
                      max_part_count * part_entry_size);
    PUT_LONG(secondary_gpt_header + PARTITION_CRC_OFFSET, crc_value);
    /* Clearing CRC fields to calculate */
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, 0);
    crc_value = crc32(0, primary_gpt_header, GPT_HEADER_SIZE);
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, crc_value);
    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, 0);
    crc_value = crc32(0, secondary_gpt_header, GPT_HEADER_SIZE);
    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, crc_value);
}
#endif
/*
 * Fill name for android partition found.
 */
static void
mbr_fill_name(struct partition_entry *partition_ent, uint32_t type)
{
    memset(partition_ent->name, 0, MAX_GPT_NAME_SIZE);

    switch (type) {
        case MBR_RPM_TYPE:
            strcpy((char *)partition_ent->name, "rpm");
            break;

        case MBR_TZ_TYPE:
            strcpy((char *)partition_ent->name, "tz");
            break;

        case MBR_BOOT_TYPE:
            strcpy((char *)partition_ent->name, "boot");
            break;

        case MBR_RECOVERY_TYPE:
            strcpy((char *)partition_ent->name, "recovery");
            break;

        case MBR_MISC_TYPE:
            strcpy((char *)partition_ent->name, "msic");
            break;

        case MBR_SSD_TYPE:
            strcpy((char *)partition_ent->name, "ssd");
            break;
    };
}

/*
 * Find index of parition in array of partition entries
 */
uint32_t ptdev_get_index(partition_device_t *part_dev,
                         const char *name)
{
    uint32_t input_string_length = strlen(name);
    unsigned n;
    int curr_slot = INVALID;
    const char *suffix_curr_actv_slot = NULL;
    char *curr_suffix = NULL;
    unsigned partition_count = part_dev->count;
    struct partition_entry *partition_entries = part_dev->partition_entries;

    if (partition_count > NUM_PARTITIONS) {
        return INVALID_PTN;
    }

    /*  We iterate through the parition entries list,
        to find the partition with active slot suffix.
    */
    for (n = 0; n < partition_count; n++) {
        if (!strncmp((const char *)name, (const char *)partition_entries[n].name,
                     input_string_length)) {
            curr_suffix = (char *)(partition_entries[n].name + input_string_length);

            /* if partition_entries.name is NULL terminated return the index */
            if (*curr_suffix == '\0')
                return n;

            if (ptdev_multislot_is_supported(part_dev)) {
                curr_slot = ptdev_find_active_slot(part_dev);

                /* If suffix string matches with current active slot suffix return index */
                if (curr_slot != INVALID) {
                    suffix_curr_actv_slot = SUFFIX_SLOT(curr_slot);

                    if (!strncmp((const char *)curr_suffix, suffix_curr_actv_slot,
                                 strlen(suffix_curr_actv_slot)))
                        return n;
                    else
                        continue;
                }
                else {
                    /* No valid active slot */
                    return INVALID_PTN;
                }
            }
        }
    }

    return INVALID_PTN;
}

/* Get size of the partition */
unsigned long long ptdev_get_size(partition_device_t *part_dev,
                                  const char *name)
{
    uint32_t block_size;
    uint32_t index;
    struct partition_entry *partition_entries;
    index = ptdev_get_index(part_dev, name);
    storage_device_t *storage = part_dev->storage;
    block_size = storage->get_block_size(storage);
    partition_entries = part_dev->partition_entries;

    if (index == (unsigned)INVALID_PTN)
        return 0;
    else {
        return partition_entries[index].size * block_size;
    }
}

/* Get offset of the partition */
unsigned long long ptdev_get_offset(partition_device_t *part_dev,
                                    const char *name)
{
    uint32_t block_size;
    uint32_t index;
    index = ptdev_get_index(part_dev, name);
    storage_device_t *storage = part_dev->storage;
    block_size = storage->get_block_size(storage);
    struct partition_entry *partition_entries = part_dev->partition_entries;

    if (index == (unsigned)INVALID_PTN)
        return 0;
    else {
        return partition_entries[index].first_lba * block_size +
               part_dev->gpt_offset;
    }
}

struct partition_info ptdev_get_info(partition_device_t *part_dev,
                                     const char *name)
{
    struct partition_info info = {0};

    if (!name) {
        DBG("Invalid partition name passed\n");
        return info;
    }

    info.offset = ptdev_get_offset(part_dev, name);
    info.size   = ptdev_get_size(part_dev, name);

    if (!info.offset || !info.size)
        DBG("Error unable to find partition : [%s]\n", name);

    return info;
}

/* Debug: Print all parsed partitions */
void ptdev_dump(partition_device_t *part_dev)
{
#if 0
    unsigned i = 0;
    unsigned partition_count = part_dev->count;
    struct partition_entry *partition_entries = part_dev->partition_entries;

    for (i = 0; i < partition_count; i++) {
        DBG("ptn[%d]:Name[%s] Size[%llu] Type[%u] First[%llu] Last[%llu]\n",
            i, partition_entries[i].name, partition_entries[i].size,
            partition_entries[i].dtype,
            partition_entries[i].first_lba,
            partition_entries[i].last_lba);
    }

#endif
}

static uint32_t
ptdev_verify_mbr_signature(uint32_t size, const unsigned char *buffer)
{
    /* Avoid checking past end of buffer */
    if ((TABLE_SIGNATURE + 1) > size) {
        return 1;
    }

    /* Check to see if signature exists */
    if ((buffer[TABLE_SIGNATURE] != MMC_MBR_SIGNATURE_BYTE_0) ||
            (buffer[TABLE_SIGNATURE + 1] != MMC_MBR_SIGNATURE_BYTE_1)) {
        DBG("MBR signature does not match.\n");
        return 1;
    }

    return 0;
}

#if 0
/* master boot record partition types should be in a byte */
static uint32_t
mbr_partition_get_type(uint32_t size, unsigned char *partition,
                       unsigned char *partition_type)
{
    uint32_t type_offset = TABLE_ENTRY_0 + OFFSET_TYPE;

    if (size < (type_offset + sizeof(*partition_type))) {
        return 1;
    }

    *partition_type = partition[type_offset];
    return 0;
}

static uint32_t
ptdev_get_type(uint32_t size, unsigned char *partition,
               uint32_t *partition_type)
{
    uint32_t ret = 0;
    /*
     * If the block contains the MBR signature, then it's likely either
     * MBR or MBR with protective type (GPT).  If the MBR signature is
     * not there, then it could be the GPT backup.
     */
    /* First check the MBR signature */
    ret = ptdev_verify_mbr_signature(size, partition);

    if (!ret) {
        unsigned char mbr_partition_type = PARTITION_TYPE_MBR;
        /* MBR signature verified.  This could be MBR, MBR + EBR, or GPT */
        ret =
            mbr_partition_get_type(size, partition,
                                   &mbr_partition_type);

        if (ret) {
            DBG("Cannot get TYPE of partition");
        }
        else if (MBR_PROTECTED_TYPE == mbr_partition_type) {
            *partition_type = PARTITION_TYPE_GPT;
        }
        else {
            *partition_type = PARTITION_TYPE_MBR;
        }
    }
    else {
        /*
         * This could be the GPT backup.  Make that assumption for now.
         * Anybody who treats the block as GPT backup should check the
         * signature.
         */
        *partition_type = PARTITION_TYPE_GPT_BACKUP;
    }

    return ret;
}
#endif
/*
 * Parse the gpt header and get the required header fields
 * Return 0 on valid signature
 */
static uint32_t
ptdev_parse_gpt_header(partition_device_t *part_dev, unsigned char *buffer,
                       uint64_t *first_usable_lba,
                       uint32_t *partition_entry_size,
                       uint32_t *header_size,
                       uint32_t *max_partition_count,
                       struct partition_entry *parent_entry,
                       bool secondary_gpt,
                       uint64_t *partition_entries_offset,
                       uint32_t *crc_entries_array,
                       bool check_lba)
{
    uint32_t ret = 0;
    uint32_t crc_val = 0;
    uint32_t crc_val_org = 0;
    uint32_t partitions_for_block = 0;
    uint64_t blocks_to_read = 0;
    unsigned long long last_usable_lba = 0;
    unsigned long long partition_0 = 0;
    unsigned long long current_lba = 0;
    storage_device_t *storage = part_dev->storage;
    uint32_t block_size = storage->get_block_size(storage);
    //uint32_t blocks_for_entries =
    //    (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
    /* Get the density of the mmc device */
    //uint64_t device_capacity;
    uint64_t parent_lba = 0;
    //uint64_t offset = 0;

    if (!parent_entry) {
        //device_capacity = storage->get_capacity(storage) - part_dev->gpt_offset;
    }
    else {
        //device_capacity = (parent_entry->last_lba -
        //                   parent_entry->first_lba + 1) * block_size;
        parent_lba = parent_entry->first_lba;
        //offset = parent_entry->first_lba * block_size;
    }

    /* Check GPT Signature */
    if (((uint32_t *) buffer)[0] != GPT_SIGNATURE_2 ||
            ((uint32_t *) buffer)[1] != GPT_SIGNATURE_1)
        return 1;

    *header_size = GET_LWORD_FROM_BYTE(&buffer[HEADER_SIZE_OFFSET]);

    /* check for header size too small */
    if (*header_size < GPT_HEADER_SIZE) {
        WARN("GPT Header size is too small\n");
        return 1;
    }

    /* check for header size too large */
    if (*header_size > block_size) {
        WARN("GPT Header size is too large\n");
        return 1;
    }

    crc_val_org = GET_LWORD_FROM_BYTE(&buffer[HEADER_CRC_OFFSET]);
    /*Write CRC to 0 before we calculate the crc of the GPT header*/
    crc_val = 0;
    PUT_LONG(&buffer[HEADER_CRC_OFFSET], crc_val);
    crc_val  = crc32(0, buffer, *header_size);

    if (crc_val != crc_val_org) {
        WARN("Header crc mismatch crc_val = 0x%08x with crc_val_org = 0x%08x\n",
             crc_val, crc_val_org);
        return 1;
    }
    else
        PUT_LONG(&buffer[HEADER_CRC_OFFSET], crc_val);

    current_lba =
        GET_LLWORD_FROM_BYTE(&buffer[PRIMARY_HEADER_OFFSET]);
    *first_usable_lba =
        GET_LLWORD_FROM_BYTE(&buffer[FIRST_USABLE_LBA_OFFSET]);
    *max_partition_count =
        GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
    *partition_entry_size =
        GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);
    last_usable_lba =
        GET_LLWORD_FROM_BYTE(&buffer[LAST_USABLE_LBA_OFFSET]);

    if (parent_entry) {
        current_lba += parent_lba;
        *first_usable_lba += parent_lba;
        last_usable_lba += parent_lba;
    }

    /* current lba and GPT lba should be same */
    if (!secondary_gpt) {
        if (current_lba != (GPT_LBA + parent_lba)) {
            WARN("Primary GPT first usable LBA mismatch\n");
            return 1;
        }
    }
    else {
#if 0

        /*
          Check only in case of reading, skip for flashing as this is patched
          in patch_gpt() later in flow.
        */
        if (check_lba && (current_lba != ((device_capacity / block_size) - 1))) {
            DBG("Secondary GPT first usable LBA mismatch\n");
            return 1;
        }

#endif
    }

#if 0

    /* check for first lba should be with in the valid range */
    if (*first_usable_lba > (device_capacity / block_size + parent_lba)) {
        DBG("Invalid first_usable_lba\n");
        return 1;
    }

    /* check for last lba should be with in the valid range */
    if (last_usable_lba > ((device_capacity + offset) / block_size)) {
        DBG("Invalid last_usable_lba\n");
        return 1;
    }

#endif

    /* check for partition entry size */
    if (*partition_entry_size != PARTITION_ENTRY_SIZE) {
        WARN("Invalid parition entry size\n");
        return 1;
    }

    if ((*max_partition_count) > (MIN_PARTITION_ARRAY_SIZE /
                                  (*partition_entry_size))) {
        WARN("Invalid maximum partition count\n");
        return 1;
    }

    partitions_for_block = block_size / (*partition_entry_size);
    blocks_to_read = (*max_partition_count) / partitions_for_block;

    if ((*max_partition_count) % partitions_for_block) {
        blocks_to_read += 1;
    }

    if (check_lba) {
        partition_0 = GET_LLWORD_FROM_BYTE(&buffer[PARTITION_ENTRIES_OFFSET]);

        /* start LBA should always be 2 in primary GPT */
        if (!secondary_gpt) {
            if (partition_0 != 0x2) {
                WARN("PrimaryGPT starting LBA mismatch\n");
                return 1;
            }
        }
        else {
#if 0

            if (partition_0 != ((device_capacity / block_size) -
                                (blocks_for_entries + GPT_HEADER_BLOCKS))) {
                DBG("BackupGPT starting LBA mismatch\n");
                return 1;
            }

#endif
        }

        *partition_entries_offset = (partition_0 + parent_lba) *
                                    (block_size) + part_dev->gpt_offset;
        crc_val_org = GET_LWORD_FROM_BYTE(&buffer[PARTITION_CRC_OFFSET]);
        *crc_entries_array = crc_val_org;
    }

    return ret;
}

bool ptdev_gpt_exists(partition_device_t *part_dev)
{
    return part_dev->gpt_partitions_exist;
}

bool partition_is_readonly(partition_device_t *part_dev, const char *name)
{
    struct partition_entry *partition_entries = part_dev->partition_entries;
    uint32_t index = ptdev_get_index(part_dev, name);

    if (index == (unsigned)INVALID_PTN) {
        WARN("Invalide partition name\n");
        return false;
    }

    return !!(partition_entries[index].attribute_flag
              & PART_ATT_READONLY_VAL);
}

static bool ptdev_sub_part_exist(struct partition_entry *partition_entries,
                                 unsigned index)
{
    return !!(partition_entries[index].attribute_flag
              & PART_ATT_SUB_PARTITION_VAL);
}

partition_device_t *ptdev_setup(storage_device_t *storage,
                                uint64_t gpt_offset)
{
    static partition_device_t
    part_dev;// = PTDEV_CALLOC(1, sizeof(partition_device_t));
    part_dev.storage = storage;
    part_dev.gpt_offset = gpt_offset;
    return &part_dev;
}

bool part_is_active(const struct partition_entry *entry)
{
    return !!(entry->attribute_flag & PART_ATT_ACTIVE_VAL);
}

static partition_device_t *init_partition_dev(const char *pt_name)
{
    int ret;
    static storage_device_t *storage = NULL;
    static partition_device_t *ptdev = NULL;

    if (!ptdev) {
        storage = setup_storage_dev(BOOT_DEV, 0, NULL);

        if (!storage) {
            INFO("init storage fail!\n");
            goto fail;
        }

        ptdev = ptdev_setup(storage,
                            storage->get_erase_group_size(storage) * BOOT_SECTOR_IDX);

        if (!ptdev) {
            INFO("init partition device fail!\n");
            goto fail;
        }

        ret = ptdev_read_table(ptdev);

        if (ret) {
            INFO("read gpt table fail!\n");
            goto fail;
        }

        ASSERT(DEFAULT_BLK_SZ == storage->get_block_size(storage));
    }

    return ptdev;
fail:
    return NULL;
}

static const uint64_t load_partition_data(const char *pt_name,
        uint8_t *buf, uint64_t len,
        uint64_t offset, uint64_t *part_size)
{
    uint64_t ptn;
    uint64_t pt_size;
    storage_device_t *storage = NULL;
    partition_device_t *ptdev = NULL;
    ptdev = init_partition_dev(pt_name);

    if (!ptdev) {
        WARN("cann't init partition device!\n");
        goto fail;
    }

    ptn = ptdev_get_offset(ptdev, pt_name);
    pt_size = ptdev_get_size(ptdev, pt_name);

    if (!ptn || pt_size <= offset) {
        WARN("cann't find partition:%s!\n", pt_name);
        goto fail;
    }

    storage = ptdev->storage;

    if (part_size)
        *part_size = pt_size;

    if (storage->read &&
            (!storage->read(storage, ptn + offset, buf, len))) {
        return len;
    }

fail:
    return 0;
}


static const uint8_t *get_partition_addr(const char *pt_name, uint32_t len,
        uint64_t offset, uint64_t *part_size)
{
    uint64_t ptn;
    uint64_t pt_size;
    storage_device_t *storage = NULL;
    partition_device_t *ptdev = NULL;
    ptdev = init_partition_dev(pt_name);

    if (!ptdev) {
        INFO("cann't init partition device!\n");
        goto fail;
    }

    ptn = ptdev_get_offset(ptdev, pt_name);
    pt_size = ptdev_get_size(ptdev, pt_name);

    if (!ptn || pt_size <= offset) {
        INFO("cann't find partition:%s!\n", pt_name);
        goto fail;
    }

    storage = ptdev->storage;

    if (part_size)
        *part_size = pt_size;

    if (storage->read_ptr)
        return (storage->read_ptr(storage, ptn + offset, len));

fail:
    return NULL;
}

static uint64_t get_partition_size(const char *pt_name)
{
    partition_device_t *ptdev = NULL;
    ptdev = init_partition_dev(pt_name);

    if (!ptdev) {
        INFO("cann't init partition device!\n");
        return 0;
    }

    return ptdev_get_size(ptdev, pt_name);
}

const uint8_t *get_partition_addr_by_name(const char *pt_name)
{
    return get_partition_addr(pt_name, 0, 0, NULL);
}

uint64_t get_img_actual_size(const char *pt_name, uint64_t *part_size)
{
    uint64_t pt_size, load_size;
    const uint8_t *footer_buf;
    AvbFooter footer, footer_raw;
    static uint8_t last_block_buf[DEFAULT_BLK_SZ] __ALIGNED(DEFAULT_BLK_SZ) = {0};

    if (!pt_name) {
        WARN("partition params error!\n");
        return 0;
    }

    footer_buf = get_partition_addr(pt_name, 0, 0, &pt_size);

    if (!footer_buf) {
        pt_size = get_partition_size(pt_name);
        load_size = load_partition_data(pt_name, last_block_buf, DEFAULT_BLK_SZ,
                                        pt_size - DEFAULT_BLK_SZ, NULL);

        if (!load_size) {
            INFO("fail to load partition data!\n");
            return 0;
        }

        footer_buf = &last_block_buf[DEFAULT_BLK_SZ - sizeof(AvbFooter)];
    }
    else {
        footer_buf += pt_size - sizeof(AvbFooter);
    }

    if (part_size)
        *part_size = pt_size;

    memcpy(&footer_raw, footer_buf, sizeof(AvbFooter));

    if (avb_footer_validate_and_byteswap(&footer_raw, &footer))
        return footer.original_image_size;

    return pt_size;
}

uint64_t get_partition_size_by_name(const char *pt_name)
{
    return get_partition_size(pt_name);
}

uint64_t load_partition_by_name(const char *pt_name, uint8_t *buf,
                                uint64_t len, uint64_t offset)
{
    uint64_t part_size, actual_sz;
    uint64_t len_origin = len;
    const uint8_t *storage_buf_ptr;

    if (!pt_name || !buf || !len) {
        WARN("partition params error!\n");
        return 0;
    }

    actual_sz = get_img_actual_size(pt_name, &part_size);
    len = (len > actual_sz) ? actual_sz : len;
    len = (len > part_size - offset) ? part_size - offset : len;
    len = ROUNDUP(len, DEFAULT_BLK_SZ);

    if (len > len_origin) {
        WARN("%s buf is too small!\n", __func__);
        return 0;
    }

    storage_buf_ptr = get_partition_addr(pt_name, len, offset, &part_size);

    if (!storage_buf_ptr) {
        return load_partition_data(pt_name, buf, len, offset, NULL);
    }

    memcpy(buf, storage_buf_ptr, len);
    return len;
}

uint64_t load_partition_force_size_by_name(const char *pt_name,
        uint8_t *buf,
        uint64_t len, uint64_t offset)
{
    uint64_t part_size;
    const uint8_t *storage_buf_ptr;

    if (!pt_name || !buf || !len) {
        WARN("partition params error!\n");
        return 0;
    }

    get_img_actual_size(pt_name, &part_size);

    storage_buf_ptr = get_partition_addr(pt_name, len, offset, &part_size);

    if (!storage_buf_ptr) {
        return load_partition_data(pt_name, buf, len, offset, NULL);
    }

    memcpy(buf, storage_buf_ptr, len);
    return len;
}
