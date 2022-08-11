/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __STORAGE_DEV_MEMDISK_H__
#define __STORAGE_DEV_MEMDISK_H__

#include <stdint.h>
#include "storage_device.h"

int memdisk_init(storage_device_t *storage_dev, uint32_t res_id,
                 void *config);
int memdisk_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
                 uint64_t size);
int memdisk_write(storage_device_t *storage_dev, uint64_t dst,
                  const uint8_t *buf, uint64_t data_len);
int memdisk_erase(storage_device_t *storage_dev, uint64_t dst,
                  uint64_t len);
uint64_t memdisk_get_capacity(storage_device_t *storage_dev);
uint32_t memdisk_get_block_size(storage_device_t *storage_dev);
int memdisk_release(storage_device_t *storage_dev);

#endif
