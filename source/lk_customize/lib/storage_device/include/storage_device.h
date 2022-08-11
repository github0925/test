/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */
#ifndef __STORAGE_DEV_H__
#define __STORAGE_DEV_H__

#include <stdint.h>
#include <string.h>

enum storage_type {
    MMC,
    OSPI,
    MEMDISK,
    STORAGE_TYPE_NR
};

typedef struct storage_device storage_device_t;

struct storage_device {
    void *priv;
    int (*init) (storage_device_t *storage_dev, uint32_t res_idx,
                 void *config);
    int (*read) (storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
                 uint64_t size);
    int (*write) (storage_device_t *storage_dev, uint64_t dst,
                  const uint8_t *buf, uint64_t data_len);
    int (*erase) (storage_device_t *storage_dev, uint64_t dst, uint64_t len);
    int (*switch_part) (storage_device_t *storage_dev, uint32_t part);
    uint64_t (*get_capacity) (storage_device_t *storage_dev);
    uint32_t (*get_erase_group_size) (storage_device_t *storage_dev);
    uint32_t (*get_block_size) (storage_device_t *storage_dev);
    bool (*need_erase) (storage_device_t *storage_dev);
    bool (*get_storage_id) (storage_device_t *storage_dev, uint8_t *buf, uint32_t len);
    int (*release) (storage_device_t *storage_dev);
};

storage_device_t *setup_storage_dev(enum storage_type type,
                                    uint32_t res_idx, void *config);

unsigned int storage_dev_destroy(storage_device_t *storage);

static inline uint64_t round_up(uint64_t size, uint64_t aligned)
{
    uint64_t mod = 0;

    if (aligned == 0 || size < aligned)
        return aligned;

    /* Sometimes, 'aligned' is not equal to power of 2 */
    mod = size % aligned;

    size += mod ? aligned - mod : 0;
    return size;
}

static inline uint64_t round_down(uint64_t size, uint64_t aligned)
{
    uint64_t mod = 0;

    if (aligned == 0 || size < aligned)
        return 0;
    /* Sometimes, 'aligned' is not equal to power of 2 */
    mod = size % aligned;

    size -= mod;
    return size;
}

#endif /* __STORAGE_DEV_H__ */
