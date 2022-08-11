/*
 * storage_dev_mem.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */

#include <debug.h>
#include "res.h"
#include "storage_dev_mem.h"


#define MAX_MEM_MAP_SIZE    (64*1024*1024)

typedef struct memdisk_info {
    addr_t   start;
    uint32_t block_size;
    uint64_t capacity;
} memdisk_info;

static memdisk_info mem_info;

int mem_init(storage_device_t *storage_dev, uint32_t res_idx,
             void *config)
{

    (void)config;

    int32_t dummy;

    mem_info.capacity = MAX_MEM_MAP_SIZE;
    mem_info.block_size = 512;

    if (!res_idx || res_get_info_by_id(res_idx, &(mem_info.start), &dummy) < 0) {
        dprintf(CRITICAL, "use MEMDISK_BASE for memdisk storage!\n");
        mem_info.start = MEMDISK_BASE;
        mem_info.capacity = MEMDISK_SIZE;
    }

    storage_dev->priv = (void *)&mem_info;

    return 0;
}

int mem_release(storage_device_t *storage_dev)
{
    storage_dev->priv = NULL;
    return 0;
}

uint32_t mem_get_block_size(storage_device_t *storage_dev)
{
    dprintf(INFO, "return virtual block size\n");
    memdisk_info *info = storage_dev->priv;

    if (info) {
        return info->block_size;
    }

    return 0;
}

int mem_read(storage_device_t *storage_dev, uint64_t offset, uint8_t *dst,
             uint64_t size)
{
    void *from = NULL;
    memdisk_info *info = storage_dev->priv;

    if (!storage_dev->priv) {
        dprintf(CRITICAL, "Do mem init firstly\n");
        goto fail;
    }

    from = (uint8_t *)info->start + offset;

    if ( offset >= info->capacity || (offset + size) > info->capacity) {
        dprintf(CRITICAL,
                "mem offset overflow, offset:0x%llx size:0x%llx capacity:0x%llx!\n", offset,
                size,
                info->capacity);
        goto fail;
    }

    memcpy(dst, from, size);

    return 0;

fail:
    return -1;
}

uint64_t mem_get_capacity(storage_device_t *storage_dev)
{
    memdisk_info *info = storage_dev->priv;

    if (info) {
        return info->capacity;
    }

    return 0;
}
