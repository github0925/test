/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "update.h"
#include "sparse_parser.h"

typedef struct SPARSE_HEADER {
    uint32_t  magic;      /* 0xed26ff3a */
    uint16_t  major_version;  /* (0x1) - reject images with higher major versions */
    uint16_t  minor_version;  /* (0x0) - allow images with higer minor versions */
    uint16_t  file_hdr_sz;    /* 28 bytes for first revision of the file format */
    uint16_t  chunk_hdr_sz;   /* 12 bytes for first revision of the file format */
    uint32_t  blk_sz;     /* block size in bytes, must be a multiple of 4 (4096) */
    uint32_t  total_blks; /* total blocks in the non-sparse output image */
    uint32_t  total_chunks;   /* total chunks in the sparse input image */
    uint32_t  image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
    /* as 0. Standard 802.3 polynomial, use a Public Domain */
    /* table implementation */
} SPARSE_HEADER_T;

const int SPARSE_HEADER_T_SIZE = sizeof(SPARSE_HEADER_T);    // Length = 28
#define SPARSE_HEADER_MAGIC 0xed26ff3a

#define CHUNK_TYPE_RAW      0xCAC1
#define CHUNK_TYPE_FILL     0xCAC2
#define CHUNK_TYPE_DONT_CARE    0xCAC3
#define CHUNK_TYPE_CRC      0xCAC4

typedef struct CHUNK_HEADER {
    uint16_t  chunk_type; /* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
    uint16_t  reserved1;
    uint32_t  chunk_sz;   /* in blocks in output image */
    uint32_t  total_sz;   /* in bytes of chunk input file including chunk header and data */
} CHUNK_HEADER_T;

const int CHUNK_HEADER_T_SIZE = sizeof(CHUNK_HEADER_T);    // Length = 12
/* Following a Raw or Fill chunk is data.  For a Raw chunk, it's the data in chunk_sz * blk_sz.
 *  For a Fill chunk, it's 4 bytes of the fill data.
 */
 
uint64_t get_sparse_img_size(int type,unsigned long long in){
    int ret;
    SPARSE_HEADER_T sparse_header  = {0};
    uint8_t buffer[SPARSE_HEADER_T_SIZE];
    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in,SPARSE_HEADER_T_SIZE,buffer);
    if(ret != 0){
        PRINTF_CRITICAL("failed get_sparse_img_size for read_img_file_from error\n");
        return -1;
    }
    memcpy(&sparse_header, buffer, SPARSE_HEADER_T_SIZE);
    return ((uint64_t)sparse_header.total_blks * (uint64_t)sparse_header.blk_sz);
}

int is_sparse_img(int type,unsigned long long in){
    int ret;
    SPARSE_HEADER_T sparse_header  = {0};
    uint8_t buffer[SPARSE_HEADER_T_SIZE];
    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in,SPARSE_HEADER_T_SIZE,buffer);
    if(ret != 0){
        PRINTF_CRITICAL("failed is_sparse_img for read_img_file_from error\n");
        return 0;
    }
    memcpy(&sparse_header, buffer, SPARSE_HEADER_T_SIZE);
    return 0xed26ff3a == sparse_header.magic ? 1 : 0;
}

int flash_sparse_img(int type,unsigned long long in,unsigned long long out,unsigned long long size,char *full_ptname,storage_device_t *storage){
    int ret;
    unsigned long long in_pos = in;
    unsigned long long out_pos = out;
    uint8_t buffer[SPARSE_HEADER_T_SIZE];
    uint8_t buffer2[CHUNK_HEADER_T_SIZE];
    unsigned long long ss = 0x213d70;
    SPARSE_HEADER_T sparse_header  = {0};
    CHUNK_HEADER_T chunk_header    = {0};
    int chunk;
    uint64_t chunk_data_sz_remain  = 0;
    uint8_t *data;
    uint32_t total_blocks   = 0;
    uint64_t chunk_data_sz  = 0;
    uint8_t fill_data[4] = {0};
    int fill_value;
    int cnt;

    if(!full_ptname || !storage){
        PRINTF_CRITICAL("failed flash_sparse_img for full_ptname or storage error\n");
        return -1;
    }
    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in_pos,SPARSE_HEADER_T_SIZE,buffer);
    if(ret != 0){
        PRINTF_CRITICAL("failed flash_sparse_img for read_img_file_from error\n");
        return -1;
    }
    in_pos += SPARSE_HEADER_T_SIZE;
    memcpy(&sparse_header, buffer, SPARSE_HEADER_T_SIZE);
    if (!sparse_header.blk_sz || (sparse_header.blk_sz % 4)) {
        PRINTF_CRITICAL("block size error:%u\n", sparse_header.blk_sz);
        return -1;
    }
    PRINTF_CRITICAL("image too ss %lu large :%llu\n",((uint64_t)sparse_header.total_blks * (uint64_t)sparse_header.blk_sz), size);
    if (((uint64_t)sparse_header.total_blks * (uint64_t)sparse_header.blk_sz)
            > size) {
        PRINTF_CRITICAL("image too large :%llu\n", size);
        return -1;
    }

    if (sparse_header.file_hdr_sz != sizeof(SPARSE_HEADER_T)) {
        PRINTF_CRITICAL("image header error!\n");
        return -1;
    }

    PRINTF_INFO("flashing sparse image partition:%s", full_ptname);
    PRINTF_INFO("=== Sparse Image Header ===\n");
    PRINTF_INFO("in: 0x%llx\n", in);
    PRINTF_INFO("magic: 0x%x\n", sparse_header.magic);
    PRINTF_INFO("major_version: 0x%x\n", sparse_header.major_version);
    PRINTF_INFO("minor_version: 0x%x\n", sparse_header.minor_version);
    PRINTF_INFO("file_hdr_sz: %d\n", sparse_header.file_hdr_sz);
    PRINTF_INFO("chunk_hdr_sz: %d\n", sparse_header.chunk_hdr_sz);
    PRINTF_INFO("blk_sz: %d\n", sparse_header.blk_sz);
    PRINTF_INFO("total_blks: %d\n", sparse_header.total_blks);
    PRINTF_INFO("total_chunks: %d\n", sparse_header.total_chunks);

    /* Start processing chunks */
    for (chunk = 0; chunk < sparse_header.total_chunks; chunk++) {
        /* Make sure the total image size does not exceed the partition size */
        if (((uint64_t)total_blocks * (uint64_t)sparse_header.blk_sz) >= size) {
            PRINTF_CRITICAL("image too large:%llu!\n", size);
            return -1;
        }

        /* Read and skip over chunk header */
        ret = read_img_file_from(type == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in_pos,CHUNK_HEADER_T_SIZE,buffer2);
        if(ret != 0){
            PRINTF_CRITICAL("failed flash_sparse_img for read_img_file_from error\n");
            return -1;
        }
        memcpy(&chunk_header, buffer2, CHUNK_HEADER_T_SIZE);
        //in_pos += CHUNK_HEADER_T_SIZE;

        PRINTF_INFO("=== Chunk Header ===\n");
        PRINTF_INFO("chunk %d\n",chunk);
        PRINTF_INFO("in_pos: 0x%llx\n", in_pos-ss);
        PRINTF_INFO("chunk_type: 0x%x\n", chunk_header.chunk_type);
        PRINTF_INFO("chunk_sz: 0x%x\n", chunk_header.chunk_sz);
        PRINTF_INFO("total_size: 0x%x\n", chunk_header.total_sz);

        if (sparse_header.chunk_hdr_sz != CHUNK_HEADER_T_SIZE) {
            PRINTF_CRITICAL("chunk header error:%u!\n", sparse_header.chunk_hdr_sz);
            return -1;
        }

        chunk_data_sz = (uint64_t)sparse_header.blk_sz * chunk_header.chunk_sz;

        /* Make sure that the chunk size calculated from sparse image does not
         * exceed partition size
         */
        if ((uint64_t)total_blocks * (uint64_t)sparse_header.blk_sz +
                chunk_data_sz > size) {
            PRINTF_CRITICAL("chunk data too large:%llu!\n", size);
            return -1;
        }

        switch (chunk_header.chunk_type) {
            case CHUNK_TYPE_RAW:
                if ((uint64_t)chunk_header.total_sz != ((uint64_t)
                                                        sparse_header.chunk_hdr_sz +
                                                        chunk_data_sz)) {
                    PRINTF_CRITICAL("chunk size:%lu error!\n", chunk_data_sz);
                    return -1;
                }
                in_pos += CHUNK_HEADER_T_SIZE;
                chunk_data_sz_remain = chunk_data_sz;
                PRINTF_INFO("chunk_data_sz_remain: 0x%lx\n", chunk_data_sz_remain);
                cnt = chunk_data_sz_remain/MAX_CALLOC_SIZE;
                data = (unsigned char *)calloc(1, (cnt != 0 ? (MAX_CALLOC_SIZE) : chunk_data_sz_remain));
                while (chunk_data_sz_remain > MAX_CALLOC_SIZE) {
                    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in_pos,MAX_CALLOC_SIZE,data);
                    if(ret != 0){
                        PRINTF_CRITICAL("failed flash_sparse_img for read_img_file_from error\n");
                        free(data);
                        return -1;
                    }
                    if (storage->write(storage, out_pos, data, MAX_CALLOC_SIZE)) {
                        PRINTF_CRITICAL("flash storage error\n");
                        free(data);
                        return -1;
                    }
                    dump_update_progress(type,MAX_CALLOC_SIZE);
                    in_pos += MAX_CALLOC_SIZE;
                    out_pos += MAX_CALLOC_SIZE;
                    chunk_data_sz_remain -= MAX_CALLOC_SIZE;
                }

                if (chunk_data_sz_remain) {
                    ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in_pos,chunk_data_sz_remain,data);
                    if(ret != 0){
                        PRINTF_CRITICAL("failed flash_sparse_img for read_img_file_from error\n");
                        free(data);
                        return -1;
                    }
                    if (storage->write(storage, out_pos, data, chunk_data_sz_remain)) {
                        PRINTF_CRITICAL("flash storage error\n");
                        free(data);
                        return -1;
                    }
                    dump_update_progress(type,chunk_data_sz_remain);
                    in_pos += chunk_data_sz_remain;
                    out_pos += chunk_data_sz_remain;
                }
                free(data);
                //in_pos += chunk_header.total_sz;
                total_blocks += chunk_header.chunk_sz;
                break;
            case CHUNK_TYPE_FILL:
                chunk_data_sz_remain = chunk_data_sz;
                cnt = chunk_data_sz_remain/MAX_CALLOC_SIZE;
                data = (unsigned char *)calloc(1, (cnt != 0 ? (MAX_CALLOC_SIZE) : chunk_data_sz_remain));
                ret = read_img_file_from(type  == EMMC_TYPE ? GLOBAL_PAC_NAME : OSPI_PAC_NAME,in_pos+CHUNK_HEADER_T_SIZE,4,fill_data);
                if(ret != 0){
                    PRINTF_CRITICAL("failed flash_sparse_img for read_img_file_from error\n");
                    free(data);
                    return -1;
                }
                memcpy(&fill_value,fill_data,4);
                PRINTF_INFO("chunk_data_sz_remain: 0x%lx fill_value %d\n", chunk_data_sz_remain,fill_value);
                while (chunk_data_sz_remain > MAX_CALLOC_SIZE) {
                    memset(data,fill_value,MAX_CALLOC_SIZE);
                    if (storage->write(storage, out_pos, data, MAX_CALLOC_SIZE)) {
                        PRINTF_CRITICAL("flash storage error\n");
                        free(data);
                        return -1;
                    }
                    dump_update_progress(type,MAX_CALLOC_SIZE);
                    out_pos += MAX_CALLOC_SIZE;
                    chunk_data_sz_remain -= MAX_CALLOC_SIZE;
                }

                if (chunk_data_sz_remain) {
                    memset(data,fill_value,chunk_data_sz_remain);
                    if (storage->write(storage, out_pos, data, chunk_data_sz_remain)) {
                        PRINTF_CRITICAL("flash storage error\n");
                        free(data);
                        return -1;
                    }
                    dump_update_progress(type,chunk_data_sz_remain);
                    out_pos += chunk_data_sz_remain;
                }
                in_pos += chunk_header.total_sz;
                //out_pos += chunk_data_sz;
                break;
            case CHUNK_TYPE_DONT_CARE:
                in_pos += chunk_header.total_sz;
                total_blocks += chunk_header.chunk_sz;
                break;

            case CHUNK_TYPE_CRC:
                if (chunk_header.total_sz != sparse_header.chunk_hdr_sz) {
                    PRINTF_CRITICAL(" chunk total size:%u error!\n", chunk_header.total_sz);
                    return -1;
                }
                in_pos += chunk_header.total_sz;
                total_blocks += chunk_header.chunk_sz;
                break;

            default:
                PRINTF_CRITICAL("Unkown chunk type: %x\n", chunk_header.chunk_type);
                return -1;
        }
    }
    return 0;
}
