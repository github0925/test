/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __STORAGE_DEV_OSPI_H__
#define __STORAGE_DEV_OSPI_H__

#include <stdint.h>
#include "storage_device.h"

int ospi_init(storage_device_t *storage_dev, uint32_t res_idx,
              void *config);
int ospi_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
              uint64_t size);
int ospi_write(storage_device_t *storage_dev, uint64_t dst,
               const uint8_t *buf, uint64_t data_len);
int ospi_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len);
uint64_t ospi_get_capacity(storage_device_t *storage_dev);
uint32_t ospi_get_block_size(storage_device_t *storage_dev);
bool ospi_need_erase(storage_device_t *storage_dev);
bool ospi_get_id (storage_device_t *storage_dev, uint8_t *buf, uint32_t len);
int ospi_release(storage_device_t *storage_dev);
uint32_t ospi_get_erase_group_size(storage_device_t *storage_dev);

#endif
