/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */
#ifndef __STORAGE_DEV_H__
#define __STORAGE_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

enum storage_type {
    MMC,
    OSPI,
    STORAGE_TYPE_NR
};

typedef enum storage_id {
    DEV_EMMC0,
    DEV_EMMC1,
    DEV_EMMC2,
    DEV_EMMC3,
    DEV_SPI_NOR0,
    DEV_SPI_NOR1,
    DEV_MAX
} storage_id_e;

typedef struct storage_device storage_device_t;

typedef struct storage_dec {
    storage_id_e id;
    uint64_t gpt_offset;
    int type;
    char dev_name[50];
    char rpmsg_name[50];
} storage_dec;

struct storage_device {
    const char *dev_name;
    int type;
    int dev_fd;
    uint64_t gpt_offset;
    int (*init) (storage_device_t *storage_dev, char const *dev_name,
                 char const *rpmsg_name);
    int (*read) (storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
                 uint64_t size);
    int (*write) (storage_device_t *storage_dev, uint64_t dst,
                  const uint8_t *buf, uint64_t data_len);
    int (*erase) (storage_device_t *storage_dev, uint64_t dst, uint64_t len);
    int (*switch_part) (storage_device_t *storage_dev, uint32_t part);
    uint64_t (*get_capacity) (storage_device_t *storage_dev);
    uint32_t (*get_erase_group_size) (storage_device_t *storage_dev);
    uint32_t (*get_block_size) (storage_device_t *storage_dev);
    int (*need_erase) (storage_device_t *storage_dev);
    int (*release) (storage_device_t *storage_dev);
    int (*copy) (storage_device_t *storage_dev, uint64_t src, uint64_t dst,
                 uint64_t size);
};


extern storage_device_t *setup_storage_dev(storage_id_e dev);
extern unsigned int storage_dev_destroy(storage_device_t *storage_dev);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STORAGE_DEV_H__ */
