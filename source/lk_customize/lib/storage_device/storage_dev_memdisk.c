/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdint.h>
#include <lib/bio.h>
#include "storage_dev_memdisk.h"
#include <lib/reg.h>
#include <arch/ops.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

const char *memdisk_name = "storage_memdisk";

int memdisk_init(storage_device_t *storage_dev, uint32_t res_idx,
                 void *config)
{
    size_t len;
    void *ptr;

    bdev_t *memdisk_dev = storage_dev->priv;

    if (!memdisk_dev) {
        len = (size_t)MEMDISK_SIZE;
        ptr = (void *)_ioaddr((addr_t)MEMDISK_BASE);
        create_membdev(memdisk_name, ptr, len);
        memdisk_dev = bio_open(memdisk_name);
        storage_dev->priv = memdisk_dev;
    }

    if (memdisk_dev) {
        printf("\t%s, size %lld, bsize %zd, ref %d\n",
               memdisk_dev->name, memdisk_dev->total_size, memdisk_dev->block_size,
               memdisk_dev->ref);
    }

    return 0;
}

int memdisk_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
                 uint64_t size)
{
    if (storage_dev->priv) {
        bio_read(storage_dev->priv, dst, src, size);
        return 0;
    }
    else {
        return -1;
    }
}

int memdisk_write(storage_device_t *storage_dev, uint64_t dst,
                  const uint8_t *buf, uint64_t data_len)
{
    if (storage_dev->priv) {
        if (data_len / 512) {
            return bio_write_block(storage_dev->priv, buf, dst / 512, data_len / 512);
        }
        else {
            return bio_write(storage_dev->priv, buf, dst, data_len);
        }
        arch_clean_cache_range((addr_t)buf, data_len);
    }
    else {
        return 0;
    }
}

int memdisk_erase(storage_device_t *storage_dev, uint64_t dst,
                  uint64_t len)
{
    if (storage_dev->priv) {
        bio_erase(storage_dev->priv, dst, len);
        return 0;
    }

    return -1;
}

uint64_t memdisk_get_capacity(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        return MEMDISK_SIZE;
    }
    else {
        return 0;
    }
}

uint32_t memdisk_get_block_size(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        return 512;
    }
    else {
        return 0;
    }
}

int memdisk_release(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        bio_close(storage_dev->priv);
    }

    return 0;
}
