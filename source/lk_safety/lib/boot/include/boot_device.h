/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef _BOOT_DEVICE_H_
#define _BOOT_DEVICE_H_

#include "boot.h"
#include "mmc_hal.h"
#include "spi_nor_hal.h"
#include "storage_device.h"

typedef struct boot_device_cfg{
    uint32_t device_type;
    uint32_t res_idex;
    char disk_name[20];
    char storage_type[8];
    union {
        struct mmc_cfg mmc_cfg;
        struct spi_nor_cfg ospi_cfg;
    } cfg;
} boot_device_cfg_t;

typedef struct boot_device_pin_cfg{
    uint32_t pin_val;
    boot_device_cfg_t *ap;
    boot_device_cfg_t *safety;
}btdev_pin_cfg_t;

extern btdev_pin_cfg_t btdev_pin_mapping[];

static inline btdev_pin_cfg_t* find_btdev(uint32_t pin)
{
    if (pin > BOOT_PIN_15)
        return NULL;
    return &btdev_pin_mapping[pin];
};
#endif
