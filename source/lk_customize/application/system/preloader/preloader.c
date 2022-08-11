/*
* app_chain.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 09/26/2019 jianyong.lu create this file
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

#include "boot.h"
#include "boot_device_cfg.h"
#include "chip_res.h"
#include "mem_image.h"
#include "lib/reg.h"
#include "lib/reboot.h"
#include "lib/sdrv_common_reg.h"
#include "mmc_hal.h"
#include "partition_parser.h"
#include "ab_partition_parser.h"
#include "preloader.h"
#include "storage_device.h"
#include "preloader_configs.h"
#include "verified_boot.h"
#include "wdg_hal.h"

#ifndef BOOT_DEVICE
#define BOOT_DEVICE "emmc1"
#endif

#ifndef BOOT_DEVICE_GPT_START
#define BOOT_DEVICE_GPT_START 0x0
#endif

#define IMG_BACKUP_LOW_BASE   (p2v(p2ap(DIL_IMAGES_MEMBASE + 0x10000000)))
#define IMG_BACKUP_LOW_SZ     (DIL_IMAGES_MEMSIZE)

#ifdef AP2_IMAGES_MEMBASE
#define IMG_BACKUP_HIGH_BASE  (p2v(AP2_IMAGES_MEMBASE))
#define IMG_BACKUP_HIGH_SZ    (AP2_IMAGES_MEMSIZE)
#else
#define IMG_BACKUP_HIGH_BASE  (0)
#define IMG_BACKUP_HIGH_SZ    (0)
#endif

#define PRELOADER_CONFIGS_COUNT (sizeof(preloader_configs)/sizeof(preloader_configs[0]))
#define ARRAY_SIZE(array)  (sizeof array / sizeof(array[0]))

static struct list_node boot_args[CPU_ID_MAX];

/* get configs */
PT_LOAD_CONFIGS(preloader_configs);

static void boot_args_init(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(boot_args); i++) {
        list_initialize(&boot_args[i]);
    }
}

static struct list_node *get_head(sd_cpu_id cpu)
{
    if (cpu <= CPU_ID_MIN || cpu >= CPU_ID_MAX) {
        return NULL;
    }

    return &boot_args[cpu];
}

static uint32_t push_boot_arg(addr_t addr, size_t size, const char *name,
                              sd_cpu_id cpu)
{
    struct image_load_info *node = NULL;
    struct list_node *head = NULL;

    if (cpu <= CPU_ID_MIN || cpu >= CPU_ID_MAX) {
        dprintf  (CRITICAL, "%s cpu id error:%d\n", __func__, cpu);
        return 1;
    }

    node = image_info_node_new(addr, size, name);

    if (!node) {
        dprintf  (CRITICAL, "%s allocate memory fail!\n", __func__);
        return 1;
    }

    head = &boot_args[cpu];
    list_add_tail(head, &node->node);
    return 0;
}

static uint32_t boot_args_count(sd_cpu_id cpu)
{
    if (cpu <= CPU_ID_MIN || cpu >= CPU_ID_MAX) {
        dprintf  (CRITICAL, "%s cpu id error:%d\n", __func__, cpu);
        return 1;
    }

    return list_length(&boot_args[cpu]);
}

static struct image_load_info *pop_boot_arg(sd_cpu_id cpu)
{
    struct image_load_info *img_info = NULL;
    struct list_node *node = NULL;

    if (cpu <= CPU_ID_MIN || cpu >= CPU_ID_MAX) {
        dprintf  (CRITICAL, "%s cpu id error:%d\n", __func__, cpu);
        return 0;
    }

    node = list_remove_tail(&boot_args[cpu]);

    if (node) {
        img_info = containerof(node, struct image_load_info, node);
    }

    return img_info;
}

static addr_t pop_boot_addr(sd_cpu_id cpu)
{
    struct image_load_info *img_info = NULL;
    addr_t addr = 0;

    img_info = pop_boot_arg(cpu);

    if (img_info) {
        addr = img_info->addr;
        image_info_node_free(img_info);
    }

    return addr;
}

static int default_prepare(void *config, void *arg)
{
    mem_image_entry_t out_info;
    size_t seeker_size = IMG_BACKUP_LOW_SZ;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    struct pt_load_config *_config = config;

    if(_config->cpu_id == CPU_ID_CURRENT ||
       !(_config->flags & PT_BAK_MASK))
    {
        return 0;
    }

    if (PT_BAK_POS(_config->flags) == PT_BAK_HIGH_F)
    {
        seeker_base = IMG_BACKUP_HIGH_BASE;
        seeker_size = IMG_BACKUP_HIGH_SZ;
    }

    if (!seeker_base || !seeker_size)
    {
        dprintf(CRITICAL, "invalid memory image base or size, base:0x%lx, size:0x%lx\n", seeker_base, seeker_size);
        return 0;
    }

    mem_image_init(seeker_base, seeker_size);

    if (!mem_image_get_entry(seeker_base, CACHE_LINE, _config->load_size, _config->pt_name, &out_info))
    {
        dprintf(CRITICAL, "fail to get backup addr\n");
        return 1;
    }

    _config->load_addr = v2p((void*)(out_info.base));
    return 0;
}

static void trigger_wdt(void)
{
    bool ret;
    void *wdg_handle;
    wdg_app_config_t wdg_cfg={0};

    if (!hal_wdg_creat_handle(&wdg_handle, RES_WATCHDOG_WDT6)) {
        dprintf(CRITICAL, "fail to get wdg handle!\n");
        return;
    }

    wdg_cfg.workMode = wdg_mode1;

    ret = hal_wdg_init(wdg_handle, &wdg_cfg);

    if (ret) {
        reboot_args_t reboot_args;
        reboot_args.val = sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);
        reboot_args.args.reason = HALT_REASON_SW_RESET;
        sdrv_common_reg_set_u32(reboot_args.val, SDRV_REG_BOOTREASON);
        hal_wdg_set_timeout(wdg_handle, 1);
        hal_wdg_enable_interrupts(wdg_handle);
        hal_wdg_enable(wdg_handle);
    }

    hal_wdg_release_handle(wdg_handle);
}

static int default_complete(void *config, void *arg)
{
    uint32_t count = 0;
    uint64_t entry = 0;
    uint64_t arg1 = 0;
    uint64_t arg2 = 0;
    uint64_t arg3 = 0;
    sd_cpu_id cpu_id;
    struct pt_load_config *_config = config;

    cpu_id = _config->cpu_id;

    if (_config->flags & PT_SAVE_F) {
        push_boot_arg(_ioaddr(_config->load_addr), _config->load_size,
                      _config->pt_name, _config->cpu_id);
    }

    if (cpu_id == CPU_ID_CURRENT
        && (_config->flags & PT_KICK_F)) {

        count = boot_args_count(cpu_id);

        if (!count)
            return 0;

#if VERIFIED_BOOT
        partition_device_t *ptdev = arg;
        if (!verify_loaded_images(ptdev, get_head(cpu_id), NULL)) {
            dprintf(CRITICAL, "verify image fail!\n");
            return -1;
        }
#endif
        if (count == 1) {
            entry = pop_boot_addr(cpu_id);
            dprintf(CRITICAL, "ENTRY:0x%llx\n", entry);
            /* never return */
            arch_chain_load((void *)entry, 0, 0, 0, 0);
        }
        else if (count == 2) {
            arg3 = v2p((void *)pop_boot_addr(cpu_id));
            arg1 = v2p((void *)pop_boot_addr(cpu_id));
        }
        else if (count == 3) {
            arg3 = v2p((void *)pop_boot_addr(cpu_id));
            arg2 = v2p((void *)pop_boot_addr(cpu_id));
            arg1 = v2p((void *)pop_boot_addr(cpu_id));
        }

        dprintf(CRITICAL, "%s %d arg1:0x%0llx arg2:0x%0llx arg3:0x%0llx\n",
                __func__, __LINE__, arg1, arg2, arg3);
        arch_disable_ints();
        arch_disable_cache(UCACHE);
        smc(SMC_RUN_IMAGE_BL31, arg1, arg2, arg3, 0, 0, 0, 0);
    }else if (_config->flags & PT_KICK_F){
        trigger_wdt();
    }

    return 0;
}

/* builtin partitions complete */
static void set_default_action(void *arg)
{
    register_load_prepare_for_all(preloader_configs, PRELOADER_CONFIGS_COUNT,
                                   default_prepare, arg);

    register_load_complete_for_all(preloader_configs, PRELOADER_CONFIGS_COUNT,
                                   default_complete, arg);
}

static void enter_next_stage(void)
{
}

static void preloader_entry(const struct app_descriptor *app, void *args)
{
    partition_device_t *ptdev  = NULL;
    storage_device_t *storage  = NULL;
    boot_device_cfg_t *btdev_cfg = NULL;
    int ret = 0;

    dprintf(INFO, "%s!\n", __func__);

    uint32_t pin = boot_get_pin();
    btdev_pin_cfg_t *btdev_pin = find_btdev(pin);

    if (btdev_pin) {
        btdev_cfg =  btdev_pin->ap;
    }

    if (btdev_cfg) {
        storage = setup_storage_dev(btdev_cfg->device_type,
                                    btdev_cfg->res_idex, (void *)&btdev_cfg->cfg);
    }

    ASSERT(storage != NULL);

    ptdev = ptdev_setup(storage, BOOT_DEVICE_GPT_START);
    ASSERT(ptdev != NULL);
    ptdev_read_table(ptdev);

    /* Only roll back when boot pin is 0b0000 to avoid "a Repeat rollback" */
    if(BOOT_PIN_0==pin){
        ptdev_roll_back_check(ptdev);
    }

    boot_args_init();

    set_default_action((void *)ptdev);
    ret = load_all_partition(preloader_configs, PRELOADER_CONFIGS_COUNT,
                             ptdev);

    if (ret) {
        dprintf(CRITICAL, "%s load all partition error!\n", __func__);
    }

    enter_next_stage();
}

static int preloader_main(int argc, const cmd_args *argv)
{
    return 0;
}
// LK console cmd
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("preloader", "A55 preloader", (console_cmd)&preloader_main)
STATIC_COMMAND_END(preloader);
#endif

APP_START(preloader)
.flags = 0,
.entry = preloader_entry,
APP_END
