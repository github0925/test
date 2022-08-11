/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */
#ifdef __cplusplus
extern "C" {
#endif

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "system_cfg.h"
#include "storage_device.h"
#include "storage_dev_ospi.h"


#define CALLOC_SIZE (256*1024)
#define MONITOR_CHANNEL "soc:ipcc@0.update-monitor.-1.81"
//#define MONITOR_CHANNEL "virtio0.update-monitor.-1.81"

static storage_dec storage_list[] = {
    {.id = DEV_EMMC0,      .type = MMC,    .gpt_offset = 0,      .dev_name = MMC_PATH(mmcblk0),   .rpmsg_name = ""},
    {.id = DEV_EMMC1,      .type = MMC,    .gpt_offset = 0,      .dev_name = MMC_PATH(mmcblk1),   .rpmsg_name = ""},
    {.id = DEV_EMMC2,      .type = MMC,    .gpt_offset = 0,      .dev_name = MMC_PATH(mmcblk2),   .rpmsg_name = ""},
    {.id = DEV_EMMC3,      .type = MMC,    .gpt_offset = 0,      .dev_name = MMC_PATH(mmcblk3),   .rpmsg_name = ""},
    {.id = DEV_SPI_NOR0,   .type = OSPI,   .gpt_offset = 0x2000, .dev_name = MTD_PATH(mtdblock0), .rpmsg_name = MONITOR_CHANNEL},
    {.id = DEV_SPI_NOR1,   .type = OSPI,   .gpt_offset = 0x2000, .dev_name = MTD_PATH(mtdblock1), .rpmsg_name = MONITOR_CHANNEL},
};

int blk_init(storage_device_t *storage_dev, char const *dev_name,
             char const *rpmsg_name)
{
    int fd;
    struct stat buf;

    if ((!storage_dev) || (!dev_name)) {
        PRINTF_CRITICAL("blk_init para error");
        return -1;
    }

    if (stat(dev_name, &buf)) {
        PRINTF_CRITICAL("no such dev file: %s\n", dev_name);
        return -1;
    }

    storage_dev->dev_name = dev_name;
    fd = open(dev_name, O_RDWR);

    if (fd < 0) {
        PRINTF_CRITICAL("filed to open %s\n", dev_name);
        return -1;
    }

    storage_dev->dev_fd = fd;
    return 0;
}

int blk_release(storage_device_t *storage_dev)
{
    if (!storage_dev) {
        PRINTF_CRITICAL("blk_release para error");
        return -1;
    }

    if (storage_dev->dev_fd >= 0) {
        close(storage_dev->dev_fd);
    }

    free(storage_dev);
    return 0;
}


uint32_t blk_get_block_size(storage_device_t *storage_dev)
{
    uint32_t block_size;

    if (!storage_dev) {
        PRINTF_CRITICAL("blk_get_block_size para error");
        return 0;
    }

    if (ioctl(storage_dev->dev_fd, BLKSSZGET, &block_size) != 0) {
        PRINTF_CRITICAL("failed to get dev block size\n");
        return 0;
    }

    return block_size;
}

int blk_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
             uint64_t size)
{
    int r;

    if ((!storage_dev) || (!dst)) {
        PRINTF_CRITICAL("blk_read para error");
        return -1;
    }

    if (lseek64(storage_dev->dev_fd, src, SEEK_SET) < 0) {
        PRINTF_CRITICAL("block dev lseek %lu failed: %s\n", (unsigned long)src,
                        strerror(errno));
        return -1;
    }

    r = read(storage_dev->dev_fd, dst, size);

    if (r < 0) {
        PRINTF_CRITICAL("block dev read failed: %s", strerror(errno));
        return r;
    }

    return 0;
}

int blk_write(storage_device_t *storage_dev, uint64_t dst,
              const uint8_t *buf, uint64_t data_len)
{
    int r;

    if ((!storage_dev) || (!buf)) {
        PRINTF_CRITICAL("blk_write para error");
        return -1;
    }

    if (lseek64(storage_dev->dev_fd, dst, SEEK_SET) < 0) {
        PRINTF_CRITICAL("block dev lseek %lu failed: %s\n", (unsigned long)dst,
                        strerror(errno));
        return -1;
    }

    r = write(storage_dev->dev_fd, buf, data_len);

    if (r < 0) {
        PRINTF_CRITICAL("block dev write failed: %s", strerror(errno));
        return r;
    }

    return 0;
}

uint64_t blk_get_capacity(storage_device_t *storage_dev)
{
    uint64_t size;

    if (!storage_dev) {
        PRINTF_CRITICAL("blk_get_capacity para error");
        return 0;
    }

    if (ioctl(storage_dev->dev_fd, BLKGETSIZE64, &size) < 0) {
        PRINTF_CRITICAL("failed to get dev size:%s\n", strerror(errno));
        return 0;
    }

    return size;
}

uint32_t blk_get_erase_group_size(storage_device_t *storage_dev)
{
    uint32_t sector_size;

    if (!storage_dev) {
        PRINTF_CRITICAL("blk_get_erase_group_size para error");
        return 0;
    }

    if (ioctl(storage_dev->dev_fd, BLKIOMIN, &sector_size) < 0) {
        fprintf(stderr, "failed to get dev size:%s\n", strerror(errno));
        return 0;
    }

    return sector_size;
}

int blk_copy(storage_device_t *storage_dev, uint64_t src, uint64_t dst,
             uint64_t size)
{
    uint64_t cnt = (size / CALLOC_SIZE);
    uint64_t left = (size % CALLOC_SIZE);
    uint8_t *data;
    int ret = -1;

    if (!storage_dev) {
        PRINTF_CRITICAL("blk_copy para error");
        return -1;
    }

    data = (uint8_t *)calloc(1, (cnt != 0 ? (CALLOC_SIZE) : size));

    if (!data) {
        PRINTF_INFO("blk_copy calloc failed\n");
        return -1;
    }

    for (unsigned long long n = 0; n < cnt; n++) {
        ret = storage_dev->read(storage_dev, src, data, CALLOC_SIZE);

        if (ret < 0) {
            PRINTF_INFO("blk_copy read failed\n");
            free(data);
            return -1;
        }

        storage_dev->write(storage_dev, dst, data, CALLOC_SIZE);

        if (ret < 0) {
            PRINTF_INFO("blk_copy write failed\n");
            free(data);
            return -1;
        }

        src  += CALLOC_SIZE;
        dst  += CALLOC_SIZE;
    }

    if (0 != left) {
        ret = storage_dev->read(storage_dev, src, data, left);

        if (ret < 0) {
            PRINTF_INFO("OTA_programming storage->read failed\n");
            free(data);
            return -1;
        }

        ret = storage_dev->write(storage_dev, dst, data, left);

        if (ret < 0) {
            PRINTF_INFO("OTA_programming storage->write failed\n");
            free(data);
            return -1;
        }
    }

    return 0;
}

storage_device_t *setup_storage_dev(storage_id_e dev)
{
    storage_device_t *storage;
    int ret = -1;
    int count = sizeof(storage_list) / sizeof(storage_list[0]);

    if (count < dev) {
        PRINTF_CRITICAL("bad storage index %d\n", dev);
        return NULL;
    }

    storage = calloc(1, sizeof(storage_device_t));

    if (!storage) {
        PRINTF_CRITICAL("alloc memory for storage dev failed\n");
        return NULL;
    }

    storage->type = storage_list[dev].type;

    if (storage->type == MMC) {
        storage->init = &blk_init;
    }
    else if (storage->type == OSPI) {
        storage->init = &remote_ospi_init;
    }
    else {
        PRINTF_CRITICAL("error: unknown storage type\n");
        return NULL;
    }

    ret = storage->init(storage, storage_list[dev].dev_name,
                        storage_list[dev].rpmsg_name);

    if (ret < 0) {
        PRINTF_CRITICAL("storage dev init failed\n");
        return NULL;
    }

    if (OSPI_REMOTE_MODE != ret) {
        storage->read =                 &blk_read;
        storage->write =                &blk_write;
        storage->get_capacity =         &blk_get_capacity;
        storage->get_erase_group_size = &blk_get_erase_group_size;
        storage->get_block_size =       &blk_get_block_size;
        storage->release =              &blk_release;
        storage->copy =                 &blk_copy;
    }
    else {
        storage->read =                 &remote_ospi_read;
        storage->write =                &remote_ospi_write;
        storage->get_capacity =         &remote_ospi_get_capacity;
        storage->get_erase_group_size = &remote_ospi_get_erase_size;
        storage->get_block_size =       &remote_ospi_get_block_size;
        storage->release =              &remote_ospi_release;
        storage->copy =                 &remote_ospi_copy;
        storage_list[dev].gpt_offset    = 2 * storage->get_erase_group_size(storage);
    }

    storage->gpt_offset = storage_list[dev].gpt_offset;
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

#ifdef __cplusplus
}
#endif /* __cplusplus */