/*
 * gd25x.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: GigaDevice GD25 series spi_nor flash driver.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>

#include "spi_nor.h"

#define GD25X_ID_MANU_OFFSET (0)
#define GD25X_ID_CAPACITY_OFFSET (2)

#define GD25X_NON_VOL_REG_READ 0xB5
#define GD25X_VOL_REG_READ 0x85
#define GD25X_NON_VOL_REG_WRITE 0xB1
#define GD25X_VOL_REG_WRITE 0x81
#define GD25X_STATUS_REG_READ 0x05
#define GD25X_STATUS_REG_WRITE 0x01
#define GD25X_WRITE_ENABLE 0x06
#define GD25X_ID_REG_READ 0x9F
#define GD25X_QUAD_OUT_FAST_READ 0x6C
#define GD25X_QUAD_DDR_IO_READ 0xEE
#define GD25X_QUAD_EXT_FAST_PROGRAM 0x3E
#define GD25X_OCTAL_OUT_FAST_READ 0x7C
#define GD25X_OCTAL_EXT_FAST_PROGRAM 0x8E
#define GD25X_QPI_ENABLE 0x38
#define GD25X_QPI_DISBALE 0xFF
#define GD25X_64K_ERASE_4B 0xDC
#define GD25X_32K_ERASE_4B 0x5C
#define GD25X_4K_ERASE_4B 0x21
#define GD25X_RESET_EN 0x66
#define GD25X_RESET 0x99

#define GD25Q_VOL_WRITE_ENABLE 0x50
#define GD25Q_STATUS_REG2_READ 0x35
#define GD25Q_STATUS_REG3_READ 0x15
#define GD25Q_STATUS_REG2_WRITE 0x31
#define GD25Q_STATUS_REG3_WRITE 0x11
#define GD25Q_FAST_READ 0x0B
#define GD25Q_QUAD_OUT_FAST_READ 0x6B
#define GD25Q_PAGE_PROGRAM 0x02
#define GD25Q_QUAD_EXT_FAST_PROGRAM 0x32
#define GD25Q_64K_ERASE 0xD8
#define GD25Q_32K_ERASE 0x52
#define GD25Q_4K_ERASE 0x20

#define SPI_ERASE_64K_SIZE (0x10000)
#define SPI_ERASE_32K_SIZE (0x8000)
#define SPI_ERASE_4K_SIZE (0x1000)

#define GD25X_SUPPORT_QPI BIT(0)
#define GD25X_SUPPORT_QUAD_DTR BIT(1)
#define GD25X_SUPPORT_OPI BIT(2)
#define GD25X_SUPPORT_OCTAL_DTR BIT(3)
#define GD25X_SUPPORT_DQS BIT(4)
#define GD25Q_SUPPORT_QSPI BIT(5)

#define GD25X_TRAINING_LENGTH (32)

#define udelay(x) spin(x)

#undef BIT
#define BIT(nr) (1U << (nr))

static const char manu_id = 0xC8;
static char devid[8] = {0};
static uint8_t s_qpi_enable = 0;
static uint8_t s_dtr_enable = 0;
static uint8_t s_opi_enable = 0;
static uint8_t s_qspi_enable = 0;
static uint8_t s_comp = 0;
static uint8_t training_pattern[GD25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
static uint8_t training_data[GD25X_TRAINING_LENGTH] __ALIGNED(CACHE_LINE) = {0};

static int gd25x_get_status(struct spi_nor *nor, u8 *status)
{
    u8 reg[2] = {0};
    int ret;
    int size = 1;

    struct spi_nor_cmd read_cmd = {
        .opcode = GD25X_STATUS_REG_READ,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .dummy = nor->status_dummy,
    };

    if (s_qpi_enable || s_dtr_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.data_width = SPI_NOR_OCTAL_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (nor->dtr_en) {
        size = 2;
    }

    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, size);

    if (ret)
        return ret;

    *status = reg[0];
    return 0;
}

static int gd25x_waite_idle(struct spi_nor *nor)
{
    int ret = 0;
    u8 flash_status;
    lk_time_t timeout = current_time() + 1000;

    while (1) {
        ret = gd25x_get_status(nor, &flash_status);

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

static int gd25x_write_enable(struct spi_nor *nor)
{
    int ret;
    struct spi_nor_cmd write_enable_cmd = {
        .opcode = GD25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (s_qpi_enable || s_dtr_enable) {
        write_enable_cmd.inst_width = SPI_NOR_QUAD_LANS;
        write_enable_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        write_enable_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        write_enable_cmd.ddr_en = s_dtr_enable;
    }

    ret = nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);

    return ret;
}

static int gd25x_flash_reg_set(struct spi_nor *nor, u8 type, u8 addr, u8 val,
                               u8 addr_bytes)
{
    int ret;

    struct spi_nor_cmd write_cmd = {
        .dummy = 0,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    if (s_qpi_enable || s_dtr_enable) {
        write_cmd.inst_width = SPI_NOR_QUAD_LANS;
        write_cmd.addr_width = SPI_NOR_QUAD_LANS;
        write_cmd.data_width = SPI_NOR_QUAD_LANS;
        write_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        write_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        write_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        write_cmd.data_width = SPI_NOR_OCTAL_LANS;
        write_cmd.ddr_en = s_dtr_enable;
    }

    if (type) {
        write_cmd.opcode = GD25X_VOL_REG_WRITE;
    }
    else {
        write_cmd.opcode = GD25X_NON_VOL_REG_WRITE;
    }

    nor->host_ops.lock(nor);
    gd25x_write_enable(nor);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &val, 1);
    nor->host_ops.unlock(nor);

    return ret;
}

static int gd25x_flash_reg_read(struct spi_nor *nor, u8 type, u8 addr,
                                u8 addr_bytes)
{
    int ret;
    u8 reg[2] = { 0 };
    int size = 1;
    struct spi_nor_cmd read_cmd = {
        .dummy = 8,
        .addr_bytes = addr_bytes,
        .addr = addr,
    };

    if (s_qpi_enable || s_dtr_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.addr_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        read_cmd.data_width = SPI_NOR_OCTAL_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (s_dtr_enable) {
        read_cmd.dummy = 8;
    }

    if (type) {
        read_cmd.opcode = GD25X_VOL_REG_READ;
    }
    else {
        read_cmd.opcode = GD25X_NON_VOL_REG_READ;
    }

    if (nor->dtr_en) {
        size = 2;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, size);
    nor->host_ops.unlock(nor);

    if (ret)
        return ret;

    return reg[0];
}

static int gd25q_get_status(struct spi_nor *nor, u8 *status, u8 reg_n)
{
    u8 reg[2] = {0};
    int ret;
    int size = 1;

    struct spi_nor_cmd read_cmd = {
        .opcode = GD25X_STATUS_REG_READ,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (reg_n == 2)
        read_cmd.opcode = GD25Q_STATUS_REG2_READ;
    else if (reg_n == 3)
        read_cmd.opcode = GD25Q_STATUS_REG3_READ;

    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, size);

    if (ret)
        return ret;

    *status = reg[0];
    return 0;
}

static int gd25q_flash_status_set(struct spi_nor *nor, u8 type, u8 reg, u8 val)
{
    int ret;
    struct spi_nor_cmd write_cmd = {0};

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = GD25X_WRITE_ENABLE,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (type) {
        write_enable_cmd.opcode = GD25Q_VOL_WRITE_ENABLE;
    }

    if (reg == 2)
        write_cmd.opcode = GD25Q_STATUS_REG2_WRITE;
    else if (reg == 3)
        write_cmd.opcode = GD25Q_STATUS_REG3_WRITE;

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_write(nor, &write_enable_cmd, NULL, 0);
    ret = nor->host_ops.reg_write(nor, &write_cmd, &val, 1);
    nor->host_ops.unlock(nor);

    return ret;
}

static void gd25x_set_dummy(struct spi_nor *nor, u32 dummy)
{
    u8 addr_bytes = SPI_NOR_ADDR_3_BYTES;

    if (nor->dtr_en) {
        addr_bytes = SPI_NOR_ADDR_4_BYTES;
    }

    gd25x_flash_reg_set(nor, 1, 1, dummy, addr_bytes);
    nor->global_read_dummy = dummy;

    return;
}

static int gd25x_flash_qpi_en(struct spi_nor *nor, u8 enable)
{
    int ret;
    struct spi_nor_cmd write_cmd = {0};

    if (enable) {
        write_cmd.opcode = GD25X_QPI_ENABLE;
        write_cmd.inst_width = SPI_NOR_SINGLE_LANS;
    }
    else {
        write_cmd.opcode = GD25X_QPI_DISBALE;
        write_cmd.inst_width = SPI_NOR_QUAD_LANS;
    }

    ret = nor->host_ops.reg_write(nor, &write_cmd, NULL, 0);

    if (ret) {
        dprintf(CRITICAL, "qpi mode switch failed!\n");
        return ret;
    }

    s_qpi_enable = enable;

    if (nor->config_data.clk > 104000000 || nor->phy_en)
        nor->status_dummy = 8;
    else
        nor->status_dummy = 0;

    return 0;
}

static int gd25q_flash_qspi_en(struct spi_nor *nor, u8 enable)
{
    uint8_t vol = 0;

    gd25q_get_status(nor, &vol, 2);

    vol &= ~(1 << 1);

    if (enable) {
        vol |= (1 << 1);
    }

    gd25q_flash_status_set(nor, 1, 2, vol);
    s_qspi_enable = enable;

    if (nor->config_data.clk > 104000000 || nor->phy_en)
        nor->status_dummy = 8;
    else
        nor->status_dummy = 0;

    return 0;
}

static int flash_id_read(struct spi_nor *nor, char *buf)
{
    int ret = 0;
    struct spi_nor_cmd read_cmd = {
        .opcode = GD25X_ID_REG_READ,
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (s_qpi_enable || s_dtr_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.data_width = SPI_NOR_QUAD_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.data_width = SPI_NOR_OCTAL_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if ((s_qpi_enable || s_opi_enable) && (nor->config_data.clk > 104000000
                                           || nor->phy_en)) {
        read_cmd.dummy = 8;
    }

    if (s_dtr_enable) {
        read_cmd.dummy = 8;
    }

    nor->host_ops.lock(nor);
    ret = nor->host_ops.reg_read(nor, &read_cmd, (u8 *)buf, 4);
    nor->host_ops.unlock(nor);
    return ret;
}

static int parse_flash_id(struct spi_nor *nor, char *id)
{
    if ((id[3] != 0xFF) && (id[1] != 0x40))
        return -1;

    switch (id[1]) {
        /* GD25Q */
        case 0x40:
            s_comp = GD25Q_SUPPORT_QSPI;
            break;

        /* GD25LB */
        case 0x67:
            s_comp = GD25X_SUPPORT_QPI;
            break;

        /* GD25LT */
        case 0x66:
            s_comp = GD25X_SUPPORT_QPI | GD25X_SUPPORT_QUAD_DTR | GD25X_SUPPORT_DQS;
            break;

        /* GD25LX */
        case 0x68:
            s_comp = GD25X_SUPPORT_OPI | GD25X_SUPPORT_OCTAL_DTR | GD25X_SUPPORT_DQS;
            break;

        default:
            return -1;
    }

    nor->size = 1 << id[GD25X_ID_CAPACITY_OFFSET];

    return 0;
}

static int training_data_check(struct spi_nor *nor)
{
    uint32_t addr = 0 + nor->block_size;

    if (spi_nor_read(nor, addr, training_data, GD25X_TRAINING_LENGTH)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        return -1;
    };

    if (memcmp(training_data, training_pattern, GD25X_TRAINING_LENGTH))
        return -1;

    return 0;
}

int spi_nor_reset(struct spi_nor *nor)
{
    struct spi_nor_cmd reset_en_cmd = {
        .opcode = GD25X_RESET_EN,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };
    struct spi_nor_cmd reset_cmd = {
        .opcode = GD25X_RESET,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    if (s_qpi_enable || s_dtr_enable) {
        reset_en_cmd.inst_width = SPI_NOR_QUAD_LANS;
        reset_en_cmd.ddr_en = s_dtr_enable;
        reset_cmd.inst_width = SPI_NOR_QUAD_LANS;
        reset_cmd.ddr_en = s_dtr_enable;
    }

    if (s_opi_enable) {
        reset_en_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        reset_en_cmd.ddr_en = s_dtr_enable;
        reset_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        reset_cmd.ddr_en = s_dtr_enable;
    }

    nor->host_ops.lock(nor);

    if (s_opi_enable) {
        if (s_dtr_enable)
            nor->dtr_en = true;

        nor->status_dummy = 8;
        nor->global_read_dummy = 16;
        nor->dqs_en = 1;
    }

    nor->host_ops.reg_write(nor, &reset_en_cmd, NULL, 0);
    nor->host_ops.reg_write(nor, &reset_cmd, NULL, 0);
    nor->host_ops.unlock(nor);

    s_qpi_enable = 0;
    s_opi_enable = 0;
    s_dtr_enable = 0;
    s_comp = 0;
    nor->dtr_en = false;
    nor->status_dummy = 0;
    nor->global_read_dummy = 0;
    nor->dqs_en = 0;

    return 0;
}

int spi_nor_init(struct spi_nor *nor)
{
    nor->page_size = 256;
    nor->block_size = SPI_ERASE_4K_SIZE;
    nor->cssot_ns = 10;
    nor->cseot_ns = 10;
    nor->csdads_ns = 30;
    nor->csda_ns = 30;

    int ret = 0;
    nor->id = devid;

    uint32_t training_addr = 0 + nor->block_size;
    uint32_t i;
    uint8_t vol = 0;

    if (spi_nor_host_init(nor)) {
        dprintf(CRITICAL, "spi_nor init controller failed\n");
        ret = -1;
        goto init_out;
    }

    spi_nor_reset(nor);
    /* read flash device id, compare with gd25x id */
    flash_id_read(nor, devid);

    if (manu_id != devid[GD25X_ID_MANU_OFFSET]) {
        dprintf(CRITICAL, "spi_nor read flash id error, id:\n");
        hexdump8(devid, 8);
        ret = -1;
        goto init_out;
    }

    if (parse_flash_id(nor, devid)) {
        ret = -1;
        goto init_out;
    }

    if (training_data_check(nor)) {
        /* test spi_nor read write */
        if (spi_nor_erase(nor, training_addr, nor->block_size)) {
            dprintf(CRITICAL, "%s: erase error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }

        if (spi_nor_write(nor, training_addr, training_pattern,
                          GD25X_TRAINING_LENGTH)) {
            dprintf(CRITICAL, "%s: write error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }
    }

    /* wait for flash idle */
    ret = gd25x_waite_idle(nor);

    if (ret) {
        goto init_out;
    }

    if (!(s_comp & GD25Q_SUPPORT_QSPI))
        gd25x_set_dummy(nor, 16);

    if (nor->config_data.octal_ddr_en
            && (s_comp & (GD25X_SUPPORT_QUAD_DTR | GD25X_SUPPORT_OCTAL_DTR))) {
        gd25x_flash_reg_set(nor, 1, 0, 0xE7, SPI_NOR_ADDR_3_BYTES);
        s_dtr_enable = true;
        nor->dqs_en = true;

        if (s_comp & GD25X_SUPPORT_OCTAL_DTR) {
            s_opi_enable = true;
            nor->dtr_en = true;
        }

        if (s_comp & GD25X_SUPPORT_QUAD_DTR)
            s_qpi_enable = true;

        if (nor->config_data.clk > 104000000 || nor->phy_en || s_dtr_enable)
            nor->status_dummy = 8;
        else
            nor->status_dummy = 0;

        if (0xE7 == gd25x_flash_reg_read(nor, 1, 0, SPI_NOR_ADDR_4_BYTES)) {
            dprintf(CRITICAL, "%s: enable norflash quad/octal dtr mode, "
                    "not support erase and write in this mode!\n", __FUNCTION__);
            s_dtr_enable = true;
            nor->dtr_en = true;
            nor->dqs_en = true;

            if (s_comp & GD25X_SUPPORT_OCTAL_DTR)
                s_opi_enable = true;

            if (s_comp & GD25X_SUPPORT_QUAD_DTR)
                s_qpi_enable = true;
        }
    }
    else if (s_comp & GD25X_SUPPORT_DQS) {
        if (s_comp & (GD25X_SUPPORT_QPI | GD25X_SUPPORT_OPI))
            vol = 0xB7;
        else
            vol = 0xFF;

        gd25x_flash_reg_set(nor, 1, 0, vol, SPI_NOR_ADDR_3_BYTES);

        if (s_comp & GD25X_SUPPORT_OPI)
            s_opi_enable = true;

        if (s_comp & GD25X_SUPPORT_QPI)
            s_qpi_enable = true;

        if (nor->config_data.clk > 104000000 || nor->phy_en || s_opi_enable)
            nor->status_dummy = 8;
        else
            nor->status_dummy = 0;

        nor->dqs_en = true;

        if (vol == gd25x_flash_reg_read(nor, 1, 0, SPI_NOR_ADDR_3_BYTES)) {
            dprintf(INFO, "%s: enable norflash quad/octal dqs mode\n", __FUNCTION__);
            nor->dqs_en = true;
        }
    }
    else if (s_comp & GD25X_SUPPORT_QPI) {
        gd25x_flash_qpi_en(nor, 1);
        dprintf(CRITICAL, "%s: gd25x enable qpi mode\n", __FUNCTION__);
    }
    else if (s_comp & GD25Q_SUPPORT_QSPI) {
        gd25q_flash_qspi_en(nor, 1);
        dprintf(CRITICAL, "%s: gd25q enable qpi mode\n", __FUNCTION__);
    }

    nor->ddr_training = s_dtr_enable;
    ret = nor->host_ops.training(nor, training_data_check);

    if (ret) {
        if (!(s_comp & GD25Q_SUPPORT_QSPI))
            gd25x_set_dummy(nor, 0);

        if (s_dtr_enable) {
            gd25x_flash_reg_set(nor, 1, 0, 0xFF, SPI_NOR_ADDR_3_BYTES);
            s_dtr_enable = false;
            nor->dtr_en = false;
            nor->status_dummy = 0;
        }
        else {
            nor->status_dummy = 8;
        }
    }

    /* wait for flash idle */
    ret = gd25x_waite_idle(nor);

    if (ret)
        goto init_out;

    if (spi_nor_read(nor, training_addr, training_data, GD25X_TRAINING_LENGTH)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        ret = -1;
        goto init_out;
    };

    uint8_t *p2 = training_pattern;

    uint8_t *p3 = training_data;

    for (i = 0; i < GD25X_TRAINING_LENGTH; i++, p2++, p3++) {
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
        .opcode = GD25X_QUAD_OUT_FAST_READ,
        .dummy = 8,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_SINGLE_LANS,
        .data_width = SPI_NOR_QUAD_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = src,
    };

    if (s_qpi_enable || s_dtr_enable) {
        read_cmd.inst_width = SPI_NOR_QUAD_LANS;
        read_cmd.addr_width = SPI_NOR_QUAD_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }

    if (s_comp & GD25Q_SUPPORT_QSPI) {
        read_cmd.opcode = GD25Q_FAST_READ;
        read_cmd.inst_width = SPI_NOR_SINGLE_LANS;
        read_cmd.addr_width = SPI_NOR_SINGLE_LANS;
        read_cmd.data_width = SPI_NOR_SINGLE_LANS;
        read_cmd.addr_bytes = SPI_NOR_ADDR_3_BYTES;

        if (s_qspi_enable) {
            read_cmd.opcode = GD25Q_QUAD_OUT_FAST_READ;
            read_cmd.data_width = SPI_NOR_QUAD_LANS;
        }
    }

    if (s_comp & GD25X_SUPPORT_OPI) {
        read_cmd.opcode = GD25X_OCTAL_OUT_FAST_READ;
        read_cmd.dummy = 16;
        read_cmd.data_width = SPI_NOR_OCTAL_LANS;
    }

    if (s_opi_enable) {
        read_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        read_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        read_cmd.ddr_en = s_dtr_enable;
    }


    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = gd25x_waite_idle(nor);

    if (ret) {
        goto read_out;
    }

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
        .opcode = GD25X_QUAD_EXT_FAST_PROGRAM,
        .dummy = 0,
        .inst_width = SPI_NOR_SINGLE_LANS,
        .addr_width = SPI_NOR_QUAD_LANS,
        .data_width = SPI_NOR_QUAD_LANS,
        .addr_bytes = SPI_NOR_ADDR_4_BYTES,
        .addr = dst,
    };

    if (s_qpi_enable || s_dtr_enable) {
        write_cmd.inst_width = SPI_NOR_QUAD_LANS;
        write_cmd.ddr_en = s_dtr_enable;
    }

    if (s_comp & GD25Q_SUPPORT_QSPI) {
        write_cmd.opcode = GD25Q_PAGE_PROGRAM;
        write_cmd.inst_width = SPI_NOR_SINGLE_LANS;
        write_cmd.addr_width = SPI_NOR_SINGLE_LANS;
        write_cmd.data_width = SPI_NOR_SINGLE_LANS;
        write_cmd.addr_bytes = SPI_NOR_ADDR_3_BYTES;

        if (s_qspi_enable) {
            write_cmd.opcode = GD25Q_QUAD_EXT_FAST_PROGRAM;
            write_cmd.data_width = SPI_NOR_QUAD_LANS;
        }
    }

    if (s_comp & GD25X_SUPPORT_OPI) {
        write_cmd.opcode = GD25X_OCTAL_EXT_FAST_PROGRAM,
        write_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        write_cmd.data_width = SPI_NOR_OCTAL_LANS;
    }

    if (s_opi_enable) {
        write_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        write_cmd.ddr_en = s_dtr_enable;
    }

    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = gd25x_waite_idle(nor);

    if (ret) {
        goto write_out;
    }

    gd25x_write_enable(nor);
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

    if (s_qpi_enable || s_dtr_enable) {
        erase_cmd.inst_width = SPI_NOR_QUAD_LANS;
        erase_cmd.addr_width = SPI_NOR_QUAD_LANS;
        erase_cmd.data_width = SPI_NOR_QUAD_LANS;
        erase_cmd.ddr_en = s_dtr_enable;
    }

    if (s_comp & GD25Q_SUPPORT_QSPI) {
        erase_cmd.inst_width = SPI_NOR_SINGLE_LANS;
        erase_cmd.addr_width = SPI_NOR_SINGLE_LANS;
        erase_cmd.data_width = SPI_NOR_SINGLE_LANS;
        erase_cmd.addr_bytes = SPI_NOR_ADDR_3_BYTES;

        if (s_qspi_enable) {
            erase_cmd.data_width = SPI_NOR_QUAD_LANS;
        }
    }

    if (s_opi_enable) {
        erase_cmd.inst_width = SPI_NOR_OCTAL_LANS;
        erase_cmd.addr_width = SPI_NOR_OCTAL_LANS;
        erase_cmd.data_width = SPI_NOR_OCTAL_LANS;
        erase_cmd.ddr_en = s_dtr_enable;
    }

    nor->host_ops.lock(nor);

    while (remaining) {
        /* deal with unaligned address */
        if (IS_ALIGNED(erase_cmd.addr, SPI_ERASE_64K_SIZE)) {
            block_size = SPI_ERASE_64K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = GD25X_64K_ERASE_4B;

            if (s_comp & GD25Q_SUPPORT_QSPI)
                erase_cmd.opcode = GD25Q_64K_ERASE;
        }
        else {
            block_size = SPI_ERASE_4K_SIZE;
            count = (remaining % SPI_ERASE_64K_SIZE) / block_size;
            erase_cmd.opcode = GD25X_4K_ERASE_4B;

            if (s_comp & GD25Q_SUPPORT_QSPI)
                erase_cmd.opcode = GD25Q_4K_ERASE;
        }

        /* deal with unaligne size*/
        if (count == 0) {
            block_size = SPI_ERASE_4K_SIZE;
            count = remaining / block_size;
            erase_cmd.opcode = GD25X_4K_ERASE_4B;

            if (s_comp & GD25Q_SUPPORT_QSPI)
                erase_cmd.opcode = GD25Q_4K_ERASE;
        }

        while (count) {
            /* wait for flash idle */
            ret = gd25x_waite_idle(nor);

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
