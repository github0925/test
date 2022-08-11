/*
 * Copyright (c) Semidrive
 */
#include <app.h>
#include <arch/defines.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <errno.h>
#include <lib/console.h>
#include <list.h>
#include <malloc.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "class_fastboot.h"
#include "fastboot_common.h"
#include "fuse_ctrl.h"
#include "hal_port.h"
#include "md5.h"
#include "mmc_hal.h"
#include "msfs.h"
#include "spi_nor_hal.h"
#include "libavb.h"
#include "lib/reg.h"
#include "lib/reboot.h"
#include "lib/sdrv_common_reg.h"
#include "partition_parser.h"
#include "sd_boot_img.h"
#include "semidrive_parser.h"
#include "storage_device.h"
#include "sparse_format.h"

#if DLOADER_USE_SPI

#include "i2c_hal.h"
#include <app.h>
#include <stdio.h>
#include <lib/console.h>
#include "spi_hal_master.h"
#include "res.h"
#include "chip_res.h"
//#include "hal_dio.h"

#include <kernel/event.h>
#include "irq.h"

#endif

#define DLOADER_VERSION      "V00.00.01"
#define SPL_PARTITION_NAME   "spl"
#define SPL_PT_MAX_SIZE      (0x20000)
#define SFS_PT_NAME          "sfs"

#define OSPI1_STORAGE_NAME   "ospi1"
#define OSPI2_STORAGE_NAME   "ospi2"
#define EMMC1_STORAGE_NAME   "emmc1"
#define EMMC2_STORAGE_NAME   "emmc2"
#define SD3_STORAGE_NAME     "sd3"
#define SD4_STORAGE_NAME     "sd4"
#define SD_SPL_OFFSET        (0x5000)
#define UINT32_HEX_STR_LEN   (sizeof(uint32_t)*2)
#define FUSE_INDEX_MAX       249

#define TOGGLE_OSPI_RESET_INDEX (0xAE)
#define TOGGLE_OSPI_RESET_BIT   (0x1U << 22)

#ifndef DL_BUF_SIZE
#error "DL_BUF_SIZE is not defined"
#endif

#ifndef DL_BUF_BASE
#error "DL_BUF_BASE is not defined"
#endif

#ifndef SPARSE_DATA_ALIGNED_BASE
#error "SPARSE_DATA_ALIGNED_BASE is not defined"
#endif

#ifndef SPARSE_DATA_ALIGNED_SIZE
#error "SPARSE_DATA_ALIGNED_SIZE is not defined"
#endif

#define EMMC_PTB_OFFSET      (0x0)
#if NO_DDR
#define EMMC_BOOT_PT_SIZE    (0x20000)
#else
#define EMMC_BOOT_PT_SIZE    (0x80000)
#endif

#define PTB_NEED_FLASH    0
#define PTB_NO_NEED_FLASH 1
#define PTB_CHECK_ERROR   2
#define INVALID_PTB_OFFSET (~0llu)

#define DISABLE_GPT_PTB_CHK 1
#define NOR_FLASH_PTB_SECTOR_INDEX 2

#define SDRV_FOOTER     "SDRVFKFT"
#define SDRV_FOOTER_LEN 8

#define DEBUG_ON 1
#if DEBUG_ON
#define DEBUG_DUMP(ptr, size, format, args...) \
    do{ \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args); \
        hexdump8(ptr, size); \
    }while(0);

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args);
#define DBG(format, args...) dprintf(INFO, \
                               "DEBUG:%s %d "format"\n", __func__, __LINE__,  ##args);
#else
#define DEBUG_DUMP(ptr, size, format, args...)
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

#if DLOADER_USE_SPI
extern const domain_res_t g_iomuxc_res;
#endif

typedef void (*fastboot_cmd_fn)(fastboot_t *fb, const char *, void *,
                                unsigned);
struct fastboot_cmd_desc {
    const char *name;
    fastboot_cmd_fn cb;
};

enum dl_err_code {
    ERR_UNKNOWN = 0x1,
    ERR_PRI_PTB_NOT_MATCH,
    ERR_SUB_PTB_NOT_MATCH,
    ERR_PT_NOT_FOUND,
    ERR_IMAGE_TOO_LARGE,
    ERR_IMAGE_FORMAT_ERR,
    ERR_PRI_PTB_NOT_FLASH,
    ERR_SUB_PTB_NOT_FLASH,
    ERR_PT_FLASH_FAIL,
    ERR_PT_ERASE_FAIL,
    ERR_PT_OVERLAP,
    ERR_PT_FULL_NAME_FORMAT,
    ERR_INVALID_BLOCK_SIZE,
    ERR_SPARSE_IMAGE_SIZE_TOO_LOW,
    ERR_SPARSE_IMAGE_HEADER,
    ERR_SPARSE_IMAGE_BUFFERED,
    ERR_SPARSE_IMAGE_CHUNK_HEADER,
    ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE,
    ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH,
    ERR_SPARSE_IMAGE_CHUNK_UNKNOWN,
    ERR_SPARSE_IMAGE_MALLOC,
    ERR_HASH_FAIL,
    ERR_EFUSE_INDEX,
    ERR_EFUSE_BURN,
    ERR_MAX,
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

/* record downloaded state */
struct storage_dl_state {
    struct list_node ptb_state_head;
    struct storage_device *storage;
    struct partition_device *ptdev;
    struct storage_info st_info;
    struct pt_name_info cur_pt_info;
};

static struct mmc_cfg mmc_cfg = {
    .voltage = MMC_VOL_1_8,
    .max_clk_rate = MMC_CLK_200MHZ,
    .bus_width = MMC_BUS_WIDTH_8BIT,
    .hs400_support = 1,
};

static struct mmc_cfg sd_mmc_cfg = {
    .voltage = MMC_VOL_3_3,
    .max_clk_rate = MMC_CLK_25MHZ,
    .bus_width = MMC_BUS_WIDTH_4BIT,
};

static struct spi_nor_cfg ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 0,
};

static struct storage_dl_state *current_dl_state;
static struct storage_dl_state storage_dl_state[] = {
    [0] = {
        .st_info = {
            .storage_name   = OSPI1_STORAGE_NAME,
            .res_id         = RES_OSPI_REG_OSPI1,
            .type           = OSPI,
            .config         = &ospi_cfg,
        }
    },
    [1] = {
        .st_info = {
            .storage_name   = OSPI2_STORAGE_NAME,
            .res_id         = RES_OSPI_REG_OSPI2,
            .type           = OSPI,
            .config         = &ospi_cfg,
        }
    },
    [2] = {
        .st_info = {
            .storage_name   = EMMC1_STORAGE_NAME,
            .res_id         = RES_MSHC_SD1,
            .ptb_offset     = EMMC_PTB_OFFSET,
            .type           = MMC,
            .config         = &mmc_cfg,
        }
    },
    [3] = {
        .st_info = {
            .storage_name   = EMMC2_STORAGE_NAME,
            .res_id         = RES_MSHC_SD2,
            .ptb_offset     = EMMC_PTB_OFFSET,
            .type           = MMC,
            .config         = &mmc_cfg,
        }
    },
    [4] = {
        .st_info = {
            .storage_name   = SD3_STORAGE_NAME,
            .res_id         = RES_MSHC_SD3,
            .ptb_offset     = EMMC_PTB_OFFSET,
            .type           = MMC,
            .config         = &sd_mmc_cfg,
            .boot_offset    = SD_SPL_OFFSET,
        }
    },
    [5] = {
        .st_info = {
            .storage_name   = SD4_STORAGE_NAME,
            .res_id         = RES_MSHC_SD4,
            .ptb_offset     = EMMC_PTB_OFFSET,
            .type           = MMC,
            .config         = &sd_mmc_cfg,
            .boot_offset    = SD_SPL_OFFSET,
        }
    }
};

static const char *err_info[] = {
    [ERR_UNKNOWN] = "unkown error",
    [ERR_PRI_PTB_NOT_MATCH] = "primary partition table not match",
    [ERR_SUB_PTB_NOT_MATCH] = "sub partition table not match",
    [ERR_PT_NOT_FOUND] = "partition not found",
    [ERR_IMAGE_TOO_LARGE] = "image too large",
    [ERR_IMAGE_FORMAT_ERR] = "image format error",
    [ERR_PRI_PTB_NOT_FLASH] = "primary partition table has not been flashed",
    [ERR_SUB_PTB_NOT_FLASH] = "sub partition table has not been flashed",
    [ERR_PT_FLASH_FAIL] = "partition flash error",
    [ERR_PT_ERASE_FAIL] = "erase partition error",
    [ERR_PT_OVERLAP] = "partition overlap",
    [ERR_PT_FULL_NAME_FORMAT] = "partition full name format error",
    [ERR_INVALID_BLOCK_SIZE] = "invalid block size",
    [ERR_SPARSE_IMAGE_SIZE_TOO_LOW] = "sparse image size too low",
    [ERR_SPARSE_IMAGE_HEADER] = "sparse image header error",
    [ERR_SPARSE_IMAGE_BUFFERED] = "buffered spare image error",
    [ERR_SPARSE_IMAGE_CHUNK_HEADER] = "sparse image chunk header error",
    [ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE] = "sparse image chunk too large",
    [ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH] = "sparse image chunk size not match type",
    [ERR_SPARSE_IMAGE_MALLOC] = "sparse image malloc error",
    [ERR_SPARSE_IMAGE_CHUNK_UNKNOWN] = "sparse image chunk unkown",
    [ERR_HASH_FAIL] = "hash check fail",
    [ERR_EFUSE_INDEX] = "efuse index error",
    [ERR_EFUSE_BURN] = "burn efuse fail",
};

static uint8_t md5_received[MD5_LEN];
static fastboot_t *fb_data;
static bool do_md5_check;

static bool patch_sfs(struct partition_device *ptdev, uint8_t *sfs_buf,
                      uint32_t len);
static uint64_t get_nor_flash_ptb_offset(struct storage_device *storage,
        uint32_t block_size);
static void cmd_flash_sparse_img(fastboot_t *fb, const char *arg,
                                 void *data, unsigned sz);
static void cmd_flash_img(fastboot_t *fb, const char *arg, void *data,
                          unsigned sz);
static void set_dl_ptb_stage(void);

static uint8_t char2hex(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return 0;
}

static void str2hex(const char *str, uint32_t str_len, uint8_t *hex,
                    uint32_t hex_len)
{
    for (uint32_t i = 0; i < str_len / 2 && i < hex_len; i++) {
        hex[i] = 0;
        hex[i] = (char2hex(str[i * 2]) & 0xF) << 4;
        hex[i] |= char2hex(str[i * 2 + 1]) & 0xF;
    }
}

static char *response_error(enum dl_err_code err, const char *pname)
{
    static char err_response[MAX_RSP_SIZE];
    const char *info = "unknown error";

    if (err < ERR_UNKNOWN || err >= ERR_MAX) {
        ERROR("unknown error code:%d\n", err);
        return NULL;
    }

    info = err_info[err];
    snprintf(err_response, sizeof(err_response), "%04x:%s - %s", err, info,
             pname);
    return err_response;
}

static bool is_normal_footer(AvbFooter *footer_raw)
{
    AvbFooter footer;
    uint32_t offset;

    if (!avb_footer_validate_and_byteswap(footer_raw, &footer))
        return false;

    offset = sizeof(footer.reserved) - SDRV_FOOTER_LEN;

    if (strncmp((char *) & (footer.reserved[offset]), SDRV_FOOTER,
                SDRV_FOOTER_LEN))
        return true;

    return false;
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
        DBG("no enough space to padding footer:%llu\n", pt_size - image_sz);
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

static int compare_gpt_entries(struct partition_entry *new,
                               struct partition_entry *old, uint32_t count)
{
    int ret = 0;

    for (uint32_t i = 0; i < count; i++) {

        ret |= strncmp((char *)new[i].name, (char *)(old[i].name),
                       MAX_GPT_NAME_SIZE);
        //ret |= memcmp(new[i].type_guid, old[i].type_guid,
        //              sizeof(new[i].type_guid));
        //ret |= memcmp(new[i].unique_partition_guid, old[i].unique_partition_guid,
        //              sizeof(new[i].unique_partition_guid));
        ret |= (new[i].first_lba != old[i].first_lba);
        ret |= (new[i].attribute_flag != old[i].attribute_flag);

        /* The last partition's last_lba may be extended to
         * storage capacity - gpt_header_block - partition_entries_blocks.
         * Skip it.
         * */
        if (i != count - 1) {
            ret |= (new[i].last_lba != old[i].last_lba);
        }

        if (ret)
            break;
    }

    return ret;
}

static int compare_gpt_header(GPT_header *new, GPT_header *old)
{
    int ret = 0;

    ret |= memcmp(new->sign, old->sign, sizeof(new->sign));
    ret |= memcmp(new->version, old->version, sizeof(new->version));
    //ret |= memcmp(new->guid, old->guid, sizeof(new->guid));
    ret |= (new->header_sz != old->header_sz);
    ret |= (new->current_lba != old->current_lba);
    //ret |= (new->backup_lba  != old->backup_lba);//this field will be re-calculated before written to emmc
    ret |= (new->first_usable_lba != old->first_usable_lba);
    //ret |= (new->last_usable_lba  != old->last_usable_lba);//this field will be re-calculated before written to emmc
    ret |= (new->partition_entry_lba != old->partition_entry_lba);
    ret |= (new->partition_entry_count != old->partition_entry_count);
    ret |= (new->partition_entry_sz != old->partition_entry_sz);
    ret |= (new->actual_entries_count != old->actual_entries_count);

    if (ret) {
        return ret;
    }

    ret |= compare_gpt_entries(new->partition_entries, old->partition_entries,
                               new->actual_entries_count);
    return ret;
}

static int check_partition_table(void *data, unsigned sz)
{
    int index           = 0;
    uint32_t ret        = PTB_CHECK_ERROR;
    uint64_t offset     = 0;
    uint64_t gpt_sz     = 0;
    uint8_t  *buffer    = NULL;
    uint32_t block_size = 0;
    const char *ptname      = current_dl_state->cur_pt_info.ptname;
    const char *sub_ptbname = current_dl_state->cur_pt_info.sub_ptbname;
    uint32_t blocks_for_entries    = 0;
    struct storage_device *storage = current_dl_state->storage;
    struct partition_device *ptdev = current_dl_state->ptdev;
    struct storage_info *st_info   = NULL;
    GPT_header buffered_gpt_header = {0};
    GPT_header storage_gpt_header  = {0};

    st_info    = &current_dl_state->st_info;
    block_size = st_info->block_size;

    if (!block_size || block_size % 4 != 0) {
        goto end;
    }

    blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
    /* MBR is in the first LBA.
     * GPT header is in the second LBA.
     * GPT entries are in the following blocks.
     * */
    gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries + 1) * block_size;

    if (!storage) {
        ERROR("storage device get error");
        goto end;
    }

    if (sz < gpt_sz) {
        ERROR("download gpt image error");
        goto end;
    }

    if (st_info->type == OSPI) {
        ret = gpt_partition_round(((uint8_t *)data + block_size),
                                  (gpt_sz - block_size) * 2, block_size, st_info->erase_grp_sz,
                                  storage->get_capacity(storage) - st_info->ptb_offset);

        if (ret) {
            ERROR("patch nor flash ptb error! ret:%d\n", ret);
            ret = PTB_CHECK_ERROR;
            goto end;
        }
    }

    /*
    * LBA0 is for MBR, skip it
    */
    ret = parse_gpt_table_from_buffer(((uint8_t *)data + block_size),
                                      gpt_sz - block_size,
                                      &buffered_gpt_header, block_size, false);

    if (ret) {
        ERROR("parse gpt header error");
        ret = PTB_NEED_FLASH;
        goto end;
    }

#if 0 //DISABLE_GPT_PTB_CHK
    ret = PTB_NEED_FLASH;
    goto end;
#endif
    buffer = memalign(st_info->block_size,
                      round_up(gpt_sz, st_info->block_size));

    if (!buffer) {
        ERROR("mem allocate buffer error");
        goto end;
    }

    /* If it is primary partition table, LBA0 is for MBR,skipping */
    if (current_dl_state->cur_pt_info.type == TYPE_PRI_PTB) {
        offset = st_info->ptb_offset;
    }
    else {
        index = ptdev_get_index(ptdev, sub_ptbname);

        if (index == INVALID_PTN) {
            goto end;
        }

        offset = ptdev_get_offset(ptdev, sub_ptbname);
    }

    DBG("ptb_offset:%lld\n", offset);
    ret = storage->read(storage, offset, (uint8_t *)buffer, gpt_sz);

    if (ret) {
        ERROR("read gpt table error:%s$%s$%s", st_info->storage_name,
              sub_ptbname, ptname);
        ret = PTB_CHECK_ERROR;
        goto end;
    }

    ret = parse_gpt_table_from_buffer((buffer + block_size),
                                      gpt_sz - block_size,
                                      &storage_gpt_header, block_size, false);

    if (ret) {
        DBG("no available gpt in device, need flash partition table\n");

        /* There is no correct gpt header in emmc,
         * so needs flash gpt image by returing PTB_NEED_FLASH
         * */
        ret = PTB_NEED_FLASH;
        goto end;
    }

    ret = compare_gpt_header(&buffered_gpt_header, &storage_gpt_header);

    if (!ret) {
        ret = PTB_NO_NEED_FLASH;
    }
    else {
        ret = PTB_CHECK_ERROR;
    }

end:

    if (buffered_gpt_header.partition_entries != NULL) {
        free(buffered_gpt_header.partition_entries);
    }

    if (storage_gpt_header.partition_entries != NULL) {
        free(storage_gpt_header.partition_entries);
    }

    if (buffer) {
        free(buffer);
    }

    return ret;
}

static void toggle_ospi_reset_fuse(void)
{
#if TOGGLE_OSPI_RESET_ENABLE
    uint32_t val, ret;
    uint32_t ospi_reset_index = TOGGLE_OSPI_RESET_INDEX;

    val = fuse_read(ospi_reset_index);
    DBG("fuse index:0x%08x val:0x%0x\n", ospi_reset_index, val);

    if (val & TOGGLE_OSPI_RESET_BIT) {
        DBG("ospi toggle reset fuse has been burnt!");
        return;
    }

    val |= TOGGLE_OSPI_RESET_BIT;
    DBG("fuse index:0x%08x val:0x%0x\n", ospi_reset_index, val);
    ret = fuse_program(ospi_reset_index, val);

    if (ret) {
        ERROR("burn efuse fail:%d  index:0x%0x val:0x%0x\n",
              ret, ospi_reset_index, val);
        return;
    }

    return;
#else
    return;
#endif

}

static bool erase_nor_flash_partition(struct partition_device *ptdev,
                                      struct storage_info *st_info,
                                      const char *full_ptname, uint64_t img_sz)
{
    struct storage_device *storage;
    uint64_t size, ptn, erase_size;

    if (!ptdev || !ptdev->storage
            || !st_info || !img_sz)
        return false;

    storage = ptdev->storage;

    size = ptdev_get_size(ptdev, full_ptname);
    ptn = ptdev_get_offset(ptdev, full_ptname);

    if (!ptn || !size) {
        ERROR("ptn or size is 0. name:%s\n", full_ptname);
        return false;
    }

    erase_size = round_up(img_sz, st_info->erase_grp_sz);
    erase_size = erase_size > size ? size : erase_size;
    erase_size = (size - erase_size > st_info->erase_grp_sz) ? erase_size :
                 size;

    if (storage->erase(storage, ptn, erase_size)) {
        ERROR("flash storage error\n");
        return false;
    }

    /* erase the last sector for footer */
    if (( size - erase_size > st_info->erase_grp_sz)
            && storage->erase(storage, ptn + size - st_info->erase_grp_sz,
                              st_info->erase_grp_sz)) {
        ERROR("flash storage error\n");
        return false;
    }

    DBG("erase_size:0x%llx size:0x%llx group:0x%x name:%s",
        erase_size, size, st_info->erase_grp_sz, full_ptname);
    return true;
}

static void cmd_flash_sparse_img(fastboot_t *fb, const char *arg,
                                 void *data, unsigned sz)
{
    uint32_t i         = 0;
    uint32_t fill_val  = 0;
    uintptr_t data_end = (uintptr_t)data + sz;
    const char *ptname = current_dl_state->cur_pt_info.ptname;
    unsigned int chunk = 0;
    uint32_t *fill_buf = NULL;
    bool fill_buf_init = false;
    void *data_ptr_temp     = NULL;
    enum dl_err_code err    = ERR_UNKNOWN;
    enum pt_type pttype     = current_dl_state->cur_pt_info.type;
    uint32_t total_blocks   = 0;
    unsigned long long ptn  = 0;
    unsigned long long pos  = 0;
    uint32_t blk_sz_actual  = 0;
    uint64_t chunk_data_sz  = 0;
    uint32_t switch_pt_num  = PART_ACCESS_DEFAULT;
    unsigned long long size = 0;
    uint8_t *block_wrapper  = NULL;
    uint64_t count_aligned  = 0;
    const char *sub_ptbname = current_dl_state->cur_pt_info.sub_ptbname;
    struct storage_info *st_info   = NULL;
    chunk_header_t *chunk_header   = NULL;
    sparse_header_t *sparse_header = NULL;
    uint64_t chunk_data_sz_remain  = 0;
    struct partition_device *ptdev = current_dl_state->ptdev;
    struct storage_device *storage = current_dl_state->storage;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};
    st_info = &current_dl_state->st_info;

    block_wrapper = (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE);

    if (!block_wrapper
            || ((uint32_t)block_wrapper % st_info->block_size) != 0
            || (SPARSE_DATA_ALIGNED_SIZE % st_info->block_size) != 0
            || SPARSE_DATA_ALIGNED_SIZE == 0) {
        ERROR("aligned  memory allocate fail!\n");
        err = ERR_SPARSE_IMAGE_MALLOC;
        goto fail;
    }

    if (!storage || !ptdev) {
        ERROR("storage get error!\n");
        err = ERR_PT_FLASH_FAIL;
        goto fail;
    }

    if (pttype >= TYPE_NOT_IN_GPT) {
        /* don't support boot area sparse image */
        err = ERR_IMAGE_TOO_LARGE;
        ERROR("boot partition doesn't use sparse image \n");
        goto fail;
    }
    else {
        switch_pt_num = PART_ACCESS_DEFAULT;

        if (st_info->type == MMC && !st_info->boot_offset) {
            storage->switch_part(storage, switch_pt_num);
        }

        if (pttype == TYPE_SUB_PT) {
            snprintf(full_ptname, sizeof(full_ptname), "%s$%s", sub_ptbname, ptname);
        }
        else if (pttype == TYPE_SUB_PT_WHOLE) {
            snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
        }
        else if (pttype == TYPE_PRI_PT) {
            snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
        }

        ptn = ptdev_get_offset(ptdev, full_ptname);

        if (ptn == 0) {
            ERROR("ptn is 0.\n");
            err = ERR_PT_NOT_FOUND;
            goto fail;
        }

        size = ptdev_get_size(ptdev, full_ptname);

        if (sz < sizeof(sparse_header_t)) {
            ERROR("spare image size:%u error\n", sz);
            err = ERR_SPARSE_IMAGE_SIZE_TOO_LOW;
            goto fail;
        }
    }

    /* Read and skip over sparse image header */
    sparse_header = (sparse_header_t *) data;

    if (!sparse_header->blk_sz || (sparse_header->blk_sz % 4)) {
        ERROR("block size error:%u\n", sparse_header->blk_sz);
        err = ERR_INVALID_BLOCK_SIZE;
        goto fail;
    }

    if (((uint64_t)sparse_header->total_blks * (uint64_t)sparse_header->blk_sz)
            > size) {
        ERROR("image too large :%llu\n", size);
        err = ERR_IMAGE_TOO_LARGE;
        goto fail;
    }

    data += sizeof(sparse_header_t);

    if (data_end < (uintptr_t)data) {
        ERROR("data end:%lu  header size:%u\n", data_end, sizeof(sparse_header_t));
        err = ERR_SPARSE_IMAGE_BUFFERED;
        goto fail;
    }

    if (sparse_header->file_hdr_sz != sizeof(sparse_header_t)) {
        ERROR("image header error!\n");
        err = ERR_SPARSE_IMAGE_HEADER;
        goto fail;
    }

    dprintf(SPEW, "=== Sparse Image Header ===\n");
    dprintf(SPEW, "magic: 0x%x\n", sparse_header->magic);
    dprintf(SPEW, "major_version: 0x%x\n", sparse_header->major_version);
    dprintf(SPEW, "minor_version: 0x%x\n", sparse_header->minor_version);
    dprintf(SPEW, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
    dprintf(SPEW, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
    dprintf(SPEW, "blk_sz: %d\n", sparse_header->blk_sz);
    dprintf(SPEW, "total_blks: %d\n", sparse_header->total_blks);
    dprintf(SPEW, "total_chunks: %d\n", sparse_header->total_chunks);

    /* Start processing chunks */
    for (chunk = 0; chunk < sparse_header->total_chunks; chunk++) {
        /* Make sure the total image size does not exceed the partition size */
        if (((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz) >= size) {
            ERROR("image too large:%llu!\n", size);
            err = ERR_IMAGE_TOO_LARGE;
            goto fail;
        }

        /* Read and skip over chunk header */
        chunk_header = (chunk_header_t *) data;
        data += sizeof(chunk_header_t);

        if (data_end < (uintptr_t)data) {
            ERROR("data end:%lu  data:%p chunk header size:%u\n",
                  data_end, data, sizeof(chunk_header_t));
            err = ERR_SPARSE_IMAGE_BUFFERED;
            goto fail;
        }

        dprintf(SPEW, "=== Chunk Header ===\n");
        dprintf(SPEW, "chunk_type: 0x%x\n", chunk_header->chunk_type);
        dprintf(SPEW, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
        dprintf(SPEW, "total_size: 0x%x\n", chunk_header->total_sz);

        if (sparse_header->chunk_hdr_sz != sizeof(chunk_header_t)) {
            ERROR("chunk header error:%u!\n", sparse_header->chunk_hdr_sz);
            err = ERR_SPARSE_IMAGE_CHUNK_HEADER;
            goto fail;
        }

        chunk_data_sz = (uint64_t)sparse_header->blk_sz * chunk_header->chunk_sz;

        /* Make sure that the chunk size calculated from sparse image does not
         * exceed partition size
         */
        if ((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz +
                chunk_data_sz > size) {
            ERROR("chunk data too large:%llu!\n", size);
            err = ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE;
            goto fail;
        }

        switch (chunk_header->chunk_type) {
            case CHUNK_TYPE_RAW:
                if ((uint64_t)chunk_header->total_sz != ((uint64_t)
                        sparse_header->chunk_hdr_sz +
                        chunk_data_sz)) {
                    ERROR("chunk size:%llu error!\n", chunk_data_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                if (data_end < (uintptr_t)data + chunk_data_sz) {
                    ERROR("data end:%lu  data:%p chunk_data_sz:%llu\n",
                          data_end, data, chunk_data_sz);
                    err = ERR_SPARSE_IMAGE_BUFFERED;
                    goto fail;
                }

                /* chunk_header->total_sz is uint32,So chunk_data_sz is now less than 2^32
                   otherwise it will return in the line above
                 */
                pos = ptn + ((uint64_t)total_blocks * sparse_header->blk_sz);
                count_aligned = 0;
                data_ptr_temp = data;
                chunk_data_sz_remain  = chunk_data_sz;

                /* for ospi nor flash,
                 * erase the partition before writting
                 * */
                if (pos == ptn && st_info->type == OSPI) {

                    if (!erase_nor_flash_partition(ptdev, st_info, full_ptname,
                                                   sparse_header->blk_sz * sparse_header->total_blks)) {
                        ERROR("erase partition fail!\n");
                        err = ERR_PT_ERASE_FAIL;
                        goto fail;
                    }
                }

                if (!IS_ALIGNED((uint32_t)data_ptr_temp, st_info->block_size)) {
                    while (chunk_data_sz_remain) {
                        count_aligned = MIN(SPARSE_DATA_ALIGNED_SIZE, chunk_data_sz_remain);
                        memset(block_wrapper, 0x0, SPARSE_DATA_ALIGNED_SIZE);
                        memcpy(block_wrapper, data_ptr_temp, count_aligned);

                        if (storage->write(storage, pos, block_wrapper,
                                           round_up(count_aligned, (uint64_t)st_info->block_size))) {
                            ERROR("flash storage error\n");
                            err = ERR_PT_FLASH_FAIL;
                            goto fail;
                        }

                        pos += count_aligned;
                        data_ptr_temp = (uint8_t *)data_ptr_temp + count_aligned;
                        chunk_data_sz_remain -= count_aligned;
                    }
                }
                else {
                    if (storage->write(storage, pos, (uint8_t *)data,
                                       round_up(chunk_data_sz, (uint64_t)st_info->block_size))) {
                        ERROR("write data error!\n");
                        err = ERR_PT_FLASH_FAIL;
                        goto fail;
                    }
                }

                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR("chunk size error:%u!\n", chunk_header->chunk_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                total_blocks += chunk_header->chunk_sz;
                data += (uint32_t)chunk_data_sz;
                break;

            case CHUNK_TYPE_FILL:
                if (chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
                                               sizeof(uint32_t))) {
                    ERROR("fill type error, size:%u!\n", chunk_header->total_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                blk_sz_actual = round_up(sparse_header->blk_sz, CACHE_LINE);

                /* Integer overflow detected */
                if (blk_sz_actual < sparse_header->blk_sz) {
                    ERROR("blk_sz_actual:%u error!\n", blk_sz_actual);
                    err = ERR_INVALID_BLOCK_SIZE;
                    goto fail;
                }

                fill_buf = (uint32_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE);

                if (!fill_buf) {
                    ERROR(" allocat memory fail!\n");
                    err = ERR_SPARSE_IMAGE_MALLOC;
                    goto fail;
                }

                if (data_end < (uintptr_t)data + sizeof(uint32_t)) {
                    ERROR("data end:%lu  data:%p \n", data_end, data);
                    err = ERR_SPARSE_IMAGE_BUFFERED;
                    goto fail;
                }

                fill_val = *(uint32_t *)data;
                data = (char *) data + sizeof(uint32_t);

                chunk_data_sz_remain  = chunk_header->chunk_sz * sparse_header->blk_sz;
                fill_buf_init = false;

                while (chunk_data_sz_remain) {

                    if (SPARSE_DATA_ALIGNED_SIZE > chunk_data_sz_remain) {
                        count_aligned = chunk_data_sz_remain;
                    }
                    else {
                        count_aligned = SPARSE_DATA_ALIGNED_SIZE;
                    }

                    if (!fill_buf_init) {
                        for (i = 0; !fill_buf_init
                                && i < (count_aligned / sizeof(fill_val)); i++) {
                            fill_buf[i] = fill_val;
                        }

                        fill_buf_init = true;
                    }

                    if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                        ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                        err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                        goto fail;
                    }

                    /* Make sure that the data written to partition does not exceed partition size */
                    if ((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz +
                            sparse_header->blk_sz > size) {
                        ERROR(" size:%llu error!\n", size);
                        err = ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE;
                        goto fail;
                    }

                    pos = ptn + ((uint64_t)total_blocks * sparse_header->blk_sz);

                    /* for ospi nor flash,
                     * erase the partition before writting
                     * */
                    if (pos == ptn && st_info->type == OSPI) {
                        if (!erase_nor_flash_partition(ptdev, st_info, full_ptname,
                                                       sparse_header->blk_sz * sparse_header->total_blks)) {
                            ERROR("erase partition fail!\n");
                            err = ERR_PT_ERASE_FAIL;
                            goto fail;
                        }
                    }

                    if (storage->write(storage, pos, (uint8_t *)fill_buf,
                                       round_up(count_aligned, (uint64_t)st_info->block_size))) {
                        ERROR(" write data error!\n");
                        err = ERR_PT_FLASH_FAIL;
                        goto fail;
                    }

                    chunk_data_sz_remain -= count_aligned;
                    total_blocks += count_aligned / sparse_header->blk_sz;
                }

                break;

            case CHUNK_TYPE_DONT_CARE:
                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                total_blocks += chunk_header->chunk_sz;
                break;

            case CHUNK_TYPE_CRC:
                if (chunk_header->total_sz != sparse_header->chunk_hdr_sz) {
                    ERROR(" chunk total size:%u error!\n", chunk_header->total_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                    err = ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    goto fail;
                }

                total_blocks += chunk_header->chunk_sz;

                if ((uintptr_t)data > UINT_MAX - chunk_data_sz) {
                    ERROR(" chunk data size:%llu error!\n", chunk_data_sz);
                    err = ERR_IMAGE_TOO_LARGE;
                    goto fail;
                }

                data += (uint32_t)chunk_data_sz;

                if (data_end < (uintptr_t)data) {
                    ERROR("data end:%lu  data:%p \n", data_end, data);
                    err = ERR_SPARSE_IMAGE_BUFFERED;
                    goto fail;
                }

                break;

            default:
                ERROR("Unkown chunk type: %x\n", chunk_header->chunk_type);
                err = ERR_SPARSE_IMAGE_CHUNK_UNKNOWN;
                goto fail;
        }
    }

    DBG("Wrote %d blocks, expected to write %d blocks\n",
        total_blocks, sparse_header->total_blks);

    if (total_blocks != sparse_header->total_blks) {
        ERROR(" total block:%u error!\n", total_blocks);
        err = ERR_PT_FLASH_FAIL;
        goto fail;
    }

    padding_avb_footer(storage, ptn, size,
                       total_blocks * sparse_header->blk_sz,
                       block_wrapper, st_info->block_size);

    set_dl_ptb_stage();
    //If the image is whole sub partition, need update partition table here.
    fastboot_common_okay(fb, "");
    return;

fail:
    fastboot_common_fail(fb, response_error(err, full_ptname));
    return;
}

static uint32_t flash_emmc_one_boot_area(fastboot_t *fb,
        struct storage_device *storage,
        void *data, unsigned sz, uint32_t part_num)
{
    unsigned long long ptn       = 0;
    unsigned long long size      = 0;
    struct storage_info *st_info = NULL;
    enum dl_err_code err         = ERR_UNKNOWN;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};
    uint8_t md5_calc[MD5_LEN] = {0};

    if (part_num != PART_ACCESS_BOOT1
            && part_num != PART_ACCESS_BOOT2) {
        fastboot_common_fail(fb, response_error(err, full_ptname));
        ERROR("boot partition num error!\n");
        return 1;
    }

    st_info = &current_dl_state->st_info;
    snprintf(full_ptname, sizeof(full_ptname), "%s",
             current_dl_state->cur_pt_info.ptname);

    if (!st_info->boot_offset && storage->switch_part(storage, part_num)) {
        err = ERR_PT_FLASH_FAIL;
        ERROR("switch boot partition fail!\n");
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }

    //size = storage->get_capacity(storage);
    size = EMMC_BOOT_PT_SIZE;
    ptn = st_info->boot_offset;

    DBG("ptn:%lld size:%lld sz:%d\n", ptn, size, sz);

    if (round_up(sz, st_info->block_size) > size) {
        err = ERR_IMAGE_TOO_LARGE;
        ERROR("boot image too large!\n");
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }
    else if (storage->write(storage, ptn, (uint8_t *)data,
                            round_up(sz, st_info->block_size) )) {
        err = ERR_PT_FLASH_FAIL;
        ERROR("write emmc error!\n");
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }

    if (do_md5_check) {
        if (storage->read(storage, ptn,
                          (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE),
                          round_up(sz, st_info->block_size))) {
            ERROR("read back error!\n");
        }
        else {
            DBG("read back success!\n");
            md5((unsigned char *)_ioaddr(SPARSE_DATA_ALIGNED_BASE), sz,
                (unsigned char *)md5_calc);

            if (memcmp(md5_received, md5_calc, MD5_LEN)) {
                ERROR("md5 check fail!\n");
                hexdump8(md5_received, MD5_LEN);
                hexdump8(md5_calc, MD5_LEN);
                fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
                return 1;
            }
        }
    }

    return 0;
}

static uint32_t flash_emmc_boot_areas(fastboot_t *fb, void *data,
                                      unsigned sz)
{
    uint32_t ret = 0;
    struct storage_info *st_info = NULL;
    struct storage_device *storage = current_dl_state->storage;

    st_info = &current_dl_state->st_info;
    ret = flash_emmc_one_boot_area(fb, storage, data, sz, PART_ACCESS_BOOT1);

    /* The current storage is emmc card */
    if (!st_info->boot_offset)
        ret |= flash_emmc_one_boot_area(fb, storage, data, sz, PART_ACCESS_BOOT2);

    return ret;
}

static bool get_matched_sfs(void **data, uint32_t *sz,
                          struct storage_device *storage)
{
    uint32_t i = 0;
    bool ret = false;
    struct sfs sfs = {0};
    flash_seq_info_t *seq_info;
    msfs_t *msfs = (msfs_t *)(*data);
    uint64_t flash_id = 0, flash_id_mask;

    if (msfs->magic != MSFS_MAGIC ||
            msfs->count == 0 ||
            msfs->count > MULTI_SFS_MAX_CNT) {
        dprintf(CRITICAL, "The sfs image is not muti-sfs image!\n");
        goto verify_sfs;
    }

    if (storage->get_storage_id &&
            storage->get_storage_id(storage, (uint8_t *)&flash_id, sizeof(flash_id))) {
        dprintf(CRITICAL, "Fail to get storage id!\n");
    }

    flash_id_mask = ((0x1ull << FLASH_ID_BITS_LEN) - 1);
    seq_info = (flash_seq_info_t *)((uint8_t *)msfs + msfs->offset);

    for (i =  0; i < msfs->count; i++, seq_info++) {
        dprintf(INFO, "%s %d flash_id:0x%0llx seq id:0x%0llx!\n", __func__, __LINE__,
                flash_id, seq_info->flash_id);

        if (!((flash_id ^ seq_info->flash_id) & flash_id_mask)) {
            *data = (uint8_t *)msfs + seq_info->offset;
            *sz = seq_info->size;
            dprintf(CRITICAL, "%s %d matched, flash_id:0x%0llx seq id:0x%0llx mask:0x%0llx!\n",
                    __func__, __LINE__, flash_id, seq_info->flash_id, flash_id_mask);
            break;
        }
    }

    if (i == msfs->count) {
        dprintf(CRITICAL, "No matched sfs, flash_id:0x%0llx!\n", flash_id);
        goto out;
    }

verify_sfs:

    if (get_sfs_info(&sfs, *data, *sz)) {
        ERROR("The sfs is not correct!\n");
    }
    else {
        ret = true;
    }

out:
    return ret;
}

static uint32_t flash_ospi_boot_areas(fastboot_t *fb, void *data,
                                      unsigned sz)
{
    bool patched = false;
    uint64_t ptn    = 0;
    uint8_t *buffer = NULL;
    struct storage_info *st_info = NULL;
    enum pt_type pttype  = current_dl_state->cur_pt_info.type;
    enum dl_err_code err = ERR_UNKNOWN;
    struct partition_device *ptdev = current_dl_state->ptdev;
    struct storage_device *storage = current_dl_state->storage;
    uint8_t md5_calc[MD5_LEN] = {0};

    st_info = &current_dl_state->st_info;

    if (pttype != TYPE_SAFETY_SFS_PT) {
        err = ERR_PT_NOT_FOUND;
        ERROR("pttype:%d error!\n", pttype);
        goto fail;
    }

    ptn = 0;

    if (!get_matched_sfs(&data, &sz, storage)) {
        err = ERR_IMAGE_FORMAT_ERR;
        goto fail;
    }

    patched = patch_sfs(ptdev, data, sz);

    if (patched)
        md5(data, sz, md5_received);

    DBG("ptn:%lld size:%d\n", ptn, sz);
    storage->erase(storage, ptn, round_up(sz, st_info->erase_grp_sz));

    if (storage->write(storage, ptn, data,
                       round_up(sz, st_info->block_size))) {
        ERROR(" write data error!\n");
        err = ERR_PT_FLASH_FAIL;
        goto fail;
    }

    if (do_md5_check) {
        memset((void *)_ioaddr(SPARSE_DATA_ALIGNED_BASE), 0x0, sz);

        if (storage->read(storage, ptn,
                          (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE),
                          round_up(sz, st_info->block_size))) {
            ERROR("read back error!\n");
            fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
            return 1;
        }
        else {
            DBG("read back success!\n");
            md5((unsigned char *)_ioaddr(SPARSE_DATA_ALIGNED_BASE), sz,
                (unsigned char *)md5_calc);

            if (memcmp(md5_received, md5_calc, MD5_LEN)) {
                ERROR("md5 check fail!\n");
                hexdump8(md5_received, MD5_LEN);
                hexdump8(md5_calc, MD5_LEN);
                fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
                return 1;
            }
        }
    }

    free(buffer);
    return 0;
fail:

    if (buffer) {
        free(buffer);
    }

    fastboot_common_fail(fb, response_error(err,
                                            current_dl_state->cur_pt_info.ptname));
    return 1;
}

static bool patch_sfs(struct partition_device *ptdev, uint8_t *buffer,
                      uint32_t len)
{
    bool patched = false;
    const char *dil = "dil";
    const char *dil_bak = "dil_bak";
    struct sfs sfs = {0};
    uint32_t crc_val = 0;
    uint64_t dil_ptn = 0;
    uint64_t dil_bak_ptn = 0;
    uint32_t normal_img_base = 0;
    uint32_t backup_img_base = 0;

    if (len < SFS_SIZE || !ptdev || !ptdev->storage) {
        return false;
    }

    dil_ptn = ptdev_get_offset(ptdev, dil);
    dil_bak_ptn = ptdev_get_offset(ptdev, dil_bak);

    if (!dil_ptn || dil_ptn >= UINT32_MAX
            || !dil_bak_ptn || dil_bak_ptn >= UINT32_MAX) {
        ERROR("get dil/dil_bak ptn fail\n");
        return false;
    }

    if (get_sfs_info(&sfs, buffer, len)) {
        ERROR("there is no available sfs in nor flash\n");
        return false;
    }

    normal_img_base = GET_LWORD_FROM_BYTE(&buffer[SFS_NIA_OFFSET]);
    backup_img_base = GET_LWORD_FROM_BYTE(&buffer[SFS_BIA_OFFSET]);

    DBG("normal image base:0x%08x  dil:0x%08x backup_img_base:0x%08x  dil_bak:0x%08x\n",
        normal_img_base, (uint32_t)dil_ptn, backup_img_base, (uint32_t)dil_bak_ptn);

    if (normal_img_base != (uint32_t)dil_ptn) {
        PUT_LONG(&buffer[SFS_NIA_OFFSET], (uint32_t)dil_ptn);
        DBG("sfs normal image base update,original base:0x%08x  base:0x%08x\n",
            normal_img_base, (uint32_t)dil_ptn);
        patched = true;
    }

    if (backup_img_base != (uint32_t)dil_bak_ptn) {
        PUT_LONG(&buffer[SFS_BIA_OFFSET], (uint32_t)dil_bak_ptn);
        DBG("sfs backup image base update, original base:0x%08x base:0x%08x\n",
            backup_img_base, (uint32_t) dil_bak_ptn);
        patched = true;
    }

    if (patched) {
        crc_val = sfs_crc32(0, buffer, SFS_SIZE - 4);
        PUT_LONG(&buffer[SFS_CRC32_OFFSET], crc_val);
    }

    return patched;
}

static uint32_t update_sfs(void)
{
    bool need_update = false;
    uint8_t *buffer = NULL;
    uint32_t ret = 0;
    uint32_t sfs_size = 0;
    struct storage_device *storage = current_dl_state->storage;
    struct partition_device *ptdev = current_dl_state->ptdev;
    struct storage_info *st_info   = &current_dl_state->st_info;

    if (st_info->type != OSPI) {
        return 1;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, st_info->block_size);
    buffer = (uint8_t *)memalign(st_info->block_size, sfs_size);

    if (!buffer) {
        ERROR("allocate memory fail\n");
        return 1;
    }

    memset(buffer, 0x0, sfs_size);

    if (storage->read(storage, 0, buffer, sfs_size)) {
        ERROR("read nor flash fail\n");
        ret = 1;
        goto end;
    }

    need_update = patch_sfs(ptdev, buffer, sfs_size);

    if (!need_update)
        goto end;

    if (storage->erase(storage, 0,
                       round_up(sfs_size, st_info->erase_grp_sz))) {
        ERROR("erase sfs fail\n");
        ret = 1;
        goto end;
    }

    if (storage->write(storage, 0, buffer, sfs_size)) {
        ERROR("write sfs fail\n");
        ret = 1;
        goto end;
    }

    ret = 0;
end:

    if (buffer)
        free(buffer);

    return ret;
}

static uint32_t flash_gpt_table(fastboot_t *fb, void *data, unsigned sz)
{
    uint32_t ret  = PTB_CHECK_ERROR;
    char *ptname  = current_dl_state->cur_pt_info.ptname;
    uint32_t part_num    = PART_ACCESS_DEFAULT;
    char *sub_ptbname    = current_dl_state->cur_pt_info.sub_ptbname;
    enum pt_type pttype  = current_dl_state->cur_pt_info.type;
    enum dl_err_code err = ERR_UNKNOWN;
    struct storage_info *st_info   = &current_dl_state->st_info;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = current_dl_state->storage;
    bool last_part_extend = true;
    uint64_t gpt_sz     = 0;
    uint64_t capacity   = 0;
    uint32_t block_size = 0;
    uint32_t blocks_for_entries = 0;


    if (st_info->type == MMC && !st_info->boot_offset) {
        DBG("emmc need switch partition\n");

        if (!storage->switch_part || storage->switch_part(storage, part_num)) {
            ERROR("switch part error\n");
            err = ERR_PT_FLASH_FAIL;
            fastboot_common_fail(fb, response_error(err, ptname));
            return 1;
        }
    }
    else if (st_info->type == OSPI) {
        last_part_extend = false;
    }
    else {
        DBG("nor flash no  need to switch partition type:%d\n", st_info->type);
    }

    ret = check_partition_table(data, sz);

    DBG("ret:%d\n", ret);

    if (ret == PTB_CHECK_ERROR) {
        err = ERR_PRI_PTB_NOT_MATCH;

        if (strlen(sub_ptbname)) {
            err = ERR_SUB_PTB_NOT_MATCH;
            ptname = sub_ptbname;
        }

        ERROR(" check error ret:%u !\n",
              ret);
        fastboot_common_fail(fb, response_error(err, ptname));
        return 1;
    }
    else if (ret == PTB_NO_NEED_FLASH) {
        DBG("partition table doesn't need to flash\n");
        goto end;
    }

    /*
     * This is primary GPT table.
     * To write primary ptb, set sub_ptbname = NULL
     */
    if (pttype == TYPE_PRI_PTB)
        sub_ptbname = NULL;

    dprintf(ALWAYS, "Attempt to write primary/sub partition:%s \n", ptname);
    DBG("ptb_offset:0x%llx\n", st_info->ptb_offset);

    /* If flashing ospi nor flash,
     * it may re-calculate ptb offset and
     * reset ptdev, so get ptdev pointer here.
     * */
    ptdev = current_dl_state->ptdev;

    if (ptdev
            && st_info->type == OSPI ) {
        block_size = st_info->block_size;
        blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
        gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries + 1) * block_size;
        capacity = storage->get_capacity(storage);

        /*
         * For nor flash, it needs to erase storage before writting data to it.
         * To reduce time consumption, it only erases primary gpt header and backup gpt header.
         * Note, the addr and the length are must be aligned to erase group size.
         * */
        storage->erase(storage, st_info->ptb_offset,
                       round_up(gpt_sz, st_info->erase_grp_sz));
        storage->erase(storage,
                       round_down(capacity - gpt_sz + block_size, st_info->erase_grp_sz),
                       round_up(gpt_sz - st_info->block_size, st_info->erase_grp_sz));

        DBG("ptb_offset:0x%0llx gpt_sz:%llu block size:%d capacity:0x%0llx \n",
            st_info->ptb_offset, gpt_sz, block_size, capacity);
    }

    if (!ptdev
            || ptdev_write_table(ptdev, sub_ptbname, sz, (unsigned char *) data,
                                 last_part_extend)) {
        ERROR("write gpt table error pt:%s ptb_offset:0x%llx\n",
              ptname, current_dl_state->st_info.ptb_offset);
        err = ERR_PT_FLASH_FAIL;
        fastboot_common_fail(fb, response_error(err, ptname));
        return 1;
    }

end:

    if (st_info->type == OSPI) {
        ret = update_sfs();

        if (ret) {
            ERROR("updata sfs fail\n");
            err = ERR_PT_FLASH_FAIL;
            fastboot_common_fail(fb, response_error(err, ptname));
            return 1;
        }
    }

    return 0;
}

static uint32_t flash_normal_partition(fastboot_t *fb, void *data,
                                       unsigned sz)
{
    char *ptname      = current_dl_state->cur_pt_info.ptname;
    char *sub_ptbname = current_dl_state->cur_pt_info.sub_ptbname;
    uint32_t part_num = PART_ACCESS_DEFAULT;
    enum pt_type pttype  = current_dl_state->cur_pt_info.type;
    enum dl_err_code err = ERR_UNKNOWN;
    unsigned long long ptn  = 0;
    unsigned long long size = 0;
    struct storage_info *st_info   = NULL;
    struct storage_device *storage = current_dl_state->storage;
    struct partition_device *ptdev = current_dl_state->ptdev;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};
    uint8_t md5_calc[MD5_LEN] = {0};
    uint8_t *scratch_buf = (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE);

    st_info = &current_dl_state->st_info;

    DBG("ptb_offset:0x%llx\n", st_info->ptb_offset);

    if (st_info->type == MMC && !st_info->boot_offset) {
        if (!storage->switch_part || storage->switch_part(storage, part_num)) {
            ERROR("switch part error\n");
            err = ERR_PT_FLASH_FAIL;
            fastboot_common_fail(fb, response_error(err, ptname));
            return 1;
        }
    }
    else if (st_info->type == OSPI) {
        DBG("ospi ptb_offset:0x%llx\n",
            get_nor_flash_ptb_offset(storage, st_info->block_size));
    }

    if (pttype == TYPE_SUB_PT) {
        /* If the partition is sub partition, we should pass the full name */
        snprintf(full_ptname, sizeof(full_ptname), "%s$%s", sub_ptbname, ptname);
    }
    else if (pttype == TYPE_SUB_PT_WHOLE) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
    }
    else {
        /* This is a primary partition */
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
    }

    ptn = ptdev_get_offset(ptdev, full_ptname);

    if (ptn == 0) {
        ERROR(" ptn is 0!\n");
        err = ERR_PT_NOT_FOUND;
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }

    size = ptdev_get_size(ptdev, full_ptname);

    DBG("ptn:%lld size:%lld\n", ptn, size);

    if (st_info->type == OSPI) {
        if (ptn % st_info->erase_grp_sz != 0
                || round_up(sz, st_info->erase_grp_sz) > size) {
            err = ERR_IMAGE_TOO_LARGE;
            fastboot_common_fail(fb, response_error(err, full_ptname));
            ERROR("the size of partition in nor flash is too large\n");
            return 1;
        }

        if (!erase_nor_flash_partition(ptdev, st_info, full_ptname, sz)) {
            ERROR("erase partition fail\n");
            err = ERR_PT_ERASE_FAIL;
            fastboot_common_fail(fb, response_error(err, full_ptname));
            return 1;
        }
    }

    if (round_up(sz, st_info->block_size) > size) {
        ERROR(" image too large:%llu!\n",
              round_up(sz, st_info->block_size));
        err = ERR_IMAGE_TOO_LARGE;
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }

    if (!storage
            || storage->write(storage, ptn, (uint8_t *)data, round_up(sz,
                              st_info->block_size))) {
        ERROR(" write data error!\n");
        err = ERR_PT_FLASH_FAIL;
        fastboot_common_fail(fb, response_error(err, full_ptname));
        return 1;
    }

    padding_avb_footer(storage, ptn, size, sz,
                       scratch_buf, st_info->block_size);

    if (do_md5_check) {
        if (storage->read(storage, ptn, scratch_buf,
                          round_up(sz, st_info->block_size))) {
            ERROR("read back error!\n");
            fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
            return 1;
        }
        else {
            DBG("read back success!\n");
            md5(scratch_buf, sz, md5_calc);

            if (memcmp(md5_received, md5_calc, MD5_LEN)) {
                ERROR("md5 check fail!\n");
                hexdump8(md5_received, MD5_LEN);
                hexdump8(md5_calc, MD5_LEN);
                fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
                return 1;
            }
        }
    }

    return 0;
}

static uint32_t flash_boot_areas(fastboot_t *fb, void *data, unsigned sz)
{
    uint32_t ret = 0;
    struct storage_info *st_info   = NULL;

    st_info = &current_dl_state->st_info;

    if (st_info->type == MMC) {
        ret = flash_emmc_boot_areas(fb, data, sz);
    }
    else if (st_info->type == OSPI) {
        ret = flash_ospi_boot_areas(fb, data, sz);
    }

    return ret;
}

static void cmd_flash_img(fastboot_t *fb, const char *arg, void *data,
                          unsigned sz)
{
    uint32_t ret = 0;
    enum pt_type pttype = current_dl_state->cur_pt_info.type;

    if (pttype == TYPE_PRI_PTB || pttype == TYPE_SUB_PTB) {
        ret = flash_gpt_table(fb, data, sz);
    }
    else if (pttype > TYPE_NOT_IN_GPT) {
        ret = flash_boot_areas(fb, data, sz);
    }
    else {
        ret = flash_normal_partition(fb, data, sz);
    }

    if (ret) {
        ERROR("flash error ret:%d!\n", ret);
        return;
    }

    set_dl_ptb_stage();

    //If the image is whole sub partition, need update partition table here.
    if  (pttype == TYPE_SUB_PT_WHOLE) {
        ptdev_read_table(current_dl_state->ptdev);
    }

    fastboot_common_okay(fb, "");
    return;
}

void cmd_flash_storage(fastboot_t *fb, const char *arg, void *data,
                       unsigned sz)
{
    sparse_header_t *sparse_header = NULL;

    sparse_header = (sparse_header_t *) data;

    if (sparse_header->magic == SPARSE_HEADER_MAGIC) {
        cmd_flash_sparse_img(fb, arg, data, sz);
    }
    else {
        cmd_flash_img(fb, arg, data, sz);
    }

    if (current_dl_state->st_info.type == OSPI)
        toggle_ospi_reset_fuse();

    return;
}

static void remove_dl_ptb_state(enum pt_type pttype, const char *ptb_name)
{
    struct ptb_dl_name *ptb = NULL;

#if DISABLE_GPT_PTB_CHK
    return;
#endif

    if (pttype != TYPE_PRI_PTB && pttype != TYPE_SUB_PTB) {
        ERROR("pttype:%d error!\n", pttype);
        return;
    }

    /* If global ptb, remove all */
    if (pttype == TYPE_PRI_PTB) {

        while ((ptb = list_remove_head_type(&current_dl_state->ptb_state_head,
                                            struct ptb_dl_name, node)) != NULL) {
            if (ptb->name) {
                free(ptb->name);
            }

            free(ptb);
        }
    }
    else {
        list_for_every_entry(&(current_dl_state->ptb_state_head), ptb,
                             struct ptb_dl_name, node) {
            if (ptb && !strcmp(ptb->name, ptb_name)) {
                list_delete(&ptb->node);
                free(ptb->name);
                free(ptb);
                break;
            }
        }
    }

    return;
}


static void set_dl_ptb_stage(void)
{
    char *ptb_name          = NULL;
    struct ptb_dl_name *ptb = NULL;
    uint32_t ptb_name_len   = 0;
    enum pt_type pttype     = current_dl_state->cur_pt_info.type;


#if DISABLE_GPT_PTB_CHK
    return;
#endif

    if (pttype == TYPE_PRI_PTB) {
        /* Primary partition table, stored name is "partition"*/
        ptb_name = current_dl_state->cur_pt_info.ptname;
    }
    else if (pttype == TYPE_SUB_PTB || pttype == TYPE_SUB_PT_WHOLE) {
        ptb_name = current_dl_state->cur_pt_info.sub_ptbname;
    }
    else {
        return;
    }

    list_for_every_entry(&(current_dl_state->ptb_state_head), ptb,
                         struct ptb_dl_name, node) {
        /* If the partition table has been flashed, skip it */
        if (!strcmp(ptb->name, ptb_name)) {
            return;
        }
    }

    ptb = (struct ptb_dl_name *)calloc(1, sizeof(struct ptb_dl_name));

    if (!ptb) {
        ERROR("memory allocate failed\n");
        return;
    }

    ptb_name_len = strlen(ptb_name);
    ptb->name = (char *)calloc(1, ptb_name_len + 1);

    if (!ptb->name) {
        ERROR("memory allocate failed\n");
        free(ptb);
        return;
    }

    strncpy(ptb->name, ptb_name, ptb_name_len);
    list_add_tail(&(current_dl_state->ptb_state_head), &(ptb->node));
}

static enum pt_type parse_pt_type(const char *sub_ptbname,
                                  const char *ptname)
{
    bool is_sfs, is_ptb, is_spl, is_whole;
#if 0
    bool is_safety, is_safety_bak;
#endif
    uint32_t sub_ptb_len = 0;
    uint32_t pt_len      = 0;

    pt_len      = strlen(ptname);
    sub_ptb_len = strlen(sub_ptbname);

    /* sub partition table name and ptname can not be null simultaneously */
    if (!sub_ptb_len && !pt_len) {
        return TYPE_PT_UNKNOWN;
    }

    is_whole      = !strcmp(ptname, "all");
    is_ptb        = !strcmp(ptname, "partition");
    is_sfs        = !strcmp(ptname, SFS_PT_NAME);
    is_spl        = !strcmp(ptname, SPL_PARTITION_NAME);

    if (is_ptb) {
        return (sub_ptb_len == 0) ? TYPE_PRI_PTB : TYPE_SUB_PTB;
    }
    else if (is_whole && sub_ptb_len == 0) {
        return TYPE_ALL_PT;
    }
    else if (is_spl && sub_ptb_len == 0) {
        return TYPE_SPL_PT;
    }
    else if (is_sfs && sub_ptb_len == 0) {
        return TYPE_SAFETY_SFS_PT;
    }
    else {
        if (sub_ptb_len == 0) {
            return TYPE_PRI_PT;
        }
        else if (pt_len == 0) {
            return TYPE_SUB_PT_WHOLE;
        }
        else {
            return TYPE_SUB_PT;
        }
    }

    return TYPE_PT_UNKNOWN;
}

static uint64_t get_nor_flash_ptb_offset(struct storage_device *storage,
        uint32_t block_size)
{
    uint32_t ret    = 0;
    uint8_t *buffer = NULL;
    uint32_t gpt_sz = 0;
    uint64_t ptb_offset = INVALID_PTB_OFFSET;
    uint64_t gpt_total_size     = 0;
    uint32_t blocks_for_entries = 0;
    uint64_t secondary_gpt_offset = 0;
    struct storage_info *st_info   = NULL;
    GPT_header secondary_gpt_header = {0};

    st_info = &(current_dl_state->st_info);
#ifdef NOR_FLASH_PTB_SECTOR_INDEX
    return st_info->erase_grp_sz * NOR_FLASH_PTB_SECTOR_INDEX;
#endif

    blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) /
                         block_size;
    /* Secondary gpt has no MBR.
     * */
    gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries) * block_size;

    DBG("gpt header size:%d blocksize:%d\n", gpt_sz,
        block_size);
    buffer = calloc(1, gpt_sz);

    if (!buffer) {
        ERROR("memory allocate fail!\n");
        goto end;
    }

    secondary_gpt_offset = storage->get_capacity(storage) - gpt_sz;
    ret = storage->read(storage, secondary_gpt_offset, buffer, gpt_sz);

    DBG("cap:0x%llx  gpt_sz:%d sec off:0x%llx\n",
        storage->get_capacity(storage), gpt_sz, secondary_gpt_offset);

    if (ret) {
        ERROR("read secondary gpt header fail offset:0x%llx gpt_sz:%d!\n",
              secondary_gpt_offset, gpt_sz);
        goto end;
    }

    ret = parse_gpt_table_from_buffer(buffer, gpt_sz, &secondary_gpt_header,
                                      block_size, true );

    if (ret) {
        ERROR("parse secondary gpt header fail!\n");
        goto end;
    }
    else {
        gpt_total_size =  secondary_gpt_header.current_lba -
                          secondary_gpt_header.backup_lba + 1 + 1;
        gpt_total_size *= block_size;
        ptb_offset = storage->get_capacity(storage) - gpt_total_size;

        /* for nor flash,
         * the offset of partition table must be aligned to erase group size
         * */
        if ((ptb_offset % st_info->erase_grp_sz) != 0) {
            ptb_offset = INVALID_PTB_OFFSET;
        }
    }

    DBG("ptb offset:0x%llx!\n", ptb_offset);
end:

    if (buffer) {
        free(buffer);
    }

    return ptb_offset;
}

static uint32_t parse_partition_name(const char *full_ptname)
{
    char *token1 = NULL;
    char *token2 = NULL;
    uint32_t token_num   = 0;
    uint32_t full_len    = 0;
    uint32_t sub_ptb_len = 0;
    uint32_t pt_len      = 0;
    struct storage_info *st_info   = NULL;
    struct storage_device *storage = NULL;
    char storage_name[MAX_GPT_NAME_SIZE + 1]  = {0};
    char sub_ptbname[MAX_GPT_NAME_SIZE + 1]   = {0};
    char short_ptname[MAX_GPT_NAME_SIZE + 1]  = {0};

    current_dl_state = NULL;

    if (!full_ptname
            || (full_len = strlen(full_ptname)) == 0) {
        return 1;
    }

    for (uint32_t i = 0; i < full_len; i++) {
        if (full_ptname[i] == '$') {
            token_num++;
        }
    }

    /* full name:xxx$yyy$zzz */
    if (token_num != 2) {
        return 2;
    }

    token1 = strchr(full_ptname, '$');
    token2 = strrchr(full_ptname, '$');

    strncpy(storage_name, full_ptname, token1 - full_ptname);
    strncpy(sub_ptbname, token1 + 1, token2 - token1 - 1);
    strncpy(short_ptname, token2 + 1, full_len - (token2 + 1 - full_ptname));

    DBG("storage_name:%s sub_ptb:%s sub_pt:%s\n", storage_name,
        sub_ptbname, short_ptname);

    for (uint32_t i = 0;
            i < sizeof(storage_dl_state) / sizeof(storage_dl_state[0]); i++) {
        if (!strcmp(storage_dl_state[i].st_info.storage_name, storage_name)) {
            current_dl_state = &storage_dl_state[i];
            st_info = &(current_dl_state->st_info);
            break;
        }
    }

    if (current_dl_state == NULL) {
        return 3;
    }

    if (!(current_dl_state->storage)) {
        current_dl_state->storage = setup_storage_dev(st_info->type,
                                    st_info->res_id, st_info->config);

        if (current_dl_state->storage) {
            st_info->block_size = current_dl_state->storage->get_block_size(
                                      current_dl_state->storage);
            st_info->erase_grp_sz = current_dl_state->storage->get_erase_group_size(
                                        current_dl_state->storage);

            ASSERT(st_info->block_size != 0);
            ASSERT(st_info->erase_grp_sz != 0);
            ASSERT(st_info->block_size % 4 == 0);
            ASSERT(st_info->erase_grp_sz % st_info->block_size == 0);
        }
        else {
            ERROR("get storage:%s error\n", storage_name);
            current_dl_state = NULL;
            return 4;
        }

        storage = current_dl_state->storage;

        if (st_info->type == OSPI) {
            st_info->ptb_offset = get_nor_flash_ptb_offset(storage,
                                  st_info->block_size);
        }
    }

    if (!(current_dl_state->ptdev)
            && st_info->ptb_offset != INVALID_PTB_OFFSET) {
        current_dl_state->ptdev = ptdev_setup(current_dl_state->storage,
                                              st_info->ptb_offset);

        if (current_dl_state->ptdev) {
            DBG("read partition table\n");
            ptdev_read_table(current_dl_state->ptdev);
        }
        else {
            ERROR("get ptdev:%s error\n", storage_name);
            current_dl_state = NULL;
            return 5;
        }
    }

    current_dl_state->cur_pt_info.type = parse_pt_type(sub_ptbname,
                                         short_ptname);

    if (current_dl_state->cur_pt_info.type == TYPE_PT_UNKNOWN) {
        current_dl_state = NULL;
        return 6;
    }

    pt_len = strlen(short_ptname);
    sub_ptb_len = strlen(sub_ptbname);

    memset(current_dl_state->cur_pt_info.sub_ptbname, 0x0,
           sizeof(current_dl_state->cur_pt_info.sub_ptbname));
    memset(current_dl_state->cur_pt_info.ptname, 0x0,
           sizeof(current_dl_state->cur_pt_info.ptname));
    strncpy(current_dl_state->cur_pt_info.sub_ptbname, sub_ptbname,
            sub_ptb_len);
    strncpy(current_dl_state->cur_pt_info.ptname, short_ptname, pt_len);

    return 0;
}

/* Primary partition table must be flashed in the first
 * Dloader must flash sub partition table before flashing a sub partition.
 * */
static uint32_t check_ptb_dl_state(fastboot_t *fb)
{
    struct ptb_dl_name *ptb = NULL;
    enum pt_type pttype = current_dl_state->cur_pt_info.type;

#if DISABLE_GPT_PTB_CHK
    return 0;
#endif

    if (pttype >= TYPE_NOT_IN_GPT) {
        return 0;
    }

    /* Primary partition table image must be the first to be flashed */
    if (list_is_empty(&(current_dl_state->ptb_state_head))
            && !(pttype == TYPE_PRI_PTB)) {
        fastboot_common_fail(fb, response_error(ERR_PRI_PTB_NOT_FLASH, ""));
        return 1;
    }

    /* It is not a sub partition */
    if (pttype != TYPE_SUB_PT) {
        return 0;
    }

    /* If not a sub partition table image or a whole sub partition image,
     * query sub partition table in state arrray
     */
    list_for_every_entry(&(current_dl_state->ptb_state_head), ptb,
                         struct ptb_dl_name, node) {
        if (!strcmp(ptb->name, current_dl_state->cur_pt_info.sub_ptbname)) {
            return 0;
        }
    }
    fastboot_common_fail(fb, response_error(ERR_SUB_PTB_NOT_FLASH,
                                            current_dl_state->cur_pt_info.sub_ptbname));
    return 2;
}

static void cmd_flash_proc(fastboot_t *fb, const char *arg, void *data,
                           unsigned sz)
{
    uint32_t ret = 0;
    uint8_t md5_calc[MD5_LEN] = {0};

    ret = parse_partition_name(arg);

    if (ret) {
        ERROR("partition name error:%d!\n", ret);
        fastboot_common_fail(fb, response_error(ERR_PT_FULL_NAME_FORMAT, arg));
        return;
    }

    ret = check_ptb_dl_state(fb);

    if (ret) {
        ERROR("check partition table state  error:%d!\n",
              ret);
        return;
    }

    DBG("calc md5 !\n");

    if (do_md5_check) {
        md5(data, sz, md5_calc);

        if (memcmp(md5_received, md5_calc, MD5_LEN)) {
            ERROR("md5 check fail!\n");
            hexdump8(md5_received, MD5_LEN);
            hexdump8(md5_calc, MD5_LEN);
            fastboot_common_fail(fb, response_error(ERR_HASH_FAIL, ""));
            return;
        }
    }

    cmd_flash_storage(fb, arg, data, sz);
}

static uint32_t erase_emmc_one_boot_area(fastboot_t *fb, uint32_t part_num)
{
    int ret = 0;
    const char *ptname   = NULL;
    enum dl_err_code err = ERR_UNKNOWN;
    unsigned long long ptn  = 0;
    unsigned long long size = 0;
    struct storage_info *st_info = NULL;
    struct storage_device *storage = NULL;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};

    if (part_num != PART_ACCESS_BOOT1
            && part_num != PART_ACCESS_BOOT2) {
        fastboot_common_fail(fb, response_error(err, full_ptname));
        ERROR("boot partition num error!\n");
        return 1;
    }

    ptname = current_dl_state->cur_pt_info.ptname;
    storage = current_dl_state->storage;
    st_info = &current_dl_state->st_info;

    snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);

    if (!st_info->boot_offset) {
        ret = storage->switch_part(storage, part_num);

        if (ret) {
            ERROR("switch partition error!\n");
            err = ERR_PT_ERASE_FAIL;
            goto fail;

        }
    }

    st_info = &current_dl_state->st_info;
    //size = storage->get_capacity(storage);
    size = EMMC_BOOT_PT_SIZE;
    ptn = st_info->boot_offset;

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (storage->erase(storage, ptn, round_up(size, st_info->block_size))) {
        ERROR(" erase data error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    return 0;
fail:
    fastboot_common_fail(fb, response_error(err, full_ptname));
    return 1;
}

static uint32_t erase_emmc_boot_areas(fastboot_t *fb)
{
    uint32_t ret = 0;
    struct storage_info *st_info = NULL;

    st_info = &current_dl_state->st_info;
    ret = erase_emmc_one_boot_area(fb, PART_ACCESS_BOOT1);

    /* The current storage is emmc card */
    if (!st_info->boot_offset)
        ret |= erase_emmc_one_boot_area(fb, PART_ACCESS_BOOT2);

    return ret;
}


static uint32_t erase_ospi_boot_areas(fastboot_t *fb)
{
    uint64_t ptn     = 0;
    uint64_t size    = 0;
    struct bpt *bpt  = NULL;
    uint8_t *buffer  = NULL;
    const char *ptname   = current_dl_state->cur_pt_info.ptname;
    enum dl_err_code err = ERR_UNKNOWN;
    enum pt_type  pttype = current_dl_state->cur_pt_info.type;
    struct storage_device *storage = NULL;
    struct storage_info *st_info   = NULL;

    storage = current_dl_state->storage;
    st_info = &current_dl_state->st_info;

    if (pttype != TYPE_SAFETY_SFS_PT && pttype != TYPE_ALL_PT) {
        err = ERR_PT_NOT_FOUND;
        ERROR("pttype:%d error!\n", pttype);
        goto fail;
    }

    ptn = 0;
    size = SFS_SIZE;

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (storage->erase(storage, ptn, round_up(size, st_info->erase_grp_sz))) {
        ERROR(" erase data error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    return 0;
fail:

    if (bpt) {
        free(bpt);
    }

    if (buffer) {
        free(buffer);
    }

    fastboot_common_fail(fb, response_error(err, ptname));
    return 0;
}

static uint32_t backup_boot(struct storage_device *storage)
{
    uint8_t *buffer = NULL;
    struct storage_info *st_info   = NULL;

    st_info = &current_dl_state->st_info;

    if (EMMC_BOOT_PT_SIZE > SPARSE_DATA_ALIGNED_SIZE) {
        return 1;
    }

    buffer = (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE);

    if (storage->read(storage, SD_SPL_OFFSET, buffer,
                      round_up(EMMC_BOOT_PT_SIZE, st_info->block_size))) {
        ERROR("read spl error!\n");
        return 2;
    }

    return 0;
}

static uint32_t restore_boot(struct storage_device *storage)
{
    uint8_t *buffer = NULL;
    struct storage_info *st_info   = NULL;

    st_info = &current_dl_state->st_info;

    if (EMMC_BOOT_PT_SIZE > SPARSE_DATA_ALIGNED_SIZE) {
        return 1;
    }

    buffer = (uint8_t *)_ioaddr(SPARSE_DATA_ALIGNED_BASE);

    if (storage->write(storage, SD_SPL_OFFSET, buffer,
                       round_up(EMMC_BOOT_PT_SIZE, st_info->block_size))) {
        ERROR("write spl error!\n");
        return 2;
    }

    return 0;
}

static uint32_t erase_gpt_tab(fastboot_t *fb)
{
    uint64_t gpt_sz = 0;
    uint64_t capacity    = 0;
    const char *ptname   = NULL;
    uint32_t block_size  = 0;
    enum pt_type pttype  = current_dl_state->cur_pt_info.type;
    enum dl_err_code err = ERR_UNKNOWN;
    uint32_t switch_pt_num  = PART_ACCESS_DEFAULT;
    unsigned long long ptn_pri  = 0;
    unsigned long long ptn_bak  = 0;
    const char *sub_ptbname = NULL;
    unsigned long long size_pri = 0;
    unsigned long long size_bak = 0;
    uint32_t blocks_for_entries    = 0;
    struct storage_info *st_info   = NULL;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = NULL;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};

    ptname = current_dl_state->cur_pt_info.ptname;
    sub_ptbname = current_dl_state->cur_pt_info.sub_ptbname;
    storage = current_dl_state->storage;
    ptdev = current_dl_state->ptdev;
    st_info = &current_dl_state->st_info;
    capacity = storage->get_capacity(storage);

    DBG("ptb_offset:0x%llx\n", st_info->ptb_offset);

    if (!ptdev) {
        ERROR("no gpt\n");
        return 0;
    }

    if (st_info->type == MMC && !st_info->boot_offset) {

        if (!storage->switch_part
                || storage->switch_part(storage, switch_pt_num)) {
            ERROR("switch partition:%d error!\n",
                  switch_pt_num);
            goto fail;
        }
    }

    DBG("get block size\n");
    block_size = st_info->block_size;

    if (!block_size || block_size % 4 != 0) {
        ERROR("block size:%d error!\n", block_size);
        goto fail;
    }

    /* MBR is in the first LBA.
     * GPT header is in the second LBA.
     * GPT entries are in the following blocks.
     * */
    blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
    gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries + 1) * block_size;
    size_pri = gpt_sz;

    /* If it is global partition table,
     * we need to erase the whole storage to clean sub gpt table data
     * */
#if 0

    if (pttype == TYPE_PRI_PTB) {
        size_pri = capacity - st_info->ptb_offset;

        /* sometimes,
         * the capacity is not an integer multiple of erase group size.
         * If round up size, some errors may occurs.
         * So, in the first, we erase the last erase group.
         * */
        if (size_pri % st_info->erase_grp_sz != 0) {
            ptn_pri = round_down(size_pri - st_info->erase_grp_sz,
                                 st_info->block_size);

            if (st_info->type == OSPI)
                ptn_pri = round_down(size_pri - st_info->erase_grp_sz,
                                     st_info->erase_grp_sz);

            storage->erase(storage, ptn_pri, st_info->erase_grp_sz);
        }

        size_pri = round_down(size_pri, st_info->erase_grp_sz);
    }
    else {

        if (st_info->type == OSPI) {
            size_pri = round_up(size_pri, st_info->erase_grp_sz);
        }
    }

#endif

    if (pttype == TYPE_SUB_PTB) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
        ptn_pri  = ptdev_get_offset(ptdev, full_ptname);
        size_bak = ptdev_get_size(ptdev, full_ptname);
        ptn_bak  = ptn_pri + size_bak - gpt_sz + st_info->block_size;
    }
    else {
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
        ptn_pri = st_info->ptb_offset;
        ptn_bak = storage->get_capacity(storage) - gpt_sz + st_info->block_size;
    }

    /* back up gpt header no mbr block*/
    size_bak = gpt_sz - st_info->block_size;

    if (st_info->type == MMC  ) {

        /* There is no boot area in sd card.
         * It needs to back up spl in the first.
         * */
        if (st_info->boot_offset > 0 && backup_boot(storage)) {
            ERROR("backup spl error!\n");
            goto fail;
        }
    }
    else if (st_info->type == OSPI  ) {
        size_pri = round_up(size_pri, st_info->erase_grp_sz);
        size_bak = round_up(size_bak, st_info->erase_grp_sz);
        ptn_bak = round_down(ptn_bak, st_info->erase_grp_sz);

        if ( ptn_pri % st_info->erase_grp_sz != 0) {
            ERROR("nor flash ptn must be aligned to erase group size!\n");
            goto fail;
        }
    }

    DBG("ptn_pri:0x%llx size_pri:0x%llx  ptn_bak:0x%llx  size_bak:0x%llx cap:0x%llx!\n",
        ptn_pri, size_pri, ptn_bak, size_bak, capacity);

    /* erase primary gpt header */
    if (storage->erase(storage, ptn_pri, size_pri)) {
        ERROR("erase primary gpt header error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    /* erase back up gpt header */
    if (storage->erase(storage, ptn_bak, size_bak)) {
        ERROR(" erase backup gpt header error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    if (st_info->type == MMC) {
        if (st_info->boot_offset > 0
                && restore_boot(storage)) {
            ERROR("restore spl error!\n");
            goto fail;
        }
    }

    remove_dl_ptb_state(pttype, full_ptname);
    return 0;
fail:
    fastboot_common_fail(fb, response_error(err, full_ptname));
    return 1;

}

static uint32_t erase_partition_in_gpt(fastboot_t *fb)
{
    const char *ptname  = NULL;
    uint32_t block_size = 0;
    enum pt_type pttype = current_dl_state->cur_pt_info.type;
    enum dl_err_code err    = ERR_UNKNOWN;
    uint32_t switch_pt_num  = PART_ACCESS_DEFAULT;
    unsigned long long ptn  = 0;
    unsigned long long size = 0;
    const char *sub_ptbname = NULL;
    struct storage_info *st_info   = NULL;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = NULL;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};

    st_info = &current_dl_state->st_info;
    ptdev = current_dl_state->ptdev;
    storage = current_dl_state->storage;
    block_size = st_info->block_size;

    DBG("ptb_offset:0x%llx\n", st_info->ptb_offset);

    /* For ospi nor flash, if ptdev is null,
     * it indicates there is no gpt in nor flash.
     * */
    if (!ptdev) {
        ERROR("ospi flash has no gpt\n");
        return 0;
    }

    if (st_info->type == MMC && !st_info->boot_offset) {
        if (!storage || storage->switch_part(storage, switch_pt_num)) {
            ERROR(" switch partition  error!\n");
            err = ERR_PT_ERASE_FAIL;
            goto fail;
        }
    }

    ptname = current_dl_state->cur_pt_info.ptname;
    sub_ptbname = current_dl_state->cur_pt_info.sub_ptbname;

    if (pttype == TYPE_SUB_PT_WHOLE) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
    }
    else if (pttype == TYPE_SUB_PT) {
        snprintf(full_ptname, sizeof(full_ptname), "%s$%s", sub_ptbname, ptname);
    }
    else {
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
    }

    ptn = ptdev_get_offset(ptdev, full_ptname);
    size = ptdev_get_size(ptdev, full_ptname);

    if (size < block_size || ptn == 0) {
        ERROR("partition not found ptn:%lld size:%lld! name:%s\n", ptn, size,
              full_ptname);
        err = ERR_PT_NOT_FOUND;
        goto fail;
    }

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (st_info->type == OSPI) {
        if (ptn % st_info->erase_grp_sz != 0
                || size % st_info->erase_grp_sz != 0) {
            ERROR("the size of the partition in nor flash is not aligned to erase group size\n");
            err = ERR_PT_OVERLAP;
            goto fail;
        }
    }

    if (storage->erase(storage, ptn, size)) {
        ERROR(" erase data error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    if (pttype == TYPE_SUB_PT_WHOLE) {
        remove_dl_ptb_state(TYPE_SUB_PTB, full_ptname);
    }

    return 0;
fail:
    fastboot_common_fail(fb, response_error(err, full_ptname));
    return 1;
}

static uint32_t erase_boot_areas(fastboot_t *fb)
{
    uint32_t ret = 0;
    struct storage_info *st_info = &current_dl_state->st_info;

    DBG("type:%d E\n", st_info->type);

    if (st_info->type == MMC) {
        ret = erase_emmc_boot_areas(fb);
    }
    else if (st_info->type == OSPI) {
        ret = erase_ospi_boot_areas(fb);
    }

    return ret;
}

static uint32_t erase_gpt_whole(fastboot_t *fb)
{
    uint64_t capacity    = 0;
    enum dl_err_code err = ERR_UNKNOWN;
    uint32_t switch_pt_num  = PART_ACCESS_DEFAULT;
    struct storage_info *st_info   = NULL;
    struct storage_device *storage = NULL;

    storage = current_dl_state->storage;
    st_info = &current_dl_state->st_info;

    if (st_info->type == MMC) {
        if (!st_info->boot_offset
                && storage->switch_part(storage, switch_pt_num)) {
            err = ERR_PT_ERASE_FAIL;
            ERROR("switch partition fail\n");
            goto fail;
        }

        /* There is no boot area in sd card.
         * It needs to back up spl in the first.
         * */
        if (st_info->boot_offset > 0 && backup_boot(storage)) {
            ERROR("backup spl error!\n");
            goto fail;
        }
    }

    capacity = storage->get_capacity(storage);
    capacity -= st_info->ptb_offset;

    if (storage->erase(storage, st_info->ptb_offset, capacity)) {
        ERROR(" erase data error!\n");
        err = ERR_PT_ERASE_FAIL;
        goto fail;
    }

    if (st_info->type == MMC) {
        if (st_info->boot_offset > 0
                && restore_boot(storage)) {
            ERROR("restore spl error!\n");
            goto fail;
        }
    }

    fastboot_common_okay(fb, "");
    return 0;
fail:
    fastboot_common_fail(fb, response_error(err, ""));
    return 1;
}

static void erase_data(fastboot_t *fb, const char *arg, void *data,
                       unsigned sz)
{
    uint32_t ret = 0;
    enum pt_type pttype = current_dl_state->cur_pt_info.type;

    DBG("pt type:%d\n", pttype);

    if (pttype == TYPE_PRI_PTB || pttype == TYPE_SUB_PTB) {
        ret = erase_gpt_tab(fb);
    }
    else if (pttype == TYPE_ALL_PT) {
        ret = erase_boot_areas(fb);
        ret = erase_gpt_whole(fb);
        ptdev_read_table(current_dl_state->ptdev);
    }
    else if (pttype > TYPE_NOT_IN_GPT) {
        ret = erase_boot_areas(fb);
    }
    else {
        ret = erase_partition_in_gpt(fb);
        /* update partition table */
        ptdev_read_table(current_dl_state->ptdev);
    }

    if (ret) {
        ERROR("erase partition error!\n");
        return;
    }

    fastboot_common_okay(fb, "");
    return;
}

static void cmd_erase_proc(fastboot_t *fb, const char *arg, void *data,
                           unsigned sz)
{
    int ret = 0;

    DBG("E\n");

    ret = parse_partition_name(arg);

    if (ret) {
        ERROR("parse pt name error!\n");
        fastboot_common_fail(fb, response_error(ERR_PT_FULL_NAME_FORMAT, arg));
        return;
    }

    erase_data(fb, arg, data, sz);
}

static void cmd_reboot_proc(fastboot_t *fb, const char *arg, void *data,
                            unsigned sz)
{
    uint32_t boot_reason = HALT_REASON_SW_RESET;
    printf("reboot device\n");
    fastboot_common_okay(fb, "");

    thread_sleep(1000);
    sdrv_common_reg_set_u32(boot_reason, SDRV_REG_BOOTREASON);
    //reboot_device(0);
}

static void cmd_md5_proc(fastboot_t *fb, const char *arg, void *data,
                         unsigned sz)
{
    char  md5_received_str[MD5_LEN * 2 + 1] = {0};

    memset(md5_received, 0x0, MD5_LEN);
    memcpy(md5_received_str, arg, MD5_LEN * 2);
    fastboot_common_okay(fb, "");

    str2hex(md5_received_str, MD5_LEN * 2, md5_received, MD5_LEN);
#if !NO_DDR
    do_md5_check = true;
#endif
}

static void cmd_read_efuse_proc(fastboot_t *fb, const char *arg,
                                void *data,
                                unsigned sz)
{
    uint32_t fuse_val   = 0;
    uint32_t fuse_index = 0;
    char fuse_response[MAX_RSP_SIZE] = {0};

    DBG("arg:%s!\n", arg);
    fuse_index = strtoul(arg, NULL, 16);

    if (errno != 0 || fuse_index > FUSE_INDEX_MAX) {
        ERROR("index error:%d!\n", errno);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_INDEX, ""));
        return;
    }

    fuse_val = fuse_read(fuse_index);
    snprintf(fuse_response, sizeof(fuse_response), "eFuseRead:%08x:%08x",
             fuse_index, fuse_val);
    fastboot_common_okay(fb, fuse_response);
    return;

}

static uint32_t get_efuse_args(const char *arg, uint32_t *index,
                               uint32_t *val)
{
    uint32_t arg_len = 0;
    const char token = ':';
    uint32_t val_str_len = 0;
    const char *val_pos  = NULL;
    const char *md5_pos  = NULL;
    uint32_t md5_str_len   = 0;
    uint8_t md5_r[MD5_LEN] = {0};
    uint32_t index_str_len    = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    char val_str[UINT32_HEX_STR_LEN + 1]   = {0};
    char index_str[UINT32_HEX_STR_LEN + 1] = {0};

    val_pos = strchr(arg, token);
    md5_pos = strrchr(arg, token);

    if (!val_pos || val_pos == md5_pos) {
        ERROR("arg error:%s!\n", arg);
        return 1;
    }

    arg_len = strlen(arg);
    index_str_len = val_pos - arg;
    val_str_len = md5_pos - val_pos - 1;
    md5_str_len = arg_len - index_str_len - val_str_len - 2;

    if ( index_str_len > UINT32_HEX_STR_LEN
            || val_str_len > UINT32_HEX_STR_LEN
            || md5_str_len != MD5_LEN * 2) {
        ERROR("arg len error:%s!\n", arg);
        return 2;
    }

    str2hex(md5_pos + 1, md5_str_len, md5_r, MD5_LEN);
    md5((const unsigned char *)arg, val_str_len + index_str_len + 1, md5_calc);

    if (memcmp(md5_calc, md5_r, MD5_LEN)) {
        ERROR("md5 check fail!\n");
        hexdump8(md5_r, MD5_LEN);
        hexdump8(md5_calc, MD5_LEN);
        return 3;
    }

    strncpy(index_str, arg, index_str_len);
    strncpy(val_str, val_pos + 1, val_str_len);

    *index = strtoul(index_str, NULL, 16);

    if (errno || *index > FUSE_INDEX_MAX) {
        ERROR("strtoul error:%d!\n", errno);
        return 4;
    }

    *val = strtoul(val_str, NULL, 16);

    if (errno) {
        ERROR("strtoul error:%d!\n", errno);
        return 5;
    }

    return 0;
}

static void cmd_program_efuse_proc(fastboot_t *fb, const char *arg,
                                   void *data,
                                   unsigned sz)
{
    uint32_t ret = 0;
    uint32_t fuse_val   = 0;
    uint32_t fuse_index = 0;

    ret = get_efuse_args(arg, &fuse_index, &fuse_val);

    if (ret) {
        ERROR("index ret:%d!\n", ret);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_INDEX, ""));
        return;
    }

    DBG("fuse index:0x%08x  val:0x%08x\n",
        fuse_index, fuse_val);

    ret = fuse_program(fuse_index, fuse_val);

    if (ret) {
        ERROR("burn efuse fail:%d!\n", ret);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_BURN, ""));
        return;
    }

    fastboot_common_okay(fb, "");
    return;
}

static const char *get_serialno(void)
{
    return "0123456789";
}

static const char *get_product(void)
{
    return "x9";
}

static void register_commands(void)
{
    static char max_download_size[MAX_RSP_SIZE] = {0};
    struct fastboot_cmd_desc cmd_list[] = {
        /* Register the following commands only for non-user builds */
        {"flash:", cmd_flash_proc},
        {"erase:", cmd_erase_proc},
        {"reboot", cmd_reboot_proc},
        {"md5:",    cmd_md5_proc},
        {"eFuseRead:", cmd_read_efuse_proc},
        {"eFuseProgram:", cmd_program_efuse_proc},
    };

    int fastboot_cmds_count = sizeof(cmd_list) / sizeof(cmd_list[0]);

    for (int i = 0; i < fastboot_cmds_count; i++)
        fastboot_register_cmd(cmd_list[i].name, cmd_list[i].cb);

    /* publish variables and their values */
    fastboot_register_var("product",  get_product());
    fastboot_register_var("serialno", get_serialno());
    fastboot_register_var("dloader-version", DLOADER_VERSION);
    fastboot_register_var("dev-stage", "dloader");

    /* Max download size supported */
    snprintf(max_download_size, MAX_RSP_SIZE, "\t0x%x", DL_BUF_SIZE);
    fastboot_register_var("max-download-size",
                          (const char *) max_download_size);
}

static int port_init(void)
{
    bool ret;
    void *handle;

    ret = hal_port_creat_handle(&handle, RES_PAD_CONTROL_SEC_TMR3_CH0);

    if (!ret) {
        dprintf(CRITICAL, "hal_port_creat_handle fail.\n");
        return -1;
    }

    ret = hal_port_init(handle);
    hal_port_release_handle(handle);

    if (!ret) {
        dprintf(CRITICAL, "hal_port_init fail.\n");
        return -1;
    }

    dprintf(CRITICAL, "port init ok.\n");
    return 0;
}

static void erase_nor_flash_sfs(uint32_t resid)
{
    struct storage_device *storage;
    uint32_t erase_grp_sz = 0;

    storage = setup_storage_dev(OSPI, resid, &ospi_cfg);

    if (!storage)
        return;

    erase_grp_sz = storage->get_erase_group_size(storage);

    if (!storage->erase(storage, 0, round_up(SFS_SIZE, erase_grp_sz))) {
        printf("erase sfs successfully!\n");
    }

    storage->release(storage);
}

#if DLOADER_USE_SPI
static int spi_to_normal(uint32_t spi_res_id)
{
    //int ret = 0;
    void *spi_handle = NULL;
    spidev_t spidev;

	if (!hal_spi_creat_handle(&spi_handle, spi_res_id)) {	
        ERROR(" %s, get spi handle fail\n", __func__);
        return -1;
    }

    spidev.slave_num = 0;
    spidev.speed_hz = 10000000;
    spidev.bit_mode = SPI_MODE_0;
    /* bits_per_word = 8,poll_mode = 0 */
    spidev.bits_per_word = 8;
    spidev.poll_mode = 0;
    uint8_t buf1 = 0xff;
    if (hal_spi_init(spi_handle, &spidev) != 0) {
        ERROR("%s, init fail\n", __func__);
        return -1;
    }
	

    if (hal_spi_read(spi_handle, (void *)(&buf1),  sizeof(buf1)) != 0) {
        ERROR("%s, read fail\n", __func__);
        return -1;
    }
	

    if (hal_spi_write(spi_handle, (void *)(&buf1),  sizeof(buf1)) != 0) {
        ERROR("%s, write fail\n", __func__);
        return -1;
    }


    printf("%s, int mode buf1 = %u\n", __func__, buf1);
    /* bits_per_word = 8,poll_mode = 1 */
    spidev.bits_per_word = 8;
    spidev.poll_mode = 1;
    buf1 = 0xf0;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        ERROR("%s, init fail\n", __func__);
        return -1;
    }

    hal_spi_write(spi_handle, (void *)(&buf1),  sizeof(buf1));
    hal_spi_read(spi_handle, (void *)(&buf1),  sizeof(buf1));
    ERROR(" %s, poll mode buf1 = %u\n", __func__, buf1);
    /* bits_per_word = 16,poll_mode = 0 */
    spidev.bits_per_word = 16;
    spidev.poll_mode = 0;
    uint16_t buf2 = 0xffff;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        ERROR("%s, init fail\n", __func__);
        return -1;
    }
    printf("%s, write \n", __func__);
    if (hal_spi_write(spi_handle, (void *)(&buf2),  sizeof(buf2)) != 0) {
        ERROR("%s, write fail\n", __func__);
        return -1;
    }
     printf("%s, read \n", __func__);
    if (hal_spi_read(spi_handle, (void *)(&buf2),  sizeof(buf2)) != 0) {
        ERROR("%s, read fail\n", __func__);
        return -1;
    }

    printf("%s, int mode buf2 = %u\n", __func__, buf2);
    /* bits_per_word = 16,poll_mode = 1 */
    spidev.bits_per_word = 16;
    spidev.poll_mode = 1;
    buf2 = 0xf0f0;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        ERROR("%s, init fail\n", __func__);
        return -1;
    }

    hal_spi_write(spi_handle, (void *)(&buf2),  sizeof(buf2));
    hal_spi_read(spi_handle, (void *)(&buf2),  sizeof(buf2));
    printf("%s, poll mode buf2 = %u\n", __func__, buf2);
    /*
     *bits_per_word = 16,poll_mode = 1,check err condition
     *len should times of [bits_per_word / 8]
    */
    uint8_t buf3[3] = {0xff, 0xff, 0xff};

    if (hal_spi_write(spi_handle, (void *)(&buf3),
                      sizeof(buf3) / sizeof(buf3[0])) != 0) {
        ERROR("%s, check wrong write\n", __func__);
    }
    else {
        ERROR("%s, check wrong write fail\n", __func__);
        return -1;
    }

    printf("%s, all function ok\n", __func__);
    hal_spi_release_handle(spi_handle);
    return 0;
}
#endif
int dloader_main(int argc, const cmd_args *argv)
{
    struct storage_dl_state *dl_state = NULL;
    vaddr_t dl_base;

    DBG("E\n");
    current_dl_state = NULL;

    for (uint32_t i = 0;
            i < sizeof(storage_dl_state) / sizeof(storage_dl_state[0]); i++) {
        dl_state = &storage_dl_state[i];
        list_initialize(&dl_state->ptb_state_head);
    }

    if (sdrv_common_reg_get_value(SDRV_REG_BOOTREASON,
                                  0xFU) == HALT_REASON_SW_UPDATE) {
        while (!sdrv_common_reg_get_value(SDRV_REG_STATUS,
                                          SDRV_REG_STATUS_HANDOVER_DONE)) {
            thread_sleep(100);
            printf("dloader wait for ospi handover done!\n");
        };
    }

    do_md5_check = false;
    port_init();
	
	#if DLOADER_USE_SPI	
	spi_to_normal(RES_SPI_SPI2);
	#endif
	
    register_commands();
#if BOOT_TYPE_EMMC
    erase_nor_flash_sfs(RES_OSPI_REG_OSPI1);
#endif
    dl_base = (vaddr_t)_ioaddr(DL_BUF_BASE);
    fb_data = fastboot_common_init((void *)dl_base, DL_BUF_SIZE);

    return 0;
}

static void dloader_entry(const struct app_descriptor *app, void *args)
{
    dloader_main(0, NULL);
}

int dloader_erase(int argc, const cmd_args *argv)
{
    lk_bigtime_t read_time;
    struct partition_device *ptdev = current_dl_state->ptdev;
#if 1
    struct storage_device *storage = NULL;
    struct storage_info *st_info   = NULL;
    uint64_t erase_start;
    uint64_t erase_start_orig;
    uint32_t erase_len;
    uint32_t erase_len_orig;
    uint32_t erase_grp_sz;
    uint8_t buffer[512] = {0};
    st_info = &current_dl_state->st_info;
    storage = current_dl_state->storage;

    erase_start = atoull(argv[1].str);
    erase_len = strtoul(argv[2].str, NULL, 10);
    erase_start_orig = erase_start;
    erase_len_orig = erase_len;
    erase_grp_sz = storage->get_erase_group_size(storage);

    if (st_info->type == OSPI) {
        erase_start = round_down(erase_start, erase_grp_sz);
        erase_len = round_up(erase_len, erase_grp_sz);
    }

    dprintf(0, "%s start:%llu len:%u erase grp sz:%u\n", __func__, erase_start,
            erase_len, erase_grp_sz);
#if 1
    storage->read(storage, erase_start_orig, buffer, erase_len_orig);
    hexdump8(buffer, 64);

    if (storage->erase(storage, erase_start, erase_len))
        dprintf(0, "%s fail to erase\n", __func__);

#endif
    dprintf(0, "%s after erase\n", __func__);
    storage->read(storage, erase_start_orig, buffer, erase_len_orig);
    hexdump8(buffer, 64);
#endif
    dprintf(0, "%s re-read gpt\n", __func__);
#if 1
    read_time = current_time_hires();
    ptdev_read_table(ptdev);
    read_time = current_time_hires() - read_time;
    dprintf(0, "%s re-read time:%llu\n", __func__, read_time);
#endif
    return 0;
}

int dloader_read(int argc, const cmd_args *argv)
{
    uint64_t ptn    = 0;
    uint32_t r_len  = 0;
    uint8_t *data   = NULL;
    uint32_t remain = 0;
    const char *full_name     = NULL;
    uint32_t read_back_addr   = 0;
    uint32_t read_chunk_size  = 0;
    uint32_t read_total_len   = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    struct storage_info *st_info   = NULL;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = NULL;

    st_info = &current_dl_state->st_info;

    read_back_addr = strtoul(argv[1].str, NULL, 16);
    read_chunk_size = strtoul(argv[2].str, NULL, 16);
    read_total_len = strtoul(argv[3].str, NULL, 16);
    full_name = argv[4].str;

    ERROR("add:0x%0x, chunk:0x%0x, len:0x%0x\n",
          read_back_addr, read_chunk_size, read_total_len);

    ptdev = current_dl_state->ptdev;
    storage = current_dl_state->storage;

    ptn = ptdev_get_offset(ptdev, full_name);
    data = (uint8_t *)read_back_addr;
    remain = round_up(read_total_len, st_info->block_size);

    while (remain) {

        if (remain > read_chunk_size) {
            r_len = read_chunk_size;
        }
        else {
            r_len = remain;
        }

        spin(1000000);
        memset(data, 0x0, r_len);

        if (storage->read(storage, ptn, data,
                          round_up(r_len, st_info->block_size))) {
            ERROR("read back error!\n");
        }
        else {
            ERROR("read back success!\n");
            md5((unsigned char *)data, r_len, (unsigned char *)md5_calc);
            hexdump8(md5_calc, MD5_LEN);
        }

        data += r_len;
        ptn += r_len;
        remain -= r_len;
    }

    return 0;
}

int dloader_init_emmc(int argc, const cmd_args *argv)
{
    struct storage_info *st_info = NULL;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = NULL;

    current_dl_state = &storage_dl_state[2];

    storage = setup_storage_dev(MMC, RES_MSHC_SD1, &mmc_cfg);

    if (!storage) {
        ERROR("storage get error!\n");
        return -1;
    }

    ptdev = ptdev_setup(storage, 0);

    if (!ptdev) {
        ERROR("ptdev get error!\n");
        return -1;
    }

    current_dl_state->storage = storage;
    current_dl_state->ptdev = ptdev;

    st_info = &current_dl_state->st_info;
    st_info->block_size = storage->get_block_size(storage);
    ptdev_read_table(ptdev);
    return 0;
}

int dloader_md5_emmc(int argc, const cmd_args *argv)
{
    uint32_t len = 0;
    uint32_t start_addr = 0;
    uint8_t md5_calc[MD5_LEN] = {0};

    start_addr = strtoul(argv[1].str, NULL, 16);
    len = strtoul(argv[2].str, NULL, 16);

    ERROR("start:0x%0x, len:0x%0x\n", start_addr, len);
    md5((unsigned char *)start_addr, len, md5_calc );
    hexdump8  (md5_calc, MD5_LEN);
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("dloader", "Secure R5 dloader", (console_cmd)&dloader_main)
STATIC_COMMAND("read_emmc", "dloader read emmc",
               (console_cmd)&dloader_read)
STATIC_COMMAND("erase_storage", "dloader erase storage",
               (console_cmd)&dloader_erase)
STATIC_COMMAND("init_emmc", "dloader read emmc",
               (console_cmd)&dloader_init_emmc)
STATIC_COMMAND("md5_emmc", "dloader md5 emmc",
               (console_cmd)&dloader_md5_emmc)
STATIC_COMMAND_END(dloader);

#endif

APP_START(dloader)
.flags = 0,
.entry = dloader_entry,
APP_END
