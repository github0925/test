/*
* sms_storage.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* -----------------
*/
#include "sms_storage.h"

#define OK   0
#define FAILED   -1

#define MAX_SIZE  5242880
#define DEBUG_ON 1
#define PAR_NAME "avm"
#define ERASE_SIZE (10*st_info->erase_grp_sz)
#define WRITE_SIZE (10*st_info->block_size)

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

unsigned long long avm_ptn  = 0;
unsigned long long avm_size = 0;
uint32_t img_len = 0;
uint32_t avm_block_Size = 0;

/* record downloaded state */
struct storage_dl_state {
    struct list_node ptb_state_head;
    struct storage_device *storage;
    struct partition_device *ptdev;
    struct storage_info st_info;
    struct pt_name_info cur_pt_info;
};



struct spi_nor_cfg ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 1,
};


struct storage_dl_state *current_dl_state;
struct storage_dl_state storage_dl_state[] = {
    [0] = {
        .st_info = {
            .storage_name   = OSPI1_STORAGE_NAME,
            .res_id         = RES_OSPI_REG_OSPI1,
            .type           = OSPI,
            .config         = &ospi_cfg,
        }
    }
};


uint64_t get_avm_ptb_offset(struct storage_device *storage,
                            uint32_t block_size)
{
    struct storage_info *st_info   = NULL;
    st_info = &(current_dl_state->st_info);
    st_info->erase_grp_sz = storage->get_erase_group_size(storage);

    DBG("st_info->erase_grp_sz size:%d NOR_FLASH_PTB_SECTOR_INDEX:%d\n",
        st_info->erase_grp_sz, NOR_FLASH_PTB_SECTOR_INDEX);
    return st_info->erase_grp_sz * NOR_FLASH_PTB_SECTOR_INDEX;
}


bool erase_avm_partition(struct partition_device *ptdev,
                         struct storage_info *st_info,
                         const char *full_ptname, uint64_t img_sz)
{
    struct storage_device *storage;
    uint64_t erase_size;

    if (!ptdev || !ptdev->storage
            || !st_info || !img_sz)
        return false;

    storage = ptdev->storage;

    if (!avm_ptn || !avm_size) {
        ERROR("ptn or size is 0. name:%s\n", PAR_NAME);
        return false;
    }

    erase_size = avm_size;
    uint32_t count = erase_size / ERASE_SIZE;
    uint32_t yus = erase_size % ERASE_SIZE;

    for (uint32_t i = 0; i < count; i++) {
        if (storage->erase(storage, avm_ptn + ERASE_SIZE * i, ERASE_SIZE)) {
            ERROR("flash avm error\n");
            return false;
        }

        thread_sleep(3U);
    }

    if (yus > 0) {
        if (storage->erase(storage, avm_ptn + ERASE_SIZE * count, yus)) {
            ERROR("flash avm error\n");
            return false;
        }
    }

    DBG("erase_size:0x%llx size:0x%llx group:0x%x name:%s",
        erase_size, avm_size, st_info->erase_grp_sz, PAR_NAME);
    return true;
}

bool make_fake_avb_footer(uint64_t image_sz, AvbFooter *footer)
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


bool padding_avb_footer(struct storage_device *storage,
                        uint64_t ptn, uint64_t pt_size,
                        uint64_t image_sz, uint8_t *scratch_buf, uint32_t block_size)
{
    AvbFooter footer;
    uint64_t  last_block;
    uint32_t footer_offset;

    /* if image is signed, pt_size is equal to image_sz */
    if (!storage || pt_size - image_sz < sizeof(AvbFooter)) {
        ERROR("no enough space to padding footer:%llu\n", pt_size - image_sz);
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

uint32_t flash_avm_partition(void *data, unsigned sz)
{
    DBG("\n flash avm start \n");
    struct storage_info *st_info = NULL;
    struct storage_device *storage = current_dl_state->storage;
    struct partition_device *ptdev = current_dl_state->ptdev;

    st_info = &current_dl_state->st_info;

    if (st_info->type == OSPI) {
        if (avm_ptn % st_info->erase_grp_sz != 0
                || round_up(sz, st_info->erase_grp_sz) > avm_size) {
            ERROR("the size of partition in nor flash is too large\n");
            return FAILED;
        }

        if (!erase_avm_partition(ptdev, st_info, PAR_NAME, sz)) {
            ERROR("erase partition fail\n");
            return FAILED;
        }
    }

    if (round_up(sz, st_info->block_size) > avm_size) {
        ERROR(" image too large:%llu!\n",
              round_up(sz, st_info->block_size));
        return FAILED;
    }

    uint32_t ds = round_up(sz, st_info->block_size);
    uint32_t count = ds / ERASE_SIZE;
    uint32_t yus = ds % ERASE_SIZE;

    for (uint32_t i = 0; i < count; i++) {
        if (!storage
                || storage->write(storage, avm_ptn + i * ERASE_SIZE,
                                  (uint8_t *)data + i * ERASE_SIZE, ERASE_SIZE)) {
            ERROR(" write data error!\n");
            return FAILED;
        }

        thread_sleep(3U);
    }

    if (yus > 0) {
        if (!storage
                || storage->write(storage, avm_ptn + count * ERASE_SIZE,
                                  (uint8_t *)data + count * ERASE_SIZE, yus)) {
            ERROR(" write data error!\n");
            return FAILED;
        }
    }

    uint8_t *scratch_buf = (uint8_t *)malloc(avm_block_Size);

    if (!scratch_buf) {
        ERROR(" malloc scratch_buf error!\n");
        return FAILED;
    }

    padding_avb_footer(storage, avm_ptn, avm_size, sz, scratch_buf,
                       st_info->block_size);
    DBG("flash avm success \n");
    free(scratch_buf);
    return OK;
}

int deinit_avm_storage(void )
{
    if (current_dl_state && current_dl_state->storage) {
        ptdev_destroy(current_dl_state->ptdev);
        storage_dev_destroy(current_dl_state->storage);
        avm_ptn = 0;
    }

    return OK;
}

uint32_t get_avm_img_size(void )
{
    if (avm_ptn <= 0 || avm_size <= 0 || avm_block_Size <= 0) {
        ERROR(" avm_ptn or avm_size is 0!\n");
        return FAILED;
    }

    struct storage_device *storage = current_dl_state->storage;

    AvbFooter *footer = (struct AvbFooter *)malloc(sizeof(struct AvbFooter));

    AvbFooter *dfooter = (struct AvbFooter *)malloc(sizeof(struct AvbFooter));

    uint8_t *fotchar = (uint8_t *)malloc(avm_block_Size);

    if (storage->read(storage, avm_ptn + avm_size - avm_block_Size, fotchar,
                      avm_block_Size)) {
        ERROR("read back error!\n");
        return FAILED;
    }

    memcpy(footer, fotchar + (avm_block_Size - sizeof(struct AvbFooter)),
           sizeof(struct AvbFooter));
    bool ret = avb_footer_validate_and_byteswap(footer, dfooter);

    if (ret) {
        img_len = dfooter->original_image_size;
    }
    else {
        img_len = 0;
    }

    return img_len;
}

uint32_t get_avm_img_state(void )
{
    if (avm_ptn <= 0 || avm_size <= 0 || avm_block_Size <= 0) {
        ERROR(" avm_ptn or avm_size is 0!\n");
        return FAILED;
    }

    uint8_t *fotchar = (uint8_t *)malloc(avm_block_Size);

    if (current_dl_state && current_dl_state->storage
            && current_dl_state->storage->read(current_dl_state->storage, avm_ptn, fotchar,
                    avm_block_Size)) {
        ERROR("get data error!\n");
        free(fotchar);
        return FAILED;
    }

    uint32_t len = (fotchar[3] & 0xFF)
                   | ((fotchar[2] << 8) & 0xFF00)
                   | ((fotchar[1] << 16) & 0xFF0000)
                   | ((fotchar[0] << 24) & 0xFF000000);

    DBG("len %d mlen %d\n", len, get_avm_img_size());

    if (get_avm_img_size() == len + 4) {
        DBG("state :%d\n", OK);
        return OK;
    }

    DBG("state :%d\n", FAILED);
    return FAILED;
}


uint32_t read_avm_partition(uint8_t *data )
{
    if (img_len > 0 && img_len < MAX_SIZE && current_dl_state &&
            current_dl_state->storage
            && current_dl_state->storage->read(current_dl_state->storage, avm_ptn + 4, data,
                    img_len)) {
        ERROR("read data error!\n");
        return FAILED;
    }

    return OK;
}

uint32_t get_avm_block_size(void)
{
    return avm_block_Size;
}

int init_avm_storage(void )
{
    struct storage_info *st_info = NULL;
    struct partition_device *ptdev = NULL;
    struct storage_device *storage = NULL;
    DBG("\n init storage start\n");
    storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1, &ospi_cfg);

    if (!storage) {
        ERROR("storage2 get error!\n");
        return FAILED;
    }

    current_dl_state = &storage_dl_state[0];
    /*uint8_t fotchar[512] __attribute__  ((aligned(512)));
    if(storage->read(storage,0,fotchar,512)){
    }
    uint8_t *scr = (uint8_t *)_ioaddr(0x4000000);
    hexdump(fotchar,512);
    hexdump(scr,512);*/
    current_dl_state->storage = storage;
    snprintf(current_dl_state->cur_pt_info.sub_ptbname,
             sizeof(current_dl_state->cur_pt_info.sub_ptbname), "%s", PAR_NAME);
    current_dl_state->cur_pt_info.type = TYPE_SUB_PT_WHOLE;

    st_info = &current_dl_state->st_info;
    st_info->block_size = storage->get_block_size(storage);
    avm_block_Size = st_info->block_size;

    //DBG("storage block_size %d\n",st_info->block_size);
    if (st_info->type == OSPI) {
        st_info->ptb_offset = get_avm_ptb_offset(storage, st_info->block_size);
    }

    ptdev = ptdev_setup(storage, st_info->ptb_offset);

    if (!ptdev) {
        ERROR("ptdev get error!\n");
        return FAILED;
    }

    current_dl_state->ptdev = ptdev;

    ptdev_read_table(ptdev);
    avm_ptn = ptdev_get_offset(ptdev, PAR_NAME);
    //DBG("init_avm_storage full_ptname :%s\n", PAR_NAME);

    if (avm_ptn <= 0) {
        ERROR("init_avm_storage ptn is error!\n");
        return FAILED;
    }

    avm_size = ptdev_get_size(ptdev, PAR_NAME);

    if (avm_size <= 0) {
        ERROR("init_avm_storage SIZE is error!\n");
        return FAILED;
    }

    DBG("\n init storage done ptn:%lld size:%lld\n", avm_ptn, avm_size);
    return OK;
}

