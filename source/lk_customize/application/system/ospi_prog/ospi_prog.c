/*
* wdg_init.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg system init .
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <debug.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include "chip_res.h"
#include "libavb.h"
#include "lib/reg.h"
#include "partition_parser.h"
#include "spi_nor_hal.h"

#define SDRV_FOOTER     "SDRVFKFT"
#define SDRV_FOOTER_LEN 8

#define NOR_FLASH_PTB_SECTOR_INDEX 2

#define DEBUG_ON 1
#if DEBUG_ON
#define DEBUG_DUMP(ptr, size, format, args...) \
    do{ \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args); \
        hexdump8(ptr, size); \
    }while(0);

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);
#define DBG(format, args...) dprintf(CRITICAL, \
                               "DEBUG:%s %d "format"\n", __func__, __LINE__,  ##args);
#else
#define DEBUG_DUMP(ptr, size, format, args...)
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

#define LOCAL_TRACE 1
typedef struct prog_cmd_args{
    uint64_t cur_sz;
    uint64_t offset;
    uint64_t total_sz;
    char storage_name[8];
    char name[MAX_GPT_NAME_SIZE];
    uint64_t dl_status;
}__PACKED prog_cmd_args;

typedef struct prog_status{
    uint32_t flag;
    uint32_t ret_code;
}__PACKED prog_status;

static struct spi_nor_cfg ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 0,
};

static struct partition_device* ospi_init(const char* storage_name)
{
    uint32_t erase_grp_sz;
    struct storage_device * storage = NULL;
    struct partition_device *ptdev = NULL;
    uint32_t res_id = RES_OSPI_REG_OSPI1;

    if (!strncmp(storage_name, "ospi1", 5))
    {
        res_id = RES_OSPI_REG_OSPI1;
    }
    else if (!strncmp(storage_name, "ospi2", 5))
    {
        res_id = RES_OSPI_REG_OSPI2;
    }else{
        LTRACEF("storage name error!\n");
        goto out;
    }

    storage = setup_storage_dev(OSPI, res_id, &ospi_cfg);
    if(!storage)
    {
        LTRACEF("storage init fail!\n");
        goto out;
    }

    erase_grp_sz = storage->get_erase_group_size(storage);
    ptdev = ptdev_setup(storage, erase_grp_sz * NOR_FLASH_PTB_SECTOR_INDEX);
    if (!ptdev)
    {
        LTRACEF("partition device setup fail!\n");
        goto out;
    }

    ptdev_read_table(ptdev);
out:
    return ptdev;
}

static void dump_cmd(prog_cmd_args * cmd_args)
{
    LTRACEF("cmd:%p \n", cmd_args);
    printf("\nstorage      :%10s\n", cmd_args->storage_name);
    printf("partition    :%10s\n", cmd_args->name);
    printf("total size   :%10llu\n", cmd_args->total_sz);
    printf("offset       :%10llu\n", cmd_args->offset);
    printf("current size :%10llu\n", cmd_args->cur_sz);
    printf("download flag:%10llu\n\n", cmd_args->dl_status);
}

static bool poll_dl_status(void)
{
    prog_cmd_args * cmd_args = (prog_cmd_args *)_ioaddr(CMD_BUF_BASE);

    while (!(cmd_args->dl_status))
    {
        //LTRACEF("no image download! cmd:%p \n", cmd_args);
        //dump_cmd(cmd_args);
        thread_sleep(200);
    }

    dump_cmd(cmd_args);
    cmd_args->dl_status = 0;
    return true;
}

static bool make_fake_avb_footer(uint64_t image_sz, AvbFooter *footer)
{
    uint32_t offset;

    if (!image_sz || !footer)
        return false;

    memset(footer, 0x0, AVB_FOOTER_SIZE);
    memcpy(footer->magic, AVB_FOOTER_MAGIC, AVB_FOOTER_MAGIC_LEN);
    footer->version_major = avb_htobe32(AVB_FOOTER_VERSION_MAJOR);
    footer->version_minor = avb_htobe32(AVB_FOOTER_VERSION_MINOR);

    footer->original_image_size = avb_htobe64(image_sz);
    offset = sizeof(footer->reserved) - SDRV_FOOTER_LEN;
    memcpy(&(footer->reserved[offset]), SDRV_FOOTER, SDRV_FOOTER_LEN);

    return true;
}

static bool padding_avb_footer(struct storage_device *storage,
                               uint64_t ptn, uint64_t pt_size,
                               uint64_t image_sz, uint8_t *scratch_buf, uint32_t block_size)
{
    AvbFooter footer;
    uint64_t  last_block;
    uint32_t footer_offset;

    /* if image is signed, pt_size is equal to image_sz */
    if (!storage || pt_size - image_sz < sizeof(AvbFooter)) {
        DBG("no enough space to padding footer,pt_size:%llu sz:%llu\n", pt_size, image_sz);
        return false;
    }

    last_block = pt_size - block_size;
    footer_offset = block_size - sizeof(AvbFooter);

    if (!(storage->read(storage, ptn + last_block,
                        scratch_buf, block_size))) {

        if (make_fake_avb_footer(image_sz, &footer)) {
            memcpy((scratch_buf + footer_offset), &footer, sizeof(AvbFooter));
            storage->write(storage, ptn + last_block, scratch_buf, block_size);
            return true;
        }
    }
    else {
        ERROR  ("read footer error!");
    }

    return false;
}
static bool erase_nor_flash_partition(struct partition_device *ptdev,
                                      const char *full_ptname, uint64_t img_sz)
{
    struct storage_device *storage;
    uint64_t size, ptn, erase_size, erase_grp_sz;

    if (!ptdev || !ptdev->storage
        || !img_sz)
        return false;

    storage = ptdev->storage;

    size = ptdev_get_size(ptdev, full_ptname);
    ptn = ptdev_get_offset(ptdev, full_ptname);

    if (!ptn || !size) {
        ERROR("ptn or size is 0. name:%s\n", full_ptname);
        return false;
    }

    erase_grp_sz = storage->get_erase_group_size(storage);
    erase_size = round_up(img_sz, erase_grp_sz);
    erase_size = erase_size > size ? size : erase_size;
    erase_size = (size - erase_size > erase_grp_sz) ? erase_size :
                 size;

    if (storage->erase(storage, ptn, erase_size)) {
        ERROR("flash storage error\n");
        return false;
    }

    /* erase the last sector for footer */
    if (( size - erase_size > erase_grp_sz)
            && storage->erase(storage,
                              ptn + size - erase_grp_sz,
                              erase_grp_sz)) {
        ERROR("flash storage error\n");
        return false;
    }

    return true;
}

static void program_flash(struct partition_device *ptdev)
{
    uint32_t block_sz;
    prog_cmd_args * cmd_args;
    uint8_t *scratch_buf = NULL;
    struct prog_status * prog_status;
    struct storage_device * storage;
    uint8_t * img_buf = (uint8_t*)_ioaddr(IMG_BUF_BASE);
    uint64_t pt_size, ptn, aligned_size, unaligned_size, offset;

    storage = ptdev->storage;
    cmd_args = (prog_cmd_args *)_ioaddr(CMD_BUF_BASE);
    prog_status = (struct prog_status*)_ioaddr(PROG_STATUS);

    prog_status->ret_code = 1;
    block_sz = storage->get_block_size(storage);
    ptn = ptdev_get_offset(ptdev, (char*)cmd_args->name);
    pt_size = ptdev_get_size(ptdev, (char*)cmd_args->name);
    offset = cmd_args->offset;
    scratch_buf = calloc(1, block_sz);

    if (!scratch_buf)
    {
        ERROR("allocate memory fail!\n");
        goto out;
    }

    if (!ptn || !pt_size)
    {
        ERROR("cann't find partition:%s\n", cmd_args->name);
        goto out;
    }

    if (pt_size <  cmd_args->cur_sz|| pt_size < cmd_args->total_sz)
    {
        ERROR("image too large cur_sz:%llu total:%llu\n", cmd_args->cur_sz, cmd_args->total_sz);
        goto out;
    }

    if(!cmd_args->offset)
        erase_nor_flash_partition(ptdev, cmd_args->name, cmd_args->total_sz);

    aligned_size = round_down(cmd_args->cur_sz, block_sz);
    unaligned_size = cmd_args->cur_sz - aligned_size;

    if (storage->write(storage, ptn + offset,
                       img_buf, aligned_size)) {
        ERROR(" write data error!\n");
        goto out;
    }

    if (unaligned_size) {
        memset(scratch_buf, 0x0, block_sz);

        if (storage->read(storage,
                          ptn + offset + aligned_size,
                          scratch_buf, block_sz)) {
            ERROR("read the last block fail!\n");
            return;
        }

        memcpy(scratch_buf, img_buf + aligned_size, unaligned_size);

        if (storage->write(storage,
                           ptn + offset + aligned_size,
                           scratch_buf, block_sz)) {
            ERROR(" write the last data!\n");
            goto out;
        }
    }

    if (cmd_args->cur_sz + offset == cmd_args->total_sz){
        padding_avb_footer(storage, ptn,pt_size,
                           cmd_args->total_sz,
                           scratch_buf, block_sz);
    }

    LTRACEF("program nor flash partition:%s successfully!\n", cmd_args->name);
    prog_status->ret_code = 0;
out:
    if (scratch_buf)
        free(scratch_buf);

    prog_status->flag = 1;
    return;
}

static void do_process(void)
{
    struct partition_device *ptdev = NULL;
    struct prog_cmd_args *cmd_args = (prog_cmd_args *)_ioaddr(CMD_BUF_BASE);

    while(poll_dl_status())
    {
        if (!ptdev)
            ptdev = ospi_init(cmd_args->storage_name);

        program_flash(ptdev);
    }
}

static void ospi_prog_entry(const struct app_descriptor *app, void *args)
{
    struct prog_status * prog_status = (struct prog_status*)_ioaddr(PROG_STATUS);
    struct prog_cmd_args * cmd_args = (prog_cmd_args *)_ioaddr(CMD_BUF_BASE);

    LTRACEF("flash nor flash!\n");
    arch_disable_cache(UCACHE);
    memset(prog_status, 0x0, sizeof(struct prog_status));
    memset(cmd_args, 0x0, sizeof(struct prog_cmd_args));
    do_process();

    return;
}

APP_START(ospi_prog)
.flags = 0,
.entry = ospi_prog_entry,
APP_END
