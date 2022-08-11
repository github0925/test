/*
 * mt35x.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Micron mt35x spi_nor flash driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/17/2019 init version
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>

#include "spi_nor.h"

#define MT35X_ID_MANU_OFFSET (0)
#define MT35X_ID_VOL_OFFSET (1)
#define MT35X_ID_CAPACITY_OFFSET (2)
#define MT35X_ID_CAPACITY_64M (0x17)

#define MT35X_ID_EXT_OFFSET (4)
#define MT35X_ID_SECTOR_SIZE_LSB (0)
#define MT35X_ID_SECTOR_SIZE_MASK (3)

#define MT35X_STATUS_REG_READ 0x05
#define MT35X_WRITE_ENABLE 0x06
#define MT35X_ID_REG_READ 0x9E
#define MT35X_NON_VOL_REG_READ 0xB5
#define MT35X_VOL_REG_READ 0x85
#define MT35X_NON_VOL_REG_WRITE 0xB1
#define MT35X_VOL_REG_WRITE 0x81
#define MT35X_OCTAL_IO_FAST_READ 0xCC
#define MT35X_OCTAL_EXT_FAST_PROGRAM 0x8E
#define MT35X_4K_SUBSECTOR_ERASE 0x21
#define MT35X_32K_SUBSECTOR_ERASE 0x5C
#define MT35X_SECTOR_ERASE 0xDC

#define SPI_ERASE_4K_SIZE (0x1000)
#define SPI_ERASE_32K_SIZE (0x8000)
#define SPI_ERASE_SECTOR_SIZE (0x20000)

#define udelay(x) spin(x)

#undef BIT
#define BIT(nr) (1U << (nr))

/* the size is 64Mb */
#define SPI_NOR_FLASH_64M (8 * 1024 * 1024)

#define MT35X_TRAINING_LENGTH (32)

static uint8_t training_pattern[MT35X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
static uint8_t training_data[MT35X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {0};

static const char manu_id = 0x2C;

static char devid[16] = { 0x2C, 0x5B, 0x1A, 0x10, 0x41 , 0x00, 0xae, 0x12};

static int mt35x_get_status(struct spi_nor *nor, u8 *status)
{
    u8 reg[2] = {0};
    int ret;

    struct spi_nor_cmd read_cmd = {
        .opcode = MT35X_STATUS_REG_READ,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .dummy = 0,
    };

    if (nor->dtr_en) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.dummy = 8;
    }

    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, 2);
    if (ret)
        return ret;

    *status = reg[0];
    return 0;
}

static int mt35x_waite_idle(struct spi_nor *nor)
{
    int ret = 0;
    u8 flash_status;
    lk_time_t timeout = current_time() + 1000;

    while (1) {
        ret = mt35x_get_status(nor, &flash_status);
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

static int mt35x_flash_reg_set(struct spi_nor *nor, u8 type, u8 addr, u8 val,
                               u8 addr_bytes)
{
    int ret;

    struct spi_nor_cmd write_cmd = {
        .dummy = 0,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = MT35X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (type) {
        write_cmd.opcode = MT35X_VOL_REG_WRITE;
    }
    else {
        write_cmd.opcode = MT35X_NON_VOL_REG_WRITE;
    }

    if (nor->dtr_en) {
        write_enable_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        write_cmd.inst_width = SPI_NOR_OCTAL_LANS;
    }

    nor->host_ops.lock(nor);
    nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &val, 1);
    nor->host_ops.unlock(nor);

    return ret;
}

static int mt35x_flash_reg_read(struct spi_nor *nor, u8 type, u8 addr,
                                u8 addr_bytes)
{
    int ret;
    u8 reg[4] = { 0 };
    int size = 1;
    struct spi_nor_cmd read_cmd = {
        .dummy = 0,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    if (type) {
        read_cmd.opcode = MT35X_VOL_REG_READ;
        read_cmd.dummy = 8;
    }
    else {
        read_cmd.opcode = MT35X_NON_VOL_REG_READ;
        read_cmd.dummy = 8;
    }

    if (nor->dtr_en) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        size = 2;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, size);
    nor->host_ops.unlock(nor);
    if (ret)
        return ret;
    return reg[0];
}

static void mt35x_set_dummy(struct spi_nor *nor, u32 dummy)
{
    u8 addr_bytes = SPI_NOR_ADDR_3_BYTES;

    if (nor->dtr_en) {
        addr_bytes = SPI_NOR_ADDR_4_BYTES;
    }

    mt35x_flash_reg_set(nor, 1, 1, dummy, addr_bytes);
    nor->global_read_dummy = dummy;

    return;
}

static int flash_id_read(struct spi_nor *nor, char *buf)
{
    int ret = 0;
    struct spi_nor_cmd read_cmd = {
        .opcode = MT35X_ID_REG_READ,
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (nor->dtr_en) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS,
        read_cmd.dummy = 8;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, (u8 *)buf, 8);
    nor->host_ops.unlock(nor);
    return ret;
}

static int parse_flash_id(struct spi_nor *nor, char *id)
{
    int index = id[MT35X_ID_CAPACITY_OFFSET] - MT35X_ID_CAPACITY_64M;
    if (index < 0)
        return -1;
    else
        nor->size = SPI_NOR_FLASH_64M * (1 << (index));

    return 0;
}

static int training_data_check(struct spi_nor *nor)
{
    uint32_t addr = 0 + nor->block_size;

    if (spi_nor_read(nor, addr, training_data, MT35X_TRAINING_LENGTH)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        return -1;
    };

    if(memcmp(training_data, training_pattern, MT35X_TRAINING_LENGTH))
        return -1;

    return 0;
}

int spi_nor_init(struct spi_nor *nor)
{
    nor->page_size = 256;
    nor->block_size = SPI_ERASE_4K_SIZE;
    nor->cssot_ns = 30;
    nor->cseot_ns = 30;
    nor->csdads_ns = 30;
    nor->csda_ns = 30;

    int ret = 0;
    nor->id = devid;
    uint32_t training_data_addr = 0 + nor->block_size;
    uint32_t training_length = MT35X_TRAINING_LENGTH;

    if (spi_nor_host_init(nor)) {
        dprintf(CRITICAL, "spi_nor init controller failed\n");
        ret = -1;
        goto init_out;
    }

    /* read flash device id, if not equal mt35x id, may be octal ddr boot */
    flash_id_read(nor, devid);
    if (manu_id != devid[0]) {
        nor->dtr_en = true;
        nor->status_dummy = 8;
        mt35x_flash_reg_set(nor, 1, 1, 20, SPI_NOR_ADDR_4_BYTES);
        if (20 != mt35x_flash_reg_read(nor, 1, 1, SPI_NOR_ADDR_4_BYTES)) {
            dprintf(CRITICAL, "%s: set dummy failed\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
        nor->global_read_dummy = 20;

        if (!nor->config_data.octal_ddr_en) {
            mt35x_flash_reg_set(nor, 1, 0, 0xFF, SPI_NOR_ADDR_4_BYTES);
            nor->dtr_en = false;
            nor->status_dummy = 0;

            if(0xFF != mt35x_flash_reg_read(nor, 1, 0, SPI_NOR_ADDR_3_BYTES)) {
                dprintf(CRITICAL, "%s: switch to extended spi failed\n", __FUNCTION__);
                ret = -1;
                goto init_out;
            }
        }
    }

    if (training_data_check(nor)) {
        dprintf(CRITICAL, "%s: training pattern check failed, reflash!\n", __FUNCTION__);
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

        /* wait for flash idle */
        ret = mt35x_waite_idle(nor);
        if (ret)
            goto init_out;
    }

    if(!nor->global_read_dummy) {
        mt35x_flash_reg_set(nor, 1, 1, 20, SPI_NOR_ADDR_3_BYTES);
        nor->global_read_dummy = 20;
    }

    if (nor->config_data.octal_ddr_en && !nor->dtr_en) {
        mt35x_flash_reg_set(nor, 1, 0, 0xE7, SPI_NOR_ADDR_3_BYTES);
        nor->dtr_en = true;
        nor->dqs_en = true;
        nor->status_dummy = 8;
    }

    ret = nor->host_ops.training(nor, training_data_check);
    if (ret) {
        if (nor->config_data.octal_ddr_en) {
            mt35x_flash_reg_set(nor, 1, 1, 0, SPI_NOR_ADDR_4_BYTES);
            nor->global_read_dummy = 0;
            mt35x_flash_reg_set(nor, 1, 0, 0xFF, SPI_NOR_ADDR_4_BYTES);
            nor->dtr_en = false;
            nor->dqs_en = false;
            nor->status_dummy = 0;
        }
        else {
            mt35x_flash_reg_set(nor, 1, 1, 0, SPI_NOR_ADDR_3_BYTES);
            nor->global_read_dummy = 0;
        }
    }

    /* wait for flash idle */
    ret = mt35x_waite_idle(nor);
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

    /* read flash device id, compare with mt35x id */
    flash_id_read(nor, devid);

    if (manu_id != devid[0]) {
        dprintf(CRITICAL, "spi_nor read flash id error, id:\n");
        hexdump8(devid, 8);
        ret = -1;
        goto init_out;
    }

    if (parse_flash_id(nor, devid))
        ret = -1;

init_out:
    return ret;
}

void spi_nor_deinit(struct spi_nor *nor)
{
    nor->dtr_en = false;
    nor->status_dummy = 0;
    nor->global_read_dummy = 0;

    return;
}

int spi_nor_read(struct spi_nor *nor, uint64_t src, uint8_t *dst,
                 uint64_t length)
{
    int ret;
    struct spi_nor_cmd read_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_READ,
        .opcode = MT35X_OCTAL_IO_FAST_READ,
        .dummy = 16,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_OCTAL_LANS,
        .data_width = SPI_NOR_OCTAL_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = src,
    };

    /*
     * if dtr protcol enable, insruction address data lans: 8-8-8,
     * need 16 dummy clock cycles for read operation.
     */
    if (nor->dtr_en) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.dummy = 16;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.transfer(nor, &read_cmd, (u8 *)dst, length);
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
        .opcode = MT35X_OCTAL_EXT_FAST_PROGRAM,
        .dummy = 0,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_OCTAL_LANS,
        .data_width = SPI_NOR_OCTAL_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = dst,
    };

    /* if dtr protcol enable, insruction address data lans: 8-8-8 */
    if (nor->dtr_en) {
        write_cmd.inst_width = SPI_NOR_OCTAL_LANS;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.transfer(nor, &write_cmd, (void *)src_buf, length);
    /* wait for flash idle */
    ret += mt35x_waite_idle(nor);
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

    /* if dtr protcol enable, insruction address data lans: 8-8-8 */
    if (nor->dtr_en) {
        erase_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        erase_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        erase_cmd.data_width = SPI_NOR_OCTAL_LANS;
    }

    nor->host_ops.lock(nor);

    while (remaining) {
        /* deal with unaligned address */
        if (IS_ALIGNED(erase_cmd.addr, SPI_ERASE_SECTOR_SIZE)) {
            block_size = SPI_ERASE_SECTOR_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = MT35X_SECTOR_ERASE;
        } else {
            block_size = SPI_ERASE_4K_SIZE;
            count = (remaining % SPI_ERASE_SECTOR_SIZE) / block_size;
            erase_cmd.opcode = MT35X_4K_SUBSECTOR_ERASE;
        }

        /* deal with unaligne size*/
        if (count == 0) {
            block_size = SPI_ERASE_4K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = MT35X_4K_SUBSECTOR_ERASE;
        }

        while (count) {
            ret = nor->host_ops.transfer(nor, &erase_cmd, NULL, 0);
            if (ret)
                break;

            /* wait for flash idle */
            ret = mt35x_waite_idle(nor);
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

