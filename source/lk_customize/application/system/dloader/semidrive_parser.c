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

#include <assert.h>
#include <debug.h>
#include <lib/cksum.h>
#include <stdlib.h>
#include <string.h>
#include "semidrive_parser.h"
#include "storage_device.h"

static uint32_t parse_gpt(uint8_t *buf, uint32_t buf_len,
                          uint32_t block_size,
                          GPT_header *gpt_header, bool is_secondary_gpt);

/*
 * Parse the gpt header and get the required header fields
 * Return 0 on valid signature
 */
static unsigned int
partition_parse_gpt_header(unsigned char *buffer,
                           unsigned long long *first_usable_lba,
                           unsigned int *partition_entry_size,
                           unsigned int *header_size,
                           unsigned int *max_partition_count,
                           GPT_header *gpt_header,
                           uint32_t block_size,
                           bool is_secondary_gpt)
{
    uint32_t crc_val = 0;
    uint32_t ret = 0;
    uint32_t crc_val_org = 0;
    unsigned long long last_usable_lba = 0;
    unsigned long long partition_0 = 0;
    unsigned long long current_lba = 0;
    unsigned long long backup_lba = 0;

    /* Check GPT Signature */
    if (((uint32_t *) buffer)[0] != GPT_SIGNATURE_2 ||
            ((uint32_t *) buffer)[1] != GPT_SIGNATURE_1) {
        dprintf(CRITICAL, "GPT signature error:0x%0x  0x%0x\n",
                ((uint32_t *) buffer)[0],
                ((uint32_t *) buffer)[1]);
        return 1;
    }

    *header_size = GET_LWORD_FROM_BYTE(&buffer[HEADER_SIZE_OFFSET]);

    /*check for header size too small*/
    if (*header_size < GPT_HEADER_SIZE) {
        dprintf(CRITICAL, "GPT Header size is too small\n");
        return 1;
    }

    /*check for header size too large*/
    if (*header_size > block_size) {
        dprintf(CRITICAL, "GPT Header size is too large\n");
        return 1;
    }

    crc_val_org = GET_LWORD_FROM_BYTE(&buffer[HEADER_CRC_OFFSET]);
    /*Write CRC to 0 before we calculate the crc of the GPT header*/
    crc_val = 0;
    PUT_LONG(&buffer[HEADER_CRC_OFFSET], crc_val);

    crc_val  = crc32(0, buffer, *header_size);
    if (crc_val != crc_val_org) {
        dprintf(CRITICAL,"Header crc mismatch crc_val = %u with crc_val_org = %u\n", crc_val,crc_val_org);
        return 1;
    }
    else
        PUT_LONG(&buffer[HEADER_CRC_OFFSET], crc_val);

    current_lba =
        GET_LLWORD_FROM_BYTE(&buffer[PRIMARY_HEADER_OFFSET]);
    backup_lba =
        GET_LLWORD_FROM_BYTE(&buffer[BACKUP_HEADER_OFFSET]);
    *first_usable_lba =
        GET_LLWORD_FROM_BYTE(&buffer[FIRST_USABLE_LBA_OFFSET]);
    *max_partition_count =
        GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
    *partition_entry_size =
        GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);
    last_usable_lba =
        GET_LLWORD_FROM_BYTE(&buffer[LAST_USABLE_LBA_OFFSET]);

    /*current lba and GPT lba should be same*/
    if (!is_secondary_gpt && current_lba != GPT_LBA) {
        dprintf(CRITICAL, "Primary GPT first usable LBA mismatch\n");
        return 1;
    }

    /*check for partition entry size*/
    if (*partition_entry_size != PARTITION_ENTRY_SIZE) {
        dprintf(CRITICAL, "Invalid parition entry size\n");
        return 1;
    }

    if ((*max_partition_count) > (MIN_PARTITION_ARRAY_SIZE /
                                  (*partition_entry_size))) {
        dprintf(CRITICAL, "Invalid maximum partition count\n");
        return 1;
    }

    partition_0 = GET_LLWORD_FROM_BYTE(&buffer[PARTITION_ENTRIES_OFFSET]);

    /*start LBA should always be 2 in primary GPT*/
    if (!is_secondary_gpt && partition_0 != 0x2) {
        dprintf(CRITICAL, "PrimaryGPT starting LBA mismatch\n");
        ret = 1;
        return ret;
    }

    memcpy((void *)(gpt_header->sign), buffer, 8);
    memcpy((void *)(gpt_header->version), buffer + 8, 4);
    memcpy((void *)(gpt_header->guid), buffer + 56, 16);
    gpt_header->header_sz             = *header_size;
    gpt_header->current_lba           = current_lba;
    gpt_header->backup_lba            = backup_lba;
    gpt_header->first_usable_lba      = *first_usable_lba;
    gpt_header->last_usable_lba       = last_usable_lba;
    gpt_header->partition_entry_lba   =  partition_0;
    gpt_header->partition_entry_count = *max_partition_count;
    gpt_header->partition_entry_sz    = *partition_entry_size;
    gpt_header->header_crc32          = crc_val;

    return ret;
}

unsigned int parse_gpt_table_from_buffer(uint8_t *buf, uint32_t buf_len,
        GPT_header *gpt_header, uint32_t block_size, bool is_secondary_gpt)
{
    unsigned int ret;
    struct partition_entry *partition_entries = gpt_header->partition_entries;

    /* Allocate partition entries array */
    if (!partition_entries) {
        partition_entries = (struct partition_entry *) calloc(NUM_PARTITIONS,
                            sizeof(struct partition_entry));
        ASSERT(partition_entries);
    }
    else {
        memset(partition_entries, 0x0,
               NUM_PARTITIONS * sizeof(struct partition_entry));
        gpt_header->actual_entries_count = 0;
    }

    gpt_header->partition_entries = partition_entries;
    ret = parse_gpt(buf, buf_len, block_size, gpt_header, is_secondary_gpt);

    if (ret) {
        dprintf(CRITICAL, "GPT read failed!\n");
        return 1;
    }

    return 0;
}

static unsigned int parse_gpt(uint8_t *buffer, uint32_t buf_len,
                              uint32_t block_size,
                              GPT_header *gpt_header, bool is_secondary_gpt)
{
    int ret = 0;
    uint32_t crc_val = 0;
    uint32_t crc_val_org = 0;
    unsigned int header_size;
    unsigned long long first_usable_lba;
    unsigned int max_partition_count = 0;
    unsigned int partition_entry_size;
    unsigned int i = 0; /* Counter for each block */
    unsigned int j = 0; /* Counter for each entry in a block */
    unsigned int n = 0; /* Counter for UTF-16 -> 8 conversion */
    unsigned char UTF16_name[MAX_GPT_NAME_SIZE];
    /* LBA of first partition -- 1 Block after Protected MBR + 1 for PT */
    uint8_t *data = NULL;
    uint32_t part_entry_cnt = block_size / PARTITION_ENTRY_SIZE;
    struct partition_entry *partition_entries = NULL;
    unsigned char *new_buffer = NULL;
    uint32_t partition_count = 0;

    if (is_secondary_gpt) {
        data = buffer + (buf_len - block_size);
    }
    else {
        data = buffer;
    }

    ret = partition_parse_gpt_header(data, &first_usable_lba,
                                     &partition_entry_size, &header_size,
                                     &max_partition_count, gpt_header, block_size,
                                     is_secondary_gpt);

    if (ret) {
        dprintf(CRITICAL, "%s ret:%d\n", __func__, ret);
        return ret;
    }

    if (is_secondary_gpt) {
        new_buffer = buffer;
    }
    else {
        new_buffer = buffer + block_size;
    }

    crc_val_org = GET_LWORD_FROM_BYTE(&data[PARTITION_CRC_OFFSET]);
    crc_val  = crc32(0, new_buffer, max_partition_count * partition_entry_size);
    if (crc_val != crc_val_org) {
        dprintf(CRITICAL,"%s Partition entires crc mismatch crc_val= 0x%08x with crc_val_org= 0x%08x\n",__func__,
                crc_val,crc_val_org);
        return 1;
    }

    gpt_header->entry_array_crc32 = crc_val;
    partition_entries = gpt_header->partition_entries;

    /* Read GPT Entries */
    for (i = 0;
            i < (ROUNDUP(max_partition_count, part_entry_cnt)) / part_entry_cnt; i++) {
        ASSERT(partition_count < NUM_PARTITIONS);

        data = (new_buffer + (i * block_size));

        for (j = 0; j < part_entry_cnt; j++) {
            memcpy(&(partition_entries[partition_count].type_guid),
                   &data[(j * partition_entry_size)],
                   PARTITION_TYPE_GUID_SIZE);

            if (partition_entries[partition_count].type_guid[0] ==
                    0x00
                    && partition_entries[partition_count].
                    type_guid[1] == 0x00) {
                i = ROUNDUP(max_partition_count, part_entry_cnt);
                break;
            }

            memcpy(&(partition_entries[partition_count].unique_partition_guid),
                   &data[(j * partition_entry_size) + UNIQUE_GUID_OFFSET],
                   UNIQUE_PARTITION_GUID_SIZE);

            partition_entries[partition_count].first_lba =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) +
                                                                      FIRST_LBA_OFFSET]);

            partition_entries[partition_count].last_lba =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) +
                                                                      LAST_LBA_OFFSET]);

            partition_entries[partition_count].size =
                partition_entries[partition_count].last_lba -
                partition_entries[partition_count].first_lba + 1;

            partition_entries[partition_count].attribute_flag =
                GET_LLWORD_FROM_BYTE(&data[(j * partition_entry_size) +
                                                                      ATTRIBUTE_FLAG_OFFSET]);

            memset(&UTF16_name, 0x00, MAX_GPT_NAME_SIZE);
            memcpy(UTF16_name, &data[(j * partition_entry_size) +
                                                                PARTITION_NAME_OFFSET], MAX_GPT_NAME_SIZE);

            /*
             * Currently partition names in *.xml are UTF-8 and lowercase
             * Only supporting english for now so removing 2nd byte of UTF-16
             */
            for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++) {
                partition_entries[partition_count].name[n] =
                    UTF16_name[n * 2];
            }

            partition_count++;
        }
    }

    gpt_header->actual_entries_count = partition_count;
    return ret;
}

/*
 * Find index of parition in array of partition entries
 */
int get_partition_index_from_header(const char *name,
                                    GPT_header *gpt_header)
{
    unsigned int input_string_length = strlen(name);
    unsigned n;
    char *curr_suffix = NULL;
    struct partition_entry *partition_entries = gpt_header->partition_entries;

    /*  We iterate through the parition entries list,
        to find the partition with active slot suffix.
    */
    for (n = 0; n < gpt_header->partition_entry_count; n++) {
        if (!strncmp((const char *)name, (const char *)partition_entries[n].name,
                     input_string_length)) {
            curr_suffix = (char *)(partition_entries[n].name + input_string_length);

            /* if partition_entries.name is NULL terminated return the index */
            if (*curr_suffix == '\0')
                return n;
        }
    }

    return INVALID_PTN;
}

/* Get size of the partition */
unsigned long long get_partition_size_from_header(int index,
        GPT_header *gpt_header, uint32_t block_size)
{
    struct partition_entry *partition_entries = gpt_header->partition_entries;

    if (index == INVALID_PTN)
        return 0;
    else {
        return partition_entries[index].size * block_size;
    }
}

/* Get offset of the partition */
unsigned long long get_partition_offset_from_header(int index,
        GPT_header *gpt_header, uint32_t block_size)
{
    struct partition_entry *partition_entries = gpt_header->partition_entries;

    if (index == INVALID_PTN)
        return 0;
    else {
        return partition_entries[index].first_lba * block_size;
    }
}

struct partition_info get_partition_info_from_header(const char *name,
        GPT_header *gpt_header, uint32_t block_size)
{
    struct partition_info info = {0};

    int index = INVALID_PTN;

    if (!name) {
        dprintf(CRITICAL, "Invalid partition name passed\n");
        goto out;
    }

    index = get_partition_index_from_header(name, gpt_header);

    if (index != INVALID_PTN) {
        info.offset = get_partition_offset_from_header(index, gpt_header,
                      block_size);
        info.size   = get_partition_size_from_header(index, gpt_header,
                      block_size);
    }
    else {
        dprintf(CRITICAL, "Error unable to find partition : [%s]\n", name);
    }

out:
    return info;
}

uint32_t gpt_partition_round(uint8_t *buffer, uint32_t buf_len,
                             uint32_t block_size, uint32_t sector_sz, uint64_t capacity)
{
    uint32_t ret = 0;
    uint32_t offset = 0;
    uint32_t gpt_sz = 0;
    uint32_t crc_val = 0;
    uint64_t size_in_lba = 0;
    uint64_t patch_size = 0;
    uint8_t *primary_header = NULL;
    uint8_t *secondary_header = NULL;
    GPT_header gpt_header_pri = {0};
    GPT_header gpt_header_bak = {0};
    uint64_t last_entry_last_lba = 0;
    uint64_t partition_entry_size = 0;
    uint64_t max_partition_count = 0;
    uint32_t blocks_for_entries  = 0;
    struct partition_entry *entry  = NULL;
    uint32_t partition_entry_array_size = 0;
    uint32_t gpt_in_sector;

    blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
    gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries) * block_size;

    if (buf_len < GPT_HEADER_SIZE * 2 + MIN_PARTITION_ARRAY_SIZE * 2
            || buf_len < gpt_sz * 2) {
        dprintf(CRITICAL, "%s ptb buffer len:%u error!\n", __func__, buf_len);
        return 1;
    }

    if (!capacity || capacity % block_size != 0) {
        dprintf(CRITICAL, "%s capacity:%llu error, block_size:%u!\n", __func__, capacity, block_size);
        return 1;
    }

    if (!sector_sz || sector_sz % block_size != 0) {
        dprintf(CRITICAL, "%s sector_sz:%u error, block_size:%u!\n", __func__, sector_sz, block_size);
        return 1;
    }
    ret = parse_gpt_table_from_buffer(buffer, buf_len - gpt_sz, &gpt_header_pri, block_size, false);
    ret |= parse_gpt_table_from_buffer(buffer + gpt_sz, buf_len - gpt_sz, &gpt_header_bak, block_size, true);

    if (ret) {
        dprintf(CRITICAL, "%s ptb check fail!\n", __func__);
        return 2;
    }

    gpt_in_sector = round_up(gpt_sz, sector_sz);
    max_partition_count =
        GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
    partition_entry_size =
        GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);

    partition_entry_array_size = partition_entry_size * max_partition_count;
    if (partition_entry_array_size < MIN_PARTITION_ARRAY_SIZE) {
        partition_entry_array_size = MIN_PARTITION_ARRAY_SIZE;
    }
    offset = partition_entry_array_size * 2;

    primary_header = buffer;
    secondary_header = buffer + block_size + offset;

    for (uint32_t i= 0; i < gpt_header_pri.actual_entries_count; i++) {
        entry = &gpt_header_pri.partition_entries[i];

        dprintf(CRITICAL, "%s %d first_lba:%llu last_lba:%llu\n",
                __func__, __LINE__, entry->first_lba, entry->last_lba);

        size_in_lba = entry->last_lba - entry->first_lba + 1;
        if (entry->first_lba <= last_entry_last_lba)
        {
            entry->first_lba = last_entry_last_lba + 1;
        }

        if ((entry->first_lba * block_size) % sector_sz !=0 ) {

            entry->first_lba = round_up(entry->first_lba * block_size, sector_sz) / block_size;
        }

        PUT_LONG_LONG(buffer + block_size + i * partition_entry_size +
                      FIRST_LBA_OFFSET, entry->first_lba);
        PUT_LONG_LONG(buffer + block_size + i * partition_entry_size +
                      partition_entry_array_size + FIRST_LBA_OFFSET, entry->first_lba);

        entry->last_lba = size_in_lba + entry->first_lba - 1;

        patch_size = (size_in_lba * block_size) % sector_sz;
        if (patch_size != 0)
        {
            patch_size = sector_sz - patch_size;
            entry->last_lba += patch_size / block_size;
        }

        if ( i == gpt_header_pri.actual_entries_count - 1) {

            /* If it is the last partition,
             * round_down its size,
             * because secondary gpt header should not be in the same sector with the last partition */
            entry->last_lba = (capacity - gpt_in_sector)/ block_size - 1;

            PUT_LONG_LONG(primary_header + LAST_USABLE_LBA_OFFSET, entry->last_lba);
            PUT_LONG_LONG(secondary_header + LAST_USABLE_LBA_OFFSET, entry->last_lba);
            if((entry->last_lba >= entry->first_lba)
                && (entry->last_lba - entry->first_lba < size_in_lba - 1))
            {
                dprintf(CRITICAL, "%s shrink the last entry!\n", __func__);
            }
        }

        if (entry->last_lba <= entry->first_lba) {
            dprintf(CRITICAL, "%s partition size error: first lba:%llu last_lba:%llu!\n", __func__,
                    entry->first_lba, entry->last_lba);
            return 3;
        }

        PUT_LONG_LONG(buffer + block_size + i * partition_entry_size +
                      LAST_LBA_OFFSET, entry->last_lba);
        PUT_LONG_LONG(buffer + block_size + i * partition_entry_size +
                      partition_entry_array_size + LAST_LBA_OFFSET, entry->last_lba);


        last_entry_last_lba = entry->last_lba;
        dprintf(CRITICAL, "%s %d after patch first_lba:%llu last_lba:%llu\n",
                __func__, __LINE__, entry->first_lba, entry->last_lba);
    }

    crc_val = crc32(0, primary_header + block_size, partition_entry_size * max_partition_count);
    PUT_LONG(primary_header + PARTITION_CRC_OFFSET, crc_val);
    dprintf(INFO, "%s %d primary partition entries crc:0x%0x!\n", __func__, __LINE__, crc_val);

    crc_val = crc32(0, primary_header + block_size + partition_entry_array_size,
                    partition_entry_size * max_partition_count);
    PUT_LONG(secondary_header + PARTITION_CRC_OFFSET, crc_val);
    dprintf(INFO, "%s %d secondary partition entries crc:0x%0x!\n", __func__, __LINE__, crc_val);

    PUT_LONG(primary_header + HEADER_CRC_OFFSET, 0);
    crc_val = crc32(0, primary_header, GPT_HEADER_SIZE);
    PUT_LONG(primary_header + HEADER_CRC_OFFSET, crc_val);
    dprintf(INFO, "%s %d primary header crc:0x%0x!\n", __func__, __LINE__, crc_val);

    PUT_LONG(secondary_header + HEADER_CRC_OFFSET, 0);
    crc_val = crc32(0, secondary_header, GPT_HEADER_SIZE);
    PUT_LONG(secondary_header + HEADER_CRC_OFFSET, crc_val);
    dprintf(INFO, "%s %d secondary header crc:0x%0x!\n", __func__, __LINE__, crc_val);

    if (gpt_header_pri.partition_entries) {
        free(gpt_header_pri.partition_entries);
    }

    if (gpt_header_bak.partition_entries) {
        free(gpt_header_bak.partition_entries);
    }
    return ret;
}

