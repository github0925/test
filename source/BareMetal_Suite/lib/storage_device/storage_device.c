/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include "storage_device.h"
#include "storage_dev_ospi.h"
#include "storage_dev_mmc.h"

storage_device_t *setup_storage_dev(enum storage_type type,
                                    uint32_t res_idex, void *config)
{
    int ret;
    static storage_device_t storage_local;
    storage_device_t *storage;

    if (type > MEMDISK || type < MMC) {
        DBG( "Bad storage type\n");
        return NULL;
    }

#if 0
    storage = calloc(1, sizeof(storage_device_t));

    if (!storage) {
        DBG( "alloc memory for storage dev failed\n");
        return NULL;
    }

#endif
    storage = &storage_local;

    switch (type) {
#ifdef EMMC_BOOT

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
#else

        case OSPI:
            storage->init = &ospi_init;
            storage->read = &ospi_read;
            storage->read_ptr = &ospi_read_ptr;
            storage->write = &ospi_write;
            storage->erase = &ospi_erase;
            storage->get_capacity = &ospi_get_capacity;
            storage->get_block_size = &ospi_get_block_size;
            storage->get_erase_group_size = &ospi_get_erase_group_size;
            storage->release = &ospi_release;
            break;
#endif

        default:
            break;
    }

    ret = storage->init(storage, res_idex, config);

    if (ret) {
        DBG( "%s init failed\n",
             (type == OSPI) ? "ospi" : (type == MMC) ? "mmc" : "mem");
        return NULL;
    }

    return storage;
}
