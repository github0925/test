/*
* sms_storage.h
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* -----------------
*/
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include <app.h>
#include <malloc.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>

#include "fuse_ctrl.h"
#include "hal_port.h"
#include "mmc_hal.h"
#include "spi_nor_hal.h"
#include "libavb.h"
#include "partition_parser.h"
#include "storage_device.h"

#define OSPI1_STORAGE_NAME   "ospi1"
#define NOR_FLASH_PTB_SECTOR_INDEX 2

#define SDRV_FOOTER     "SDRVFKFT"
#define SDRV_FOOTER_LEN 8


enum sms_cmd {
    SMS_CMD_START = 0x05,
    SMS_CMD_PROCESSING = 0x01,
    SMS_CMD_END = 0x06,
};

enum pt_type {
    TYPE_PT_UNKNOWN = 0,
    TYPE_PRI_PTB,
    TYPE_PRI_PT,
    TYPE_SUB_PTB,
    TYPE_SUB_PT,
    TYPE_SUB_PT_WHOLE,
    TYPE_ALL_PT,/* only for erase all partitions in the gpt */
    TYPE_NOT_IN_GPT = 0x100,
    TYPE_SPL_PT,
    TYPE_SAFETY_SFS_PT,
};


struct pt_name_info {
    char sub_ptbname[MAX_GPT_NAME_SIZE + 1];
    char ptname[MAX_GPT_NAME_SIZE + 1];
    enum pt_type type;
};

struct ptb_dl_name {
    struct list_node node;
    char *name;
};

struct storage_device_cfg {
    uint32_t device_type;
    uint32_t res_idex;
    union {
        struct mmc_cfg mmc_cfg;
        struct spi_nor_cfg ospi_cfg;
    } cfg;
    uint64_t offset;
};

struct storage_info {
    const char storage_name[MAX_GPT_NAME_SIZE + 1];
    uint32_t res_id;
    enum storage_type type;
    uint64_t ptb_offset;
    uint32_t block_size;
    void *config;
    /* If the device connected to mshc is SD card, this field indicates spl offset.
     * Non-zero means SD card type and indicates the offset of spl.
     * Zero means emmc type.
     * */
    const uint64_t boot_offset;
    uint32_t erase_grp_sz;
};

int init_avm_storage(void );
uint32_t get_avm_block_size(void);
int deinit_avm_storage(void );
uint32_t get_avm_img_state(void );
uint32_t get_avm_img_size(void );
uint32_t read_avm_partition(uint8_t *data );
uint32_t flash_avm_partition(void *data, unsigned sz);

