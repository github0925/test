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

#ifndef __SEMIDRIVE_PARSER_H
#define __SEMIDRIVE_PARSER_H
#include "partition_parser.h"

typedef struct {
    uint8_t  sign[8];
    uint8_t  version[4];
    uint32_t header_sz;
    uint32_t header_crc32;
    uint8_t  reserve[4];
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t  guid[16];
    uint64_t partition_entry_lba;
    uint32_t partition_entry_count;
    uint32_t partition_entry_sz;
    uint32_t entry_array_crc32;
    struct partition_entry *partition_entries;
    uint32_t actual_entries_count;
} GPT_header;

unsigned int parse_gpt_table_from_buffer(uint8_t *buf, uint32_t buf_len,
        GPT_header *gpt_header, uint32_t block_size, bool is_secondary_gpt);
int get_partition_index_from_header(const char *name,
                                    GPT_header *gpt_header);
unsigned long long get_partition_size_from_header(int index,
        GPT_header *gpt_header, uint32_t block_size);
unsigned long long get_partition_offset_from_header(int index,
        GPT_header *gpt_header, uint32_t block_size);
struct partition_info get_partition_info_from_header(const char *name,
        GPT_header *gpt_header, uint32_t block_size);
uint32_t gpt_partition_round(uint8_t *buffer, uint32_t buf_len,
                                uint32_t block_size, uint32_t sector_sz, uint64_t cap);
#endif
