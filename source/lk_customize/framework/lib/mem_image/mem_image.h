/*
 * mem_image.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _IMG_SEEKER_H_
#define _IMG_SEEKER_H_
#include "sys/types.h"

#ifndef MAX_GPT_NAME_SIZE
#define MAX_GPT_NAME_SIZE 72
#endif

#define MAX_IMG_SEEKER_ENTRY_CNT 8
#define IMG_SEEKER_MAGIC  0x77737771 /* MIMG */

typedef struct mem_image_info {
    uint64_t base;
    uint64_t sz;
} mem_image_entry_t;

typedef struct mem_image_header {
    uint32_t magic;
    uint32_t version;
    uint64_t total_sz;
    uint32_t img_cnt;
    uint8_t  inited;
    uint8_t reserved[43];
} mem_image_header_t;

uint32_t mem_image_init(addr_t header, size_t);
addr_t mem_image_get_entry(addr_t header, uint64_t align, uint64_t size,
                           const char *name, mem_image_entry_t *out_info);

uint32_t mem_image_delete_entry(addr_t header, addr_t base);
uint32_t mem_image_seek(addr_t header, const char *name, mem_image_entry_t *info);

#endif
