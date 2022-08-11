/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __STORAGE_DEV_EMMC_H__
#define __STORAGE_DEV_EMMC_H__

#include <stdint.h>
#include "storage_device.h"

int mmc_init(storage_device_t *storage_dev, uint32_t res_idx,
             void *config);
int mmc_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
             uint64_t size);
int mmc_write(storage_device_t *storage_dev, uint64_t dst,
              const uint8_t *buf, uint64_t data_len);
int mmc_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len);
int mmc_switch_part(storage_device_t *storage_dev, uint32_t part);
uint64_t mmc_get_capacity(storage_device_t *storage_dev);
uint32_t mmc_get_block_size(storage_device_t *storage_dev);
int mmc_release(storage_device_t *storage_dev);
uint32_t mmc_get_erase_group_size(storage_device_t *storage_dev);

#endif
