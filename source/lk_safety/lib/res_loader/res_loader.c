#include <assert.h>
#include <string.h>
#include <heap.h>
#include <ff.h>
#include <platform.h>
#include <lk/init.h>
#include "res_loader.h"

#if VERIFIED_BOOT
#include "image_cfg.h"
#include "lib/reg.h"
#include "partition_parser.h"
#include "spi_nor_hal.h"
#include "storage_device.h"
#include "verified_boot.h"
#endif

#define FPATH_LEN   128
#define RES_IMAGE_NAME "res"
#define ASSERT_OR_VERIFY(x)          \
    if (!(x)){                        \
        if (DISKD == 7)              \
            goto do_verify;          \
        else                         \
            ASSERT(false); \
    }

static  FATFS fsd;

static int verified_res_image(void)
{
#if 0
//#if VERIFIED_BOOT
    bool ret = false;
    uint64_t res_offset, res_size = MEMDISK_SIZE;
    uint8_t * res_buf = (uint8_t *)_ioaddr(MEMDISK_BASE);
    uint8_t *vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE);
    uint32_t vbmeta_sz = VBMETA_MEMSIZE;
    struct list_node verified_images_list;
    partition_device_t  *ptdev = NULL;
    storage_device_t *storage = NULL;
    addr_t flash_start;
    int32_t dummy;
    struct spi_nor_cfg ospi_cfg = {
        .cs = SPI_NOR_CS0,
        .bus_clk = SPI_NOR_CLK_25MHZ,
        .octal_ddr_en = 1,
    };

    list_initialize(&verified_images_list);

    storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1, &ospi_cfg);
    ASSERT_OR_VERIFY(storage);

    dummy = res_get_info_by_id(RES_OSPI_OSPI1, &flash_start, &dummy);
    ASSERT_OR_VERIFY(dummy >= 0);

    ptdev = ptdev_setup(storage, storage->get_erase_group_size(storage) * 2);
    ASSERT_OR_VERIFY(ptdev);
    ptdev_read_table(ptdev);

    res_offset = ptdev_get_offset(ptdev, RES_IMAGE_NAME);
    res_size = ptdev_get_size(ptdev, RES_IMAGE_NAME);
    if (!res_size)
    {
        dprintf(CRITICAL, "%s %d cann't get res partition size!\n", __func__, __LINE__);
        return -1;
    }

#if 1
    res_buf = (uint8_t *)_ioaddr(flash_start + res_offset);
#else
    //res_buf = memalign(storage->get_block_size(storage), res_size);
    res_buf = (uint8_t *)_ioaddr(MEMBASE + 0x4000000);
    printf("%s buf:%p\n", __func__, res_buf);
    storage->read(storage, res_offset, res_buf, res_size);
#endif
do_verify:
    ret = add_verified_image_list(&verified_images_list,
                                  res_buf, res_size, RES_IMAGE_NAME);
    ASSERT(ret);

    ret = add_verified_image_list(&verified_images_list,
                                  vbmeta_buf, vbmeta_sz, VBMETA_PARTITION_NAME);
    ASSERT(ret);

    if (!verify_loaded_images(ptdev, &verified_images_list, NULL)) {
            dprintf(ALWAYS, "%s %d verify images fail\n", __func__, __LINE__);
            free_image_info_list(&verified_images_list);
            return -1;
    }
    free_image_info_list(&verified_images_list);
    printf("res image verified ok\n");
    if (storage)
        storage->release(storage);
#endif

    return 0;
}


int path_joint(char* fpath,const char* path)
{
    char diskd[3] = {DISKD+0x30,':',0};
    if(strlen(path) >= FPATH_LEN - 2)
    {
        return -1;
    }
    fpath[sprintf(fpath,"%s/%s",diskd,path)] = 0;

    return 0;
}

static int res_fs_init(void)
{
    char diskd[3] = {DISKD+0x30,':',0};
    FRESULT r;
    lk_bigtime_t start;

    start = current_time_hires();
    if (verified_res_image())
    {
        printf("fail to verify res image!\n");
        return -1;
    }
    start = current_time_hires() - start;
    printf("verified time:%llu\n", start);

    r = f_mount(&fsd,diskd,1);
    if(r)
    {
        printf("FS init fail. ret code:%d\n",r);
    }
    else
    {
        RLDBG("FS init succ.\n");
    }

    return r;
}


int res_size(const char* path)
{
    FIL f;
    int fsz = 0;

    char* fpath = malloc(FPATH_LEN);

    if(!fpath)
    {
        printf("ERROR:fpath Out of memory\n");
        return -1;
    }

    if(path_joint(fpath,path))
    {
        printf("ERROR:fpath is too long\n");
        free(fpath);
        return -1;
    }

    FRESULT rc = f_open(&f,fpath,FA_READ);
    if(rc)
    {
        printf("ERROR:f_open fail %d\n",rc);
        free(fpath);
        return -1;
    }

    fsz = f_size(&f);

    RLDBG("res_size: %s -> 0x%x\n",fpath,fsz);

    free(fpath);

    f_close(&f);

    return fsz;
}

int res_load(const char* path, void* buf, uint32_t size, uint32_t offset)
{

    if(!buf)
    {
        printf("ERROR:invalid buf para\n");
        return -1;
    }

    if((addr_t)buf % 32 || size % 32)
    {
        printf("ERROR:read buf & size must be 32bytes aligned.\n");
        return -1;
    }

    char* fpath = malloc(FPATH_LEN);

    if(!fpath)
    {
        printf("ERROR:fpath Out of memory\n");
        return -1;
    }

    if(path_joint(fpath,path))
    {
        printf("ERROR:fpath is too long\n");
        free(fpath);
        return -1;
    }

    UINT br = -1;
    FIL f;
    FRESULT rc = f_open(&f,fpath,FA_READ);
    if(rc)
    {
        printf("ERROR:open %s fail %d\n",fpath,rc);
        free(fpath);
        return -1;
    }

    if(offset)
    {
        rc = f_lseek(&f,offset);

        if(rc)
        {
            printf("ERROR:seek %s : 0x%x fail %d\n",fpath,offset,rc);
            goto F_CLOSE;
        }
    }

    rc = f_read(&f,buf,size,&br);

    if(rc)
    {
        printf("ERROR:read %s : 0x%x fail %d\n",fpath,offset,rc);
        br = -1;
        goto F_CLOSE;
    }

    arch_clean_cache_range((addr_t)buf,size);

    RLDBG("res_load: %s -> %p Wanted 0x%x Bytes @ 0x%x offset br 0x%x\n",fpath,buf,size,offset,br);

F_CLOSE:
    free(fpath);
    f_close(&f);
    return (int)br;
}

LK_INIT_HOOK(res_loader, (lk_init_hook)res_fs_init, LK_INIT_LEVEL_TARGET+1);
