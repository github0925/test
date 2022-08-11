/*
* cluster_preloader.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 12/03/2021 jianyong.lu create this file
*/
#include <arch.h>
#include <app.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <lib/console.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>

#include "mem_image.h"
#include "lib/reg.h"
#include "storage_device.h"
#include "partition_load_configs.h"
#include "preloader_configs.h"
#include "verified_boot.h"
#include "wdg_hal.h"

#ifdef AP2_IMAGES_MEMBASE
#define IMG_BACKUP_HIGH_BASE  (p2v(AP2_IMAGES_MEMBASE))
#define IMG_BACKUP_HIGH_SZ    (AP2_IMAGES_MEMSIZE)
#else
#define IMG_BACKUP_HIGH_BASE  (0)
#define IMG_BACKUP_HIGH_SZ    (0)
#endif

#define PARTITION_NAME_CLUSTER_ATF "cluster_atf"
#define PARTITION_NAME_CLUSTER_TOS "cluster_tos"
#define PARTITION_NAME_CLUSTER_BOOTLOADER "cluster_bootloader"

#define PRELOADER_CONFIGS_COUNT (sizeof(preloader_configs)/sizeof(preloader_configs[0]))
#define ARRAY_SIZE(array)  (sizeof(array) / sizeof(array[0]))

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);

PT_LOAD_CONFIGS_START(preloader_configs)
CONFIGS_AP2
PT_LOAD_CONFIGS_END

static void preloader_entry(const struct app_descriptor *app, void *args)
{
    addr_t entry;
    mem_image_entry_t out_info;
    size_t seeker_size = IMG_BACKUP_HIGH_SZ;
    addr_t seeker_base = IMG_BACKUP_HIGH_BASE;
    struct pt_load_config *config = preloader_configs;
#if VERIFIED_BOOT
    bool ret;
    uint8_t *vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE + 0x10000000);
    uint32_t vbmeta_sz = VBMETA_MEMSIZE;
    struct list_node verified_images_list = LIST_INITIAL_VALUE(
            verified_images_list);
#endif
#if TARGET_REFERENCE_D9P
    uint64_t arg1 = 0;
    uint64_t arg2 = 0;
    uint64_t arg3 = 0;
    uint32_t count = 0;
#endif

    if (!seeker_base || !seeker_size) {
        ERROR("invalid memory image info, base:0x%lx size:0x%lx\n", seeker_base,
              seeker_size);
        return;
    }

    mem_image_init(seeker_base, seeker_size);

    for (uint32_t i = 0; i < PRELOADER_CONFIGS_COUNT; i++, config++) {
        if (mem_image_seek(seeker_base, config->pt_name, &out_info)) {
            ERROR("cann't find image:%s\n", config->pt_name);
            return;
        }

        if (config->load_size < out_info.sz) {
            ERROR("no enough memory for image:%s\n", config->pt_name);
            return;
        }

#if TARGET_REFERENCE_D9P
        if (!strcmp(PARTITION_NAME_CLUSTER_ATF, config->pt_name)) {
            arg1 = config->load_addr;
            count++;
        } else if (!strcmp(PARTITION_NAME_CLUSTER_TOS, config->pt_name)) {
            arg2 = config->load_addr;
            count++;
        } else if (!strcmp(PARTITION_NAME_CLUSTER_BOOTLOADER, config->pt_name)) {
            arg3 = config->load_addr;
            count++;
        }
#endif

        entry = p2v(config->load_addr);
        memcpy((void *)entry, (void *)(out_info.base), out_info.sz);

        if (config->flags & PT_KICK_F) {
#if VERIFIED_BOOT
            ret = add_verified_image_list(&verified_images_list,
                                          vbmeta_buf, vbmeta_sz,
                                          VBMETA_PARTITION_NAME);
            ASSERT(ret);

            ret = add_verified_image_list(&verified_images_list,
                                          (void *)entry, out_info.sz,
                                          config->pt_name);
            ASSERT(ret);

            if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
                dprintf(ALWAYS, "%s %d verify images fail\n", __func__, __LINE__);
                free_image_info_list(&verified_images_list);
                return;
            }
            free_image_info_list(&verified_images_list);
#endif
#if TARGET_REFERENCE_D9P
            dprintf(CRITICAL, "%s %d arg1:0x%0llx arg2:0x%0llx arg3:0x%0llx\n",
                    __func__, __LINE__, arg1, arg2, arg3);
            if (count > 1) {
                arch_disable_ints();
                arch_disable_cache(UCACHE);
                smc(SMC_RUN_IMAGE_BL31, arg1, arg2, arg3, 0, 0, 0, 0);
            } else {
                /* cache will be disabled and cleaned in arch_chain_load */
                arch_chain_load((void *)entry, 0, 0, 0, 0);
            }
#else
            /* cache will be disabled and cleaned in arch_chain_load */
            arch_chain_load((void *)entry, 0, 0, 0, 0);
#endif
        }
    }

    return;

}

APP_START(preloader)
.flags = 0,
.entry = preloader_entry,
APP_END
