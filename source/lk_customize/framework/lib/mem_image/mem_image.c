/*
 * mem_image.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <arch/ops.h>
#include <debug.h>
#include <shared/lk/macros.h>
#include <string.h>
#include "mem_image.h"

#define HEADER_ENTRY_INFO_SIZE (sizeof(mem_image_header_t) + MAX_IMG_SEEKER_ENTRY_CNT * sizeof(struct __mem_image_entry_info))

struct __mem_image_entry_info {
    mem_image_entry_t addr_info;
    char name[MAX_GPT_NAME_SIZE];
    uint8_t reserved[40];
};

uint32_t mem_image_init(addr_t base, size_t sz)
{
    mem_image_header_t *header = (mem_image_header_t *)base;
    uint32_t info_sz;

    info_sz = sizeof(mem_image_header_t);
    arch_invalidate_cache_range(base, info_sz);
    if ( header->magic == IMG_SEEKER_MAGIC
            && header->inited) {
        return 1;
    }

    memset(header, 0x0, info_sz);
    header->magic = IMG_SEEKER_MAGIC;
    header->inited = 1;
    header->total_sz = sz;
    arch_clean_invalidate_cache_range(base, info_sz);
    return 0;
}

static uint64_t calc_sz(struct __mem_image_entry_info *info, uint32_t count)
{
    uint64_t total_sz = 0;
    for (uint32_t i = 0; i < count; i++, info++) {
        total_sz += info->addr_info.sz;
    }

    return total_sz;
}

addr_t mem_image_get_entry(addr_t h, uint64_t align, uint64_t size,
                           const char *name, mem_image_entry_t *out_info)
{
    uint32_t name_len;
    addr_t addr = 0, addr_base;
    mem_image_entry_t temp;
    mem_image_header_t *header = (mem_image_header_t *)h;
    struct __mem_image_entry_info *info = (struct __mem_image_entry_info *)(h + sizeof(mem_image_header_t));

    arch_invalidate_cache_range(h, HEADER_ENTRY_INFO_SIZE);
    if (header->magic != IMG_SEEKER_MAGIC
            || !header->inited) {
        goto out;
    }

    name_len = strlen(name);
    if (header->img_cnt >= MAX_IMG_SEEKER_ENTRY_CNT ||
            name_len == 0 ||
            name_len > MAX_GPT_NAME_SIZE ||
            size > header->total_sz ||
            !out_info) {
        //INFO("%s %d img_cnt:%llu name:%llu size:%llu info:%p\n", __func__, __LINE__, header->img_cnt);
        goto out;
    }

    if (!mem_image_seek(h, name, &temp)) {
        goto out;
    }

    addr_base = sizeof(mem_image_header_t) + sizeof(struct __mem_image_entry_info) * MAX_IMG_SEEKER_ENTRY_CNT;

    if (header->img_cnt > 0) {
        addr_base += calc_sz(info, header->img_cnt);
        info += header->img_cnt;
    }

    size = ROUNDUP(size, align),
    addr += addr_base;
    addr = ROUNDUP(addr, align);

    if (addr +  size > h + header->total_sz) {
        addr = 0;
        goto out;
    }

    header->img_cnt++;
    memset((void *)info, 0x0, sizeof(struct __mem_image_entry_info));
    info->addr_info.base = addr;
    info->addr_info.sz = size;
    memcpy(info->name, name, name_len);
    out_info->base = addr + h;
    out_info->sz = size;
    arch_clean_invalidate_cache_range(h, HEADER_ENTRY_INFO_SIZE);
out:
    return addr;
}

uint32_t mem_image_delete_entry(addr_t h, addr_t base)
{
    mem_image_header_t *header = (mem_image_header_t *)h;
    struct __mem_image_entry_info *info = (struct __mem_image_entry_info *)(h + sizeof(mem_image_header_t));

    arch_invalidate_cache_range(h, HEADER_ENTRY_INFO_SIZE);
    if (header->magic != IMG_SEEKER_MAGIC
            || !header->inited) {
        return 1;
    }

    for (uint32_t i = 0; i < header->img_cnt; i++, info++) {
        if (info->addr_info.base == base && info->addr_info.base + info->addr_info.sz <= (h + header->total_sz)) {
            memset(info, 0x0, sizeof(struct __mem_image_entry_info));
            header->img_cnt--;
            arch_clean_invalidate_cache_range(h, HEADER_ENTRY_INFO_SIZE);
            return 0;
        }
    }

    return 1;
}

uint32_t mem_image_seek(addr_t h, const char *name, mem_image_entry_t *info)
{
    uint32_t name_len;
    mem_image_header_t *header = (mem_image_header_t *)h;
    struct __mem_image_entry_info *__info = (struct __mem_image_entry_info *)(h + sizeof(mem_image_header_t));

    arch_invalidate_cache_range(h, HEADER_ENTRY_INFO_SIZE);
    name_len = strlen(name);
    if (header->magic != IMG_SEEKER_MAGIC
            || !header->inited) {
        goto out;
    }

    if (header->img_cnt > MAX_IMG_SEEKER_ENTRY_CNT || name_len == 0 || name_len > MAX_GPT_NAME_SIZE) {
        goto out;
    }

    for (uint32_t i = 0; i < header->img_cnt; i++, __info++) {
        if (!strncmp(__info->name, name, strlen(name))) {
            info->base = __info->addr_info.base + h;
            info->sz = __info->addr_info.sz;
            return 0;
        }
    }
out:
    return 1;
}


