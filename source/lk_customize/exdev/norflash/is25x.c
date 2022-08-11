/*
 * is25x.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ISSI is25x spi_nor flash driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>

#include "spi_nor.h"

#define IS25X_ID_MANU_OFFSET (0)
#define IS25X_ID_VOL_OFFSET (1)
#define IS25X_ID_CAPACITY_OFFSET (2)
#define IS25X_ID_CAPACITY_64M (0x17)

#define IS25X_ID_EXT_OFFSET (4)
#define IS25X_ID_SECTOR_SIZE_LSB (0)
#define IS25X_ID_SECTOR_SIZE_MASK (3)

#define IS25X_STATUS_REG_READ 0x05
#define IS25X_STATUS_REG_WRITE 0x01
#define IS25X_WRITE_ENABLE 0x06
#define IS25X_READ_PARA_SET 0x63
#define IS25X_QUAD_OUT_FAST_READ 0x6B
#define IS25X_QUAD_OUT_FAST_READ_4B 0x6C
#define IS25X_QUAD_IO_FAST_READ 0xEB
#define IS25X_QUAD_IO_FAST_READ_4B 0xEC
#define IS25X_ID_REG_READ 0x9F
#define IS25X_QUAD_IN_PAGE_PROGRAM 0x38
#define IS25X_QUAD_IN_PAGE_PROGRAM_4B 0x3E
#define IS25X_64K_ERASE 0xD8
#define IS25X_32K_ERASE 0x52
#define IS25X_SECTOR_ERASE 0xD7
#define IS25X_64K_ERASE_4B 0xDC
#define IS25X_32K_ERASE_4B 0x5C
#define IS25X_SECTOR_ERASE_4B 0x21
#define SPI_ERASE_64K_SIZE (0x10000)
#define SPI_ERASE_32K_SIZE (0x8000)
#define SPI_ERASE_SECTOR_SIZE (0x1000)

#define udelay(x) spin(x)

#undef BIT
#define BIT(nr) (1U << (nr))

static const char manu_id = 0x9d;
static char devid[8] = {0};
static bool use_4b_addr = false;

/* the size is 64Mb */
#define SPI_NOR_FLASH_64M (8 * 1024 * 1024)

#define IS25X_TRAINING_LENGTH (32)

static uint8_t training_pattern[IS25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
static uint8_t training_data[IS25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {0};

static int is25x_get_status(struct spi_nor *nor, u8 *status)
{
    u8 reg;
    int ret;

    struct spi_nor_cmd read_cmd = {
        .opcode = IS25X_STATUS_REG_READ,
    };

    ret = nor->host_ops.reg_read(nor, &read_cmd, &reg, 1);

    if (ret)
        return ret;

    *status = reg;
    return 0;
}

static int is25x_waite_idle(struct spi_nor *nor)
{
    int ret = 0;
    u8 flash_status;
    lk_time_t timeout = current_time() + 1000;

    while (1) {
        ret = is25x_get_status(nor, &flash_status);

        if (ret) {
            dprintf(CRITICAL, "spi_nor get flash status failed, ret: %d!\n",
                    ret);
            break;
        }

        dprintf(INFO, "flash_status = 0x%x \n", flash_status);

        if (!(flash_status & BIT(0)))
            break;

        if (current_time() > timeout) {
            ret = -ETIMEDOUT;
            dprintf(CRITICAL, "wait flash idle timeout, ret = %d!\n", ret);
            break;
        }

        udelay(1);
    }

    return ret;
}

static int is25x_status_reg_set(struct spi_nor *nor, u8 val)
{
    int ret;

    struct spi_nor_cmd write_cmd = {
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .opcode = IS25X_STATUS_REG_WRITE,
    };

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = IS25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,

    };

    nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &val, 1);

    return ret;
}

static int is25x_set_dummy(struct spi_nor *nor, u8 dummy)
{
    int ret;
    uint8_t read_para = dummy << 3u;

    struct spi_nor_cmd write_cmd = {
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .opcode = IS25X_READ_PARA_SET,
    };

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = IS25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,

    };

    nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &read_para, 1);
    if (ret == 0) {
        nor->global_read_dummy = dummy;
    }

    return ret;
}

static int flash_id_read(struct spi_nor *nor, char *buf)
{
    int ret = 0;
    struct spi_nor_cmd read_cmd = {
        .opcode = IS25X_ID_REG_READ,
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, (u8 *)buf, 4);
    nor->host_ops.unlock(nor);
    return ret;
}

static int parse_flash_id(struct spi_nor *nor, char *id)
{
    nor->size = 1 << (id[IS25X_ID_CAPACITY_OFFSET]);

    /* Use 4bytes cmd and addr bytes when size bigger than 128Mb */
    if (nor->size > (1 << 24))
        use_4b_addr = true;

    return 0;
}

static int training_data_check(struct spi_nor *nor)
{
    uint32_t addr = 0 + nor->block_size;

    if (spi_nor_read(nor, addr, training_data, IS25X_TRAINING_LENGTH)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        return -1;
    };

    if (memcmp(training_data, training_pattern, IS25X_TRAINING_LENGTH))
        return -1;

    return 0;
}

int spi_nor_init(struct spi_nor *nor)
{
    int ret = 0;
    nor->page_size = 256;
    nor->block_size = SPI_ERASE_SECTOR_SIZE;
    u8 status_reg;

    uint32_t training_data_addr = 0 + nor->block_size;
    uint32_t training_length = IS25X_TRAINING_LENGTH;

    if (spi_nor_host_init(nor)) {
        dprintf(CRITICAL, "spi_nor init controller failed\n");
        ret = -1;
        goto init_out;
    }

    /* read flash device id, compare with is25x id */
    flash_id_read(nor, devid);

    if (manu_id != devid[IS25X_ID_MANU_OFFSET]) {
        dprintf(CRITICAL, "spi_nor read flash id error, id:\n");
        hexdump8(devid, 8);
        ret = -1;
        goto init_out;
    }

    if (parse_flash_id(nor, devid)) {
        ret = -1;
        goto init_out;
    }

    nor->host_ops.lock(nor);
    ret = is25x_get_status(nor, &status_reg);
    nor->host_ops.unlock(nor);

    if (ret) {
        goto init_out;
    }

    /*quad output enable*/
    if (!(status_reg & BIT(6))) {
        if (is25x_status_reg_set(nor, status_reg | BIT(6))) {
            ret = -1;
            goto init_out;
        }
    }

    if (training_data_check(nor)) {
        if (spi_nor_erase(nor, training_data_addr, nor->block_size)) {
            dprintf(CRITICAL, "%s: erase error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }

        if (spi_nor_write(nor, training_data_addr, training_pattern,
                          training_length)) {
            dprintf(CRITICAL, "%s: write error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
    }

    nor->host_ops.training(nor, training_data_check);

    /* wait for flash idle */
    ret = is25x_waite_idle(nor);

    if (ret)
        goto init_out;

    if (spi_nor_read(nor, training_data_addr, training_data, training_length)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        ret = -1;
        goto init_out;
    };

    uint32_t i;

    uint8_t *p2 = training_pattern;

    uint8_t *p3 = training_data;

    for (i = 0; i < training_length; i++, p2++, p3++) {
        if (*p2 != *p3) {
            dprintf(CRITICAL,
                    "\n spi_nor training data error: 0x%x - 0x%x : i=%d\n", *p3,
                    *p2, i);
            ret = -1;
            goto init_out;
        }
    }

init_out:

    if (ret)
        dprintf(CRITICAL, "spi_nor init failed, ret: %d!\n", ret);

    return ret;
}

void spi_nor_deinit(struct spi_nor *nor) { return; }

int spi_nor_read(struct spi_nor *nor, uint64_t src, uint8_t *dst,
                 uint64_t length)
{
    int ret;
    struct spi_nor_cmd read_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_READ,
        .opcode =
            use_4b_addr ? IS25X_QUAD_OUT_FAST_READ_4B : IS25X_QUAD_OUT_FAST_READ,
        .dummy = 8,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_SINGLE_LANS,
        .data_width = SPI_NOR_QUAD_LANS,
        .addr_bytes = use_4b_addr ? SPI_NOR_ADDR_4_BYTES : SPI_NOR_ADDR_3_BYTES,
        .addr = src,
    };

    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = is25x_waite_idle(nor);

    if (ret)
        goto read_out;

    ret = nor->host_ops.transfer(nor, &read_cmd, (u8 *)dst, length);
read_out:
    nor->host_ops.unlock(nor);

    if (ret)
        dprintf(CRITICAL, "spi_nor read data failed, ret: %d!\n", ret);

    return ret;
}

int spi_nor_write(struct spi_nor *nor, uint64_t dst, const uint8_t *src_buf,
                  uint64_t length)
{
    int ret = 0;
    struct spi_nor_cmd write_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_WRITE,
        .opcode = use_4b_addr ? IS25X_QUAD_IN_PAGE_PROGRAM_4B
        : IS25X_QUAD_IN_PAGE_PROGRAM,
        .dummy = 0,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_SINGLE_LANS,
        .data_width = SPI_NOR_QUAD_LANS,
        .addr_bytes = use_4b_addr ? SPI_NOR_ADDR_4_BYTES : SPI_NOR_ADDR_3_BYTES,
        .addr = dst,
    };

    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = is25x_waite_idle(nor);

    if (ret)
        goto write_out;

    ret = nor->host_ops.transfer(nor, &write_cmd, (void *)src_buf, length);

write_out:
    nor->host_ops.unlock(nor);

    if (ret) {
        dprintf(CRITICAL, "spi_nor write array failed, ret: %d!\n", ret);
    }

    return ret;
}

/* the arguments erase length and dst address must 4K alined */
int spi_nor_erase(struct spi_nor *nor, uint64_t dst, uint64_t length)
{
    int ret = 0;
    uint32_t count;
    uint64_t remaining = length;
    uint32_t block_size;
    struct spi_nor_cmd erase_cmd = {
        .type = SPI_NOR_OPS_ERASE,
        .addr_bytes = use_4b_addr ? SPI_NOR_ADDR_4_BYTES : SPI_NOR_ADDR_3_BYTES,
        .addr = dst,
    };

    nor->host_ops.lock(nor);

    while (remaining) {
        /* deal with unaligned address */
        if (IS_ALIGNED(erase_cmd.addr, SPI_ERASE_64K_SIZE)) {
            block_size = SPI_ERASE_64K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode =
                use_4b_addr ? IS25X_64K_ERASE_4B : IS25X_64K_ERASE;
        }
        else {
            block_size = SPI_ERASE_SECTOR_SIZE;
            count = (remaining % SPI_ERASE_64K_SIZE) / block_size;
            erase_cmd.opcode =
                use_4b_addr ? IS25X_SECTOR_ERASE_4B : IS25X_SECTOR_ERASE;
        }

        /* deal with unaligne size */
        if (count == 0) {
            block_size = SPI_ERASE_SECTOR_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode =
                use_4b_addr ? IS25X_SECTOR_ERASE_4B : IS25X_SECTOR_ERASE;
        }

        while (count) {
            /* wait for flash idle */
            ret = is25x_waite_idle(nor);

            if (ret)
                break;

            ret = nor->host_ops.transfer(nor, &erase_cmd, NULL, 0);

            if (ret)
                break;

            erase_cmd.addr += block_size;
            remaining -= block_size;
            count--;
        }
    }

    nor->host_ops.unlock(nor);

    if (ret) {
        dprintf(CRITICAL, "spi_nor erase failed, ret: %d!\n", ret);
    }

    return ret;
}

inline int spi_nor_cancel(struct spi_nor *nor)
{
    return nor->host_ops.cancel(nor);
}

inline uint64_t spi_nor_get_capacity(struct spi_nor *nor)
{
    if (nor) {
        /* calculate flash size from pow2 multiply 512M */
        return nor->size;
    }
    else {
        return 0;
    }
}

inline uint64_t spi_nor_get_flash_id(struct spi_nor *nor)
{
    uint64_t flash_id = nor ? *(uint64_t *)&nor->id[0] : 0;
    return flash_id;
}

