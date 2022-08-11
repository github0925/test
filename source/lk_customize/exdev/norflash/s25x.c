/*
 * s25x.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cypress S25H series spi_nor flash driver.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>

#include "spi_nor.h"

#define S25X_ID_MANU_OFFSET (0)
#define S25X_ID_CAPACITY_OFFSET (2)

#define S25X_READ_REG 0x65
#define S25X_WRITE_REG 0x71
#define S25X_STATUS_REG_READ 0x05
#define S25X_WRITE_ENABLE 0x06
#define S25X_ID_REG_READ 0x9F
#define S25X_SDR_FAST_READ 0xC          // 8dummy+8mode
#define S25X_QUAD_OUT_FAST_READ 0x6C    // 8dummy
#define S25X_QUAD_IO_FAST_READ 0xEC     // 8dummy+2mode
#define S25X_QUAD_DDR_IO_READ 0xEE      // not support 166Mhz
#define S25X_QUAD_EXT_FAST_PROGRAM 0x3E
#define S25X_256K_ERASE 0xDC
#define S25X_4K_ERASE 0x21
#define S25X_CHIP_ERASE 0x60

#define SPI_ERASE_256K_SIZE (0x40000)
#define SPI_ERASE_4K_SIZE (0x1000)

#define udelay(x) spin(x)

#undef BIT
#define BIT(nr) (1U << (nr))

#define S25X_TRAINING_LENGTH (32)

static uint8_t training_pattern[S25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
static uint8_t training_data[S25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {0};

static const char manu_id = 0x34;

static char devid[8] = {0};

static uint8_t s_qpi_enable = 0;

static int s25x_get_status(struct spi_nor *nor, u8 *status)
{
    u8 reg[2] = {0};
    int ret;

    struct spi_nor_cmd read_cmd = {
        .opcode = S25X_STATUS_REG_READ,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .dummy = nor->status_dummy,
    };

    if (s_qpi_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
    }
    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, 1);
    nor->host_ops.unlock(nor);
    if (ret)
        return ret;

    *status = reg[0];
    return 0;
}

static int s25x_waite_idle(struct spi_nor *nor)
{
    int ret = 0;
    u8 flash_status;
    lk_time_t timeout = current_time() + 1000000;

    while (1) {
        ret = s25x_get_status(nor, &flash_status);
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

static int s25x_flash_reg_set(struct spi_nor *nor, u8 type, u32 addr, u8 val,
                               u8 addr_bytes)
{
    int ret;

    struct spi_nor_cmd write_cmd = {
        .opcode = S25X_WRITE_REG,
        .dummy = 0,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = S25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (s_qpi_enable) {
        write_cmd.inst_width = SPI_NOR_QUAD_LANS;
        write_cmd.addr_width = SPI_NOR_QUAD_LANS;
        write_cmd.data_width = SPI_NOR_QUAD_LANS;
        write_enable_cmd.inst_width = SPI_NOR_QUAD_LANS;
    }

    nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &val, 1);

    return ret;
}

static int s25x_flash_reg_read(struct spi_nor *nor, u8 type, u32 addr,
                                u8 addr_bytes)
{
    int ret;
    u8 reg[2] = { 0 };
    int size = 1;
    struct spi_nor_cmd read_cmd = {
        .opcode = S25X_READ_REG,
        .dummy = 8,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    if (s_qpi_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.addr_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
    }

    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, size);
    if (ret)
        return ret;

    return reg[0];
}

static void s25x_set_dummy(struct spi_nor *nor, u32 dummy)
{
    u8 addr_bytes = SPI_NOR_ADDR_3_BYTES;

    s25x_flash_reg_set(nor, 1, 1, dummy, addr_bytes);
    nor->global_read_dummy = dummy;

    return;
}


static int flash_id_read(struct spi_nor *nor, char *buf)
{
    int ret = 0;
    struct spi_nor_cmd read_cmd = {
        .opcode = S25X_ID_REG_READ,
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (s_qpi_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, (u8 *)buf, 4);
    nor->host_ops.unlock(nor);
    return ret;
}

static int parse_flash_id(struct spi_nor *nor, char *id)
{
    nor->size = 1 << id[S25X_ID_CAPACITY_OFFSET];
    return 0;
}

int spi_nor_erase_chip(struct spi_nor *nor)
{
    struct spi_nor_cmd write_cmd = {
        .opcode = S25X_CHIP_ERASE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = S25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    nor->host_ops.lock(nor);
    nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    nor->host_ops.reg_write(nor, &write_cmd, NULL, 0);
    nor->host_ops.unlock(nor);

    /* wait for flash idle */
    return s25x_waite_idle(nor);
}

static int training_data_check(struct spi_nor *nor)
{
    uint32_t addr = 0 + nor->block_size;

    if (spi_nor_read(nor, addr, training_data, S25X_TRAINING_LENGTH)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        return -1;
    };

    if(memcmp(training_data, training_pattern, S25X_TRAINING_LENGTH))
        return -1;

    return 0;
}

int spi_nor_init(struct spi_nor *nor)
{
    nor->page_size = 256;
    nor->block_size = SPI_ERASE_256K_SIZE;
    nor->cssot_ns = 30;
    nor->cseot_ns = 30;
    nor->csdads_ns = 30;
    nor->csda_ns = 30;
    uint8_t reg = 0;

    int ret = 0;
    nor->id = devid;

    uint32_t training_data_addr = 0 + nor->block_size;
    uint32_t training_length = S25X_TRAINING_LENGTH;

    if (spi_nor_host_init(nor)) {
        dprintf(CRITICAL, "spi_nor init controller failed\n");
        ret = -1;
        goto init_out;
    }

    /* read flash device id, compare with s25x id */
    flash_id_read(nor, devid);
    if (manu_id != devid[S25X_ID_MANU_OFFSET]) {
        dprintf(CRITICAL, "spi_nor read flash id error, id:\n");
        hexdump8(devid, 8);
        ret = -1;
        goto init_out;
    }

    if (parse_flash_id(nor, devid)) {
        ret = -1;
        goto init_out;
    }

    /* set sector size to 256KB */
    reg = s25x_flash_reg_read(nor, 0, 0x800004, SPI_NOR_ADDR_3_BYTES);
    if ((reg & BIT(3)) == 0) {
        reg = BIT(3);
        s25x_flash_reg_set(nor, 0, 0x00004, reg, SPI_NOR_ADDR_3_BYTES);
        spin(10*1000);
        reg = s25x_flash_reg_read(nor, 0, 0x800004, SPI_NOR_ADDR_3_BYTES);
        if ((reg & BIT(3)) == 0) {
            dprintf(CRITICAL, "%s: set erase sector size failed\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
    }

    /* enable quad spi protocol */
    reg = s25x_flash_reg_read(nor, 0, 0x800002, SPI_NOR_ADDR_3_BYTES);
    if ((reg & BIT(1)) == 0) {
        reg = BIT(1);
        s25x_flash_reg_set(nor, 0, 0x00002, reg, SPI_NOR_ADDR_3_BYTES);
        spin(10*1000);
        reg = s25x_flash_reg_read(nor, 0, 0x800002, SPI_NOR_ADDR_3_BYTES);
        if ((reg & BIT(1)) == 0) {
            dprintf(CRITICAL, "%s: enable quad spi protocol failed\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
    }

    if (training_data_check(nor)) {
        /* test spi_nor read write */
        if (spi_nor_erase(nor, training_data_addr, nor->block_size)) {
            dprintf(CRITICAL, "%s: erase error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }

        if (spi_nor_write(nor, training_data_addr, training_pattern, training_length)) {
            dprintf(CRITICAL, "%s: write error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
    }

    /* wait for flash idle */
    ret = s25x_waite_idle(nor);
    if (ret)
        goto init_out;

    ret = nor->host_ops.training(nor, training_data_check);
    if (ret)
        goto init_out;

    /* wait for flash idle */
    ret = s25x_waite_idle(nor);
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
            dprintf(CRITICAL, "\n spi_nor training data error: 0x%x - 0x%x : i=%d\n",
                    *p3, *p2, i);
            ret = -1;
            goto init_out;
        }
    }

init_out:
    return ret;
}

void spi_nor_deinit(struct spi_nor *nor)
{
    return;
}

int spi_nor_read(struct spi_nor *nor, uint64_t src, uint8_t *dst,
                 uint64_t length)
{
    int ret;
    struct spi_nor_cmd read_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_READ,
        .opcode = S25X_QUAD_IO_FAST_READ,
        .dummy = 10,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_QUAD_LANS,
        .data_width = SPI_NOR_QUAD_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = src,
    };

    if (s_qpi_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.addr_width = SPI_NOR_QUAD_LANS;
    }

    /* wait for flash idle */
    ret = s25x_waite_idle(nor);
    if (ret)
        goto read_out;

    nor->host_ops.lock(nor);
    ret = nor->host_ops.transfer(nor, &read_cmd, (u8 *)dst, length);
    nor->host_ops.unlock(nor);

read_out:

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
        .opcode = 0x12,
        .dummy = 0,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_SINGLE_LANS,
        .data_width = SPI_NOR_SINGLE_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = dst,
    };

    if (s_qpi_enable) {
        write_cmd.inst_width = SPI_NOR_QUAD_LANS;
        write_cmd.addr_width = SPI_NOR_QUAD_LANS;
        write_cmd.data_width = SPI_NOR_QUAD_LANS;
    }

    /* wait for flash idle */
    ret = s25x_waite_idle(nor);
    if (ret)
        goto write_out;

    nor->host_ops.lock(nor);
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
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = dst,
    };

    if (s_qpi_enable) {
        erase_cmd.inst_width = SPI_NOR_QUAD_LANS;
        erase_cmd.addr_width = SPI_NOR_QUAD_LANS;
        erase_cmd.data_width = SPI_NOR_QUAD_LANS;
    }

    while (remaining) {
        /* deal with unaligned address */
        if (IS_ALIGNED(erase_cmd.addr, SPI_ERASE_256K_SIZE)) {
            block_size = SPI_ERASE_256K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = S25X_256K_ERASE;
        } else {
            block_size = SPI_ERASE_4K_SIZE;
            count = (remaining % SPI_ERASE_256K_SIZE) / block_size;
            erase_cmd.opcode = S25X_4K_ERASE;
        }

        /* deal with unaligne size*/
        if (count == 0) {
            block_size = SPI_ERASE_4K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = S25X_4K_ERASE;
        }

        while (count) {
            /* wait for flash idle */
            ret = s25x_waite_idle(nor);
            if (ret)
                break;

            nor->host_ops.lock(nor);
            ret = nor->host_ops.transfer(nor, &erase_cmd, NULL, 0);
            nor->host_ops.unlock(nor);
            if (ret)
                break;

            erase_cmd.addr += block_size;
            remaining -= block_size;
            count--;
        }
    }

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

