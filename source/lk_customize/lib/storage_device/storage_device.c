/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include "storage_device.h"
#include "storage_dev_mmc.h"
#include "storage_dev_ospi.h"
#include "storage_dev_memdisk.h"

storage_device_t *setup_storage_dev(enum storage_type type,
                                    uint32_t res_idex, void *config)
{
    storage_device_t *storage;
    int ret;

    if (type > MEMDISK || type < MMC) {
        dprintf(CRITICAL, "Bad storage type\n");
        return NULL;
    }

    storage = calloc(1, sizeof(storage_device_t));

    if (!storage) {
        dprintf(CRITICAL, "alloc memory for storage dev failed\n");
        return NULL;
    }

    switch (type) {
        case MMC:
            storage->init = &mmc_init;
            storage->read = &mmc_read;
            storage->write = &mmc_write;
            storage->erase = &mmc_erase;
            storage->switch_part = &mmc_switch_part;
            storage->get_capacity = &mmc_get_capacity;
            storage->get_erase_group_size = &mmc_get_erase_group_size;
            storage->get_block_size = &mmc_get_block_size;
            storage->release = &mmc_release;
            break;

        case OSPI:
            storage->init = &ospi_init;
            storage->read = &ospi_read;
            storage->write = &ospi_write;
            storage->erase = &ospi_erase;
            storage->need_erase = &ospi_need_erase;
            storage->get_storage_id = &ospi_get_id;
            storage->get_capacity = &ospi_get_capacity;
            storage->get_block_size = &ospi_get_block_size;
            storage->get_erase_group_size = &ospi_get_erase_group_size;
            storage->release = &ospi_release;
            break;

        case MEMDISK:
            storage->init = &memdisk_init;
            storage->read = &memdisk_read;
            storage->write = &memdisk_write;
            storage->erase = &memdisk_erase;
            storage->get_capacity = &memdisk_get_capacity;
            storage->get_block_size = &memdisk_get_block_size;
            storage->release = &memdisk_release;
            break;

        default:
            break;
    }

    ret = storage->init(storage, res_idex, config);

    if (ret) {
        dprintf(CRITICAL, "%s init failed\n",
                (type == OSPI) ? "ospi" : (type == MMC) ? "mmc" : "mem");
        free(storage);
        return NULL;
    }

    return storage;
}

unsigned int storage_dev_destroy(storage_device_t *storage)
{
    if (storage) {
        if (storage->release)
            storage->release(storage);

        free(storage);
        return 0;
    }

    return -1;
}
