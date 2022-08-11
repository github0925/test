/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <mutex.h>
#include <spinlock.h>
#include "storage_device.h"
#include "storage_dev_mmc.h"
#include "storage_dev_ospi.h"
#include "storage_dev_mem.h"

#define DEFAULT_STORAGE_MAX_NUM (32)

static storage_ctrl_t storage_ctrl_array[DEFAULT_STORAGE_MAX_NUM] = {0};

spin_lock_t storage_spin_lock = SPIN_LOCK_INITIAL_VALUE;
mutex_t storage_mutex_lock = 0;

static inline int get_storage_ctrl(uint32_t id)
{
    int blank_index = -1;

    for (int i = 0; i < DEFAULT_STORAGE_MAX_NUM; i++) {
        if (storage_ctrl_array[i].id == id)
            return i;
        if (blank_index == -1) {
            if (storage_ctrl_array[i].handle == NULL) {
                blank_index = i;
            }
        }
    }

    return blank_index;
}

storage_device_t *setup_storage_dev(enum storage_type type,
                                    uint32_t res_idex, void *config)
{
    storage_device_t *storage;
    spin_lock_saved_state_t states;
    int ret = 0;
    int ctrl_index;

    if (type > MEMDISK || type < MMC) {
        dprintf(CRITICAL, "Bad storage type\n");
        return NULL;
    }

    spin_lock_irqsave(&storage_spin_lock, states);
    if (storage_mutex_lock == 0) {
        mutex_init(&storage_mutex_lock);
    }
    spin_lock_irqsave(&storage_spin_lock, states);

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
            storage->cached_write = &ospi_cached_write;
            storage->erase = &ospi_erase;
            storage->need_erase = &ospi_need_erase;
            storage->get_capacity = &ospi_get_capacity;
            storage->get_block_size = &ospi_get_block_size;
            storage->get_erase_group_size = &ospi_get_erase_group_size;
            storage->release = &ospi_release;
            break;

        case MEMDISK:
            storage->init = &mem_init;
            storage->read = &mem_read;
            storage->get_capacity = &mem_get_capacity;
            storage->get_block_size = &mem_get_block_size;
            storage->release = &mem_release;
            break;

        default:
            break;
    }

    mutex_acquire(&storage_mutex_lock);
    ctrl_index = get_storage_ctrl(res_idex);

    if (ctrl_index >= 0) {
        if (storage_ctrl_array[ctrl_index].handle != NULL) {
                storage->priv = storage_ctrl_array[ctrl_index].handle;
                storage_ctrl_array[ctrl_index].count += 1u;
        } else {
            ret = storage->init(storage, res_idex, config);
            if (ret == 0) {
                storage_ctrl_array[ctrl_index].count = 1u;
                storage_ctrl_array[ctrl_index].id = res_idex;
                storage_ctrl_array[ctrl_index].handle = storage->priv;
            }
        }
    } else {
        ret = -1;
    }
    mutex_release(&storage_mutex_lock);

    if (ret) {
        dprintf(CRITICAL, "%s init failed\n", (type == OSPI) ? "ospi": (type == MMC) ? "mmc" : "mem");
        free(storage);
        return NULL;
    }

    storage->ctrl = &storage_ctrl_array[ctrl_index];
    return storage;
}

unsigned int storage_dev_destroy(storage_device_t *storage)
{

    if (storage)
    {
        mutex_acquire(&storage_mutex_lock);
        storage->ctrl->count -= 1u;

        if (storage->ctrl->count == 0u) {
            if (storage->release)
                storage->release(storage);

            storage->ctrl->id = 0u;
            storage->ctrl->handle = NULL;
        }

        mutex_release(&storage_mutex_lock);

        free(storage);
        return 0;
    }
    return -1;
}
